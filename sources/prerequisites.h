#pragma once

#include "math/math_types.h"
#include "utils/Ptr.h"

namespace brUGE
{
	using utils::RefCount;
	using utils::Ptr;
}

//-- turn off something useless warnings.
#pragma warning(disable : 4267) //-- 'var' : conversion from 'size_t' to 'type', possible loss of data
#pragma warning(disable : 4244) //-- 'conversion' conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable : 4312) //-- 'operation' : conversion from 'type1' to 'type2' of greater size
#pragma warning(disable : 4201) //-- nonstandard extension used : nameless struct/union 
#pragma warning(disable : 4996) //-- 'function': was declared deprecated

typedef int					Handle;
typedef unsigned char		uchar;
typedef uchar				byte;
typedef unsigned short		word;
typedef unsigned int		dword;
typedef unsigned int		uint;

typedef char				int8;
typedef uchar				uint8;
typedef short				int16;
typedef word				uint16;
typedef int					int32;
typedef uint				uint32;

typedef __int64				int64;
typedef unsigned __int64	uint64;

//-- constants.
const Handle CONST_INVALID_HANDLE = -1;

//-- setup logger.
#define INFO_MSG	brUGE::utils::LogManager::instance().info
#define WARNING_MSG brUGE::utils::LogManager::instance().warning
#define ERROR_MSG	brUGE::utils::LogManager::instance().error
