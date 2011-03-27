#pragma once

#include "prerequisites.hpp"
#include "utils/Singleton.h"
#include "utils/Timer.h"
#include "console/Console.h"
#include "console/WatchersPanel.h"
#include "console/TimingPanel.h"
#include "control/InputManager.h"
#include "loader/ResourcesManager.h"
#include "os/WinApp.h"
#include "os/FileSystem.h"
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


	//-- Optional dialog showed before starting the engine. Gives opportunity to choice desired
	//-- engine options.
	//----------------------------------------------------------------------------------------------
	class EngineConfigDialog : public NonCopyable
	{
	public:
		EngineConfigDialog();
		~EngineConfigDialog();

		bool display();
	};


	//-- Entry point of the brUGE.
	//----------------------------------------------------------------------------------------------
	class Engine : public utils::Singleton<Engine>, public IInputListener, public NonCopyable
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

		virtual void				handleMouseClick(const MouseEvent &me);
		virtual void				handleMouseMove(const MouseAxisEvent &mae);
		virtual void				handleKeyboardEvent(const KeyboardEvent &ke);

		HINSTANCE					getHInstance()	  { return m_hInstance; }
		
		render::VideoMode&			getVideoMode()    { return m_videoMode; }
		void						setVideoMode(const render::VideoMode& mode) { m_videoMode = mode; }

		GameWorld&					gameWorld()			{ return *m_gameWorld.get();	}
		render::RenderWorld&		renderWorld()		{ return *m_renderWorld.get();	}
		render::RenderSystem&		renderSystem()		{ return m_renderSys;			}
		physic::PhysicWorld&		physicWorld()		{ return *m_physicWorld.get();	}
		render::AnimationEngine&	animationEngine()	{ return *m_animEngine.get();	}

	private:
		//-- ToDo: reconsider. It seems useless.
		static DWORD WINAPI _eventGenerator(LPVOID lpParameter);
		static DWORD WINAPI _bgTask_loadGameResources(LPVOID lpParameter);

		bool _initRenderSystem(render::ERenderAPIType api, const render::VideoMode& videoMode);
		void _releaseRenderSystem();

		void _fps();

		//-- declare console functions.
		int _exit();
		int _setMaxFPS(int fps);
		int _showFPS(bool show);

	private:
		os::FileSystem		 						m_fileSystem;
		utils::LogManager	 						m_logManager; 

		std::string			 						m_title;
		HINSTANCE			 						m_hInstance;
		HANDLE				 						m_hThread;
		DWORD				 						m_threadId;
		HWND				 						m_hWnd;

		static uint									m_maxFPS;
		static HANDLE		 						m_hEvent;
		static bool			 						m_isWorking;
		
		std::unique_ptr<IDemo>						m_demo;
		std::unique_ptr<Console>					m_console;
		std::unique_ptr<WatchersPanel>				m_watchersPanel;
		std::unique_ptr<TimingPanel>				m_timingPanel;
		render::VideoMode	 						m_videoMode;
		utils::Timer		 						m_timer;
		os::WinApp			 						m_mainWindow;
		render::RenderSystem 						m_renderSys;
		InputManager		 						m_inputSystem;
		EngineConfigDialog							m_configDialog;
	
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
