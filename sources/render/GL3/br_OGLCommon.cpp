#include "br_OGLCommon.h"

namespace brUGE
{
namespace render
{
	void glClearErrors()
	{
		while (glGetError() != GL_NO_ERROR) {}
	}
}
}
