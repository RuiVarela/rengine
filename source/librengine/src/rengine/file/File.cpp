// __!!rengine_copyright!!__ //

#include <rengine/file/File.h>


#if RENGINE_PLATFORM == RENGINE_PLATFORM_WIN32

#include <Io.h>
#include <Windows.h>
#include <Winbase.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h>

#define mkdir(x,y) _mkdir((x))
#define stat64 _stati64

#ifndef F_OK
#define F_OK 4
#endif //F_OK

#ifndef _IFMT
#define _IFMT          0170000
#endif //_IFMT

#ifndef _IFDIR
#define _IFDIR         0040000
#endif //_IFDIR

#ifndef S_ISDIR
#define S_ISDIR(m)  (((m)&_IFMT) == _IFDIR)
#endif //S_ISDIR

#else //RENGINE_PLATFORM != RENGINE_PLATFORM_WIN32

#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#endif //RENGINE_PLATFORM != RENGINE_PLATFORM_WIN32

#include <errno.h>
#include <fstream>
#include <cstring>
#include <iostream>

namespace rengine
{

#if RENGINE_PLATFORM == RENGINE_PLATFORM_WIN32
	DirectoryContents getDirectoryContents(std::string const& directory_name)
	{
		DirectoryContents contents;

		WIN32_FIND_DATA data;
		HANDLE handle = FindFirstFile((directory_name + "\\*").c_str(), &data);
		if (handle != INVALID_HANDLE_VALUE)
		{
			do
			{
				contents.push_back(data.cFileName);
			}
			while (FindNextFile(handle, &data) != 0);

			FindClose(handle);
		}
		return contents;
	}

	std::string getCurrentDirectory()
	{
		Uint size = GetCurrentDirectory(0, 0);
		Char * current_dir = new Char[size + 1];
		GetCurrentDirectory(size, current_dir);

		std::string current_directory = std::string(current_dir);
		delete[](current_dir);
		return current_directory;
	}
#else //RENGINE_PLATFORM != RENGINE_PLATFORM_WIN32

	DirectoryContents getDirectoryContents(std::string const& directory_name)
	{
		DirectoryContents contents;

		DIR *handle = opendir(directory_name.c_str());
		if (handle)
		{
			dirent *rc;
			while ((rc = readdir(handle)) != NULL)
			{
				contents.push_back(rc->d_name);
			}
			closedir(handle);
		}

		return contents;
	}

	std::string getCurrentDirectory()
	{
		std::string path_as_string;

		Char path[PATH_MAX];
		if(getcwd(&path[0], PATH_MAX))
		{
			path_as_string = std::string(path);
		}

		return path_as_string;
	}

#endif //RENGINE_PLATFORM != RENGINE_PLATFORM_WIN32

	Bool makeDirectory(std::string const& path)
	{
		if (path.empty())
		{
			return false;
		}

		struct stat64 stbuf;
		if (stat64(path.c_str(), &stbuf) == 0)
		{
			return (S_ISDIR(stbuf.st_mode));
		}

		std::string dir = path;
		std::vector<std::string> paths;
		while (true)
		{
			if (dir.empty())
			{
				break;
			}

			if (stat64(dir.c_str(), &stbuf) < 0)
			{
				switch (errno)
				{
					case ENOENT:
					case ENOTDIR:
					paths.push_back(dir);
						break;

					default:
					return false;
				}
			}
			dir = getFilePath(std::string(dir));
		}

		while (!paths.empty())
		{
			std::string dir = paths.back();

#if RENGINE_PLATFORM == RENGINE_PLATFORM_WIN32
			if (dir.size() == 2 && dir.c_str()[1] == ':')
			{
				paths.pop_back();
				continue;
			}
#endif //RENGINE_PLATFORM == RENGINE_PLATFORM_WIN32

			if (mkdir(dir.c_str(), 0755) < 0)
			{
				return false;
			}
			paths.pop_back();
		}
		return true;
	}

	FileType fileType(std::string const& filename)
	{
		struct stat64 file_stat;
		if (stat64(filename.c_str(), &file_stat) == 0)
		{
			if (file_stat.st_mode & S_IFDIR)
			{
				return FileDirectory;
			}
			else if (file_stat.st_mode & S_IFREG)
			{
				return FileRegular;
			}
		}
		return FileNotFound;
	}

	Bool makeDirectoryForFile(std::string const& path)
	{
		return makeDirectory(getFilePath(path));
	}

	Uint64 fileSize(std::string const& filename)
	{
		Uint64 size = 0;

		struct stat64 file_stat;
		if (stat64(filename.c_str(), &file_stat) == 0)
		{
			size = file_stat.st_size;
		}

		return size;
	}

	Bool fileExists(std::string const& filename)
	{
#if RENGINE_PLATFORM == RENGINE_PLATFORM_WIN32

		return _access(filename.c_str(), F_OK) == 0;

#else //RENGINE_PLATFORM != RENGINE_PLATFORM_WIN32

		return access(filename.c_str(), F_OK) == 0;

#endif //RENGINE_PLATFORM == RENGINE_PLATFORM_WIN32
	}

	std::string getFilePath(std::string const& filename)
	{
		std::string::size_type slash1 = filename.find_last_of('/');
		std::string::size_type slash2 = filename.find_last_of('\\');

		if (slash1 == std::string::npos)
		{
			if (slash2 == std::string::npos)
			{
				return std::string();
			}
			return std::string(filename, 0, slash2);
		}

		if (slash2 == std::string::npos)
		{
			return std::string(filename, 0, slash1);
		}

		return std::string(filename, 0, slash1 > slash2 ? slash1 : slash2);
	}

