// __!!rengine_copyright!!__ //

#include <rengine/file/Zip.h>
#include <rengine/lang/debug/Debug.h>

#include <fstream>
#include <algorithm>

//ZIP format
//
//Byte order: Little-endian
//Overall zipfile format:
//[Local file header + Compressed data [+ Extended local header]?]*
//[Central directory]*
//[End of central directory record]
//
//Local file header:
//Offset   Length   Contents
//  0      4 bytes  Local file header signature (0x04034b50)
//  4      2 bytes  Version needed to extract
//  6      2 bytes  General purpose bit flag
//  8      2 bytes  Compression method
// 10      2 bytes  Last mod file time
// 12      2 bytes  Last mod file date
// 14      4 bytes  CRC-32
// 18      4 bytes  Compressed size (n)
// 22      4 bytes  Uncompressed size
// 26      2 bytes  Filename length (f)
// 28      2 bytes  Extra field length (e)
//        (f)bytes  Filename
//        (e)bytes  Extra field
//        (n)bytes  Compressed data
//
//Extended local header:
//Offset   Length   Contents
//  0      4 bytes  Extended Local file header signature (0x08074b50)
//  4      4 bytes  CRC-32
//  8      4 bytes  Compressed size
// 12      4 bytes  Uncompressed size
//
//Central directory:
//Offset   Length   Contents
//  0      4 bytes  Central file header signature (0x02014b50)
//  4      2 bytes  Version made by
//  6      2 bytes  Version needed to extract
//  8      2 bytes  General purpose bit flag
// 10      2 bytes  Compression method
// 12      2 bytes  Last mod file time
// 14      2 bytes  Last mod file date
// 16      4 bytes  CRC-32
// 20      4 bytes  Compressed size
// 24      4 bytes  Uncompressed size
// 28      2 bytes  Filename length (f)
// 30      2 bytes  Extra field length (e)
// 32      2 bytes  File comment length (c)
// 34      2 bytes  Disk number start
// 36      2 bytes  Internal file attributes
// 38      4 bytes  External file attributes
// 42      4 bytes  Relative offset of local header
// 46     (f)bytes  Filename
//        (e)bytes  Extra field
//        (c)bytes  File comment
//
//End of central directory record:
//Offset   Length   Contents
//  0      4 bytes  End of central dir signature (0x06054b50)
//  4      2 bytes  Number of this disk
//  6      2 bytes  Number of the disk with the start of the central directory
//  8      2 bytes  Total number of entries in the central dir on this disk
// 10      2 bytes  Total number of entries in the central dir
// 12      4 bytes  Size of the central directory
// 16      4 bytes  Offset of start of central directory with respect to the starting disk number
// 20      2 bytes  zipfile comment length (c)
// 22     (c)bytes  zipfile comment


//
// Version needed
//
//	1.0 - Default value
//	1.1 - File is a volume label
//	2.0 - File is a folder (directory)
//	2.0 - File is compressed using Deflate compression
//	2.0 - File is encrypted using traditional PKWARE encryption
//	2.1 - File is compressed using Deflate64(tm)
//	2.5 - File is compressed using PKWARE DCL Implode 
//	2.7 - File is a patch data set 
//	4.5 - File uses ZIP64 format extensions
//	4.6 - File is compressed using BZIP2 compression*
//	5.0 - File is encrypted using DES
//	5.0 - File is encrypted using 3DES
//	5.0 - File is encrypted using original RC2 encryption
//	5.0 - File is encrypted using RC4 encryption
//	5.1 - File is encrypted using AES encryption
//	5.1 - File is encrypted using corrected RC2 encryption**
//	5.2 - File is encrypted using corrected RC2-64 encryption**
//	6.1 - File is encrypted using non-OAEP key wrapping***
//	6.2 - Central directory encryption
//	6.3 - File is compressed using LZMA
//	6.3 - File is compressed using PPMd+
//	6.3 - File is encrypted using Blowfish
//	6.3 - File is encrypted using Twofish

//
//	Compression method
//
//	0 - The file is stored (no compression)
//	1 - The file is Shrunk
//	2 - The file is Reduced with compression factor 1
//	3 - The file is Reduced with compression factor 2
//	4 - The file is Reduced with compression factor 3
//	5 - The file is Reduced with compression factor 4
//	6 - The file is Imploded
//	7 - Reserved for Tokenizing compression algorithm
//	8 - The file is Deflated
//	9 - Enhanced Deflating using Deflate64(tm)
//	10 - PKWARE Data Compression Library Imploding (old IBM TERSE)
//	11 - Reserved by PKWARE
//	12 - File is compressed using BZIP2 algorithm
//	13 - Reserved by PKWARE
//	14 - LZMA (EFS)
//	15 - Reserved by PKWARE
//	16 - Reserved by PKWARE
//	17 - Reserved by PKWARE
//	18 - File is compressed using IBM TERSE (new)
//	19 - IBM LZ77 z Architecture (PFS)
//	97 - WavPack compressed data
//	98 - PPMd version I, Rev 1



static int puff(unsigned char *dest, unsigned long *destlen, unsigned char *source, unsigned long *sourcelen);

extern "C"
{
	unsigned long computeCrc(unsigned long crc, unsigned char* data, unsigned int size);
}

namespace rengine
{
	Zip::Zip()
	{

	}

	Zip::Zip(std::string const filename)
	{
		load(filename);
	}

	Zip::~Zip()
	{
		close();
	}

	DirectoryContents Zip::getDirectoryContents(std::string const& directory_name) const
	{
		std::string native_filename = convertFileNameToUnixStyle(directory_name);

		std::string prefix = "";
		if ( !(native_filename.empty() || (native_filename == ".") || (native_filename == "./")) )
		{
			Char lastChar = native_filename[native_filename.size() - 1];
			Bool has_backslash = (lastChar == '/');

			if (has_backslash)
			{
				prefix = directory_name;
			}
			else
			{
				prefix = directory_name + "/";
			}
		}


		DirectoryContents directory_contents;

		if (prefix == "")
		{
			for(RecordMap::const_iterator i = m_records.begin(); i != m_records.end(); ++i)
			{
				directory_contents.push_back(i->first);
			}
			directory_contents.push_back(".");
		}
		else
		{
			for(RecordMap::const_iterator i = m_records.begin(); i != m_records.end(); ++i)
			{
				if (i->first == prefix)
				{
					directory_contents.push_back(".");
				}
				else if (startsWith(i->first, prefix))
				{
					std::string file = i->first.substr( prefix.size() );
					Int slash_count = std::count(file.begin(), file.end(), '/');
					Bool ends_with_slash = false;

					if (!file.empty())
					{
						Char lastChar = file[file.size() - 1];
						ends_with_slash = (lastChar == '/');
					}

					if ( !file.empty() && 
						 ((slash_count == 0) || ((slash_count == 1) && ends_with_slash)) )
					{
						if (ends_with_slash)
						{
							file = file.substr(0, file.size() - 1);
						}

						if (!file.empty())
						{
							directory_contents.push_back(file);
						}
					}
				}
			}

			if (!directory_contents.empty())
			{
				directory_contents.push_back("..");
			}
		}

		return directory_contents;
	}

