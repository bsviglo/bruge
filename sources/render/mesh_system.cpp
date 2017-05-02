#include "mesh_system.hpp"

#include "mesh_collector.hpp"

#include "engine/Engine.h"
#include "render_system.hpp"
#include "loader/ResourcesManager.h"
#include "scene/transform_system.hpp"
#include "render/culling_system.hpp"

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

	using namespace render;

	RTTR_REGISTRATION
	{
		rttr::registration::class_<StaticMeshComponent>("StaticMeshComponent")
			.constructor<>()
			.property_readonly("typeID", StaticMeshComponent::typeID)
			.property_readonly("systemTypeID", MeshSystem::typeID);
			
		rttr::registration::class_<SkinnedMeshComponent>("SkinnedMeshComponent")
			.constructor<>()
			.property_readonly("typeID", SkinnedMeshComponent::typeID)
			.property_readonly("systemTypeID", MeshSystem::typeID);

		rttr::registration::class_<MeshSystem>("MeshSystem")
			.constructor<>()
			.property_readonly("typeID", MeshSystem::typeID);
	}

//----------------------------------------------------------------------------------------------------------------------
namespace render
{
	const IComponent::TypeID	StaticMeshComponent::m_typeID;
	const IComponent::TypeID	SkinnedMeshComponent::m_typeID;
	const ISystem::TypeID		MeshSystem::m_typeID;

	//------------------------------------------------------------------------------------------------------------------
	MeshSystem::MeshSystem()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	MeshSystem::~MeshSystem()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	bool MeshSystem::init(const pugi::xml_node& cfg)
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
		auto&		c = static_cast<Context&>(context(contextHandle));
		const auto& w = static_cast<const World&>(c.world());

		auto&		r = *c.m_rops;

		//-- process static meshes
		for (auto& h : c.m_visibilitySet->m_buckets[StaticMeshComponent::typeID()])
		{
			auto& inst = w.m_staticMeshInstances[h.handle()];

			//-- if mesh collector doesn't want to get this instance then process it as usual.
			if (!c.m_meshCollector->addMeshInstance(*inst.get()))
			{
				uint count = inst->m_mesh->gatherROPs(c.m_passType, c.m_useInstancing, r);
				for (uint i = r.size() - count; i < r.size(); ++i)
				{
					r[i].m_worldMat = &inst->m_transform->m_worldMat;
				}
			}
		}

		//-- 
		c.m_meshCollector->gatherROPs(r);
		c.m_meshCollector->end();

