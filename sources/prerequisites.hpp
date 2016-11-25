#pragma once

#include "math/math_types.hpp"
#include "utils/Ptr.h"
#include "utils/NonCopyable.hpp"

namespace brUGE
{
	using utils::RefCount;
	using utils::Ptr;
	using utils::NonCopyable;
}

//-- turn off something useless warnings.
#pragma warning(disable : 4267) //-- 'var' : conversion from 'size_t' to 'type', possible loss of data
#pragma warning(disable : 4244) //-- 'conversion' conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable : 4312) //-- 'operation' : conversion from 'type1' to 'type2' of greater size
#pragma warning(disable : 4201) //-- nonstandard extension used : nameless struct/union 
#pragma warning(disable : 4996) //-- 'function': was declared deprecated
#pragma warning(disable : 4800) //-- forcing value to bool 'true' or 'false' (performance warning)s

//-- ToDo: reconsider it
//-- https://connect.microsoft.com/VisualStudio/feedback/details/1355600/c4458-c4459-declaration-hides-other-declaration-are-overeager-to-the-point-of-being-completely-useless
#pragma warning(disable : 4456)
#pragma warning(disable : 4457)
#pragma warning(disable : 4458)
#pragma warning(disable : 4459)

//-- ToDo: reconsider it. Bullet library integration
#pragma warning(disable : 4359) //-- warning C4359: 'btContactConstraint': Alignment specifier is less than actual alignment (128), and will be ignored.
#pragma warning(disable : 4316) //-- warning C4316: 'btCollisionDispatcher': object allocated on the heap may not be aligned 16
#pragma warning(disable : 4305) //-- warning C4305: 'argument': truncation from 'double' to 'const btScalar'

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
