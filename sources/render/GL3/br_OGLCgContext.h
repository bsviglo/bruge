#ifndef _BR_OGLCGCONTEXT_H_
#define _BR_OGLCGCONTEXT_H_

#include "br_OGLCommon.h"

namespace brUGE
{
namespace render
{
	class brOGLCgContext
	{
	public:
		static void addRef();
		static void release();
		static CGcontext& getCgContext();
		static const char* getLastListing();

	private:
		static CGcontext context_;
		static uint	refCount_;
	};
}
}

#endif /*_BR_OGLCGCONTEXT_H_*/