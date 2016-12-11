#include "FileSystem.h"

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <windows.h>

#include "utils/string_utils.h"
#include "utils/LogManager.h"

#include <iostream>
#include <fstream>

#include "watchdog/Watchdog.h"

using namespace std;
using namespace brUGE::utils;

//-- start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	//--------------------------------------------------------------------------------------------------
	size_t fileSize(const string& file) 
	{ 
		ifstream fs; 
		fs.open(file, ios::binary); 

		assert(fs); 
		assert(fs.is_open()); 

		fs.seekg(0, ios::beg); 
		const ios::pos_type start_pos = fs.tellg(); 

		fs.seekg(0, ios::end); 
		const ios::pos_type end_pos = fs.tellg(); 

		const size_t ret_filesize (static_cast<size_t>(end_pos - start_pos)); 

		fs.close(); 
		assert(!fs.is_open()); 

		return ret_filesize; 
	}

}
//--------------------------------------------------------------------------------------------------
//-- end unnamed namespace.

namespace brUGE
{

	DEFINE_SINGLETON(os::FileSystem)

namespace os
{
	
	//------------------------------------------
	FileSystem::FileSystem()
	{
		
		//-- ToDo: Reconsider memory paths for file system.

		//char currentDirectory[MAX_PATH];
		//GetModuleFileName(NULL, currentDirectory, MAX_PATH);
		//dirList.push_back(brStr(currentDirectory));

		// ToDo: By defaults is the path '..'
		dirList.push_back("..");
	}
	
	//------------------------------------------
	FileSystem::~FileSystem()
	{

	}
	
	//------------------------------------------
	bool FileSystem::getFileFullPath(const std::string& shortName, std::string& fullName) const
	{
		WIN32_FIND_DATA resultData;
		std::string tmpStr;

		for (uint i = 0; i < dirList.size(); ++i)
		{
			tmpStr.clear();				
			tmpStr.append(dirList[i]);
			tmpStr.push_back('/');
			tmpStr.append(shortName);
			if (FindFirstFile(tmpStr.c_str(), &resultData) != INVALID_HANDLE_VALUE)
			{
				fullName = tmpStr;
				return true;
			}
		}

		return false;
	}

	//------------------------------------------
	bool FileSystem::checkFile(const std::string& file) const
	{
		WIN32_FIND_DATA resultData;
		std::string tmpStr;

		for (uint i = 0; i < dirList.size(); ++i)
		{
			tmpStr.clear();				
			tmpStr.append(dirList[i]);
			tmpStr.push_back('/');
			tmpStr.append(file);
			if (FindFirstFile(tmpStr.c_str(), &resultData) != INVALID_HANDLE_VALUE)
				return true;
		}
		return false;
	}
	
	//------------------------------------------
	bool FileSystem::checkFileFullPath(const std::string& file)
	{
		WIN32_FIND_DATA resultData;
		if (FindFirstFile(file.c_str(), &resultData) != INVALID_HANDLE_VALUE)
			return true;

		return false;
	}
	
	//------------------------------------------
	bool FileSystem::checkDir(const std::string& dir) const
	{
		return checkFile(dir);
	}
	
	//------------------------------------------
	bool FileSystem::checkDirFullPath(const std::string& dir)
	{
		return checkFileFullPath(dir);
	}
	
	//------------------------------------------
	/*static*/ std::string FileSystem::getFileExt(const std::string& name)
	{
		size_t pos = name.rfind('.', name.length());
		if (pos != std::string::npos)
		{
			return name.substr(pos + 1, name.length());
		}

		return "";
	}
	
	//------------------------------------------
	/*static*/ std::string FileSystem::getFileWithoutExt(const std::string& name)
	{
		size_t pos = name.rfind('.', name.length());
		if (pos != std::string::npos)
		{
			return name.substr(0, pos);
		}

		return "";
	}

	//------------------------------------------
	/*static*/ std::string FileSystem::getLastNameInPath(const std::string& fileName)
	{
		size_t pos = fileName.rfind('\\', fileName.length());
		if (pos != std::string::npos)
			return fileName.substr(pos + 1, fileName.length() - pos);

		return "";
	}
	
	//------------------------------------------
	bool FileSystem::getFilesInDir(
		const std::string& dir, std::vector<std::string>& oFiles, const char* extFilter, bool removeExt)
	{
		WIN32_FIND_DATA findFileData;
		HANDLE			hFind = INVALID_HANDLE_VALUE;
		std::string		tmpStr;
		
		tmpStr.append(dir);
		tmpStr.append("\\*");

		hFind = FindFirstFile(tmpStr.c_str(), &findFileData);

		if (hFind == INVALID_HANDLE_VALUE)
		{
			return false;
		}
		else
		{
			//-- skip first two elements because they contain trash(garbage)
			//-- i.e. '.','..', and then there are normal data.
			FindNextFile(hFind, &findFileData);

			while (FindNextFile(hFind, &findFileData) != 0)
			{
				if (!extFilter || getFileExt(findFileData.cFileName) == extFilter)
				{
					std::string oFile = findFileData.cFileName;

					oFiles.push_back(removeExt ? getFileWithoutExt(oFile) : oFile);
				}
			}

			DWORD dwError = GetLastError();
			FindClose(hFind);
			if(dwError != ERROR_NO_MORE_FILES)
				return false;

			return true;
		}
	}
	
