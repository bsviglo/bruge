#pragma once

#include "prerequisites.hpp"
#include "utils/Singleton.h"
#include "console/Console.h"
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

	namespace render
	{
		class AnimationEngine;
		class RenderWorld;
	}

	namespace physic
	{
		class PhysicWorld;
	}

	//-- Entry point of the brUGE.
	//--------------------------------------------------------------------------------------------------
	class Engine : public utils::Singleton<Engine>, public NonCopyable
	{
	public:
		Engine();
		~Engine();
		
		void						init(HINSTANCE hInstance, IDemo* demo);
		void						shutdown();
		
		//-- entry point of engine.
		int							run();
		void						stop();
		
		void						updateFrame(float dt);
		void						drawFrame(float dt);
		
		render::VideoMode&			getVideoMode()    { return m_videoMode; }
		void						setVideoMode(const render::VideoMode& mode) { m_videoMode = mode; }

		GameWorld&					gameWorld()			{ return *m_gameWorld.get();	}
		render::RenderWorld&		renderWorld()		{ return *m_renderWorld.get();	}
		render::RenderSystem&		renderSystem()		{ return m_renderSys;			}
		physic::PhysicWorld&		physicWorld()		{ return *m_physicWorld.get();	}
		render::AnimationEngine&	animationEngine()	{ return *m_animEngine.get();	}

	private:
		void _fps();

		//-- declare console functions.
		int _exit();
		int _setMaxFPS(int fps);
		int _showFPS(bool show);

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

		static uint									m_maxFPS;
		static bool			 						m_isRunning;
		
		std::unique_ptr<IDemo>						m_demo;
		std::unique_ptr<Console>					m_console;
		std::unique_ptr<WatchersPanel>				m_watchersPanel;
		std::unique_ptr<TimingPanel>				m_timingPanel;
		render::VideoMode	 						m_videoMode;
		render::RenderSystem 						m_renderSys;
	
		std::unique_ptr<ResourcesManager>			m_resManager;
		std::unique_ptr<GameWorld>					m_gameWorld;
		std::unique_ptr<render::RenderWorld>		m_renderWorld;
		std::unique_ptr<physic::PhysicWorld>		m_physicWorld;
		std::unique_ptr<render::AnimationEngine>	m_animEngine;

		//-- ToDo: Try to find better approach to bring together imgui with our engine.
		struct imguiInput
		{
			int  mx, my;
			byte button;
			int  scroll;
		};
		imguiInput						m_imguiInput;
		
		float				 			m_fps;
	};

} // brUGE
