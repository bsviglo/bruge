#include "FileSystem.h"
#include "console/Console.h"

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <windows.h>

#include "utils/string_utils.h"

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
		std::string tmp = "";
		utils::StrTokenizer tokenizer(name, ".");
		for (int i = 0; i < tokenizer.numTokens(); ++i)
			tokenizer.nextToken(tmp);

		return tmp;
	}
	
	//------------------------------------------
	/*static*/ std::string getFileWithoutExt(const std::string& name)
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
	bool FileSystem::getFilesInDir(const std::string& dir, std::vector<std::string>& files)
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
			files.push_back(findFileData.cFileName);

			while(FindNextFile(hFind, &findFileData) != 0)
				files.push_back(findFileData.cFileName);

			DWORD dwError = GetLastError();
			FindClose(hFind);
			if(dwError != ERROR_NO_MORE_FILES)
				return false;
			
			// delete first two elements because they contain trash(garbage)
			// i.e. '.','..', and then there are normal data.
			files.erase(files.begin());
			files.erase(files.begin());
			return true;
		}
	}
	
	//------------------------------------------
	void FileSystem::setDirToList(const std::string& dir)
	{
		dirList.push_back(dir);
	}
	
	//------------------------------------------
	RODataPtr FileSystem::readFile(const std::string& fileName) const
	{
		std::string fullName;

		if (!getFileFullPath(fileName, fullName))
		{
			ERROR_MSG("File '%s'.", fileName.c_str());
			return RODataPtr(NULL);
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
			return RODataPtr(NULL);
		}

		DWORD fileSize = GetFileSize(hFile, NULL);
		if (fileSize == INVALID_FILE_SIZE)
		{
			ERROR_MSG("Could not retrieve file size from '%s' (error %d).", fullName.c_str(), GetLastError());
			return RODataPtr(NULL);
		}

		DWORD readedBytes = 0;
		byte* buffer = new byte[fileSize];

		if (!ReadFile(hFile, &buffer[0], fileSize, &readedBytes, NULL))
		{
			delete [] buffer;
			ERROR_MSG("Could not read data from file '%s' (error %d).", fullName.c_str(), GetLastError());
			return RODataPtr(NULL);
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

		return RODataPtr(new utils::ROData(buffer, readedBytes));
	}

	//------------------------------------------
	bool FileSystem::writeFile(const std::string& fileName, const utils::ROData& data) const
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

		return true;
	}

} // os
} // brUGE