	Bool Zip::fileExists(std::string const& filename) const
	{
		return (this->fileType(filename) != FileNotFound);
	}

	FileType Zip::fileType(std::string const& filename) const
	{
		std::string native_filename = convertFileNameToUnixStyle(filename);

		if (native_filename.empty() || (native_filename == ".") || (native_filename == "./"))
		{
			return FileDirectory;
		}
		
		Char lastChar = native_filename[native_filename.size() - 1];
		Bool has_backslash = (lastChar == '/');

		RecordMap::const_iterator found = m_records.end();

		if (has_backslash)
		{
			found = m_records.find(native_filename);
		}
		else
		{
			found = m_records.find(native_filename);

			if (found != m_records.end())
			{
				return FileRegular;
			}

			found = m_records.find(native_filename + "/");
		}

		if (found != m_records.end())
		{
			return FileDirectory;
		}

		return FileNotFound;
	}


	bool Zip::load(std::string const& filename)
	{
		close();

		m_filename = filename;

		file.open(m_filename.c_str(), std::ios::in | std::ios::binary);
		if (!file.is_open())
		{
			close();
			return false;
		}

		if (!loadEndOfCentralDirectory())
		{
			close();
			return false;
		}

		if (!loadCentralDirectory())
		{
			close();
			return false;
		}

		return true;
	}

	bool Zip::loadCentralDirectory()
	{
		if (m_end_of_central_directory.entries_central_dir == 0)
		{
			return true;
		}


		file.seekg(0, std::ios::end);
		if (!file)
		{
			return false;
		}
		Int pos = file.tellg();
		Int total_size = pos;
		Int needed_read_size = 4 + (6 * 2) + (3 * 4) + (5 * 2) + (2 * 4);

		file.seekg(m_end_of_central_directory.central_dir_offset, std::ios::beg);
		pos = file.tellg();

		if (!file || ((total_size - pos) < needed_read_size))
		{
			return false;
		}

		m_central_directory.resize(m_end_of_central_directory.entries_central_dir);

		int current = 0;
		do
		{
			CentralDirectory& directory = m_central_directory[current];
			memset(&directory, 0, sizeof(CentralDirectory));

			file.read((Char*)&directory.signature, 4);
			if (directory.signature != Uint32(CentralDirectorySignature))
			{
				break;
			}
			
			file.read((Char*)&directory.version_made, 2);
			file.read((Char*)&directory.version_needed, 2);
			file.read((Char*)&directory.flags, 2);
			file.read((Char*)&directory.compression_method, 2);
			file.read((Char*)&directory.last_mod_time, 2);
			file.read((Char*)&directory.last_mod_date, 2);
			file.read((Char*)&directory.crc, 4);
			file.read((Char*)&directory.compressed_size, 4);
			file.read((Char*)&directory.uncompressed_size, 4);
			file.read((Char*)&directory.filename_length, 2);
			file.read((Char*)&directory.extra_field_length, 2);
			file.read((Char*)&directory.comment_length, 2);
			file.read((Char*)&directory.disk_number_start, 2);
			file.read((Char*)&directory.internal_file_attributes, 2);
			file.read((Char*)&directory.external_file_attributes, 4);
			file.read((Char*)&directory.offset, 4);

			
			if (directory.filename_length > (max_filename_size - 1))
			{
				directory.filename_length = max_filename_size - 1;
			}

			if (directory.filename_length)
			{
				file.read((Char*)&directory.filename, directory.filename_length);
			}


			if (directory.extra_field_length > (max_extra_field_size - 1))
			{
				directory.extra_field_length = max_extra_field_size - 1;
			}

			if (directory.extra_field_length)
			{
				file.read((Char*)&directory.extra_field, directory.extra_field_length);
			}


			if (directory.comment_length > (max_comment_size - 1))
			{
				directory.comment_length = max_comment_size - 1;
			}

			if (directory.comment_length)
			{
				file.read((Char*)&directory.comment, directory.comment_length);
			}

			pos = file.tellg();

			if (file.good())
			{
				m_records[directory.filename] = current;
				++current;
			}
			
		}
		while (file.good() && ((total_size - pos) >= needed_read_size));
		m_central_directory.resize(current);


		if (!file)
		{
			return false;
		}
	
		return file.good();
	}

	bool Zip::loadEndOfCentralDirectory()
	{
		// Scan from end of file to EndOfCentralDirectory
		file.seekg(0, std::ios::end);

		if (!file)
		{
			return false;
		}

		Int size = file.tellg();
		if (size < 4)
		{
			return false;
		}

		file.seekg(-4, std::ios::cur);
		do
		{
			file.read((Char*)&m_end_of_central_directory.signature, 4);

			if (m_end_of_central_directory.signature == Uint32(EndOfCentralDirectorySignature))
			{
				break;
			}

			file.seekg(-(4 + 1), std::ios::cur);
			size = file.tellg();
		}
		while(size);


		if (m_end_of_central_directory.signature != Uint32(EndOfCentralDirectorySignature))
		{
			return false;
		}

		file.read((Char*)&m_end_of_central_directory.disk_number, 2);
		file.read((Char*)&m_end_of_central_directory.disk_number_central_dir, 2);
		file.read((Char*)&m_end_of_central_directory.entries, 2);
		file.read((Char*)&m_end_of_central_directory.entries_central_dir, 2);
		file.read((Char*)&m_end_of_central_directory.central_dir_size, 4);
		file.read((Char*)&m_end_of_central_directory.central_dir_offset, 4);
		file.read((Char*)&m_end_of_central_directory.comment_length, 2);

		if (m_end_of_central_directory.comment_length > (max_comment_size - 1))
		{
			m_end_of_central_directory.comment_length = max_comment_size - 1;
		}

		if (m_end_of_central_directory.comment_length)
		{
			file.read((Char*)&m_end_of_central_directory.comment, m_end_of_central_directory.comment_length);
		}

		return file.good();
	}

