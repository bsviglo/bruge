#pragma once

#include "prerequisites.hpp"
#include "engine/ISystem.hpp"
#include "utils/Data.hpp"
#include "math/AABB.hpp"
#include "math/Matrix4x4.hpp"
#include "SDL/SDL_events.h"
#include "pugixml/pugixml.hpp"
#include <vector>

namespace brUGE
{

	//------------------------------------------------------------------------------------------------------------------
	class SceneSystem : public ISystem
	{
	public:

		//--------------------------------------------------------------------------------------------------------------
		class Scene
		{
		public:
			Scene();
			~Scene();

			bool			init();
			bool			init(const pugi::xml_node& data);

			Handle			createGameObject(uint32 componentMask = IComponent::TYPE_TRANSFORM);
			Handle			createGameObject(const pugi::xml_node& data);
			Handle			cloneGameObject(Handle gameObj);
			bool			removeGameObject(Handle gameObj);

			GameObject*		gameObject(Handle id);

		private:
			template<typename T>
			typename T::World* world(Engine::ESystemType type) { return static_cast<typename T::World*>(m_sytemWorlds[type]); }

		private:
			std::vector<std::unique_ptr<GameObject>>							m_gameObjects;
			std::array<std::shared_ptr<ISystem::IWorld>, Engine::SYSTEM_COUNT>	m_sytemWorlds;
		};

		//--------------------------------------------------------------------------------------------------------------
		class Context
		{
		public:

		private:
			std::vector<std::unique_ptr<ISystem::IContext>> m_contexts;
		};

	public:
		SceneSystem();
		virtual ~SceneSystem() override;

		virtual bool	init() override;
		virtual void	update(IWorld* world, const DeltaTime& dt) override;
		virtual void	process(IContext* context) override;

		Handle			addScene();
		Handle			addScene(const utils::ROData& iData);
		bool			delScene(Handle handle);
		Scene*			scene(Handle handle) { return m_scenes[handle].get(); }

	private:
		std::vector<std::unique_ptr<Scene>>		m_scenes;;
		std::vector<std::unique_ptr<Context>>	m_contexts;
	};




	//-- Exists for every node of every model.
	//------------------------------------------------------------------------------------------------------------------
	class Node : public NonCopyable
	{
	public:
		Node(const char* name, mat4f& matrix) : m_name(name), m_matrix(matrix) { }
		~Node() { }

		void		 matrix	(const mat4f& mat)	{ m_matrix = mat; }
		const mat4f& matrix	() const			{ return m_matrix; }
		const char*  name	() const			{ return m_name; }

	private:
		const char* m_name;
		mat4f&		m_matrix;
	};
	typedef std::vector<std::unique_ptr<Node>> Nodes;


	//------------------------------------------------------------------------------------------------------------------
	struct Transform : public NonCopyable
	{
		Transform();
		~Transform();

		AABB  m_localBounds;
		AABB  m_worldBounds;
		mat4f m_worldMat;
		Nodes m_nodes;
	};


	//-- Simple event system implementation.
	//-- ToDo: Rework it.
	//------------------------------------------------------------------------------------------------------------------
	struct GameEvent
	{
		const char* m_name;
		void*		m_data;
		uint		m_size;
	};


	//-- The minimal point of the engines game objects subsystem.
	//------------------------------------------------------------------------------------------------------------------
	class IGameObj : public NonCopyable
	{
	public:
		IGameObj();
		virtual ~IGameObj();

		//-- serialization functions.
		virtual bool load(const utils::ROData& inData, Handle objID, const mat4f* orient = NULL);
		virtual bool save(utils::WOData& outData);

		virtual void receiveEvent(const GameEvent& /*event*/) { }

		virtual void beginUpdate(float /*dt*/) { }
		virtual void preAnimUpdate() { }
		virtual void postAnimUpdate() { }
		virtual void endUpdate() { }

		//-- ToDo: remove.
		Handle			animCtrl() { return m_animCtrl; }
		Handle			physObj()  { return m_physObj; }
		const mat4f&	worldPos() { return m_transform.m_worldMat; }

	protected:
		Handle			m_self;
		Handle			m_meshInst;
		Handle			m_animCtrl;
		Handle			m_physObj;
		//SoundData*	m_soundData;
		//ScriptData*	m_scriptData;
		//AIData*		m_aiData;
		Transform		m_transform;
	};


	//-- Player is a game object and game actor at the same time. It has additional properties and 
	//-- abilities. For example it can intercept mouse and keyboard messages and does something.
	//------------------------------------------------------------------------------------------------------------------
	class IPlayerObj : public IGameObj
	{
	public:
		IPlayerObj() { }
		virtual ~IPlayerObj() { }

		virtual bool handleMouseButtonEvent(const SDL_MouseButtonEvent& e) = 0;
		virtual bool handleMouseMotionEvent(const SDL_MouseMotionEvent& e) = 0;
		virtual bool handleMouseWheelEvent(const SDL_MouseWheelEvent& e) = 0;
		virtual bool handleKeyboardEvent(const SDL_KeyboardEvent& e) = 0;
	};


	//-- Represents the all game world. It manages game specific logic of all the game objects.
	//------------------------------------------------------------------------------------------------------------------
	class GameWorld : public NonCopyable
	{
	public:
		GameWorld();
		~GameWorld();

		bool			init();

		bool			handleMouseButtonEvent(const SDL_MouseButtonEvent& e);
		bool			handleMouseMotionEvent(const SDL_MouseMotionEvent& e);
		bool			handleMouseWheelEvent(const SDL_MouseWheelEvent& e);
		bool			handleKeyboardEvent(const SDL_KeyboardEvent& e);

		//-- load and save map.
		bool			loadMap(const char* mapName);
		bool			saveMap(const char* mapName);
		
		//-- player game object.
		bool			addPlayer(IPlayerObj* player, const char* desc, const mat4f* orient);
		IPlayerObj*		getPlayer() { return m_playerObj.get(); }

		//-- add/delete some game objects to/from game world.
		Handle			addGameObj(IGameObj* obj, const char* desc, const mat4f* orient = NULL);
		Handle			addGameObj(const char* desc, const mat4f* orient = NULL);
		bool			delGameObj(Handle handle);
		IGameObj*		getGameObj(Handle handle) { return m_objs[handle]; }

		//-- update functions bucket.
		void			beginUpdate(float /*dt*/);
		void			preAnimUpdate();
		void			postAnimUpdate();
		void			endUpdate();

	private:
		typedef std::vector<IGameObj*> GameObjs;
		GameObjs m_objs;
		std::unique_ptr<IPlayerObj> m_playerObj;
	};

} //-- brUGE