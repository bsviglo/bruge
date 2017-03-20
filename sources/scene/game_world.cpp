#include "game_world.hpp"

#include "engine/Engine.h"
#include "os/FileSystem.h"
#include "render/animation_engine.hpp"
#include "render/render_system.hpp"
#include "render/render_world.hpp"
#include "render/mesh_manager.hpp"
#include "physics/physic_world.hpp"

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
	Handle SceneSystem::addScene(const utils::ROData& iData)
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	bool SceneSystem::Scene::init(const pugi::xml_node& data)
	{
		for (auto gameObjCfg : data.children("gameObject"))
		{
			createGameObject(gameObjCfg);
		}
	}

	//------------------------------------------------------------------------------------------------------------------
	Handle SceneSystem::Scene::createGameObject(const pugi::xml_node& data)
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
				auto type			= rttr::type::get_by_name(typeName);
				auto typeID			= type.get_property_value("typeID").get_value<uint32>();
				auto systemTypeID	= type.get_property_value("systemTypeID").get_value<uint32>();
				auto world			= static_cast<ISystem::IComponentWorld*>(m_systemWorlds[systemTypeID].get());

				//-- create new component in desired system's world
				world->createComponent(gameObjID, typeID, componentCfg);
			}
		}
		
	}

	//------------------------------------------------------------------------------------------------------------------
	bool SceneSystem::Scene::removeGameObject(Handle handle)
	{
		removeGameObjectRecursively(handle);
	}

	//------------------------------------------------------------------------------------------------------------------
	Handle SceneSystem::Scene::cloneGameObject(Handle handle)
	{
		auto gameObj = m_gameObjects[handle].get();

		//-- ToDo:
	}

	//------------------------------------------------------------------------------------------------------------------
	bool SceneSystem::Scene::removeGameObjectRecursively(Handle handle)
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
	Handle SceneSystem::Scene::cloneGameObjectRecursively(Handle hanle)
	{

	}


	//-- ToDo: Legacy



	//----------------------------------------------------------------------------------------------
	GameWorld::GameWorld()
	{

	}

	//----------------------------------------------------------------------------------------------
	GameWorld::~GameWorld()
	{
		for (uint i = 0; i < m_objs.size(); ++i)
		{
			delete m_objs[i];
		}
		m_objs.clear();
	}
	
	//----------------------------------------------------------------------------------------------
	bool GameWorld::init()
	{
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool GameWorld::handleMouseButtonEvent(const SDL_MouseButtonEvent& e)
	{
		if (m_playerObj)
			return m_playerObj->handleMouseButtonEvent(e);

		return false;
	}

	//----------------------------------------------------------------------------------------------
	bool GameWorld::handleMouseMotionEvent(const SDL_MouseMotionEvent& e)
	{
		if (m_playerObj)
			return m_playerObj->handleMouseMotionEvent(e);

		return false;
	}

	//----------------------------------------------------------------------------------------------
	bool GameWorld::handleKeyboardEvent(const SDL_KeyboardEvent& e)
	{
		if (m_playerObj)
			return m_playerObj->handleKeyboardEvent(e);

		return false;
	}

	//----------------------------------------------------------------------------------------------
	bool GameWorld::handleMouseWheelEvent(const SDL_MouseWheelEvent& e)
	{
		if (m_playerObj)
			return m_playerObj->handleMouseWheelEvent(e);

		return false;
	}

	//----------------------------------------------------------------------------------------------
	bool GameWorld::loadMap(const char* /*mapName*/)
	{
		return false;
	}

	//----------------------------------------------------------------------------------------------
	bool GameWorld::saveMap(const char* /*mapName*/)
	{
		assert(0);

		return false;
	}

	//----------------------------------------------------------------------------------------------
	bool GameWorld::addPlayer(IPlayerObj* player, const char* desc, const mat4f* orient)
	{
		m_playerObj.reset(player);

		RODataPtr data = os::FileSystem::instance().readFile(desc);
		if (!data)
			return false;

		if (!m_playerObj->load(*data.get(), m_objs.size(), orient))
		{
			return false;
		}

		return true;
	}

	//----------------------------------------------------------------------------------------------
	Handle GameWorld::addGameObj(const char* desc, const mat4f* orient/* = NULL*/)
	{
		std::unique_ptr<IGameObj> obj(new IGameObj());

		return addGameObj(obj.release(), desc, orient);
	}

	//----------------------------------------------------------------------------------------------
	Handle GameWorld::addGameObj(IGameObj* inObj, const char* desc, const mat4f* orient)
	{
		std::unique_ptr<IGameObj> obj(inObj);

		RODataPtr data = os::FileSystem::instance().readFile(desc);
		if (!data)
			return CONST_INVALID_HANDLE;

		if (!obj->load(*data.get(), m_objs.size(), orient))
		{
			return CONST_INVALID_HANDLE;
		}
		else
		{
			m_objs.push_back(obj.release());
			return m_objs.size() - 1;
		}
	}

	//----------------------------------------------------------------------------------------------
	bool GameWorld::delGameObj(Handle handle)
	{
		if (handle == CONST_INVALID_HANDLE || static_cast<size_t>(handle) >= m_objs.size())
			return false;

		delete m_objs[handle];
		m_objs[handle] = m_objs.back();
		m_objs.pop_back();

		return true;
	}

	//----------------------------------------------------------------------------------------------
	void GameWorld::beginUpdate(float dt)
	{
		if (m_playerObj)
			m_playerObj->beginUpdate(dt);

		for (uint i = 0; i < m_objs.size(); ++i)
		{
			m_objs[i]->beginUpdate(dt);
		}
	}

	//----------------------------------------------------------------------------------------------
	void GameWorld::preAnimUpdate()
	{

	}

	//----------------------------------------------------------------------------------------------
	void GameWorld::postAnimUpdate()
	{

	}

	//----------------------------------------------------------------------------------------------
	void GameWorld::endUpdate()
	{

	}

	//----------------------------------------------------------------------------------------------
	IGameObj::IGameObj()
		:	m_self(CONST_INVALID_HANDLE), m_animCtrl(CONST_INVALID_HANDLE),
			m_meshInst(CONST_INVALID_HANDLE), m_physObj(CONST_INVALID_HANDLE)
	{

	}

	//----------------------------------------------------------------------------------------------
	IGameObj::~IGameObj()
	{
		if (m_meshInst != CONST_INVALID_HANDLE)
		{
			Engine::instance().renderWorld().meshManager().removeMeshInstance(m_meshInst);
			m_meshInst = CONST_INVALID_HANDLE;
		}

		if (m_animCtrl != CONST_INVALID_HANDLE)
		{
			Engine::instance().animationEngine().removeAnimationController(m_animCtrl);
			m_animCtrl = CONST_INVALID_HANDLE;
		}

		if (m_physObj != CONST_INVALID_HANDLE)
		{
			Engine::instance().physicsWorld().removePhysicsObject(m_physObj);
			m_physObj = CONST_INVALID_HANDLE;
		}
	}

	//----------------------------------------------------------------------------------------------
	bool IGameObj::load(const ROData& data, Handle objID, const mat4f* orient/* = NULL*/)
	{
		pugi::xml_document	   doc;
		pugi::xml_parse_result result = doc.load_buffer(data.ptr(), data.length());

		if (!result)
			return false;

		pugi::xml_node objectDesc = doc.document_element();

		MeshManager& meshManager = Engine::instance().renderWorld().meshManager();

		//-- 1. setup root matrix.
		{
			if (orient)	m_transform.m_worldMat = *orient;
			else		m_transform.m_worldMat.setIdentity();

			m_transform.m_nodes.emplace_back(std::make_unique<Node>("root", m_transform.m_worldMat));
		}

		//-- 2. load render part of the game object.
		{
			pugi::xml_node renderNode = objectDesc.child("render");
			if (renderNode.empty())
			{
				m_meshInst = CONST_INVALID_HANDLE;
			}
			else
			{
				MeshInstance::Desc desc;
				desc.fileName = renderNode.attribute("file").value();

				m_meshInst = meshManager.createMeshInstance(desc, &m_transform);
			}
		}

		//-- 3. parse physics/collision data of the game object.
		{
			pugi::xml_node physicsNode = objectDesc.child("physics");
			if (physicsNode.empty())
			{
				m_physObj = CONST_INVALID_HANDLE;
			}
			else
			{
				if (auto desc = physicsNode.attribute("file"))
				{
					m_physObj = Engine::instance().physicsWorld().createPhysicsObject(
						desc.value(), &m_transform, objID
						);
				}
			}
		}

		//-- 4. setup animation part.
		{
			MeshInstance& meshInst = meshManager.getMeshInstance(m_meshInst);

			if (meshInst.m_skinnedMesh)
			{
				AnimationController::Desc desc;
				desc.m_meshInst  = &meshInst;
				desc.m_transform = &m_transform;

				m_animCtrl = Engine::instance().animationEngine().createAnimationController(desc);
			}
			else
			{
				m_animCtrl = CONST_INVALID_HANDLE;
			}
		}

		//-- store its own unique handle in the world after complete successfully initialization.
		m_self = objID;

		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool IGameObj::save(WOData& /*inData*/)
	{
		assert(0);
		return false;
	}

	//----------------------------------------------------------------------------------------------
	Transform::Transform()
	{

	}

	//----------------------------------------------------------------------------------------------
	Transform::~Transform()
	{

	}

} //-- brUGE