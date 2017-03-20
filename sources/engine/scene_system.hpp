#pragma once

#include "prerequisites.hpp"
#include "IComponent.hpp"
#include "ISystem.hpp"
#include "GameObject.hpp"

namespace brUGE
{

	//------------------------------------------------------------------------------------------------------------------
	class SceneSystem
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

			Handle			createGameObject(const pugi::xml_node& cfg);
			Handle			cloneGameObject(Handle gameObj);
			bool			removeGameObject(Handle gameObj);

			GameObject*		gameObject(Handle id);

		private:
			bool			removeGameObjectRecursively(Handle gameObj);
			Handle			cloneGameObjectRecursively(Handle gameObj);

		private:
			std::vector<std::unique_ptr<GameObject>>								m_gameObjects;
			std::unordered_map<ISystem::TypeID, std::unique_ptr<ISystem::IWorld>>	m_systemWorlds;
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
		~SceneSystem();;

		virtual bool	init();

		Handle			addScene(const pugi::xml_node* cfg = nullptr);
		bool			delScene(Handle handle);
		Scene*			scene(Handle handle) { return m_scenes[handle].get(); }

	private:
		std::vector<std::unique_ptr<Scene>>		m_scenes;;
		std::vector<std::unique_ptr<Context>>	m_contexts;
	};

}