#pragma once

#include "prerequisites.hpp"
#include "IComponent.hpp"
#include "ISystem.hpp"
#include "GameObject.hpp"

namespace brUGE
{

	//------------------------------------------------------------------------------------------------------------------
	class Universe : public NonCopyable
	{
	public:

		//--------------------------------------------------------------------------------------------------------------
		class World
		{
		public:
			World();
			~World();

			bool			init();
			bool			init(const pugi::xml_node& data);

			Handle			createGameObject(const pugi::xml_node& cfg);
			Handle			cloneGameObject(Handle gameObj);
			bool			removeGameObject(Handle gameObj);

			GameObject*		gameObject(Handle id);

		private:
			bool			removeGameObjectRecursively(Handle gameObj);
			Handle			cloneGameObjectRecursively(Handle gameObj);

		private:
			std::vector<std::unique_ptr<GameObject>>	m_gameObjects;
			std::unordered_map<ISystem::TypeID, Handle>	m_worlds;
		};

		//--------------------------------------------------------------------------------------------------------------
		class Context
		{
		public:

		private:
			std::unordered_map<ISystem::TypeID, Handle> m_contexts;
		};

	public:
		Universe();
		~Universe();

		virtual bool	init();

		Handle			createWorld(const pugi::xml_node& cfg = pugi::xml_node());
		bool			removeWorld(Handle handle);
		World*			world(Handle handle) const { return m_worlds[handle].get(); }

	private:
		std::vector<std::unique_ptr<World>>		m_worlds;
		std::vector<std::unique_ptr<Context>>	m_contexts;
	};

}