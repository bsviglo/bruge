#include "render_world.hpp"
#include "game_world.hpp"
#include "Camera.h"
#include "loader/ResourcesManager.h"
#include "utils/string_utils.h"
#include "gui/imgui_render.hpp"
#include "console/Console.h"
#include "console/TimingPanel.h"
#include "DebugDrawer.h"
#include "gui/imgui_render.hpp"
#include "decal_manager.hpp"
#include "light_manager.hpp"
#include "mesh_manager.hpp"
#include "shadow_manager.hpp"
#include "post_processing.hpp"
#include "SkyBox.hpp"
#include "terrain_system.hpp"

using namespace brUGE;
using namespace brUGE::utils;
using namespace brUGE::math;


namespace brUGE
{
namespace render
{
	//----------------------------------------------------------------------------------------------
	RenderWorld::RenderWorld()
		:	m_debugDrawer(new DebugDrawer),
			m_decalManager(new DecalManager),
			m_imguiRender(new imguiRender),
			m_lightsManager(new LightsManager),
			m_meshManager(new MeshManager),
			m_shadowManager(new ShadowManager),
			m_postProcessing(new PostProcessing),
			m_skyBox(new SkyBox),
			m_terrainSystem(new TerrainSystem)
	{

	}

	//----------------------------------------------------------------------------------------------
	RenderWorld::~RenderWorld()
	{

	}

	//----------------------------------------------------------------------------------------------
	bool RenderWorld::init()
	{
		bool success = true;
		success &= m_debugDrawer->init();
		success &= m_decalManager->init();
		success &= m_imguiRender->init();
		success &= m_lightsManager->init();
		success &= m_meshManager->init();
		success &= m_skyBox->init();
		success &= m_terrainSystem->init();
		
		//-- ToDo: for now shadow manager initialization depends of post-processing framework. Fix this.
		success &= m_postProcessing->init();
		success &= m_shadowManager->init();
		return success;
	}

	//----------------------------------------------------------------------------------------------
	void RenderWorld::update(float dt)
	{
		m_meshManager->update(dt);
		m_lightsManager->update(dt);
		m_decalManager->update(dt);
		m_shadowManager->update(dt);
		m_postProcessing->update(dt);
	}

	//----------------------------------------------------------------------------------------------
	void RenderWorld::setCamera(const Ptr<Camera>& cam)
	{
		m_camera = cam;
	}

	//----------------------------------------------------------------------------------------------
	void RenderWorld::draw()
	{
		//-- 1. z-only pass.
		{
			SCOPED_TIME_MEASURER_EX("z-pass")

			//-- gather all ROPs.
			RenderOps ops;
			{
				SCOPED_TIME_MEASURER_EX("resolve visibility")
				{
					SCOPED_TIME_MEASURER_EX("meshes")
					m_meshManager->gatherROPs(
						RenderSystem::PASS_Z_ONLY, false, ops, m_camera->renderCam().m_viewProj
						);
				}
				{
					SCOPED_TIME_MEASURER_EX("terrain")
					m_terrainSystem->gatherROPs(
						RenderSystem::PASS_Z_ONLY, ops, m_camera->renderCam().m_viewProj,
						m_camera->renderCam().m_invView.applyToOrigin()
						);
				}
			}
			
			rs().beginPass(RenderSystem::PASS_Z_ONLY);
			rs().setCamera(&m_camera->renderCam());
			rs().shaderContext().updatePerFrameViewConstants();
			rs().addROPs(ops);
			rs().endPass();
		}

		//-- 2. decal manager.
		{
			SCOPED_TIME_MEASURER_EX("decal-pass")

			RenderOps ops;
			m_decalManager->gatherRenderOps(ops);

			rs().beginPass(RenderSystem::PASS_DECAL);
			rs().setCamera(&m_camera->renderCam());
			rs().shaderContext().updatePerFrameViewConstants();
			rs().addROPs(ops);
			rs().endPass();
		}

		//-- 3. light pass
		{
			SCOPED_TIME_MEASURER_EX("light-pass")

			RenderOps ops;
			m_lightsManager->gatherROPs(ops);

			rs().beginPass(RenderSystem::PASS_LIGHT);
			rs().setCamera(&m_camera->renderCam());
			rs().shaderContext().updatePerFrameViewConstants();
			rs().addROPs(ops);
			rs().endPass();
		}

		//-- 4. cast shadows
		{
			SCOPED_TIME_MEASURER_EX("cast shadows")

			m_shadowManager->castShadows(m_camera->renderCam(), *m_lightsManager.get(), *m_meshManager.get());
		}

		//-- 5. resolve shadows.
		{
			SCOPED_TIME_MEASURER_EX("resolve shadows")


			m_shadowManager->receiveShadows(&m_camera->renderCam());
		}
		
		//-- 6. main pass.
		{
			SCOPED_TIME_MEASURER_EX("main-pass")

			//-- gather all ROPs.
			RenderOps ops;
			{
				SCOPED_TIME_MEASURER_EX("resolve visibility")
				{
					SCOPED_TIME_MEASURER_EX("meshes")
						m_meshManager->gatherROPs(
						RenderSystem::PASS_MAIN_COLOR, false, ops, m_camera->renderCam().m_viewProj
						);
				}
				{
					SCOPED_TIME_MEASURER_EX("terrain")
						m_terrainSystem->gatherROPs(
						RenderSystem::PASS_MAIN_COLOR, ops, m_camera->renderCam().m_viewProj,
						m_camera->renderCam().m_invView.applyToOrigin()
						);
				}
			}

			rs().beginPass(RenderSystem::PASS_MAIN_COLOR);
			rs().setCamera(&m_camera->renderCam());
			rs().shaderContext().updatePerFrameViewConstants();
			rs().addROPs(ops);
			rs().endPass();
		}

		//-- 7. sky
		{
			SCOPED_TIME_MEASURER_EX("sky")
			
			RenderOps ops;
			m_skyBox->gatherROPs(ops);
			rs().beginPass(RenderSystem::PASS_SKY);
			rs().setCamera(&m_camera->renderCam());
			rs().shaderContext().updatePerFrameViewConstants();
			rs().addROPs(ops);
			rs().endPass();
		}

		//-- 8. draw post-processing.
		{
			SCOPED_TIME_MEASURER_EX("post-processing")

			rs().beginPass(RenderSystem::PASS_POST_PROCESSING);
			rs().addROPs(RenderOps());
			rs().setCamera(&m_camera->renderCam());
			rs().shaderContext().updatePerFrameViewConstants();
			m_postProcessing->draw();
			rs().endPass();
		}

		//-- 9. update debug drawer.
		//-- Note: It implicitly activate passes PASS_DEBUG_WIRE and PASS_DEBUG_SOLID.
		//-- ToDo: reconsider.
		{
			SCOPED_TIME_MEASURER_EX("debug drawer")

			m_debugDrawer->draw();
		}

		//-- 10. draw imgui.
		{
			SCOPED_TIME_MEASURER_EX("imgui")

			m_imguiRender->draw();
		}
	}

} //-- render
} //-- brUGE