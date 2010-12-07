#include "string_utils.h"
#include <sstream>
#include <iostream>

using namespace std;

namespace brUGE
{
namespace utils
{

	//-- remove white space elements only for right end of string.
	string rtrim(const string& str, const string& delims)
	{
		string result(str);
		string::size_type i(result.find_last_not_of(delims));
		if (i == string::npos)
			return "";
		else
			return result.erase(result.find_last_not_of(delims) + 1);
	}  
	
	//-- remove white space elements only for left end of string.
	string ltrim(const string& str, const string& delims)
	{
		string result(str);
		return result.erase (0, result.find_first_not_of(delims));
	}  
	
	//-- remove white space elements from both ends of string.
	string trim(const string& str, const string& delims)
	{
		string result(str);
		return ltrim(rtrim(result, delims), delims);
	}  

	//-- Convert boolean value to string.
	string boolToStr(bool str, bool yesno/* = false*/)
	{
		if(yesno)
		{
			if (str) return "yes";
			else	 return "no";
		}
		else
		{
			if (str) return "true";
			else 	 return "false";
		}
	}

	//-- Convert boolean value to string.
	string makeStr(const char* format, ...)
	{
		va_list	args;
		char buffer[2048]; // 2 kbytes.
		va_start(args, format);
		_vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);
		string result = buffer;
		va_end(args);

		return result;
	}

	//-- returns file extension.
	string getFileExt(const string& str)
	{
		size_t i = str.rfind('.', str.length());
		if (i != string::npos)
		{
			return str.substr(i + 1, str.length() - 1);
		}
		return "";
	}

	//-- Convert int value to string.
	string intToStr(int str, int width/* = 0*/, char fill/* = ' '*/,
		ios::fmtflags flags/* = ios::fmtflags(0)*/)
	{
		ostringstream strStream;
		strStream.width(width);
		strStream.fill(fill);
		if (flags)
		{
			if(flags & ios::hex) //unset decimal mode
				strStream.unsetf(ios_base::dec);

			strStream.setf(flags);
		}
		strStream << str;
		return strStream.str();
	}

	//-- Convert float value to string.
	string floatToStr(float str, int precision/* = 6*/,
		int width/* = 0*/, char fill/* = ' '*/, ios::fmtflags flags/* = ios::fmtflags(0)*/)
	{
		ostringstream strStream;
		strStream.precision(precision);
		strStream.width(width);
		strStream.fill(fill);

		if (flags)	
			strStream.setf(flags);

		strStream << str;
		return strStream.str();
	}

	//-- Convert boolean value to string.
	int strToInt(const string& str)
	{
		istringstream istr(str);
		int result = 0;
		istr >> result;
		return result;
	}

	//-- Convert boolean value to string.
	float strToFloat(const string& str, int precision/* = 4*/)
	{
		istringstream istr(str);
		istr.precision(precision);
		float result = 0;
		istr >> result;
		return result;
	}

	//-- Convert boolean value to string.
	bool strToBool(const string& str)
	{
		return (!str.compare(0, str.length(), "true") ||
				!str.compare(0, str.length(), "1") || 
				!str.compare(0, str.length(), "yes"));
	}


	//------------------------------------------
	StrTokenizer::StrTokenizer(const string& str, const char* delims)
		:	m_string(str), m_numTokens(0), m_begin(0), m_end(0) 
	{
		if (!delims) m_delimiters = " \f\n\r\t\v";
		else		 m_delimiters = delims;

		m_begin = m_string.find_first_not_of(m_delimiters);
		m_end   = m_string.find_first_of(m_delimiters, m_begin);
	}

	//------------------------------------------
	int StrTokenizer::numTokens()
	{
		if(m_numTokens >= 0)
			return static_cast<int>(m_numTokens);

		string::size_type n = 0;
		string::size_type i = 0;

		for (;;)
		{
			//find first not delimiters element.
			if ((i = m_string.find_first_not_of(m_delimiters, i)) == string::npos)
				break;

			//find first delimiter.
			i = m_string.find_first_of(m_delimiters, i);

			//increment counter
			n++;

			if (i == string::npos)
				break;
		}
		return  static_cast<int>(m_numTokens = n);
	}

	//------------------------------------------
	bool StrTokenizer::hasMoreTokens() const
	{
		return (m_begin != m_end);
	}

	//------------------------------------------
	void StrTokenizer::nextToken(string& nextToken)
	{
		if (m_begin != string::npos && m_end != string::npos)
		{
			nextToken = m_string.substr(m_begin, m_end - m_begin);
			m_begin   = m_string.find_first_not_of(m_delimiters, m_end);
			m_end     = m_string.find_first_of(m_delimiters, m_begin);
		}
		else if (m_begin != string::npos && m_end == string::npos)
		{
			nextToken = m_string.substr(m_begin, m_string.length() - m_begin);
			m_begin   = m_string.find_first_not_of(m_delimiters, m_end);
		}
	}

} // utils
} // brUGE