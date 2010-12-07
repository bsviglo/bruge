#include "render/render_dll_interface.h"
#include "DXRenderDevice.hpp"
#include "utils/LogManager.h"

using namespace brUGE::render;
using namespace brUGE::utils;

// logger from main program.
LogManager* g_logger = NULL;

namespace
{
	RenderDllInterface g_exports;
}

extern "C" void createRender(RenderDllInterface* rdi, LogManager* logger)
{
	DXRenderDevice* rd = new DXRenderDevice;
	g_exports.device = rd;
	*rdi = g_exports;

	g_logger = logger;
}

extern "C" void destroyRender()
{
	assert(g_exports.device != NULL);
	delete static_cast<DXRenderDevice*>(g_exports.device);
	g_exports.device = NULL;

	g_logger = NULL;
}