	Zip::SharedData Zip::loadFile(Uint index, LocalFileHeader& local_header)
	{
		RENGINE_ASSERT(index < m_central_directory.size());

		CentralDirectory& directory = m_central_directory[index];

		
		SharedData data;

		//
		// LocalFileHeader
		//
		file.seekg(0, std::ios::end);
		if (!file)
		{
			return data;
		}

		Int pos = file.tellg();
		Int total_size = pos;
		Int needed_read_size = 4 + (5 * 2) + (3 * 4) + (2 * 2);

		file.seekg(directory.offset, std::ios::beg);
		pos = file.tellg();

		if (!file || ((total_size - pos) < needed_read_size))
		{
			return data;
		}

		file.read((Char*)&local_header.signature, 4);

		if (local_header.signature != Uint32(LocalFileHeaderSignature))
		{
			return data;
		}

		file.read((Char*)&local_header.version_needed, 2);
		file.read((Char*)&local_header.flags, 2);
		file.read((Char*)&local_header.compression_method, 2);
		file.read((Char*)&local_header.last_mod_time, 2);
		file.read((Char*)&local_header.last_mod_date, 2);
		file.read((Char*)&local_header.crc, 4);
		file.read((Char*)&local_header.compressed_size, 4);
		file.read((Char*)&local_header.uncompressed_size, 4);
		file.read((Char*)&local_header.filename_length, 2);
		file.read((Char*)&local_header.extra_field_length, 2);
		
		if (local_header.filename_length > (max_filename_size - 1))
		{
			local_header.filename_length = max_filename_size - 1;
		}

		if (local_header.filename_length)
		{
			file.read((Char*)&local_header.filename, local_header.filename_length);
		}

		if (local_header.extra_field_length > (max_extra_field_size - 1))
		{
			local_header.extra_field_length = max_extra_field_size - 1;
		}

		if (local_header.extra_field_length)
		{
			file.read((Char*)&local_header.extra_field, local_header.extra_field_length);
		}

		Int32 size = local_header.compressed_size;
		if (!size && local_header.uncompressed_size)
		{
			size = local_header.uncompressed_size;
		}

		if (size)
		{
			data = new Uint8[size];
			if (!data) { return data; }

			file.read((Char*) data.get(), size);
		}

		if (!file)
		{
			data.reset();
		}

		return data;
	}

	void Zip::close()
	{
		if (file.is_open())
		{
			file.close();
		}

		m_filename = "";
		m_current_directory = ".";
		memset(&m_end_of_central_directory, 0, sizeof(EndOfCentralDirectory));
		m_central_directory.clear();
		m_records.clear();
	}

	Zip::FileData Zip::read(std::string const& filename)
	{
		FileData data;
		data.size = 0;

		if (fileType(filename) == FileRegular)
		{
			std::string native_filename = convertFileNameToUnixStyle(filename);
			RecordMap::const_iterator found = m_records.find(native_filename);

			if (found != m_records.end())
			{
				LocalFileHeader local_file_header;
				memset(&local_file_header, 0, sizeof(LocalFileHeader));

				data.data = loadFile(found->second, local_file_header);

				if (data.data && local_file_header.uncompressed_size)
				{
					if (local_file_header.compression_method == 8) // deflate
					{
						data.size = local_file_header.compressed_size;
					}
					else if (local_file_header.compression_method == 0) // no compression
					{
						data.size = local_file_header.uncompressed_size;
					}
					else
					{
						data.data.reset();
						data.size = 0;
					}
				}

				// uncompresse data
				if (data.data && data.size)
				{
					if (local_file_header.compression_method == 8) // deflate
					{
						SharedData uncompressed = new Uint8[local_file_header.uncompressed_size];
						unsigned long compressed_size = local_file_header.compressed_size;
						unsigned long uncompressed_size = local_file_header.uncompressed_size;

						int result = puff( (unsigned char*) uncompressed.get(), &uncompressed_size,  (unsigned char*) data.data.get(), &compressed_size);

						if (result == 0)
						{
							data.data = uncompressed;
							data.size = local_file_header.uncompressed_size;
						}
						else
						{
							data.data.reset();
							data.size = 0;
						}

					}
				}



				// compute crc
				if (data.data && data.size)
				{
					unsigned long crc = 0xffffffffL;
					crc = computeCrc(crc, (unsigned char*)data.data.get(), data.size);

					if (crc != local_file_header.crc)
					{
						data.data.reset();
						data.size = 0;
					}
				}

				
			}
		}

		return data;
	}

} // namespace rengine


//
// Compression code
//

// CRC 32 Table
//
// Generate a table for a byte-wise 32-bit CRC calculation on the polynomial:
// x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1.
//
// Polynomials over GF(2) are represented in binary, one bit per coefficient,
// with the lowest powers in the most significant bit.  Then adding polynomials
// is just exclusive-or, and multiplying a polynomial by x is a right shift by
// one.  If we call the above polynomial p, and represent a byte as the
// polynomial q, also with the lowest power in the most significant bit (so the
// byte 0xb1 is the polynomial x^7+x^3+x+1), then the CRC is (q*x^32) mod p,
// where a mod b means the remainder after dividing a by b.
//
// This calculation is done using the shift-register method of multiplying and
// taking the remainder.  The register is initialized to zero, and for each
// incoming bit, x^32 is added mod p to the register if the bit is a one (where
// x^32 mod p is p+x^32 = x^26+...+1), and the register is multiplied mod p by
// x (which is shifting right by one and adding x^32 mod p if the bit shifted
// out is a one).  We start with the highest power (least significant bit) of
// q and repeat for all eight bits of q.
//
//The table is simply the CRC of all possible eight bit values.  This is all
//the information needed to generate CRC's on data a byte at a time for all
//combinations of CRC register values and incoming bytes.
//
static unsigned long* crcTable()
{
	static unsigned long crc_table[256];
	static bool initialized = false;

	if (!initialized)
	{
		unsigned long c;      // crc shift register
		unsigned long e;      // polynomial exclusive-or pattern
		int k;                // byte being shifted into crc apparatus

		// terms of polynomial defining this crc (except x^32): 
		static int p[] = {0, 1, 2, 4, 5, 7, 8, 10, 11, 12, 16, 22, 23, 26};

		// Make exclusive-or pattern from polynomial (0xedb88320)
		e = 0;
		for (unsigned int i = 0; i < sizeof(p) / sizeof(int); i++)
		{
			e |= 1L << (31 - p[i]);
		}

		// Compute table of CRC's
		crc_table[0] = 0;
		for (int i = 1; i < 256; i++)
		{
			c = i;
			//The idea to initialize the register with the byte instead of
			//zero was stolen from Haruhiko Okumura's ar002

			for (k = 8; k; k--)
			{
				c = c & 1 ? (c >> 1) ^ e : c >> 1;
			}
			crc_table[i] = c;
		}

		initialized = true;
	}

	return crc_table;
}

