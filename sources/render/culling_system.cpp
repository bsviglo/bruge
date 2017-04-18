#include "culling_system.hpp"

namespace brUGE
{
namespace render
{

	const ISystem::TypeID CullingSystem::m_typeID;

	//------------------------------------------------------------------------------------------------------------------
	CullingSystem::CullingSystem()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	CullingSystem::~CullingSystem()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	bool CullingSystem::init(const pugi::xml_node& cfg)
	{
		//-- ToDo:
		return true;
	}

	//------------------------------------------------------------------------------------------------------------------
	void CullingSystem::update(Handle world, const DeltaTime& dt) const
	{
		//-- ToDo:
	}

	//------------------------------------------------------------------------------------------------------------------
	void CullingSystem::process(Handle handle) const
	{
		auto&		c = static_cast<Context&>(context(handle));
		const auto& w = static_cast<const World&>(c.world());

		for (const auto& bucket : w.m_objects)
		{
			for (const auto& o : bucket)
			{
				if (o.second.calculateOutcode(c.m_camera->m_viewProj) != OUTCODE_MASK)
				{
					c.m_visibilitySet[o.first.typeID()].emplace_back(o.first);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------------------------
	CullingSystem::World::World(const ISystem& system, Handle universeWorld)
		:	IWorld(system, universeWorld)
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	CullingSystem::World::~World()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	bool CullingSystem::World::init(const pugi::xml_node& cfg)
	{
		//-- ToDo: load some static representation.
		return true;
	}

	//------------------------------------------------------------------------------------------------------------------
	void CullingSystem::World::activate()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	void CullingSystem::World::deactivate()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	void CullingSystem::World::add(IComponent::Handle handle, const AABB& aabb)
	{
		m_objects[handle.typeID()].emplace_back(std::make_pair(handle, aabb));
	}

	//------------------------------------------------------------------------------------------------------------------
	void CullingSystem::World::modify(IComponent::Handle handle, const AABB& aabb)
	{
		auto& bucket = m_objects[handle.typeID()];

		auto r = std::find_if(bucket.begin(), bucket.end(), [&] (const auto& o) { 
			o.first == handle
		});

		if (r != bucket.end())
			r->second = aabb;
	}

	//------------------------------------------------------------------------------------------------------------------
	void CullingSystem::World::remove(IComponent::Handle handle)
	{
		auto& bucket = m_objects[handle.typeID()];

		auto r = std::find_if(bucket.begin(), bucket.end(), [&] (const auto& o) { 
			o.first == handle
		});

		if (r != bucket.end())
		{
			r->first = IComponent::Handle(IComponent::TypeID::C_INVALID, ISystem::TypeID::C_INVALID, CONST_INVALID_HANDLE);
		}
	}

	//------------------------------------------------------------------------------------------------------------------
	CullingSystem::Context::Context(const ISystem& system, const IWorld& world)
		:	IContext(system, world)
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	CullingSystem::Context::~Context()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	bool CullingSystem::Context::init()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------------------------
}
}