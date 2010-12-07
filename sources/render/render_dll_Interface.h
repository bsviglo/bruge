#pragma once

namespace brUGE
{
namespace utils
{
	class LogManager;
}

namespace render
{
	// interface to dll.
	struct RenderDllInterface
	{
		class IRenderDevice* device;
	};

	typedef void (*createRender) (RenderDllInterface* rcc, utils::LogManager* logger);
	typedef void (*destroyRender)();

} // render
} // brUGE
