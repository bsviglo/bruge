#include "mesh_manager.hpp"
#include "render_system.hpp"
#include "mesh_collector.hpp"
#include "scene/game_world.hpp"
#include "DebugDrawer.h"
#include "utils/string_utils.h"
#include "loader/ResourcesManager.h"
#include "rttr/registration"

using namespace brUGE::utils;
using namespace brUGE::math;

//----------------------------------------------------------------------------------------------------------------------
namespace
{
	//-- console variables.
	bool g_enableCulling = true;
	bool g_showVisibilityBoxes = false;
	bool g_enableInstancing = true;
}

//----------------------------------------------------------------------------------------------------------------------
namespace brUGE
{

	using namespace rttr;
	using namespace render;

	RTTR_REGISTRATION
	{
		registration::class_<StaticMeshComponent>("render::StaticMeshComponent")
			.constructor<>()
			.property_readonly("typeID", StaticMeshComponent::typeID)
			.property_readonly("systemTypeID", MeshSystem::typeID);
			
		registration::class_<SkinnedMeshComponent>("render::SkinnedMeshComponent")
			.constructor<>()
			.property_readonly("typeID", SkinnedMeshComponent::typeID)
			.property_readonly("systemTypeID", MeshSystem::typeID);

		registration::class_<MeshSystem>("render::MeshSystem")
			.constructor<>()
			.property_readonly("typeID", MeshSystem::typeID);
	}

//----------------------------------------------------------------------------------------------------------------------
namespace render
{
	const IComponent::TypeID StaticMeshComponent::m_typeID;
	const IComponent::TypeID SkinnedMeshComponent::m_typeID;

	const ISystem::TypeID MeshSystem::m_typeID;

	//------------------------------------------------------------------------------------------------------------------
	MeshSystem::MeshSystem()
	{
	}

	//------------------------------------------------------------------------------------------------------------------
	Handle MeshSystem::World::createComponent(Handle gameObj, IComponent::TypeID typeID)
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	Handle MeshSystem::World::createComponent(Handle gameObj, IComponent::TypeID typeID, const pugi::xml_node& cfg)
	{
		auto& universeWorld = Engine::universe().world(m_universeWorld);

		universeWorld.world(ResourceSystem::typeID());
		universeWorld.world(TransformSystem::typeID());



		//-- ToDo:
		ResourcesManager& rm = ResourcesManager::instance();
		auto mInst = std::make_unique<MeshInstance>();

		if (typeID == StaticMeshComponent::typeID())
		{
			auto resourceName = std::string(cfg.find_child("resource").text().as_string());

			auto mesh = rm.loadMesh(resourceName.c_str());
			if (!mesh)
			{
				return CONST_INVALID_HANDLE;
			}

			mInst->m_mesh	   = mesh;
			mInst->m_transform = transform;

			transform->m_localBounds = mesh->bounds();
			transform->m_worldBounds = mesh->bounds().getTranformed(transform->m_worldMat);
		}
		else if (typeID == SkinnedMeshComponent::typeID())
		{

		}
		else
		{
			assert(false && "Invalid TypeID of the component");
		}
	}

	//------------------------------------------------------------------------------------------------------------------
	bool MeshSystem::init()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------------------------
	void MeshSystem::update(Handle world, const DeltaTime& dt) const
	{
		//-- update world.
	}

	//------------------------------------------------------------------------------------------------------------------
	void MeshSystem::process(Handle contextHandle) const
	{
		auto& c = *context<Context>(contextHandle);

		for (auto instHandle : c->m_visibilitySet.m_buckets[MeshSystem::typeID()])
		{
			auto inst = c.world().m_meshInstances[instHandle].get();

			//-- 2. gather render operations.
			if (inst->m_mesh)
			{
				//-- if mesh collector doesn't want to get this instance then process it as usual.
				if (!g_enableInstancing || (g_enableInstancing && !c.m_meshCollector->addMeshInstance(*inst.get())))
				{
					uint count = inst->m_mesh->gatherROPs(pass, instanced, c.m_rops);
					for (uint i = c.m_rops.size() - count; i < c.m_rops.size(); ++i)
					{
						c.m_rops[i].m_worldMat = &inst->m_transform->m_worldMat;
					}
				}
			}
			else if (inst->m_skinnedMesh)
			{
				uint count = inst->m_skinnedMesh->gatherROPs(pass, instanced, c.m_rops);
				for (uint i = c.m_rops.size() - count; i < c.m_rops.size(); ++i)
				{
					auto& rop = c.m_rops[i];

					rop.m_worldMat = &inst->m_transform->m_worldMat;
					rop.m_matrixPalette = &inst->m_worldPalette[0];
					rop.m_matrixPaletteCount = inst->m_worldPalette.size();
				}
			}
		}

		c.m_meshCollector->gatherROPs(c.m_rops);
		c.m_meshCollector->end();
	}