// Compute CRC-32
// 
// Run a set of bytes through the crc shift register.  If s is a NULL
// pointer, then initialize the crc shift register contents instead.
// Return the current crc in either case.
//
unsigned long computeCrc(unsigned long crc, unsigned char* data, unsigned int size)
{
	register unsigned long c;         // temporary variable 

	if (data == 0) 
	{
		c = 0xffffffffL; //shift register contents
	} 
	else 
	{
		c = crc;

		if (size) 
			
		do 
		{
			c =  crcTable()[ ((int)c ^ (*data++)) & 0xff ] ^ (c >> 8);
		} 
		while (--size);
	}

	crc = c;

	return c ^ 0xffffffffL;       // (instead of ~c for 64-bit machines)
}


/* 
  Copyright (C) 2002-2010 Mark Adler, all rights reserved
  version 2.1, 4 Apr 2010

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the author be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Mark Adler    madler@alumni.caltech.edu
*/

/*
 * puff.c is a simple inflate written to be an unambiguous way to specify the
 * deflate format.  It is not written for speed but rather simplicity.  As a
 * side benefit, this code might actually be useful when small code is more
 * important than speed, such as bootstrap applications.  For typical deflate
 * data, zlib's inflate() is about four times as fast as puff().  zlib's
 * inflate compiles to around 20K on my machine, whereas puff.c compiles to
 * around 4K on my machine (a PowerPC using GNU cc).  If the faster decode()
 * function here is used, then puff() is only twice as slow as zlib's
 * inflate().
 *
 * All dynamically allocated memory comes from the stack.  The stack required
 * is less than 2K bytes.  This code is compatible with 16-bit int's and
 * assumes that long's are at least 32 bits.  puff.c uses the short data type,
 * assumed to be 16 bits, for arrays in order to to conserve memory.  The code
 * works whether integers are stored big endian or little endian.
 *
 * In the comments below are "Format notes" that describe the inflate process
 * and document some of the less obvious aspects of the format.  This source
 * code is meant to supplement RFC 1951, which formally describes the deflate
 * format:
 *
 *    http://www.zlib.org/rfc-deflate.html
 */

/*
 * Change history:
 *
 * 1.0  10 Feb 2002     - First version
 * 1.1  17 Feb 2002     - Clarifications of some comments and notes
 *                      - Update puff() dest and source pointers on negative
 *                        errors to facilitate debugging deflators
 *                      - Remove longest from struct huffman -- not needed
 *                      - Simplify offs[] index in construct()
 *                      - Add input size and checking, using longjmp() to
 *                        maintain easy readability
 *                      - Use short data type for large arrays
 *                      - Use pointers instead of long to specify source and
 *                        destination sizes to avoid arbitrary 4 GB limits
 * 1.2  17 Mar 2002     - Add faster version of decode(), doubles speed (!),
 *                        but leave simple version for readabilty
 *                      - Make sure invalid distances detected if pointers
 *                        are 16 bits
 *                      - Fix fixed codes table error
 *                      - Provide a scanning mode for determining size of
 *                        uncompressed data
 * 1.3  20 Mar 2002     - Go back to lengths for puff() parameters [Jean-loup]
 *                      - Add a puff.h file for the interface
 *                      - Add braces in puff() for else do [Jean-loup]
 *                      - Use indexes instead of pointers for readability
 * 1.4  31 Mar 2002     - Simplify construct() code set check
 *                      - Fix some comments
 *                      - Add FIXLCODES #define
 * 1.5   6 Apr 2002     - Minor comment fixes
 * 1.6   7 Aug 2002     - Minor format changes
 * 1.7   3 Mar 2003     - Added test code for distribution
 *                      - Added zlib-like license
 * 1.8   9 Jan 2004     - Added some comments on no distance codes case
 * 1.9  21 Feb 2008     - Fix bug on 16-bit integer architectures [Pohland]
 *                      - Catch missing end-of-block symbol error
 * 2.0  25 Jul 2008     - Add #define to permit distance too far back
 *                      - Add option in TEST code for puff to write the data
 *                      - Add option in TEST code to skip input bytes
 *                      - Allow TEST code to read from piped stdin
 * 2.1   4 Apr 2010     - Avoid variable initialization for happier compilers
 *                      - Avoid unsigned comparisons for even happier compilers
 *		   Fev 2011		- Fixed ba initialization code for static -> Rui Varela
 */

#include <setjmp.h>             /* for setjmp(), longjmp(), and jmp_buf */

#define local static            /* for local function definitions */
#define NIL ((unsigned char *)0)        /* for no output option */

/*
 * Maximums for allocations and loops.  It is not useful to change these --
 * they are fixed by the deflate format.
 */
#define MAXBITS 15              /* maximum bits in a code */
#define MAXLCODES 286           /* maximum number of literal/length codes */
#define MAXDCODES 30            /* maximum number of distance codes */
#define MAXCODES (MAXLCODES+MAXDCODES)  /* maximum codes lengths to read */
#define FIXLCODES 288           /* number of fixed literal/length codes */

/* input and output state */
struct state {
    /* output state */
    unsigned char *out;         /* output buffer */
    unsigned long outlen;       /* available space at out */
    unsigned long outcnt;       /* bytes written to out so far */

    /* input state */
    unsigned char *in;          /* input buffer */
    unsigned long inlen;        /* available input at in */
    unsigned long incnt;        /* bytes read so far */
    int bitbuf;                 /* bit buffer */
    int bitcnt;                 /* number of bits in bit buffer */

    /* input limit error return state for bits() and decode() */
    jmp_buf env;
};

/*
 * Return need bits from the input stream.  This always leaves less than
 * eight bits in the buffer.  bits() works properly for need == 0.
 *
 * Format notes:
 *
 * - Bits are stored in bytes from the least significant bit to the most
 *   significant bit.  Therefore bits are dropped from the bottom of the bit
 *   buffer, using shift right, and new bytes are appended to the top of the
 *   bit buffer, using shift left.
 */
local int bits(struct state *s, int need)
{
    long val;           /* bit accumulator (can use up to 20 bits) */

    /* load at least need bits into val */
    val = s->bitbuf;
    while (s->bitcnt < need) {
        if (s->incnt == s->inlen) longjmp(s->env, 1);   /* out of input */
        val |= (long)(s->in[s->incnt++]) << s->bitcnt;  /* load eight bits */
        s->bitcnt += 8;
    }

    /* drop need bits and update buffer, always zero to seven bits left */
    s->bitbuf = (int)(val >> need);
    s->bitcnt -= need;

    /* return need bits, zeroing the bits above that */
    return (int)(val & ((1L << need) - 1));
}

