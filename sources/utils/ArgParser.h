#pragma once

#include <string>

namespace brUGE
{
namespace utils
{
	//-- Transform to the desired type from the input string. 
	//---------------------------------------------------------------------------------------------
	template <typename T>
	T parseTo(const std::string& str);

	//-- Transform the desired type to the string.
	//---------------------------------------------------------------------------------------------
	template <typename T>
	std::string parseFrom(const T& value);
	
} // utils
} // brUGE
