#include "universe.hpp"

//-- http://pugixml.org/
#include "pugixml/pugixml.hpp"

#include "rttr/type"

using namespace brUGE::math;
using namespace brUGE::utils;
using namespace brUGE::os;
using namespace brUGE::render;

namespace brUGE
{

	//------------------------------------------------------------------------------------------------------------------
	bool Universe::World::init(const pugi::xml_node& data)
	{
		for (auto gameObjCfg : data.children("gameObject"))
		{
			createGameObject(gameObjCfg);
		}
	}

	//------------------------------------------------------------------------------------------------------------------
	Handle Universe::createWorld(const pugi::xml_node& cfg)
	{
		return C_INVALID_HANDLE;
	}

	//------------------------------------------------------------------------------------------------------------------
	bool Universe::removeWorld(Handle handle)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------------------------
	Handle Universe::World::createGameObject(const pugi::xml_node& data)
	{
		auto gameObj = std::make_unique<GameObject>();

		//-- register it in scene
		m_gameObjects.push_back(std::move(gameObj));
		Handle gameObjID = m_gameObjects.size() - 1;

		if (auto componentsCfg = data.child("components"))
		{
			for (auto componentCfg : componentsCfg.children("component"))
			{
				auto typeName = std::string(componentCfg.attribute("type").value());

				//-- find out the appropriate world for the component type to create in.
				auto type				= rttr::type::get_by_name(typeName);
				auto typeID				= type.get_property_value("typeID").get_value<uint32>();
				auto systemTypeIDPath	= type.get_property_value("systemTypeIDPath").get_value<ISystem::TypeIDPath>();
				auto world				= m_worlds[systemTypeIDPath[0]].get();

				//-- create new component in desired system's world
				world->createComponent(*this, gameObjID, typeID, componentCfg);
			}
		}
		
	}

	//------------------------------------------------------------------------------------------------------------------
	bool Universe::World::removeGameObject(Handle handle)
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
	bool Universe::World::removeGameObjectRecursively(Handle handle)
	{
		//-- 
		auto gameObj = std::move(m_gameObjects[handle]);

		//-- remove all the components associated with this game object
		for (auto& component : gameObj->components())
		{
			auto systemTypeID = component.systemTypeID();
			static_cast<ISystem::IComponentWorld*>(m_systemWorlds[systemTypeID].get())->removeComponent(component);
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

	}

}