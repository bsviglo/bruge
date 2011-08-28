#include "SkyBox.hpp"
#include "Mesh.hpp"
#include "loader/ResourcesManager.h"

namespace brUGE
{
namespace render
{

	//----------------------------------------------------------------------------------------------
	SkyBox::SkyBox()
	{
	}

	//----------------------------------------------------------------------------------------------
	SkyBox::~SkyBox()
	{

	}

	//----------------------------------------------------------------------------------------------
	bool SkyBox::init()
	{
		//-- load sky box mesh
		{
			m_skybox = ResourcesManager::instance().loadMesh("models/sky_box", true);

			if (!m_skybox.isValid())
			{
				ERROR_MSG("Can't load system sky box mesh.");
				return false;
			}
		}
		return true;
	}

	//----------------------------------------------------------------------------------------------
	uint SkyBox::gatherROPs(RenderOps& rops) const
	{
		return m_skybox->gatherROPs(RenderSystem::PASS_MAIN_COLOR, false, rops);
	}
	
} //-- render
} //-- brUGE