	//------------------------------------------
	void FileSystem::setDirToList(const std::string& dir)
	{
		dirList.push_back(dir);
	}

/*
	//------------------------------------------
	RODataPtr FileSystem::readFile(const std::string& fileName) const
	{
		RODataPtr   out;
		std::string fullName;

		if (!getFileFullPath(fileName, fullName))
		{
			ERROR_MSG("File '%s'.", fileName.c_str());
			return out;
		}

		ifstream iFile;
		iFile.open(fullName, ios_base::in | ios_base::binary);
		if (iFile.is_open())
		{
			//-- read file in memory based on the file size.
			uint32 size = fileSize(fullName);
			byte* bytes = new byte[size];

			iFile.read((char*)bytes, size);
			if (iFile.gcount() != size)
			{
				delete [] bytes;
				return out;
			}

			//-- adjust holder size and make it owner of the earlier allocated memory. 
			out = new ROData(bytes, size);

			//-- close file.
			iFile.close();
		}

		return out;
	}
	*/

	//------------------------------------------
	std::shared_ptr<ROData> FileSystem::readFile(const std::string& fileName) const
	{
		std::string fullName;

		if (!getFileFullPath(fileName, fullName))
		{
			ERROR_MSG("File '%s'.", fileName.c_str());
			return nullptr;
		}

		HANDLE hFile = CreateFile(
			TEXT(fullName.c_str()),// file to open
			GENERIC_READ,          // open for reading
			FILE_SHARE_READ,       // share for reading
			NULL,                  // default security
			OPEN_EXISTING,         // existing file only
			FILE_ATTRIBUTE_NORMAL, // normal file
			NULL                   // no attr. template
			);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			ERROR_MSG("Could not open file '%s' (error %d).", fullName.c_str(), GetLastError());
			return nullptr;
		}

		DWORD fileSize = GetFileSize(hFile, NULL);
		if (fileSize == INVALID_FILE_SIZE)
		{
			ERROR_MSG("Could not retrieve file size from '%s' (error %d).", fullName.c_str(), GetLastError());
			return nullptr;
		}

		DWORD readedBytes = 0;
		byte* buffer = new byte[fileSize];

		if (!ReadFile(hFile, &buffer[0], fileSize, &readedBytes, NULL))
		{
			delete [] buffer;
			ERROR_MSG("Could not read data from file '%s' (error %d).", fullName.c_str(), GetLastError());
			return nullptr;
		}
		
		//-- if the initial size of the buffer is greater then the readed bytes, then adjust the
		//-- buffer size.
		if (fileSize != readedBytes)
		{
			std::vector<byte> tmpBuffer(buffer, buffer + readedBytes - 1);
			delete [] buffer;
			buffer = new byte[readedBytes];
			memcpy(buffer, &tmpBuffer[0], readedBytes);
		}

		//-- close file.
		if (!CloseHandle(hFile))
		{
			ERROR_MSG("Could not close the file '%s' (error %d).", fullName.c_str(), GetLastError());
			return nullptr;
		}

		return std::make_shared<ROData>(buffer, readedBytes);
	}

	//------------------------------------------
	bool FileSystem::writeFile(const std::string& fileName, const ROData& data) const
	{
		std::string fullName = dirList[0] + "/" + fileName;

		HANDLE hFile = CreateFile(
			TEXT(fullName.c_str()),// file to open
			GENERIC_WRITE,         // open for writing
			FILE_SHARE_WRITE,      // share for reading
			NULL,                  // default security
			OPEN_ALWAYS,           // existing file only
			FILE_ATTRIBUTE_NORMAL, // normal file
			NULL                   // no attr. template
			);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			ERROR_MSG("Could not open file '%s' for writing (error %d).", fullName.c_str(), GetLastError());
			return false;
		}

		DWORD writedBytes = 0;
		BOOL  success	  = WriteFile(hFile, data.ptr(0), data.length(), &writedBytes, NULL);

		if (!success || writedBytes != data.length())
		{
			ERROR_MSG("Could not write data to file '%s' (error %d).", fullName.c_str(), GetLastError());
			return false;
		}

		//-- close file.
		if (!CloseHandle(hFile))
		{
			ERROR_MSG("Could not close the file '%s' (error %d).", fullName.c_str(), GetLastError());
			return false;
		}

		return true;
	}

} // os
} // brUGE