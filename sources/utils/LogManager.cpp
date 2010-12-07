#include "LogManager.h"
#include "os/os_utils.hpp"
#include "utils/string_utils.h"
#include <fstream>

using namespace std;

namespace brUGE
{

DEFINE_SINGLETON(utils::LogManager)

namespace utils
{
	
	//------------------------------------------
	LogManager::LogManager(const string &logFileName, bool splitLine, bool useHTML)
		:	m_fileName(logFileName + (useHTML ? ".html" : ".txt")),
			m_buffer(new byte[LOG_BUFFER_SIZE], LOG_BUFFER_SIZE),
			m_warningCount(0),
			m_errorCount(0),
			m_splitLine(splitLine),
			m_useHTML(useHTML)
	{
		if (m_useHTML)
		{
			write
			(\
				"<HTML>"\
				"<head>"\
					"<Title> log file </Title>"\
					"<style type=\"text/css\"> TR { font-family: monospace } </style>"\
				"</head>"\
				"<Body bgCOLOR = White>"\
				"<B> <Font = 2 Color = Blue> log file started. - at " + systemTime() + "</B> <BR>"\
				"<TABLE>"\
			);
		}
		else
		{
			write("\nlog file started. - at " + systemTime() + "\n");
		}
	}
	
	//------------------------------------------
	LogManager::~LogManager()
	{
		if (m_useHTML)
		{
			write
			(\
				"</TABLE>"\
				"<B> <Font = 2 Color = Black> log file statistic: </B> <TABLE>"\
				"<TR>"\
					"<TD><B> <Font = 2 Color = Black> total warnings number : </TD>"\
					"<TD>" + intToStr(m_warningCount) + "</TD></B>"\
				"</TR>"\
				"<TR>"\
					"<TD><B> <Font = 2 Color = Black> total errors number : </TD>"\
					"<TD>" + intToStr(m_errorCount) + "</TD></B>"\
				"</TR> </TABLE>"\
				"<B> <Font = 2 Color = Blue> log file ended. - at " + systemTime() + "</B> <BR>"\
			"</Body> </HTML>"\
			);
		}
		else
		{
			write("log file ended. - at " + systemTime() + "\n");
		}
		flush();
	}
	
	//------------------------------------------
	void LogManager::writeLogTxt(const std::string& preStr, const std::string& msg)
	{
		if (m_splitLine)
		{
			string str;
			StrTokenizer spliter(msg, "\n");
			while (spliter.hasMoreTokens())
			{
				spliter.nextToken(str);
				write(systemTime_HMS() + preStr + str + "\n");
			}
		}
		else
		{
			write(systemTime_HMS() + preStr + msg + "\n");
		}
	}

	//------------------------------------------
	void LogManager::writeLogHtml(const string& msg, ELogColor color, int fontSize)
	{
		string c = utils::intToStr(color, 6, '0', ios_base::hex);
		if (m_splitLine)
		{
			string str;
			StrTokenizer spliter(msg, "\n");
			while (spliter.hasMoreTokens())
			{
				spliter.nextToken(str);
				write
				(\
					"<TR Font = " + intToStr(fontSize) + " bgcolor=" + "#" + c + ">"\
						"<TD>" + systemTime_HMS() + "</TD>"\
						"<TD>" + str + "</TD>"\
					"</TR>"\
				);
			}
		}
		else
		{
			write
			(\
				"<TR Font = " + intToStr(fontSize) + " bgcolor=" + "#" + c + ">"\
				"<TD>" + systemTime_HMS() + "</TD>"\
				"<TD>" + msg + "</TD>"\
				"</TR>"\
			);
		}
	}

	//------------------------------------------
	void LogManager::info(const std::string& msg)
	{
		if (m_useHTML)	writeLogHtml(msg, utils::WHITE);
		else			writeLogTxt(" [INFO] ", msg);
	}

	//------------------------------------------
	void LogManager::warning(const std::string& msg)
	{
		if (m_useHTML)	writeLogHtml(msg, utils::YELLOW);
		else			writeLogTxt(" [WARNING] ", msg);
	}

	//------------------------------------------
	void LogManager::error(const std::string& msg)
	{
		if (m_useHTML)	writeLogHtml(msg, utils::RED);
		else			writeLogTxt(" [ERROR] ", msg);
	}

#undef  PARSE_STR
#define PARSE_STR(func)\
	{\
		va_list args;\
		char buffer[1024];\
		va_start(args, msg);\
		_vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, msg, args);\
		func(std::string(buffer));\
		va_end(args);\
	}

	//------------------------------------------
	void LogManager::info(const char *msg, ...)
	{
		PARSE_STR(info)
	}

	//------------------------------------------
	void LogManager::warning(const char *msg, ...)
	{
		PARSE_STR(warning)
	}

	//------------------------------------------
	void LogManager::error(const char *msg, ...)
	{
		PARSE_STR(error)
	}

#undef PARSE_STR
	
	//------------------------------------------
	void LogManager::write(const std::string& text)
	{
		uint		size   = 0;
		uint		length = text.length();
		const byte* cstr   = reinterpret_cast<const byte*>(text.c_str());

		do
		{
			length -= size;
			size    = m_buffer.writeBytes(cstr + size, length);

			// the buffer is full, flush it.
			if (size < length)
			{
				flush();
				m_buffer.seekAbs(0); // go to the start of the buffer.
			}
			else
			{
				break;
			}
		} while (size);
	}
	
	//------------------------------------------
	void LogManager::flush()
	{
		ofstream logfile;
		logfile.open(m_fileName.c_str(), ios::app);
		if (logfile.is_open() && m_buffer.pos())
			logfile.write(m_buffer.c_str(), m_buffer.pos());
		logfile.close();
	}

} // utils
} // brUGE
