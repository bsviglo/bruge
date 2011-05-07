#include "mesh_manager.hpp"
#include "mesh_collector.hpp"
#include "game_world.hpp"
#include "DebugDrawer.h"
#include "utils/string_utils.h"
#include "console/Console.h"
#include "loader/ResourcesManager.h"

using namespace brUGE::utils;
using namespace brUGE::math;

//-- start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	//-- console variables.
	bool g_enableCulling = true;
	bool g_showVisibilityBoxes = false;
	bool g_enableInstancing = true;

	//-- converts one transformation presentation (quat, vec3f) to another mat4f.
	//----------------------------------------------------------------------------------------------
	inline mat4f quatToMat4x4(const quat& q, const vec3f& pos)
	{
		mat4f out;

		//-- 1. rotation part.
		out = q.toMat4();
		out.transpose();

		//-- 2. translation part.
		out.m30 = pos.x; out.m31 = pos.y; out.m32 = pos.z;

		return out;
	}

}
//--------------------------------------------------------------------------------------------------
//-- end unnamed namespace.

namespace brUGE
{
namespace render
{

	//----------------------------------------------------------------------------------------------
	MeshManager::MeshManager() : m_meshCollector(new MeshCollector)
	{

	}

	//----------------------------------------------------------------------------------------------
	MeshManager::~MeshManager()
	{
		for (uint i = 0; i < m_meshInstances.size(); ++i)
			delete m_meshInstances[i];

		m_meshInstances.clear();
	}

	//----------------------------------------------------------------------------------------------
	bool MeshManager::init()
	{
		REGISTER_CONSOLE_VALUE("r_showVisibilityBoxes",		bool, g_showVisibilityBoxes);
		REGISTER_CONSOLE_VALUE("r_enableVisibilityCulling",	bool, g_enableCulling);
		REGISTER_CONSOLE_VALUE("r_enableInstancing",		bool, g_enableInstancing);

		return m_meshCollector->init();
	}

	//----------------------------------------------------------------------------------------------
	void MeshManager::update(float /*dt*/)
	{

	}

	//----------------------------------------------------------------------------------------------
	uint MeshManager::gatherROPs(
		RenderSystem::EPassType pass, bool instanced, RenderOps& rops,
		const mat4f& viewPort, AABB* aabb)
	{
		m_meshCollector->begin(pass);

		for (uint i = 0; i < m_meshInstances.size(); ++i)
		{
			const MeshInstance& inst = *m_meshInstances[i];

			//-- 1. cull frustum against AABB.
			if (g_enableCulling && inst.m_transform->m_worldBounds.calculateOutcode(viewPort) != 0)
			{
				continue;
			}
			else if (g_showVisibilityBoxes)
			{
				DebugDrawer::instance().drawAABB(inst.m_transform->m_worldBounds, Color(1,0,0,0));
			}
			
			if (aabb)
			{
				aabb->combine(inst.m_transform->m_worldBounds);
			}

			//-- 2. gather render operations.
			if (inst.m_mesh)
			{
				//-- if mesh collector doesn't want to get this instance then process it as usual.
				if (!g_enableInstancing || (g_enableInstancing && !m_meshCollector->addMeshInstance(inst)))
				{
					uint count = inst.m_mesh->gatherROPs(pass, instanced, rops);
					for (uint i = rops.size() - count; i < rops.size(); ++i)
					{
						RenderOp& rop = rops[i];
						rop.m_worldMat = &inst.m_transform->m_worldMat;
					}
				}
			}
			else if (inst.m_skinnedMesh)
			{
				uint count = inst.m_skinnedMesh->gatherROPs(pass, instanced, rops);
				for (uint i = rops.size() - count; i < rops.size(); ++i)
				{
					RenderOp& rop = rops[i];

					rop.m_worldMat			 = &inst.m_transform->m_worldMat;
					rop.m_matrixPalette		 = &inst.m_worldPalette[0];
					rop.m_matrixPaletteCount = inst.m_worldPalette.size();
				}
			}
		}

		m_meshCollector->gatherROPs(rops);
		m_meshCollector->end();

		return rops.size();
	}

	//----------------------------------------------------------------------------------------------
	Handle MeshManager::addMesh(const MeshInstance::Desc& desc, Transform* transform)
	{
		ResourcesManager& rm = ResourcesManager::instance();
		std::unique_ptr<MeshInstance> mInst(new MeshInstance);

		//-- 1. load skinned mesh.
		if (getFileExt(desc.fileName) == "md5mesh")
		{
			Ptr<SkinnedMesh> mesh = rm.loadSkinnedMesh(desc.fileName);
			if (!mesh)
			{
				return CONST_INVALID_HANDLE;
			}

			mInst->m_skinnedMesh = mesh;
			mInst->m_transform   = transform;

			transform->m_localBounds = mesh->bounds();
			transform->m_worldBounds = mesh->bounds().getTranformed(transform->m_worldMat);
		}
		//-- 2. load static mesh.
		else
		{
			Ptr<Mesh> mesh = rm.loadMesh(desc.fileName);
			if (!mesh)
			{
				return CONST_INVALID_HANDLE;
			}

			mInst->m_mesh	   = mesh;
			mInst->m_transform = transform;

			transform->m_localBounds = mesh->bounds();
			transform->m_worldBounds = mesh->bounds().getTranformed(transform->m_worldMat);
		}

		//-- 3. setup nodes bucket in case if mesh is skinned.
		if (SkinnedMesh* skMesh = mInst->m_skinnedMesh.get())
		{
			//-- 3.1. resize mesh world palette to match the bones count in the skinned mesh.
			mInst->m_worldPalette.resize(skMesh->skeleton().size());

			//-- 3.2. initialize nodes. 
			for (uint i = 0; i < mInst->m_worldPalette.size(); ++i)
			{
				const Joint& joint   = skMesh->skeleton()[i];
				mat4f&		 nodeMat = mInst->m_worldPalette[i];

				//-- set default initial value.
				nodeMat = quatToMat4x4(joint.m_transform.orient, joint.m_transform.pos);
				transform->m_nodes.push_back(new Node(joint.m_name.c_str(), nodeMat));
			}
		}

		m_meshInstances.push_back(mInst.release());
		return m_meshInstances.size() - 1;
	}
	
	//----------------------------------------------------------------------------------------------
	void MeshManager::delMesh(Handle handle)
	{
		delete m_meshInstances[handle];
		m_meshInstances[handle] = NULL;
	}

	//----------------------------------------------------------------------------------------------
	MeshInstance& MeshManager::getMesh(Handle handle)
	{
		return *m_meshInstances[handle];
	}

} //-- render
} //-- brUGE