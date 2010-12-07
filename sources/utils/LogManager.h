#pragma once

#include "utils/Singleton.h"
#include "utils/Data.hpp"
#include <string>

#define LOG_FILE_NAME	"log"
#define LOG_BUFFER_SIZE 1024

//forward declaration
namespace brUGE
{
namespace utils
{
	enum ELogColor
	{
		BLACK	= 0x000000,
		GREEN	= 0x008000,
		SILVER	= 0xC0C0C0,
		LIME	= 0x00FF00,
		GRAY	= 0x808080,
		OLIVE	= 0x808000,
		WHITE	= 0xFFFFFF,
		YELLOW	= 0xFFFF00,
		MAROON	= 0x800000,
		NAVY	= 0x000080,
		RED		= 0xFF0000,
		BLUE	= 0x0000FF,
		PURPLE	= 0x800080,
		TEAL	= 0x008080,
		FUCHSIA = 0xFF00FF,
		AQUA	= 0x00FFFF
	};

	//------------------------------------------
	class LogManager : public Singleton<LogManager>
	{
	public:
		LogManager(const std::string& logFileName = LOG_FILE_NAME, bool splitLine = true, bool useHTML = true);
		~LogManager();

		void info	(const std::string& msg);
		void warning(const std::string& msg);
		void error	(const std::string& msg);
		void info	(const char* msg, ...);
		void warning(const char* msg, ...);
		void error  (const char* msg, ...);
	
	private:
		void write(const std::string& text);
		void writeLogHtml(const std::string& msg, ELogColor color, int fontSize = 1);
		void writeLogTxt(const std::string& preStr, const std::string& msg);
		void flush();

		RWData		m_buffer;
		bool		m_splitLine;
		bool		m_useHTML;
		int			m_errorCount;
		int			m_warningCount;
		std::string	m_fileName;
	};

} // utils
} // brUGE
