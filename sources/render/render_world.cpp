#include "render_world.hpp"
#include "game_world.hpp"
#include "Camera.h"
#include "DebugDrawer.h"
#include "loader/ResourcesManager.h"
#include "utils/string_utils.h"

using namespace brUGE;
using namespace brUGE::utils;

namespace brUGE
{
namespace render
{

	//----------------------------------------------------------------------------------------------
	bool RenderWorld::init()
	{
		bool success = true;

		m_renderOps.reserve(2000);
		m_debugDrawer.reset(new DebugDrawer);
		success &= m_debugDrawer->init();

		return success;
	}

	//----------------------------------------------------------------------------------------------
	bool RenderWorld::fini()
	{
		bool success = true;

		for (uint i = 0; i < m_lightInstances.size(); ++i)
			delete m_lightInstances[i];

		for (uint i = 0; i < m_meshInstances.size(); ++i)
			delete m_meshInstances[i];

		m_renderOps.clear();
		m_lightInstances.clear();
		m_meshInstances.clear();

		if (m_debugDrawer)
			m_debugDrawer->destroy();

		return success;
	}

	//----------------------------------------------------------------------------------------------
	void RenderWorld::setCamera(const Ptr<Camera>& cam)
	{
		m_camera = cam;
	}

	//----------------------------------------------------------------------------------------------
	void RenderWorld::resolveVisibility()
	{
		RenderSystem& rs = RenderSystem::instance();

		for (uint i = 0; i < m_meshInstances.size(); ++i)
		{
			const MeshInstance& inst = *m_meshInstances[i];

			//-- 1. cull frustum against AABB.

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

		//-- 1. z-only pass.
		rs.beginPass(RenderSystem::PASS_Z_ONLY);
		rs.setCamera(m_camera.get());
		rs.addRenderOps(m_renderOps);
		rs.endPass();

		//-- 2. main pass.
		rs.beginPass(RenderSystem::PASS_MAIN_COLOR);
		rs.setCamera(m_camera.get());
		rs.addRenderOps(m_renderOps);
		rs.endPass();

		m_renderOps.clear();
	}

	//----------------------------------------------------------------------------------------------
	Handle RenderWorld::addLightDef(const LightInstance::Desc& desc)
	{
		LightInstance* lInst = new LightInstance();
		lInst->m_data = desc;

		m_lightInstances.push_back(lInst);
		return m_lightInstances.size() - 1;
	}

	//----------------------------------------------------------------------------------------------
	bool RenderWorld::delLightDef(Handle handle)
	{
		if (handle == CONST_INVALID_HANDLE || static_cast<size_t>(handle) >= m_lightInstances.size())
			return false;

		delete m_lightInstances[handle];
		m_lightInstances[handle] = NULL;

		return true;
	}

	//----------------------------------------------------------------------------------------------
	const LightInstance* RenderWorld::getLightDef(Handle handle)
	{
		if (handle == CONST_INVALID_HANDLE || static_cast<size_t>(handle) >= m_lightInstances.size())
			return nullptr;

		return m_lightInstances[handle];
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
	const MeshInstance* RenderWorld::getMeshDef(Handle handle)
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