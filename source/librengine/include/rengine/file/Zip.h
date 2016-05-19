// __!!rengine_copyright!!__ //

#ifndef __RENGINE_ZIP__
#define __RENGINE_ZIP__

#include <rengine/file/File.h>
#include <fstream>
#include <map>

namespace rengine
{
	// Description
	//	Zip Filesystem with deflate/inflate compression support
	class Zip : public FileSystem
	{
	public:
		Zip();
		Zip(std::string const filename);
		virtual ~Zip();

		typedef SharedArray<Uint8> SharedData;

		struct FileData
		{
			SharedData data;
			Uint size;
		};

		// Description
		//	Filesystem methods
		virtual DirectoryContents getDirectoryContents(std::string const& directory_name) const;
		virtual Bool fileExists(std::string const& filename) const;
		virtual FileType fileType(std::string const& filename) const;

		// Description
		//	Load a Zip File from disk;
		bool load(std::string const& filename);

		// Description
		//	Closes the zip file
		void close();

		// Description
		//	reads a file from zip archive
		// Arguments
		//	filename - filename to read from archive
		FileData read(std::string const& filename);

		// Description
		//	zip filename getter
		std::string const& GetFilename() const;
	private:
		
		static Int const max_filename_size = 2048;
		static Int const max_extra_field_size = 100;
		static Int const max_comment_size = 100;

		enum ZipSignature
		{
			LocalFileHeaderSignature		= 0x04034b50,
			CentralDirectorySignature		= 0x02014b50,
			EndOfCentralDirectorySignature	= 0x06054b50
		};

		struct LocalFileHeader
		{
			Uint32 signature;					//Local file header signature (0x04034b50)
			Uint16 version_needed;				//Version needed to extract
			Uint16 flags;						//General purpose bit flag
			Uint16 compression_method;			//Compression method
			Uint16 last_mod_time;				//Last mod file time
			Uint16 last_mod_date;				//Last mod file date
			Uint32 crc;							//CRC-32
			Uint32 compressed_size;				//Compressed size
			Uint32 uncompressed_size;			//Uncompressed size
			Uint16 filename_length;				//Filename length
			Uint16 extra_field_length;			//Extra field length
			Char filename[max_filename_size];	//Filename
			Char extra_field[max_extra_field_size];	//Extra field
			//Compressed data
		};


		struct EndOfCentralDirectory
		{
			Uint32 signature;					//End of central dir signature (0x06054b50)
			Uint16 disk_number;					//Number of this disk
			Uint16 disk_number_central_dir;		//Number of the disk with the start of the central directory
			Uint16 entries;						//Total number of entries in the central dir on this disk
			Uint16 entries_central_dir;			//Total number of entries in the central dir
			Uint32 central_dir_size;			//Size of the central directory
			Uint32 central_dir_offset;			//Offset of start of central directory with respect to the starting disk number
			Uint16 comment_length;				//zipfile comment length 
			Char comment[max_comment_size];		//zipfile comment
		};

		struct CentralDirectory
		{
			Uint32 signature;					//Central file header signature (0x02014b50)
			Uint16 version_made;				//Version made by
			Uint16 version_needed;				//Version needed to extract
			Uint16 flags;						//General purpose bit flag
			Uint16 compression_method;			//Compression method
			Uint16 last_mod_time;				//Last mod file time
			Uint16 last_mod_date;				//Last mod file date
			Uint32 crc;							//CRC-32
			Uint32 compressed_size;				//Compressed size
			Uint32 uncompressed_size;			//Uncompressed size
			Uint16 filename_length;				//Filename length
			Uint16 extra_field_length;			//Extra field length
			Uint16 comment_length;				//File comment length (c)
			Uint16 disk_number_start;			//Disk number start
			Uint16 internal_file_attributes;	//Uinternal file attributes
			Uint32 external_file_attributes;	//External file attributes
			Uint32 offset;						//Relative offset of local header
			Char filename[max_filename_size];	//Filename
			Char extra_field[max_extra_field_size];	//Extra field
			Char comment[max_comment_size];		//File comment
		};
		typedef std::vector<CentralDirectory> CentralDirectoryVector;

		std::string m_filename;
		std::string m_current_directory;
		std::ifstream file;

		SharedData loadFile(Uint index, LocalFileHeader& local_header);
		bool loadCentralDirectory();
		bool loadEndOfCentralDirectory();
		EndOfCentralDirectory m_end_of_central_directory;
		CentralDirectoryVector m_central_directory;
		
		typedef std::map<std::string, Int> RecordMap; // name -> index
		RecordMap m_records;
	};

	//
	// Implementation
	//
	RENGINE_INLINE std::string const& Zip::GetFilename() const
	{
		return m_filename;
	}

} // namespace rengine

#endif //__RENGINE_ZIP__
