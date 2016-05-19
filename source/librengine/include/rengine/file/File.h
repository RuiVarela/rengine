// __!!rengine_copyright!!__ //

#ifndef __RENGINE_FILE_UTIL__
#define __RENGINE_FILE_UTIL__

#include <rengine/string/String.h>
#include <rengine/lang/Lang.h>
#include <vector>

namespace rengine
{
	enum FileType
	{
		FileNotFound,
		FileRegular,
		FileDirectory
	};

	typedef std::vector<std::string> DirectoryContents;
	DirectoryContents getDirectoryContents(std::string const& directory_name);
	std::string getCurrentDirectory();

	Uint64 fileSize(std::string const& filename);
	Bool fileExists(std::string const& filename);
	FileType fileType(std::string const& filename);

	Bool makeDirectoryForFile(std::string const& path);
	Bool makeDirectory(std::string const& path);

	std::string getFilePath(std::string const& filename);
	std::string getFileExtension(std::string const& filename);
	std::string getLowerCaseFileExtension(std::string const& filename);
	std::string getSimpleFileName(std::string const& fileName);
	std::string getNameLessExtension(std::string const& fileName);
	std::string getStrippedName(std::string const& fileName);

	std::string convertFileNameToWindowsStyle(std::string const& filename);
	std::string convertFileNameToUnixStyle(std::string const& filename);
	Bool isFileNameNativeStyle(std::string const& fileName);
	std::string convertFileNameToNativeStyle(std::string const& filename);

	typedef std::vector<std::string> TextLines;
	Bool readTextFile(std::string const& filename, TextLines& text_lines);
	Bool readRawText(std::string const& filename, std::string& text);

	Bool isTextWindowsStyle(std::string const& text);
	Bool isTextLinuxStyle(std::string const& text);
	Bool isTextMacStyle(std::string const& text);

	// converts the text from [Windows, Linux, Mac] to Linux Style
	void convertTextToNativeStyle(std::string& text);

	//
	// File System abstraction
	//
	class FileSystem
	{
	public:

		FileSystem();
		virtual ~FileSystem();

		virtual DirectoryContents getDirectoryContents(std::string const& directory_name) const;
		virtual Bool fileExists(std::string const& filename) const;
		virtual FileType fileType(std::string const& filename) const;
	};

	//
	// Find Function functors
	//

	template <typename Comparator>
	DirectoryContents find(std::string const& directory, Comparator comparator, Bool recursive = false, FileSystem const& filesystem = FileSystem());

	class EqualFilename
	{
	public:
		EqualFilename(std::string const& match_filename, CaseSensitivity match_case_sensitivity = CaseSensitive);
		Bool operator()(std::string const& current_directory, std::string const& current_filename, std::string const& directory_filename);
	private:
		std::string filename;
		CaseSensitivity case_sensitivity;
	};

	class EqualExtension
	{
	public:
		EqualExtension(std::string const& match_extension);
		Bool operator()(std::string const& current_directory, std::string const& current_filename, std::string const& directory_filename);
	private:
		std::string extension;
	};

	//
	// Implementation
	//

	RENGINE_INLINE FileSystem::FileSystem() 
	{
	}

	RENGINE_INLINE FileSystem::~FileSystem() 
	{
	}

	RENGINE_INLINE DirectoryContents FileSystem::getDirectoryContents(std::string const& directory_name) const
	{
		return rengine::getDirectoryContents(directory_name);
	}

	RENGINE_INLINE Bool FileSystem::fileExists(std::string const& filename) const
	{
		return rengine::fileExists(filename);
	}

	RENGINE_INLINE FileType FileSystem::fileType(std::string const& filename) const
	{
		return rengine::fileType(filename);
	}

	template <typename Comparator>
	DirectoryContents find(std::string const& directory, Comparator comparator, Bool recursive, FileSystem const& filesystem)
	{
		Bool need_following_backslash = false;
		Bool need_directory_name = true;

		DirectoryContents dc;
		if (directory.empty() || (directory == ".") || (directory == "./") || (directory == ".\\"))
		{
			dc = filesystem.getDirectoryContents(".");
			need_directory_name = false;
		}
		else
		{
			dc = filesystem.getDirectoryContents(directory);
			Char lastChar = directory[directory.size() - 1];
			need_following_backslash = ! ((lastChar == '/') || (lastChar == '\\')) ;
		}

		DirectoryContents results;
		for(DirectoryContents::iterator itr = dc.begin(); itr != dc.end(); ++itr)
		{
			std::string path;
			if (!need_directory_name)
			{
				path = *itr;
			}
			else if (need_following_backslash)
			{
				path = directory + '/' + *itr;
			}
			else
			{
				path = directory + *itr;
			}

			if (comparator(directory, *itr, path))
			{
				path = convertFileNameToNativeStyle(path);
				results.push_back(path);
			}

			if (recursive && (*itr != ".") && (*itr != "..") && (fileType(path) == FileDirectory) )
			{
				DirectoryContents find_results = find(path, comparator, recursive, filesystem);
				for (Uint i = 0; i != find_results.size(); ++i)
				{
					results.push_back(find_results[i]);
				}
			}
		}

		return results;
	}
}

#endif //__RENGINE_FILE_UTIL__