/*
 * Process a stored block.
 *
 * Format notes:
 *
 * - After the two-bit stored block type (00), the stored block length and
 *   stored bytes are byte-aligned for fast copying.  Therefore any leftover
 *   bits in the byte that has the last bit of the type, as many as seven, are
 *   discarded.  The value of the discarded bits are not defined and should not
 *   be checked against any expectation.
 *
 * - The second inverted copy of the stored block length does not have to be
 *   checked, but it's probably a good idea to do so anyway.
 *
 * - A stored block can have zero length.  This is sometimes used to byte-align
 *   subsets of the compressed data for random access or partial recovery.
 */
local int stored(struct state *s)
{
    unsigned len;       /* length of stored block */

    /* discard leftover bits from current byte (assumes s->bitcnt < 8) */
    s->bitbuf = 0;
    s->bitcnt = 0;

    /* get length and check against its one's complement */
    if (s->incnt + 4 > s->inlen) return 2;      /* not enough input */
    len = s->in[s->incnt++];
    len |= s->in[s->incnt++] << 8;
    if (s->in[s->incnt++] != (~len & 0xff) ||
        s->in[s->incnt++] != ((~len >> 8) & 0xff))
        return -2;                              /* didn't match complement! */

    /* copy len bytes from in to out */
    if (s->incnt + len > s->inlen) return 2;    /* not enough input */
    if (s->out != NIL) {
        if (s->outcnt + len > s->outlen)
            return 1;                           /* not enough output space */
        while (len--)
            s->out[s->outcnt++] = s->in[s->incnt++];
    }
    else {                                      /* just scanning */
        s->outcnt += len;
        s->incnt += len;
    }

    /* done with a valid stored block */
    return 0;
}

/*
 * Huffman code decoding tables.  count[1..MAXBITS] is the number of symbols of
 * each length, which for a canonical code are stepped through in order.
 * symbol[] are the symbol values in canonical order, where the number of
 * entries is the sum of the counts in count[].  The decoding process can be
 * seen in the function decode() below.
 */
struct huffman {
    short *count;       /* number of symbols of each length */
    short *symbol;      /* canonically ordered symbols */
};

/*
 * Decode a code from the stream s using huffman table h.  Return the symbol or
 * a negative value if there is an error.  If all of the lengths are zero, i.e.
 * an empty code, or if the code is incomplete and an invalid code is received,
 * then -10 is returned after reading MAXBITS bits.
 *
 * Format notes:
 *
 * - The codes as stored in the compressed data are bit-reversed relative to
 *   a simple integer ordering of codes of the same lengths.  Hence below the
 *   bits are pulled from the compressed data one at a time and used to
 *   build the code value reversed from what is in the stream in order to
 *   permit simple integer comparisons for decoding.  A table-based decoding
 *   scheme (as used in zlib) does not need to do this reversal.
 *
 * - The first code for the shortest length is all zeros.  Subsequent codes of
 *   the same length are simply integer increments of the previous code.  When
 *   moving up a length, a zero bit is appended to the code.  For a complete
 *   code, the last code of the longest length will be all ones.
 *
 * - Incomplete codes are handled by this decoder, since they are permitted
 *   in the deflate format.  See the format notes for fixed() and dynamic().
 */
#ifdef SLOW
local int decode(struct state *s, struct huffman *h)
{
    int len;            /* current number of bits in code */
    int code;           /* len bits being decoded */
    int first;          /* first code of length len */
    int count;          /* number of codes of length len */
    int index;          /* index of first code of length len in symbol table */

    code = first = index = 0;
    for (len = 1; len <= MAXBITS; len++) {
        code |= bits(s, 1);             /* get next bit */
        count = h->count[len];
        if (code - count < first)       /* if length len, return symbol */
            return h->symbol[index + (code - first)];
        index += count;                 /* else update for next length */
        first += count;
        first <<= 1;
        code <<= 1;
    }
    return -10;                         /* ran out of codes */
}

/*
 * A faster version of decode() for real applications of this code.   It's not
 * as readable, but it makes puff() twice as fast.  And it only makes the code
 * a few percent larger.
 */
#else /* !SLOW */
local int decode(struct state *s, struct huffman *h)
{
    int len;            /* current number of bits in code */
    int code;           /* len bits being decoded */
    int first;          /* first code of length len */
    int count;          /* number of codes of length len */
    int index;          /* index of first code of length len in symbol table */
    int bitbuf;         /* bits from stream */
    int left;           /* bits left in next or left to process */
    short *next;        /* next number of codes */

    bitbuf = s->bitbuf;
    left = s->bitcnt;
    code = first = index = 0;
    len = 1;
    next = h->count + 1;
    while (1) {
        while (left--) {
            code |= bitbuf & 1;
            bitbuf >>= 1;
            count = *next++;
            if (code - count < first) { /* if length len, return symbol */
                s->bitbuf = bitbuf;
                s->bitcnt = (s->bitcnt - len) & 7;
                return h->symbol[index + (code - first)];
            }
            index += count;             /* else update for next length */
            first += count;
            first <<= 1;
            code <<= 1;
            len++;
        }
        left = (MAXBITS+1) - len;
        if (left == 0) break;
        if (s->incnt == s->inlen) longjmp(s->env, 1);   /* out of input */
        bitbuf = s->in[s->incnt++];
        if (left > 8) left = 8;
    }
    return -10;                         /* ran out of codes */
}
#endif /* SLOW */

/*
 * Given the list of code lengths length[0..n-1] representing a canonical
 * Huffman code for n symbols, construct the tables required to decode those
 * codes.  Those tables are the number of codes of each length, and the symbols
 * sorted by length, retaining their original order within each length.  The
 * return value is zero for a complete code set, negative for an over-
 * subscribed code set, and positive for an incomplete code set.  The tables
 * can be used if the return value is zero or positive, but they cannot be used
 * if the return value is negative.  If the return value is zero, it is not
 * possible for decode() using that table to return an error--any stream of
 * enough bits will resolve to a symbol.  If the return value is positive, then
 * it is possible for decode() using that table to return an error for received
 * codes past the end of the incomplete lengths.
 *
 * Not used by decode(), but used for error checking, h->count[0] is the number
 * of the n symbols not in the code.  So n - h->count[0] is the number of
 * codes.  This is useful for checking for incomplete codes that have more than
 * one symbol, which is an error in a dynamic block.
 *
 * Assumption: for all i in 0..n-1, 0 <= length[i] <= MAXBITS
 * This is assured by the construction of the length arrays in dynamic() and
 * fixed() and is not verified by construct().
 *
 * Format notes:
 *
 * - Permitted and expected examples of incomplete codes are one of the fixed
 *   codes and any code with a single symbol which in deflate is coded as one
 *   bit instead of zero bits.  See the format notes for fixed() and dynamic().
 *
 * - Within a given code length, the symbols are kept in ascending order for
 *   the code bits definition.
 */
