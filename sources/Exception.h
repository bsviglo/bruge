#pragma once

#include "utils/string_utils.h"
#include <stdarg.h>

namespace brUGE
{
	
	//----------------------------------------------------------------------------------------------
	class Exception
	{
	public:
		Exception(const std::string& header, const std::string& desc)
		{
			m_message = "[" + header + "]: " + desc;
		}

		Exception(const char* header, const char* format, ...)
		{
			va_list		args;
			char		buffer[1024];
			va_start(args, format);
			_vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);

			m_message = "[" + std::string(header) + "]: " + std::string(buffer);

			va_end(args);
		}

		~Exception() { }

		const std::string& getMessage() {	return m_message; }
		
	private:
		std::string m_message;
	};

} // brUGE

#define BR_EXCEPT(desc)				throw Exception(utils::makeStr("%s:%d %s", __FILE__, __LINE__, __FUNCTION__).c_str(), desc)
#define BR_EXCEPT_F(format, ...)	throw Exception(utils::makeStr("%s:%d %s", __FILE__, __LINE__, __FUNCTION__).c_str(), format, ## __VA_ARGS__)