		//-- process skinned meshes
		for (auto& h : c.m_visibilitySet->m_buckets[SkinnedMeshComponent::typeID()])
		{
			auto& inst = w.m_skinnedMeshInstances[h.handle()];

			uint count = inst->m_skinnedMesh->gatherROPs(c.m_passType, c.m_useInstancing, r);
			for (uint i = r.size() - count; i < r.size(); ++i)
			{
				auto& rop = r[i];

				rop.m_worldMat = &inst->m_transform->m_worldMat;
				rop.m_matrixPalette = &inst->m_worldPalette[0];
				rop.m_matrixPaletteCount = inst->m_worldPalette.size();
			}
		}
	}


	//------------------------------------------------------------------------------------------------------------------
	MeshSystem::World::World(const ISystem& system, Handle universeWorld) : IWorld(system, universeWorld)
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	MeshSystem::World::~World()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	bool MeshSystem::World::init(const pugi::xml_node& cfg)
	{
		//-- ToDo:
		return true;
	}

	//------------------------------------------------------------------------------------------------------------------
	void MeshSystem::World::activate()
	{
		//-- ToDo:
	}

	//------------------------------------------------------------------------------------------------------------------
	void MeshSystem::World::deactivate()
	{
		//-- ToDo:
	}

	//------------------------------------------------------------------------------------------------------------------
	IComponent::Handle MeshSystem::World::createComponent(Handle gameObj, IComponent::TypeID typeID)
	{
		return createComponent(gameObj, typeID, pugi::xml_node());
	}

	//------------------------------------------------------------------------------------------------------------------
	IComponent::Handle MeshSystem::World::createComponent(Handle gameObj, IComponent::TypeID typeID, const pugi::xml_node& cfg)
	{
		//-- retrieve Worlds for TransformSystem and ResourceSystem
		auto& resourceWorld		= engine().world<ResourceSystem::World>(m_universeWorld);
		auto& transformWorld	= engine().world<TransformSystem::World>(m_universeWorld);
		auto& cullingWorld		= engine().world<CullingSystem::World>(m_universeWorld);

		//-- retrieve TransformComponent from GameObject
		auto  transformComponent = engine().universe().world(m_universeWorld).gameObject(gameObj)->getComponent<TransformComponent>();
		auto& transform			 = transformWorld.component(transformComponent);

		auto mInst = std::make_unique<MeshInstance>();

		if (typeID == StaticMeshComponent::typeID())
		{
			mInst->m_mesh = resourceWorld.loadMesh(cfg.find_child("resource").text().as_string());
		}
		else if (typeID == SkinnedMeshComponent::typeID())
		{
			mInst->m_skinnedMesh = resourceWorld.loadSkinnedMesh(cfg.find_child("resource").text().as_string());

			//-- resize mesh world palette to match the bones count in the skinned mesh.
			mInst->m_worldPalette.resize(mInst->m_skinnedMesh->skeleton().size());

			//-- initialize nodes. 
			for (uint i = 0; i < mInst->m_worldPalette.size(); ++i)
			{
				const Joint& joint   = mInst->m_skinnedMesh->skeleton()[i];
				mat4f&		 nodeMat = mInst->m_worldPalette[i];

				//-- set default initial value.
				nodeMat.setIdentity();
				mInst->m_transform->m_nodes.push_back(std::make_unique<Node>(joint.m_name, nodeMat));
			}
		}
		else
		{
			assert(false && "Invalid TypeID of the component");
		}

		//-- common params
		mInst->m_transform		= &transform.transform();
		mInst->m_localBounds	= mInst->m_mesh->bounds();
		mInst->m_worldBounds	= mInst->m_mesh->bounds().getTranformed(mInst->m_transform->m_worldMat);

		const auto&		   aabb = mInst->m_worldBounds;
		IComponent::Handle oHandle;

		//--
		if (typeID == StaticMeshComponent::typeID())
		{
			auto component = std::make_unique<StaticMeshComponent>(gameObj, *mInst.get());
			m_staticMeshComponets.push_back(std::move(component));
			m_staticMeshInstances.push_back(std::move(mInst));

			oHandle = IComponent::Handle(StaticMeshComponent::typeID(), MeshSystem::typeID(), m_staticMeshComponets.size() - 1);
		}
		else
		{
			auto component = std::make_unique<SkinnedMeshComponent>(gameObj, *mInst.get());
			m_skinnedMeshComponents.push_back(std::move(component));
			m_skinnedMeshInstances.push_back(std::move(mInst));

			oHandle = IComponent::Handle(StaticMeshComponent::typeID(), MeshSystem::typeID(), m_skinnedMeshComponents.size() - 1);
		}

		//-- register in culling System
		cullingWorld.add(oHandle, aabb);

		return oHandle;
	}

	//------------------------------------------------------------------------------------------------------------------
	IComponent::Handle MeshSystem::World::cloneComponent(Handle srcGameObj, Handle dstGameObj, IComponent::TypeID typeID)
	{
		//-- ToDo:
		return IComponent::Handle(typeID, MeshSystem::typeID(), CONST_INVALID_HANDLE);
	}

	//------------------------------------------------------------------------------------------------------------------
	void MeshSystem::World::removeComponent(IComponent::Handle component)
	{
		assert(component.systemTypeID() == MeshSystem::typeID());
		assert(component.handle() != CONST_INVALID_HANDLE);

		if (component.typeID() == SkinnedMeshComponent::typeID())
		{
			m_skinnedMeshComponents[component.handle()].reset();
			m_skinnedMeshInstances[component.handle()].reset();
		}
		else
		{
			m_staticMeshComponets[component.handle()].reset();
			m_staticMeshInstances[component.handle()].reset();
		}
	}

	//------------------------------------------------------------------------------------------------------------------
	MeshSystem::Context::Context(const ISystem& system, const IWorld& world)
		:	IContext(system, world), m_visibilitySet(nullptr), m_renderCamera(nullptr),
			m_useInstancing(false), m_passType(Renderer::PASS_MAIN_COLOR)
	{
	}

	//------------------------------------------------------------------------------------------------------------------
	MeshSystem::Context::~Context()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	bool MeshSystem::Context::init()
	{
		m_meshCollector = std::make_unique<MeshCollector>();

		return true;
	}

} //-- render
} //-- brUGE