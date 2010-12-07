#include "br_OGLCgContext.h"

namespace brUGE
{
namespace render
{
	CGcontext brOGLCgContext::context_ = 0;
	uint brOGLCgContext::refCount_ = 0;

	//------------------------------------------
	void brOGLCgContext::addRef()
	{
		if (refCount_ == 0)
		{
			context_ = cgCreateContext();
			if (!context_)
				throw brException("brOGLCgContext::addRef", "Failed to create Cg context.");

#ifdef _DEBUG
			cgGLSetDebugMode(CG_TRUE);
#else
			cgGLSetDebugMode(CG_FALSE);
#endif

			cgSetAutoCompile(context_, CG_COMPILE_MANUAL);
		}
		++refCount_;
	}
	
	//------------------------------------------
	void brOGLCgContext::release()
	{
		if (refCount_ == 0)
			return;

		if (--refCount_ == 0)
		{
			cgDestroyContext(context_);
			CGerror error = cgGetError();
			if (error != CG_NO_ERROR)
				throw brException("brOGLCgContext::release", "Failed to destroy Cg context.");
		}
	}

	//------------------------------------------
	CGcontext& brOGLCgContext::getCgContext()
	{
		return context_;
	}

	const char* brOGLCgContext::getLastListing()
	{
		return cgGetLastListing(context_);
	}

}
}