#include "br_GLCgContext.h"

namespace brUGE
{
namespace render
{
	CGcontext brGLCgContext::m_context = 0;
	uint brGLCgContext::m_refCount = 0;

	//------------------------------------------
	void brGLCgContext::addRef()
	{
		if (m_refCount == 0)
		{
			m_context = cgCreateContext();
			if (!m_context)
				BR_EXCEPT("brOGLCgContext::addRef", "Failed to create Cg context.");
			
			// очень сильно понижает быстродействие.
			cgGLSetDebugMode(CG_FALSE);

			cgSetAutoCompile(m_context, CG_COMPILE_MANUAL);
			cgGLSetManageTextureParameters(m_context, CG_FALSE);

#if BR_CG_ERROR_CALLBACK_FUNC
			cgSetErrorCallback(_cgErrorCallback);
#endif

		}
		++m_refCount;
	}
	
	//------------------------------------------
	void brGLCgContext::release()
	{
		if (m_refCount == 0)
			return;

		if (--m_refCount == 0)
		{
			cgDestroyContext(m_context);
			if (cgGetError() != CG_NO_ERROR)
				BR_EXCEPT("brOGLCgContext::release", "Failed to destroy Cg context.");
		}
	}

	//------------------------------------------
	CGcontext& brGLCgContext::getCgContext()
	{
		return m_context;
	}

	const char* brGLCgContext::getLastListing()
	{
		return cgGetLastListing(m_context);
	}
	
	//------------------------------------------
#if BR_CG_ERROR_CALLBACK_FUNC
	void brGLCgContext::_cgErrorCallback()
	{
		CGerror error = cgGetError();
		FATAL_F("Cg error : %s.", cgGetErrorString(error));
	}
#endif

} // render
} // brUGE