local int construct(struct huffman *h, short *length, int n)
{
    int symbol;         /* current symbol when stepping through length[] */
    int len;            /* current length when stepping through h->count[] */
    int left;           /* number of possible codes left of current length */
    short offs[MAXBITS+1];      /* offsets in symbol table for each length */

    /* count number of codes of each length */
    for (len = 0; len <= MAXBITS; len++)
        h->count[len] = 0;
    for (symbol = 0; symbol < n; symbol++)
        (h->count[length[symbol]])++;   /* assumes lengths are within bounds */
    if (h->count[0] == n)               /* no codes! */
        return 0;                       /* complete, but decode() will fail */

    /* check for an over-subscribed or incomplete set of lengths */
    left = 1;                           /* one possible code of zero length */
    for (len = 1; len <= MAXBITS; len++) {
        left <<= 1;                     /* one more bit, double codes left */
        left -= h->count[len];          /* deduct count from possible codes */
        if (left < 0) return left;      /* over-subscribed--return negative */
    }                                   /* left > 0 means incomplete */

    /* generate offsets into symbol table for each length for sorting */
    offs[1] = 0;
    for (len = 1; len < MAXBITS; len++)
        offs[len + 1] = offs[len] + h->count[len];

    /*
     * put symbols in table sorted by length, by symbol order within each
     * length
     */
    for (symbol = 0; symbol < n; symbol++)
        if (length[symbol] != 0)
            h->symbol[offs[length[symbol]]++] = symbol;

    /* return zero for complete set, positive for incomplete set */
    return left;
}

/*
 * Decode literal/length and distance codes until an end-of-block code.
 *
 * Format notes:
 *
 * - Compressed data that is after the block type if fixed or after the code
 *   description if dynamic is a combination of literals and length/distance
 *   pairs terminated by and end-of-block code.  Literals are simply Huffman
 *   coded bytes.  A length/distance pair is a coded length followed by a
 *   coded distance to represent a string that occurs earlier in the
 *   uncompressed data that occurs again at the current location.
 *
 * - Literals, lengths, and the end-of-block code are combined into a single
 *   code of up to 286 symbols.  They are 256 literals (0..255), 29 length
 *   symbols (257..285), and the end-of-block symbol (256).
 *
 * - There are 256 possible lengths (3..258), and so 29 symbols are not enough
 *   to represent all of those.  Lengths 3..10 and 258 are in fact represented
 *   by just a length symbol.  Lengths 11..257 are represented as a symbol and
 *   some number of extra bits that are added as an integer to the base length
 *   of the length symbol.  The number of extra bits is determined by the base
 *   length symbol.  These are in the static arrays below, lens[] for the base
 *   lengths and lext[] for the corresponding number of extra bits.
 *
 * - The reason that 258 gets its own symbol is that the longest length is used
 *   often in highly redundant files.  Note that 258 can also be coded as the
 *   base value 227 plus the maximum extra value of 31.  While a good deflate
 *   should never do this, it is not an error, and should be decoded properly.
 *
 * - If a length is decoded, including its extra bits if any, then it is
 *   followed a distance code.  There are up to 30 distance symbols.  Again
 *   there are many more possible distances (1..32768), so extra bits are added
 *   to a base value represented by the symbol.  The distances 1..4 get their
 *   own symbol, but the rest require extra bits.  The base distances and
 *   corresponding number of extra bits are below in the static arrays dist[]
 *   and dext[].
 *
 * - Literal bytes are simply written to the output.  A length/distance pair is
 *   an instruction to copy previously uncompressed bytes to the output.  The
 *   copy is from distance bytes back in the output stream, copying for length
 *   bytes.
 *
 * - Distances pointing before the beginning of the output data are not
 *   permitted.
 *
 * - Overlapped copies, where the length is greater than the distance, are
 *   allowed and common.  For example, a distance of one and a length of 258
 *   simply copies the last byte 258 times.  A distance of four and a length of
 *   twelve copies the last four bytes three times.  A simple forward copy
 *   ignoring whether the length is greater than the distance or not implements
 *   this correctly.  You should not use memcpy() since its behavior is not
 *   defined for overlapped arrays.  You should not use memmove() or bcopy()
 *   since though their behavior -is- defined for overlapping arrays, it is
 *   defined to do the wrong thing in this case.
 */
