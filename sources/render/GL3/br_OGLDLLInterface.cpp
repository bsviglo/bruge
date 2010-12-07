#include "render/br_RenderContextCommon.h"
#include "utils/br_LogManager.h"

#include "br_OGLRenderDevice.h"
#include "br_GLBufferManager.h"
#include "br_OGLCgShaderManager.h"
#include "br_GLStateObjectsManager.h"


using namespace brUGE::render;
using namespace brUGE::utils;

static brRenderContextCommon exports_;
static brLogManager ShaderLog = brLogManager("RenderLog.html");


extern "C" void createRender(brRenderContextCommon* rcc)
{
	exports_.device			= new brOGLRenderDevice;
	exports_.bufferManager	= new brGLBufferManager;
	exports_.shaderManager	= new brOGLCgShaderManager;
	exports_.stateManager	= new brGLStateObjectsManager;
	
	*rcc = exports_;
}

extern "C" void destroyRender()
{
	// нечего уничтожать. Еще ничего не было создано.
	assert(exports_.device != NULL);
	delete exports_.device;
	exports_.device = NULL;
	
	assert(exports_.bufferManager != NULL);
	delete exports_.bufferManager;
	exports_.bufferManager = NULL;

	assert(exports_.shaderManager != NULL);
	delete exports_.shaderManager;
	exports_.shaderManager = NULL;

	assert(exports_.stateManager != NULL);
	delete exports_.stateManager;
	exports_.stateManager = NULL;
	
}
