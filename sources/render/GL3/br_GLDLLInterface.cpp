#include "render/br_RenderContextCommon.h"
#include "utils/br_LogManager.h"

#include "br_GLRenderDevice.h"

using namespace brUGE::render;
using namespace brUGE::utils;

static brRenderDllInterface exports_;
static LogManager ShaderLog = LogManager("RenderLog.html");


extern "C" void createRender(brRenderDllInterface* rcc)
{
	exports_.device			= new brGLRenderDevice;
	*rcc = exports_;
}

extern "C" void destroyRender()
{
	// нечего уничтожать. Еще ничего не было создано.
	assert(exports_.device != NULL);
	delete exports_.device;
	exports_.device = NULL;
}