local int codes(struct state *s,
                struct huffman *lencode,
                struct huffman *distcode)
{
    int symbol;         /* decoded symbol */
    int len;            /* length for copy */
    unsigned dist;      /* distance for copy */
    static const short lens[29] = { /* Size base for length codes 257..285 */
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
    static const short lext[29] = { /* Extra bits for length codes 257..285 */
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
    static const short dists[30] = { /* Offset base for distance codes 0..29 */
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577};
    static const short dext[30] = { /* Extra bits for distance codes 0..29 */
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
        12, 12, 13, 13};

    /* decode literals and length/distance pairs */
    do {
        symbol = decode(s, lencode);
        if (symbol < 0) return symbol;  /* invalid symbol */
        if (symbol < 256) {             /* literal: symbol is the byte */
            /* write out the literal */
            if (s->out != NIL) {
                if (s->outcnt == s->outlen) return 1;
                s->out[s->outcnt] = symbol;
            }
            s->outcnt++;
        }
        else if (symbol > 256) {        /* length */
            /* get and compute length */
            symbol -= 257;
            if (symbol >= 29) return -10;       /* invalid fixed code */
            len = lens[symbol] + bits(s, lext[symbol]);

            /* get and check distance */
            symbol = decode(s, distcode);
            if (symbol < 0) return symbol;      /* invalid symbol */
            dist = dists[symbol] + bits(s, dext[symbol]);
#ifndef INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
            if (dist > s->outcnt)
                return -11;     /* distance too far back */
#endif

            /* copy length bytes from distance bytes back */
            if (s->out != NIL) {
                if (s->outcnt + len > s->outlen) return 1;
                while (len--) {
                    s->out[s->outcnt] =
#ifdef INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
                        dist > s->outcnt ? 0 :
#endif
                        s->out[s->outcnt - dist];
                    s->outcnt++;
                }
            }
            else
                s->outcnt += len;
        }
    } while (symbol != 256);            /* end of block symbol */

    /* done with a valid fixed or dynamic block */
    return 0;
}

/*
 * Process a fixed codes block.
 *
 * Format notes:
 *
 * - This block type can be useful for compressing small amounts of data for
 *   which the size of the code descriptions in a dynamic block exceeds the
 *   benefit of custom codes for that block.  For fixed codes, no bits are
 *   spent on code descriptions.  Instead the code lengths for literal/length
 *   codes and distance codes are fixed.  The specific lengths for each symbol
 *   can be seen in the "for" loops below.
 *
 * - The literal/length code is complete, but has two symbols that are invalid
 *   and should result in an error if received.  This cannot be implemented
 *   simply as an incomplete code since those two symbols are in the "middle"
 *   of the code.  They are eight bits long and the longest literal/length\
 *   code is nine bits.  Therefore the code must be constructed with those
 *   symbols, and the invalid symbols must be detected after decoding.
 *
 * - The fixed distance codes also have two invalid symbols that should result
 *   in an error if received.  Since all of the distance codes are the same
 *   length, this can be implemented as an incomplete code.  Then the invalid
 *   codes are detected while decoding.
 */
local int fixed(struct state *s)
{
    static int virgin = 1;
    static short lencnt[MAXBITS+1], lensym[FIXLCODES];
    static short distcnt[MAXBITS+1], distsym[MAXDCODES];
    static struct huffman lencode, distcode;

    /* build fixed huffman tables if first call (may not be thread safe) */
    if (virgin) 
	{
		/* construct lencode and distcode */
        lencode.count = lencnt;
        lencode.symbol = lensym;
        distcode.count = distcnt;
        distcode.symbol = distsym;

        int symbol;
        short lengths[FIXLCODES];

        /* literal/length table */
        for (symbol = 0; symbol < 144; symbol++)
            lengths[symbol] = 8;
        for (; symbol < 256; symbol++)
            lengths[symbol] = 9;
        for (; symbol < 280; symbol++)
            lengths[symbol] = 7;
        for (; symbol < FIXLCODES; symbol++)
            lengths[symbol] = 8;
        construct(&lencode, lengths, FIXLCODES);

        /* distance table */
        for (symbol = 0; symbol < MAXDCODES; symbol++)
            lengths[symbol] = 5;
        construct(&distcode, lengths, MAXDCODES);



        /* do this just once */
        virgin = 0;
    }

    /* decode data until end-of-block code */
    return codes(s, &lencode, &distcode);
}

/*
 * Process a dynamic codes block.
 *
 * Format notes:
 *
 * - A dynamic block starts with a description of the literal/length and
 *   distance codes for that block.  New dynamic blocks allow the compressor to
 *   rapidly adapt to changing data with new codes optimized for that data.
 *
 * - The codes used by the deflate format are "canonical", which means that
 *   the actual bits of the codes are generated in an unambiguous way simply
 *   from the number of bits in each code.  Therefore the code descriptions
 *   are simply a list of code lengths for each symbol.
 *
 * - The code lengths are stored in order for the symbols, so lengths are
 *   provided for each of the literal/length symbols, and for each of the
 *   distance symbols.
 *
 * - If a symbol is not used in the block, this is represented by a zero as
 *   as the code length.  This does not mean a zero-length code, but rather
 *   that no code should be created for this symbol.  There is no way in the
 *   deflate format to represent a zero-length code.
 *
 * - The maximum number of bits in a code is 15, so the possible lengths for
 *   any code are 1..15.
 *
 * - The fact that a length of zero is not permitted for a code has an
 *   interesting consequence.  Normally if only one symbol is used for a given
 *   code, then in fact that code could be represented with zero bits.  However
 *   in deflate, that code has to be at least one bit.  So for example, if
 *   only a single distance base symbol appears in a block, then it will be
 *   represented by a single code of length one, in particular one 0 bit.  This
 *   is an incomplete code, since if a 1 bit is received, it has no meaning,
 *   and should result in an error.  So incomplete distance codes of one symbol
 *   should be permitted, and the receipt of invalid codes should be handled.
 *
 * - It is also possible to have a single literal/length code, but that code
 *   must be the end-of-block code, since every dynamic block has one.  This
 *   is not the most efficient way to create an empty block (an empty fixed
 *   block is fewer bits), but it is allowed by the format.  So incomplete
 *   literal/length codes of one symbol should also be permitted.
 *
 * - If there are only literal codes and no lengths, then there are no distance
 *   codes.  This is represented by one distance code with zero bits.
 *
 * - The list of up to 286 length/literal lengths and up to 30 distance lengths
 *   are themselves compressed using Huffman codes and run-length encoding.  In
 *   the list of code lengths, a 0 symbol means no code, a 1..15 symbol means
 *   that length, and the symbols 16, 17, and 18 are run-length instructions.
 *   Each of 16, 17, and 18 are follwed by extra bits to define the length of
 *   the run.  16 copies the last length 3 to 6 times.  17 represents 3 to 10
 *   zero lengths, and 18 represents 11 to 138 zero lengths.  Unused symbols
 *   are common, hence the special coding for zero lengths.
 *
 * - The symbols for 0..18 are Huffman coded, and so that code must be
 *   described first.  This is simply a sequence of up to 19 three-bit values
 *   representing no code (0) or the code length for that symbol (1..7).
 *
 * - A dynamic block starts with three fixed-size counts from which is computed
 *   the number of literal/length code lengths, the number of distance code
 *   lengths, and the number of code length code lengths (ok, you come up with
 *   a better name!) in the code descriptions.  For the literal/length and
 *   distance codes, lengths after those provided are considered zero, i.e. no
 *   code.  The code length code lengths are received in a permuted order (see
 *   the order[] array below) to make a short code length code length list more
 *   likely.  As it turns out, very short and very long codes are less likely
 *   to be seen in a dynamic code description, hence what may appear initially
 *   to be a peculiar ordering.
 *
 * - Given the number of literal/length code lengths (nlen) and distance code
 *   lengths (ndist), then they are treated as one long list of nlen + ndist
 *   code lengths.  Therefore run-length coding can and often does cross the
 *   boundary between the two sets of lengths.
 *
 * - So to summarize, the code description at the start of a dynamic block is
 *   three counts for the number of code lengths for the literal/length codes,
 *   the distance codes, and the code length codes.  This is followed by the
 *   code length code lengths, three bits each.  This is used to construct the
 *   code length code which is used to read the remainder of the lengths.  Then
 *   the literal/length code lengths and distance lengths are read as a single
 *   set of lengths using the code length codes.  Codes are constructed from
 *   the resulting two sets of lengths, and then finally you can start
 *   decoding actual compressed data in the block.
 *
 * - For reference, a "typical" size for the code description in a dynamic
 *   block is around 80 bytes.
 */
local int dynamic(struct state *s)
{
    int nlen, ndist, ncode;             /* number of lengths in descriptor */
    int index;                          /* index of lengths[] */
    int err;                            /* construct() return value */
    short lengths[MAXCODES];            /* descriptor code lengths */
    short lencnt[MAXBITS+1], lensym[MAXLCODES];         /* lencode memory */
    short distcnt[MAXBITS+1], distsym[MAXDCODES];       /* distcode memory */
    struct huffman lencode, distcode;   /* length and distance codes */
    static const short order[19] =      /* permutation of code length codes */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    /* construct lencode and distcode */
    lencode.count = lencnt;
    lencode.symbol = lensym;
    distcode.count = distcnt;
    distcode.symbol = distsym;

    /* get number of lengths in each table, check lengths */
    nlen = bits(s, 5) + 257;
    ndist = bits(s, 5) + 1;
    ncode = bits(s, 4) + 4;
    if (nlen > MAXLCODES || ndist > MAXDCODES)
        return -3;                      /* bad counts */

    /* read code length code lengths (really), missing lengths are zero */
    for (index = 0; index < ncode; index++)
        lengths[order[index]] = bits(s, 3);
    for (; index < 19; index++)
        lengths[order[index]] = 0;

    /* build huffman table for code lengths codes (use lencode temporarily) */
    err = construct(&lencode, lengths, 19);
    if (err != 0) return -4;            /* require complete code set here */

    /* read length/literal and distance code length tables */
    index = 0;
    while (index < nlen + ndist) {
        int symbol;             /* decoded value */
        int len;                /* last length to repeat */

        symbol = decode(s, &lencode);
        if (symbol < 16)                /* length in 0..15 */
            lengths[index++] = symbol;
        else {                          /* repeat instruction */
            len = 0;                    /* assume repeating zeros */
            if (symbol == 16) {         /* repeat last length 3..6 times */
                if (index == 0) return -5;      /* no last length! */
                len = lengths[index - 1];       /* last length */
                symbol = 3 + bits(s, 2);
            }
            else if (symbol == 17)      /* repeat zero 3..10 times */
                symbol = 3 + bits(s, 3);
            else                        /* == 18, repeat zero 11..138 times */
                symbol = 11 + bits(s, 7);
            if (index + symbol > nlen + ndist)
                return -6;              /* too many lengths! */
            while (symbol--)            /* repeat last or zero symbol times */
                lengths[index++] = len;
        }
    }

    /* check for end-of-block code -- there better be one! */
    if (lengths[256] == 0)
        return -9;

    /* build huffman table for literal/length codes */
    err = construct(&lencode, lengths, nlen);
    if (err < 0 || (err > 0 && nlen - lencode.count[0] != 1))
        return -7;      /* only allow incomplete codes if just one code */

    /* build huffman table for distance codes */
    err = construct(&distcode, lengths + nlen, ndist);
    if (err < 0 || (err > 0 && ndist - distcode.count[0] != 1))
        return -8;      /* only allow incomplete codes if just one code */

    /* decode data until end-of-block code */
    return codes(s, &lencode, &distcode);
}

/*
 * Inflate source to dest.  On return, destlen and sourcelen are updated to the
 * size of the uncompressed data and the size of the deflate data respectively.
 * On success, the return value of puff() is zero.  If there is an error in the
 * source data, i.e. it is not in the deflate format, then a negative value is
 * returned.  If there is not enough input available or there is not enough
 * output space, then a positive error is returned.  In that case, destlen and
 * sourcelen are not updated to facilitate retrying from the beginning with the
 * provision of more input data or more output space.  In the case of invalid
 * inflate data (a negative error), the dest and source pointers are updated to
 * facilitate the debugging of deflators.
 *
 * puff() also has a mode to determine the size of the uncompressed output with
 * no output written.  For this dest must be (unsigned char *)0.  In this case,
 * the input value of *destlen is ignored, and on return *destlen is set to the
 * size of the uncompressed output.
 *
 * The return codes are:
 *
 *   2:  available inflate data did not terminate
 *   1:  output space exhausted before completing inflate
 *   0:  successful inflate
 *  -1:  invalid block type (type == 3)
 *  -2:  stored block length did not match one's complement
 *  -3:  dynamic block code description: too many length or distance codes
 *  -4:  dynamic block code description: code lengths codes incomplete
 *  -5:  dynamic block code description: repeat lengths with no first length
 *  -6:  dynamic block code description: repeat more than specified lengths
 *  -7:  dynamic block code description: invalid literal/length code lengths
 *  -8:  dynamic block code description: invalid distance code lengths
 *  -9:  dynamic block code description: missing end-of-block code
 * -10:  invalid literal/length or distance code in fixed or dynamic block
 * -11:  distance is too far back in fixed or dynamic block
 *
 * Format notes:
 *
 * - Three bits are read for each block to determine the kind of block and
 *   whether or not it is the last block.  Then the block is decoded and the
 *   process repeated if it was not the last block.
 *
 * - The leftover bits in the last byte of the deflate data after the last
 *   block (if it was a fixed or dynamic block) are undefined and have no
 *   expected values to check.
 */
int puff(unsigned char *dest,           /* pointer to destination pointer */
         unsigned long *destlen,        /* amount of output space */
         unsigned char *source,         /* pointer to source data pointer */
         unsigned long *sourcelen)      /* amount of input available */
{
    struct state s;             /* input/output state */
    int last, type;             /* block information */
    int err;                    /* return value */

    /* initialize output state */
    s.out = dest;
    s.outlen = *destlen;                /* ignored if dest is NIL */
    s.outcnt = 0;

    /* initialize input state */
    s.in = source;
    s.inlen = *sourcelen;
    s.incnt = 0;
    s.bitbuf = 0;
    s.bitcnt = 0;

    /* return if bits() or decode() tries to read past available input */
    if (setjmp(s.env) != 0)             /* if came back here via longjmp() */
        err = 2;                        /* then skip do-loop, return error */
    else {
        /* process blocks until last block or error */
        do {
            last = bits(&s, 1);         /* one if last block */
            type = bits(&s, 2);         /* block type 0..3 */
            err = type == 0 ? stored(&s) :
                  (type == 1 ? fixed(&s) :
                   (type == 2 ? dynamic(&s) :
                    -1));               /* type == 3, invalid */
            if (err != 0) break;        /* return with error */
        } while (!last);
    }

    /* update the lengths and return */
    if (err <= 0) {
        *destlen = s.outcnt;
        *sourcelen = s.incnt;
    }
    return err;
}
