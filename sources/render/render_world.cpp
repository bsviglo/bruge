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

using namespace brUGE;
using namespace brUGE::utils;
using namespace brUGE::math;


//-- start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{

	bool g_showVisibilityBoxes = false;

}
//--------------------------------------------------------------------------------------------------
//-- end unnamed namespace.


namespace brUGE
{
namespace render
{
	//----------------------------------------------------------------------------------------------
	RenderWorld::RenderWorld()
	{

	}

	//----------------------------------------------------------------------------------------------
	RenderWorld::~RenderWorld()
	{

	}

	//----------------------------------------------------------------------------------------------
	bool RenderWorld::init()
	{

		REGISTER_CONSOLE_VALUE("r_showVisibilityBoxes", bool, g_showVisibilityBoxes);

		bool success = true;

		m_renderOps.reserve(2000);
		m_debugDrawer.reset(new DebugDrawer);
		m_decalManager.reset(new DecalManager);
		m_imguiRender.reset(new imguiRender);
		m_lightsManager.reset(new LightsManager);

		success &= m_debugDrawer->init();
		success &= m_decalManager->init();
		success &= m_imguiRender->init();
		success &= m_lightsManager->init();

		return success;
	}

	//----------------------------------------------------------------------------------------------
	bool RenderWorld::fini()
	{
		bool success = true;

		for (uint i = 0; i < m_meshInstances.size(); ++i)
			delete m_meshInstances[i];

		m_renderOps.clear();
		m_meshInstances.clear();

		if (m_debugDrawer)		m_debugDrawer->fini();
		if (m_decalManager)		m_decalManager->fini();
		if (m_imguiRender)		m_imguiRender->fini();
		if (m_lightsManager)	m_lightsManager->fini();

		m_debugDrawer.reset();
		m_decalManager.reset();
		m_imguiRender.reset();
		m_lightsManager.reset();

		return success;
	}

	//----------------------------------------------------------------------------------------------
	void RenderWorld::update(float dt)
	{
		m_lightsManager->update(dt);
		m_decalManager->update(dt);
		RenderSystem::instance().postProcessing()->update(dt);
	}

	//----------------------------------------------------------------------------------------------
	void RenderWorld::setCamera(const Ptr<Camera>& cam)
	{
		m_camera = cam;
	}

