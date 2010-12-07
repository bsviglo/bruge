#ifndef _BR_OGLCOMMON_H_
#define _BR_OGLCOMMON_H_

// используем статически прилинкованный GLEW.
#define GLEW_STATIC

#include "render\br_RenderCommon.h"

// http://glew.sourceforge.net/
#include "glew\glew.h"
#include "glew\wglew.h"

// http://developer.nvidia.com/object/cg_toolkit.html
#include "Cg\cg.h"
#include "Cg\cgGL.h"

namespace brUGE
{
namespace render
{
	// производит очистку стека ошибок.
	void glClearErrors();
	
	// предекларация.
	class brOGLVertexLayout;
	class brGLVertexBuffer;
	class brGLIndexBuffer;
	class brGLCgShadingProgram;
	class brOGLSwapChain;
}
}

#endif /*_BR_OGLCOMMON_H_*/