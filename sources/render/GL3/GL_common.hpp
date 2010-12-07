#pragma once

//-- Note: We use direct state access DAC extension, and we hope that this extension will be
//--	   approved by ARB in the next OpenGL specification. Perhaps it will be OpenGL 3.4 or 4.2

// use static linked GLEW.
#define GLEW_STATIC

#include "render\render_common.h"

// http://glew.sourceforge.net/
#include "glew\glew.h"
#include "glew\wglew.h"

namespace brUGE
{
namespace render
{
	//-- some function to handle wrong behavior of the OpenGL API.
	void		glClearErrors();
	const char* glErrorString(GLenum error);
	bool		glHasErrors();
	
	//-- some predeclarations.
	class GLVertexLayout;
	class GLVertexBuffer;
	class GLIndexBuffer;
	class GLCgShadingProgram;
	class GLCgSamplerUniform;
	class GLSwapChain;
	class GLSamplerState;
	class GLRasterizerState;
	class GLDepthStencilState;
	class GLBlendState;
	class GLTexture;
	class GLFrameBuffer;

} //-- render
} //-- brUGE

#ifdef _DEBUG

#define GL_CLEAR_ERRORS glClearErrors();

#define GL_LOG_ERRORS														\
	{																		\
		GLenum error = glGetError();										\
		if (error != GL_NO_ERROR)											\
			ERROR_MSG("OpenGL error ocurred '%s'", glErrorString(error));	\
	}

#else // _DEBUG		

#define GL_CLEAR_ERRORS {}
#define GL_THROW_ERRORS {}
#define GL_LOG_ERRORS	{}

#endif