	//------------------------------------------------------------------------------------------------------------------



	//-- ToDo: legacy


	//----------------------------------------------------------------------------------------------
	MeshManager::MeshManager() : m_meshCollector(new MeshCollector)
	{

	}

	//----------------------------------------------------------------------------------------------
	MeshManager::~MeshManager()
	{

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
		Renderer::EPassType pass, bool instanced, RenderOps& rops,
		const mat4f& viewPort, AABB* aabb)
	{
		m_meshCollector->begin(pass);

		for (const auto& inst : m_meshInstances)
		{
			if (!inst)
				continue;

			//-- 1. cull frustum against AABB.
			if (g_enableCulling && inst->m_transform->m_worldBounds.calculateOutcode(viewPort) != 0)
			{
				continue;
			}
			else if (g_showVisibilityBoxes)
			{
				DebugDrawer::instance().drawAABB(inst->m_transform->m_worldBounds, Color(1,0,0,0));
			}
			
			if (aabb)
			{
				aabb->combine(inst->m_transform->m_worldBounds);
			}

			//-- 2. gather render operations.
			if (inst->m_mesh)
			{
				//-- if mesh collector doesn't want to get this instance then process it as usual.
				if (!g_enableInstancing || (g_enableInstancing && !m_meshCollector->addMeshInstance(*inst.get())))
				{
					uint count = inst->m_mesh->gatherROPs(pass, instanced, rops);
					for (uint i = rops.size() - count; i < rops.size(); ++i)
					{
						RenderOp& rop = rops[i];
						rop.m_worldMat = &inst->m_transform->m_worldMat;
					}
				}
			}
			else if (inst->m_skinnedMesh)
			{
				uint count = inst->m_skinnedMesh->gatherROPs(pass, instanced, rops);
				for (uint i = rops.size() - count; i < rops.size(); ++i)
				{
					RenderOp& rop = rops[i];

					rop.m_worldMat			 = &inst->m_transform->m_worldMat;
					rop.m_matrixPalette		 = &inst->m_worldPalette[0];
					rop.m_matrixPaletteCount = inst->m_worldPalette.size();
				}
			}
		}

		m_meshCollector->gatherROPs(rops);
		m_meshCollector->end();

		return rops.size();
	}

	//----------------------------------------------------------------------------------------------
	Handle MeshManager::createMeshInstance(const MeshInstance::Desc& desc, Transform* transform)
	{
		ResourcesManager& rm = ResourcesManager::instance();
		auto mInst = std::make_unique<MeshInstance>();

		//-- 1. load skinned mesh.
		if (getFileExt(desc.fileName) == "skinnedmesh")
		{
			auto mesh = rm.loadSkinnedMesh(desc.fileName);
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
			auto mesh = rm.loadMesh(desc.fileName);
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
				nodeMat.setIdentity();
				transform->m_nodes.push_back(std::make_unique<Node>(joint.m_name, nodeMat));
			}
		}

		m_meshInstances.push_back(std::move(mInst));
		return m_meshInstances.size() - 1;
	}
	
	//----------------------------------------------------------------------------------------------
	void MeshManager::removeMeshInstance(Handle handle)
	{
		m_meshInstances[handle].reset();
	}

	//----------------------------------------------------------------------------------------------
	MeshInstance& MeshManager::getMeshInstance(Handle handle)
	{
		return *m_meshInstances[handle].get();
	}

} //-- render
} //-- brUGE