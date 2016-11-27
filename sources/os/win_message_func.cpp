#include "engine/Engine.h"
#include "render/render_system.hpp"
#include "../win32Res/resource.h"

using namespace brUGE;
using namespace brUGE::render;

bool g_needToStartApp	   = true;
ERenderAPIType g_renderAPI = RENDER_API_D3D11;

// start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{

}
//--------------------------------------------------------------------------------------------------
// end unnamed namespace.


namespace brUGE
{
namespace os
{

} // os
} // brUGE
