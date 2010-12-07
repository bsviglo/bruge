#pragma once

#include <string>
#include <iomanip>
#include <stdarg.h>

//-- ToDo: needed very large optimization.

namespace brUGE
{
namespace utils
{
	
	//-- remove white space elements from both ends of string.
	std::string trim (const std::string& str, const std::string& delims = " \t\r\n");

	//-- remove white space elements only for left end of string.
	std::string ltrim(const std::string& str, const std::string& delims = " \t\r\n");

	//-- remove white space elements only for right end of string.
	std::string rtrim(const std::string& str, const std::string& delims = " \t\r\n");

	//-- Convert boolean value to string.
	std::string makeStr(const char* format, ...);
	
	//-- returns file extension.
	std::string getFileExt(const std::string& str);

	//-- Convert boolean value to string
	std::string boolToStr(bool str, bool yesno = false);

	//-- Convert int value to string.
	std::string intToStr(int str, int width = 0, char fill = ' ',
		std::ios::fmtflags flags = std::ios::fmtflags(0));

	//-- Convert float value to string.
	std::string floatToStr(float str, int precision = 6,
		int width = 0, char fill = ' ', std::ios::fmtflags flags = std::ios::fmtflags(0));

	//-- Convert boolean value to string.
	int strToInt(const std::string& str);

	//-- Convert boolean value to string.
	float strToFloat(const std::string& str, int precision = 4);

	//-- Convert boolean value to string.
	bool strToBool(const std::string& str);


	//Simple class to divide input string to token with modified delimiters.
	//----------------------------------------------------------------------------------------------
	class StrTokenizer
	{
	public:
		StrTokenizer(const std::string& str, const char* delims = 0);

		//-- Get tokens count.
		int numTokens();

		//-- return true if given string has more token.
		bool hasMoreTokens() const;

		//-- Get next token in given string.
		void nextToken(std::string& nextToken);

	private:
		//-- can't create an object with default constructor
		StrTokenizer() {}

		std::string				m_delimiters;
		std::string				m_string;
		std::string::size_type	m_numTokens;
		std::string::size_type	m_begin;
		std::string::size_type	m_end;
	};

} // utils
} // brUGE
