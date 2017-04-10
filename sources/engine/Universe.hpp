#pragma once

#include "prerequisites.hpp"
#include "IComponent.hpp"
#include "ISystem.hpp"
#include "GameObject.hpp"

#include <array>

namespace brUGE
{

	//-- The entry point of the all game specific stuff
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

			bool			init(const pugi::xml_node& cfg = pugi::xml_node());

			Handle			createGameObject(const pugi::xml_node& cfg);
			Handle			cloneGameObject(Handle gameObj);
			bool			removeGameObject(Handle gameObj);

			GameObject*		gameObject(Handle id) const;
			Handle			world(ISystem::TypeID typeID) const;		

		private:
			bool			removeGameObjectRecursively(Handle gameObj);
			Handle			cloneGameObjectRecursively(Handle gameObj);

		private:
			std::vector<std::unique_ptr<GameObject>>				m_gameObjects;
			std::array<Handle, ISystem::TypeID::C_MAX_SYSTEM_TYPES>	m_worlds;
		};

		//--------------------------------------------------------------------------------------------------------------
		class Context
		{
		public:

		private:
			std::array<Handle, ISystem::TypeID::C_MAX_SYSTEM_TYPES>	m_contexts;
		};

	public:
		typedef std::vector<std::unique_ptr<World>>		Worlds;
		typedef std::vector<std::unique_ptr<Context>>	Contexts;

	public:
		Universe();
		~Universe();

		virtual bool	init();

		Handle			createWorld(const pugi::xml_node& cfg = pugi::xml_node());
		bool			removeWorld(Handle handle);

		Handle			createContext(Handle world);
		bool			removeContext(Handle context);

		World&			world(Handle handle) const		{ return *m_worlds[handle].get(); }
		Context&		context(Handle handle) const	{ return *m_contexts[handle].get(); }

		Contexts&		contexts()						{ return m_contexts; }

	private:
		Worlds		m_worlds;
		Contexts	m_contexts;
	};

}