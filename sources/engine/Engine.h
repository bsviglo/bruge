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
	//--------------------------------------------------------------------------------------------------
	class Engine : public utils::Singleton<Engine>
	{
	public:
		Engine();
		~Engine();
		
		void						init(HINSTANCE hInstance, IDemo* demo);
		void						shutdown();
		
		//-- entry point of engine.
		int							run();
		void						stop();
		
		render::VideoMode&			getVideoMode()    { return m_videoMode; }
		void						setVideoMode(const render::VideoMode& mode) { m_videoMode = mode; }

		GameWorld&					gameWorld()			{ return *m_gameWorld.get();	}
		render::RenderWorld&		renderWorld()		{ return *m_renderWorld.get();	}
		render::Renderer&		renderSystem()		{ return m_renderSys;			}
		physics::PhysicsWorld&		physicsWorld()		{ return *m_physicWorld.get();	}
		render::AnimationEngine&	animationEngine()	{ return *m_animEngine.get();	}

		//-- ToDo:
		template<typename T>
		T* system(ESystemType type) { return static_cast<T*>(m_systems[type]); }

	private:

		//-- declare console functions.
		int _exit();
		void displayStatistics(float dt);

		void						handleMouseButtonEvent(const SDL_MouseButtonEvent& e);
		void						handleMouseMotionEvent(const SDL_MouseMotionEvent& e);
		void						handleMouseWheelEvent(const SDL_MouseWheelEvent& e);
		void						handleKeyboardEvent(const SDL_KeyboardEvent& e);
		void						handleTextInputEvent(const SDL_TextInputEvent& e);

	private:
		os::FileSystem		 						m_fileSystem;
		utils::LogManager	 						m_logManager; 

		std::string			 						m_title;
		HWND				 						m_hWnd;
		
		std::unique_ptr<IDemo>						m_demo;
		std::unique_ptr<WatchersPanel>				m_watchersPanel;
		std::unique_ptr<TimingPanel>				m_timingPanel;
		std::unique_ptr<ui::System>					m_uiSystem;
		render::VideoMode	 						m_videoMode;
		render::Renderer 						m_renderSys;
	
		std::unique_ptr<ResourcesManager>			m_resManager;
		std::unique_ptr<GameWorld>					m_gameWorld;
		std::unique_ptr<render::RenderWorld>		m_renderWorld;
		std::unique_ptr<physics::PhysicsWorld>		m_physicWorld;
		std::unique_ptr<render::AnimationEngine>	m_animEngine;

		//-- new
		std::array<std::unique_ptr<ISystem>, ISystem::TypeID::C_MAX_SYSTEM_TYPES> m_systems;
	};

} // brUGE
