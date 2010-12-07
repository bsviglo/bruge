#include "GL_Common.hpp"

// start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	//-- #define GL_INVALID_ENUM 0x0500
	//-- #define GL_INVALID_VALUE 0x0501
	//-- #define GL_INVALID_OPERATION 0x0502
	//-- #define GL_STACK_OVERFLOW 0x0503
	//-- #define GL_STACK_UNDERFLOW 0x0504
	//-- #define GL_OUT_OF_MEMORY 0x0505
	//-- #define GL_INVALID_FRAMEBUFFER_OPERATION 0x0506
	const char * const glErrors[] =
	{
		"enum argument out of range",
		"Numeric argument out of range",
		"Operation illegal in current state",
		"Stack is overflowed",
		"Stack is underflowed",
		"Not enough memory left to execute command",
		"Framebuffer object is not complete"
	};

}
//--------------------------------------------------------------------------------------------------
// end unnamed namespace.


namespace brUGE
{
namespace render
{
	
	//----------------------------------------------------------------------------------------------
	void glClearErrors()
	{
		while (glGetError() != GL_NO_ERROR) {}
	}
	
	//----------------------------------------------------------------------------------------------
	const char* glErrorString(GLenum error)
	{
		assert(error >= 0x0500 && error <= 0x0506);
		return glErrors[error - 0x0500];
	}

	//----------------------------------------------------------------------------------------------
	bool glHasErrors()
	{

	}

} // render
} // brUGE
