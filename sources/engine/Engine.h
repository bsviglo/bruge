#pragma once

#include "prerequisites.hpp"
#include "utils/Singleton.h"
#include "console/WatchersPanel.h"
#include "console/TimingPanel.h"
#include "loader/ResourcesManager.h"
#include "os/FileSystem.h"
#include "SDL/SDL_events.h"
#include <memory>

namespace brUGE
{
	class Universe;


	class IDemo;
	class GameWorld;

	namespace ui
	{
		class System;
	}

	namespace render
	{
		class AnimationEngine;
		class RenderWorld;
	}

	namespace physics
	{
		class PhysicsWorld;
	}

	//-- Entry point of the brUGE.
	//------------------------------------------------------------------------------------------------------------------
	class Engine : public utils::Singleton<Engine>
	{
	public:
		typedef std::array<std::unique_ptr<ISystem>, ISystem::TypeID::C_MAX_SYSTEM_TYPES> Systems;

	public:
		Engine();
		~Engine();
		
		bool	init(HINSTANCE hInstance, IDemo* demo);
		int		run();
		void	stop();

	public:

		Systems&						systems() { return m_systems; }
		ISystem&						system(ISystem::TypeID typeID) const { return *m_systems[typeID].get(); }
		std::vector<ISystem::TypeID>	systemOrderList() { return m_systemInitOrder; }


		//-- access to
		template<typename T>
		T&								system() const { return static_cast<T&>(*m_systems[T::typeID()].get()); }
		Universe&						universe() const { return *m_universe.get(); }

		template<typename T>
		T&								word(Handle universeWorld) const
		{
			return static_cast<T&>(m_systems[T::typeID()]->world(m_universe->world(universeWorld).world(T::typeID())));
		}

		template<typename T>
		T&								context(Handle universeContext) const;

	private:

		template<typename T>
		T& create()
		{
			//-- add to the order list
			m_systemInitOrder.push_back(T::typeID());

			//-- add to the system list.
			auto& s = m_systems[T::typeID()];
			assert(!s);
			s.reset(new T());
			return static_cast<T&>(*s);
		}

		//--
		void process(Universe::Context& context);

		void displayStatistics(float dt);

		void handleMouseButtonEvent(const SDL_MouseButtonEvent& e);
		void handleMouseMotionEvent(const SDL_MouseMotionEvent& e);
		void handleMouseWheelEvent(const SDL_MouseWheelEvent& e);
		void handleKeyboardEvent(const SDL_KeyboardEvent& e);
		void handleTextInputEvent(const SDL_TextInputEvent& e);

	private:
		std::string			 						m_title;
		HWND				 						m_hWnd;
		
		//-- ToDo: reconsider old stuff
		os::FileSystem		 						m_fileSystem;
		utils::LogManager	 						m_logManager;

		std::unique_ptr<IDemo>						m_demo;
		std::unique_ptr<WatchersPanel>				m_watchersPanel;
		std::unique_ptr<TimingPanel>				m_timingPanel;
		std::unique_ptr<ui::UISystem>				m_uiSystem;
		render::VideoMode	 						m_videoMode;
		render::Renderer 							m_renderSys;
	
		std::unique_ptr<ResourcesManager>			m_resManager;
		std::unique_ptr<GameWorld>					m_gameWorld;
		std::unique_ptr<render::RenderWorld>		m_renderWorld;
		std::unique_ptr<physics::PhysicsWorld>		m_physicWorld;
		std::unique_ptr<render::AnimationEngine>	m_animEngine;

		//-- new
		std::unique_ptr<Universe>		m_universe;
		Systems							m_systems;
		std::vector<ISystem::TypeID>	m_systemInitOrder;
	};

	//------------------------------------------------------------------------------------------------------------------
	inline Engine& engine() { return Engine::instance(); }

} // brUGE
