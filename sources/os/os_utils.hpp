#pragma once

#include <string>

namespace brUGE
{
namespace utils
{

	//-- get system time in format: "[Day]:[Month]:[Year] [Hour]:[Minute]:[Second]:[Milliseconds]".
	std::string systemTime();

	//-- get system time in format: "[Day]:[Month]:[Year]".
	std::string systemTime_DMY();

	//-- get system time in format: "[Hour]:[Minute]:[Second]:[Milliseconds]".
	std::string systemTime_HMS();

} // utils
} // brUGE