	//----------------------------------------------------------------------------------------------
	void RenderWorld::resolveVisibility()
	{
		//-- 1. resolve visibility
		{
			SCOPED_TIME_MEASURER_EX("resolve visibility")

			const mat4f& vpMat = m_camera->viewProjMatrix();

			for (uint i = 0; i < m_meshInstances.size(); ++i)
			{
				const MeshInstance& inst = *m_meshInstances[i];

				//-- 1. cull frustum against AABB.
				Outcode outCode = inst.m_transform->m_worldBounds.calculateOutcode(vpMat);
				if (outCode != 0)
				{
					continue;
				}
				else if (g_showVisibilityBoxes)
				{
					m_debugDrawer->drawAABB(inst.m_transform->m_worldBounds, Color(1,0,0,0));
				}

				//-- 2. gather render operations.
				if (inst.m_mesh)
				{
					uint count = inst.m_mesh->gatherRenderOps(m_renderOps);
					for (uint i = m_renderOps.size() - count; i < m_renderOps.size(); ++i)
					{
						RenderOp& rop = m_renderOps[i];
						rop.m_worldMat = &inst.m_transform->m_worldMat;
					}
				}
				else if (inst.m_skinnedMesh)
				{
					uint count = inst.m_skinnedMesh->gatherRenderOps(m_renderOps);
					for (uint i = m_renderOps.size() - count; i < m_renderOps.size(); ++i)
					{
						RenderOp& rop = m_renderOps[i];

						rop.m_worldMat			 = &inst.m_transform->m_worldMat;
						rop.m_matrixPalette		 = &inst.m_worldPalette[0];
						rop.m_matrixPaletteCount = inst.m_worldPalette.size();
					}
				}
			}
		}
		
		//-- 2. z-only pass.
		{
			SCOPED_TIME_MEASURER_EX("z-pass")
			
			rs().beginPass(RenderSystem::PASS_Z_ONLY);
			rs().setCamera(m_camera.get());
			rs().addRenderOps(m_renderOps);
			rs().endPass();
		}

		//-- 3. decal manager.
		{
			SCOPED_TIME_MEASURER_EX("decal-pass")

			RenderOps ops;
			m_decalManager->gatherRenderOps(ops);

			rs().beginPass(RenderSystem::PASS_DECAL);
			rs().setCamera(m_camera.get());
			rs().addRenderOps(ops);
			rs().endPass();
		}

		//-- 4. light pass
		{
			SCOPED_TIME_MEASURER_EX("light-pass")

			RenderOps ops;
			m_lightsManager->gatherROPs(ops);

			rs().beginPass(RenderSystem::PASS_LIGHT);
			rs().setCamera(m_camera.get());
			rs().addRenderOps(ops);
			rs().endPass();
		}
		
		//-- 5. main pass.
		{
			SCOPED_TIME_MEASURER_EX("main-pass")

			rs().beginPass(RenderSystem::PASS_MAIN_COLOR);
			rs().setCamera(m_camera.get());
			rs().addRenderOps(m_renderOps);
			rs().endPass();
		}

		//-- 6. draw post-processing.
		{
			SCOPED_TIME_MEASURER_EX("post-processing")

			rs().beginPass(RenderSystem::PASS_POST_PROCESSING);
			rs().addRenderOps(RenderOps());
			rs().postProcessing()->draw();
			rs().endPass();
		}

		//-- 7. update debug drawer.
		//-- Note: It implicitly activate passes PASS_DEBUG_WIRE and PASS_DEBUG_SOLID.
		//-- ToDo: reconsider.
		{
			SCOPED_TIME_MEASURER_EX("debug drawer")

			m_debugDrawer->draw();
		}

		//-- 8. draw imgui.
		{
			SCOPED_TIME_MEASURER_EX("imgui")

			m_imguiRender->draw();
		}

		m_renderOps.clear();
	}

	//----------------------------------------------------------------------------------------------
	Handle RenderWorld::addMeshDef(const MeshInstance::Desc& desc, Transform* transform)
	{
		MeshInstance* mInst = setupMeshInst(desc, transform);
		if (mInst)
		{
			m_meshInstances.push_back(mInst);
			return m_meshInstances.size() - 1;
		}
		
		return CONST_INVALID_HANDLE;
	}

	//----------------------------------------------------------------------------------------------
	bool RenderWorld::delMeshDef(Handle handle)
	{
		if (handle == CONST_INVALID_HANDLE || static_cast<size_t>(handle) >= m_meshInstances.size())
			return false;

		delete m_meshInstances[handle];
		m_meshInstances[handle] = NULL;

		return true;
	}

	//----------------------------------------------------------------------------------------------
	MeshInstance* RenderWorld::getMeshDef(Handle handle)
	{
		if (handle == CONST_INVALID_HANDLE || static_cast<size_t>(handle) >= m_meshInstances.size())
			return nullptr;

		return m_meshInstances[handle];
	}

	//----------------------------------------------------------------------------------------------
	MeshInstance* RenderWorld::setupMeshInst(const MeshInstance::Desc& desc, Transform* transform)
	{
		ResourcesManager& rm = ResourcesManager::instance();
		std::unique_ptr<MeshInstance> mInst(new MeshInstance);

		//-- 1. load skinned mesh.
		if (getFileExt(desc.fileName) == "md5mesh")
		{
			Ptr<SkinnedMesh> mesh = rm.loadSkinnedMesh(desc.fileName);
			if (!mesh)
			{
				return nullptr;
			}

			mInst->m_skinnedMesh = mesh;
			mInst->m_transform   = transform;

			return mInst.release();
		}
		//-- 2. load static mesh.
		else
		{
			Ptr<Mesh> mesh = rm.loadMesh(desc.fileName);
			if (!mesh)
			{
				return nullptr;
			}

			mInst->m_mesh		= mesh;
			mInst->m_transform	= transform;

			return mInst.release();
		}
	}

} //-- render
} //-- brUGE