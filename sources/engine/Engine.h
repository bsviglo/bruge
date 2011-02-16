#pragma once

#include "prerequisites.h"
#include "utils/Singleton.h"
#include "utils/string_utils.h"
#include "utils/Timer.h"
#include "console/Console.h"
#include "console/WatchersPanel.h"
#include "console/TimingPanel.h"
#include "render/Camera.h"
#include "render/FreeCamera.h"
#include "control/InputManager.h"
#include "loader/ResourcesManager.h"
#include "os/WinApp.h"
#include "os/FileSystem.h"
#include <memory>

#include "render/render_system.hpp"
#include "render/render_world.hpp"
#include "render/animation_engine.hpp"
#include "render/game_world.hpp"
#include "render/physic_world.hpp"

namespace brUGE
{
	class IDemo;
	
	// Optional dialog showed before starting the engine.
	// Give opportunity to choice desired engine options.
	//----------------------------------------------------------------------------------------------
	class EngineConfigDialog
	{
	public:
		EngineConfigDialog();
		~EngineConfigDialog();

		bool display();
	};

	//-- Entry point of the brUGE.
	//----------------------------------------------------------------------------------------------
	class Engine : public utils::Singleton<Engine>, public IInputListener
	{
	public:
		Engine();
		~Engine();
		
		//-- init sub-systems of engine.
		void init(HINSTANCE hInstance, IDemo* demo);
		
		//-- entry point of engine.
		int  run();
		void stop();
		
		//-- de-init sub-systems of the engine.
		//-- Note: must not throwing exceptions.
		void shutdown();

		void updateFrame(float dt);
		void drawFrame(float dt);

		virtual void handleMouseClick	(const MouseEvent &me);
		virtual void handleMouseMove	(const MouseAxisEvent &mae);
		virtual void handleKeyboardEvent(const KeyboardEvent &ke);

		HINSTANCE					getHInstance()	  { return m_hInstance; }
		
		render::VideoMode&			getVideoMode()    { return m_videoMode; }
		void						setVideoMode(const render::VideoMode& mode) { m_videoMode = mode; }


		GameWorld&					gameWorld()			{ return m_gameWorld; }
		render::RenderWorld&		renderWorld()		{ return m_renderWorld; }
		render::RenderSystem&		renderSystem()		{ return m_renderSys; }
		physic::PhysicWorld&		physicWorld()		{ return m_physicWorld; }
		render::AnimationEngine&	animationEngine()	{ return m_animEngine; }

	private:
		// Функция вторичного потока, она генерирует новое сообщение каждые 20 милесекунд
		// Это нужно для поддержания стабильного FPS.
		//------------------------------------------
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
		os::FileSystem		 			m_fileSystem;
		utils::LogManager	 			m_logManager; 

		std::string			 			m_title;
		HINSTANCE			 			m_hInstance;
		HANDLE				 			m_hThread;
		DWORD				 			m_threadId;
		HWND				 			m_hWnd;

		static uint						m_maxFPS;
		static HANDLE		 			m_hEvent;
		static bool			 			m_isWorking;
		
		std::unique_ptr<IDemo>			m_demo;
		std::unique_ptr<Console>		m_console;
		std::unique_ptr<WatchersPanel>	m_watchersPanel;
		std::unique_ptr<TimingPanel>	m_timingPanel;
		render::VideoMode	 			m_videoMode;
		utils::Timer		 			m_timer;
		os::WinApp			 			m_mainWindow;
		render::RenderSystem 			m_renderSys;
		InputManager		 			m_inputSystem;
		ResourcesManager	 			m_resManager;
		EngineConfigDialog				m_configDialog;

		GameWorld						m_gameWorld;
		render::RenderWorld				m_renderWorld;
		physic::PhysicWorld				m_physicWorld;
		render::AnimationEngine			m_animEngine;
		//render::CollisionWorld			m_collisionWorld;

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
