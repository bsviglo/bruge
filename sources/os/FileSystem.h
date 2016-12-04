#pragma once

#include "prerequisites.hpp"
#include "Exception.h"
#include "utils/Data.hpp"
#include "utils/Singleton.h"

#include <string>
#include <vector>

namespace brUGE
{
namespace os
{

	// Trough this class engine get the virtual file system.
	//----------------------------------------------------------------------------------------------
	class FileSystem : public utils::Singleton<FileSystem>
	{
	public:
		FileSystem();
		~FileSystem();

		bool checkFile(const std::string& file) const;
		bool checkDir(const std::string& dir) const;

		//-- convert relative path to absolute.
		bool getFileFullPath(const std::string& shortName, std::string& fullName) const;

		std::shared_ptr<utils::ROData> readFile (const std::string& fileName) const;
		bool	writeFile(const std::string& fileName, const utils::ROData& data) const;

		static std::string getLastNameInPath(const std::string& fileName);
		static std::string getFileExt(const std::string& name);
		static std::string getFileWithoutExt(const std::string& name);
	
		static bool getFilesInDir(
			const std::string& dir,
			std::vector<std::string>& oFiles,
			const char* extFilter = nullptr,
			bool removeExt = false
			);
		static bool checkFileFullPath(const std::string& file);
		static bool checkDirFullPath (const std::string& dir);

		//void setCurrentDir(const brString &dir);
		void setDirToList(const std::string &dir);

	private:
		std::vector<std::string> dirList;	
	};

} // os
} // brUGE
