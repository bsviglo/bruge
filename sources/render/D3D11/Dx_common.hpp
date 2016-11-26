#pragma once

#include "render\render_common.h"

#ifdef _DEBUG
	//-- include visual memory leak detector.
	//#include <vld.h>
#endif

#undef NOMINMAX
#define NOMINMAX // required to stop windows.h screwing up std::min definition
#include <d3d11.h>
#include <d3d11shader.h>

#include "utils/ComPtr.h"

namespace brUGE
{
	using utils::ComPtr;

namespace render
{
	class DXDevice;
	class DXBuffer;
	class DXShader;
	class DXTexture;
	class DXRenderDevice;
	// ...

} // render
} // brUGE

// reassign logger in this dll.
extern brUGE::utils::LogManager* g_logger;

#undef INFO_MSG
#undef WARNING_MSG
#undef ERROR_MSG

#define INFO_MSG	g_logger->info
#define WARNING_MSG g_logger->warning
#define ERROR_MSG	g_logger->error

//-- compile-time configurations flags.
#define USE_FORCE_DEBUG_MODE 0
