#include "os/os_utils.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#	define NOMINMAX
#endif
#include <windows.h>

namespace brUGE
{
namespace utils
{

	//-- get system time in format: "[Day]:[Month]:[Year] [Hour]:[Minute]:[Second]:[Milliseconds]".
	std::string systemTime()
	{
		char timeStr[256];
		SYSTEMTIME t;
		GetLocalTime(&t);
		sprintf_s(timeStr, "%02d.%02d.%04d %02d:%02d:%02d.%03d",
			t.wDay, t.wMonth, t.wYear, t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
		return timeStr;
	}

	//-- get system time in format: "[Day]:[Month]:[Year]".
	std::string systemTime_DMY()
	{
		char timeStr[256];
		SYSTEMTIME t;
		GetLocalTime(&t);
		sprintf_s(timeStr, "%02d.%02d.%04d", t.wDay, t.wMonth, t.wYear);
		return timeStr;
	}

	//-- get system time in format: "[Hour]:[Minute]:[Second]:[Milliseconds]".
	std::string systemTime_HMS()
	{
		char timeStr[256];
		SYSTEMTIME t;
		GetLocalTime(&t);
		sprintf_s(timeStr, "%02d:%02d:%02d.%03d", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
		return timeStr;
	}

} // utils
} // brUGE