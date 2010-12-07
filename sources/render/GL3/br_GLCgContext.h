#ifndef _BR_GLCGCONTEXT_H_
#define _BR_GLCGCONTEXT_H_

#include "br_GLCommon.h"

// set the error callback function 
#define BR_CG_ERROR_CALLBACK_FUNC 0

namespace brUGE
{
namespace render
{
	class brGLCgContext
	{
	public:
		static void addRef();
		static void release();
		static CGcontext& getCgContext();
		static const char* getLastListing();

#if BR_CG_ERROR_CALLBACK_FUNC			
	private:
		static void _cgErrorCallback();
#endif

	private:
		static CGcontext	m_context;
		static uint			m_refCount;
	};
}
}

#endif /*_BR_GLCGCONTEXT_H_*/