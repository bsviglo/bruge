#include "universe.hpp"
#include "Engine.h"
#include "rttr/type"

namespace brUGE
{

	//------------------------------------------------------------------------------------------------------------------
	Handle Universe::createWorld(const pugi::xml_node& cfg)
	{
		auto   world = std::make_unique<World>();
		Handle handle = m_worlds.size();

		if (world->init(handle, cfg))
		{
			m_worlds.push_back(std::move(world));
			return handle;
		}
		else
		{
			return CONST_INVALID_HANDLE;
		}
	}

	//------------------------------------------------------------------------------------------------------------------
	void Universe::removeWorld(Handle handle)
	{
		m_worlds[handle].reset();
	}

	//------------------------------------------------------------------------------------------------------------------
	Universe::World::World() : m_worlds{ CONST_INVALID_HANDLE }
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	Universe::World::~World()
	{
		//-- delete all related worlds
		for (auto s = engine().systemOrderList().rbegin(); s != engine().systemOrderList().rend(); ++s)
			engine().system(*s).removeWorld(m_worlds[*s]);
	}

	//------------------------------------------------------------------------------------------------------------------
	bool Universe::World::temporal_hardcoded_init()
	{
		//-- Zombie
		createGameObjectFromPrefab("prefabs/zombie.xml");

		return true;
	}

	//------------------------------------------------------------------------------------------------------------------
	bool Universe::World::init(Handle self, const pugi::xml_node& cfg)
	{
		bool ok = true;

		//-- create all related worlds in all engine's sub-systems
		for (auto typeID : engine().systemOrderList())
		{
			m_worlds[typeID] = engine().system(typeID).createWorld(self, cfg);
			ok &= (m_worlds[typeID] != CONST_INVALID_HANDLE);
		}

		//-- load game objects
		for (auto gameObjCfg : cfg.children("gameObject"))
		{
			createGameObject(gameObjCfg);
		}

		return ok;
	}

	//------------------------------------------------------------------------------------------------------------------
	Handle Universe::World::createGameObject(const pugi::xml_node& cfg)
	{
		return createGameObjectRecursively(cfg);
	}

	//------------------------------------------------------------------------------------------------------------------
	Handle Universe::World::createGameObjectRecursively(const pugi::xml_node& cfg)
	{
		auto gameObj = std::make_unique<GameObject>();

		//-- register it in scene
		m_gameObjects.push_back(std::move(gameObj));
		Handle gameObjHandle = m_gameObjects.size() - 1;

		if (auto componentsCfg = cfg.child("components"))
		{
			for (auto componentCfg : componentsCfg.children("component"))
			{
				auto typeName = std::string(componentCfg.attribute("type").value());

				//-- find out the appropriate world for the component type to create in.
				auto  type			= rttr::type::get_by_name(typeName);
				auto  typeID		= type.get_property_value("typeID").get_value<IComponent::TypeID>();
				auto  systemTypeID	= type.get_property_value("systemTypeID").get_value<ISystem::TypeID>();
				auto& system		= engine().system(systemTypeID);
				auto& world			= system.world(m_worlds[systemTypeID]);

				//-- create new component in desired system's world
				auto component = world.createComponent(gameObjHandle, typeID, componentCfg);
				
				gameObj->addComponent(component);
			}
		}

		//--
		for (auto childCfg : cfg.children("gameObject"))
		{
			auto childHandle = createGameObjectRecursively(childCfg);
			gameObj->addChild(childHandle);
		}

		return gameObjHandle;
	}

	//------------------------------------------------------------------------------------------------------------------
	void Universe::World::removeGameObject(Handle handle)
	{
		removeGameObjectRecursively(handle);
	}

	//------------------------------------------------------------------------------------------------------------------
	Handle Universe::World::cloneGameObject(Handle handle)
	{
		auto gameObj = m_gameObjects[handle].get();

		//-- ToDo:
	}

	//------------------------------------------------------------------------------------------------------------------
	void Universe::World::removeGameObjectRecursively(Handle handle)
	{
		//-- 
		auto gameObj = std::move(m_gameObjects[handle]);

		//-- remove all the components associated with this game object
		for (auto& component : gameObj->components())
		{
			auto  systemTypeID	= static_cast<ISystem::TypeID>(component.systemTypeID());
			auto& system		= engine().system(systemTypeID);
			auto& world			= system.world(m_worlds[systemTypeID]);

			world.removeComponent(component);
		}

		//-- now iterate over the all child game object and remove them
		for (auto childGameObj : gameObj->children())
		{
			removeGameObjectRecursively(childGameObj);
		}
	}

	//------------------------------------------------------------------------------------------------------------------
	Handle Universe::World::cloneGameObjectRecursively(Handle hanle)
	{
		//-- ToDo: implement
	}

	//------------------------------------------------------------------------------------------------------------------
	Universe::Context::Context()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	Universe::Context::~Context()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	bool Universe::Context::init()
	{
		return true;
	}
}