	std::string getSimpleFileName(std::string const& filename)
	{
		std::string::size_type slash1 = filename.find_last_of('/');
		std::string::size_type slash2 = filename.find_last_of('\\');

		if (slash1 == std::string::npos)
		{
			if (slash2 == std::string::npos)
			{
				return filename;
			}

			return std::string(filename.begin() + slash2 + 1, filename.end());
		}

		if (slash2 == std::string::npos)
		{
			return std::string(filename.begin() + slash1 + 1, filename.end());
		}

		return std::string(filename.begin() + (slash1 > slash2 ? slash1 : slash2) + 1, filename.end());
	}

	std::string getFileExtension(std::string const& filename)
	{
		std::string::size_type dot = filename.find_last_of('.');

		if (dot == std::string::npos)
		{
			return std::string("");
		}

		return std::string(filename.begin() + dot + 1, filename.end());
	}

	std::string getLowerCaseFileExtension(std::string const& filename)
	{
		std::string ext = getFileExtension(filename);

		for (std::string::iterator itr = ext.begin(); itr != ext.end(); ++itr)
		{
			*itr = tolower(*itr);
		}

		return ext;
	}

	std::string getNameLessExtension(std::string const& filename)
	{
		std::string::size_type dot = filename.find_last_of('.');

		if (dot == std::string::npos)
		{
			return filename;
		}

		return std::string(filename.begin(), filename.begin() + dot);
	}

	std::string getStrippedName(std::string const& filename)
	{
		std::string simpleName = getSimpleFileName(filename);
		return getNameLessExtension(simpleName);
	}

	Bool readRawText(std::string const& filename, std::string& text)
	{
		Bool is_good = true;

		std::ifstream in(filename.c_str(), std::ios::in | std::ios::ate | std::ios::binary);
		is_good = in.good();

		if (is_good)
		{
			Ulong end_position = Ulong(in.tellg());

			Char* script_c_str = new Char[end_position + 1UL];

			in.seekg(0, std::ios::beg);
			in.read(script_c_str, end_position);
			script_c_str[end_position] = '\0';
			is_good = in.good();
			in.close();

			text = script_c_str;
			delete[](script_c_str);
		}

		convertTextToNativeStyle(text);

		return is_good;
	}

	Bool readTextFile(std::string const& filename, TextLines& text_lines)
	{
		std::ifstream in(filename.c_str());

		if (in.good())
		{
			while (in.good() && !in.eof())
			{
				std::string current_line;
				std::getline(in, current_line);
				text_lines.push_back(current_line);
			}
			in.close();
		}
		else
		{
			return false;
		}

		return true;
	}

	EqualFilename::EqualFilename(std::string const& match_filename, CaseSensitivity match_case_sensitivity) :
		filename(match_filename), case_sensitivity(match_case_sensitivity)
	{
	}

	Bool EqualFilename::operator()(std::string const& current_directory, std::string const& current_filename, std::string const& directory_filename)
	{
		return ( (case_sensitivity == CaseInsensitive &&
				  equalCaseInsensitive(filename, current_filename)) ||
				  (filename == current_filename) );
	}

	EqualExtension::EqualExtension(std::string const& match_extension)
	:extension(match_extension)
	{
	}

	Bool EqualExtension::operator()(std::string const& current_directory, std::string const& current_filename, std::string const& directory_filename)
	{
		return equalCaseInsensitive(getFileExtension(current_filename), extension );
	}

	std::string convertFileNameToWindowsStyle(std::string const& filename)
	{
		std::string new_filename(filename);
		std::string::size_type slash = 0;

		while( (slash = new_filename.find_first_of('/', slash)) != std::string::npos)
		{
			new_filename[slash]= '\\';
		}
		return new_filename;
	}

	std::string convertFileNameToUnixStyle(std::string const& filename)
	{
		std::string new_filename(filename);
		std::string::size_type slash = 0;

		while( (slash = new_filename.find_first_of('\\', slash)) != std::string::npos)
		{
			new_filename[slash]='/';
		}

		return new_filename;
	}

	Bool isFileNameNativeStyle(std::string const& filename)
	{
#if RENGINE_PLATFORM == RENGINE_PLATFORM_WIN32
		return filename.find('/') == std::string::npos;
#else //RENGINE_PLATFORM != RENGINE_PLATFORM_WIN32
		return filename.find('\\') == std::string::npos;
#endif //RENGINE_PLATFORM != RENGINE_PLATFORM_WIN32
	}

	std::string convertFileNameToNativeStyle(std::string const& filename)
	{
#if RENGINE_PLATFORM == RENGINE_PLATFORM_WIN32
		return convertFileNameToWindowsStyle(filename);
#else //RENGINE_PLATFORM != RENGINE_PLATFORM_WIN32
		return convertFileNameToUnixStyle(filename);
#endif //RENGINE_PLATFORM != RENGINE_PLATFORM_WIN32
	}

	Bool isTextWindowsStyle(std::string const& text)
	{
		return text.find("\r\n") != std::string::npos;
	}

	Bool isTextLinuxStyle(std::string const& text)
	{
		Bool r_found = text.find("\r") != std::string::npos;
		Bool n_found = text.find("\n") != std::string::npos;

		return !r_found && n_found;
	}

	Bool isTextMacStyle(std::string const& text)
	{
		Bool r_found = text.find("\r") != std::string::npos;
		Bool n_found = text.find("\n") != std::string::npos;

		return r_found && !n_found;
	}

	void convertTextToNativeStyle(std::string& text)
	{
		if (isTextWindowsStyle(text))
		{
			replace(text, "\r\n", "\n");
		}
		else if (isTextMacStyle(text))
		{
			replace(text, "\r", "\n");
		}
	}

} // namespace rengine


