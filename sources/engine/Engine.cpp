#include "Engine.h"
#include "IDemo.h"
#include "Exception.h"
#include "utils/Thread.h"
#include "render/IRenderDevice.h"

using namespace brUGE;
using namespace brUGE::render;
using namespace brUGE::utils;
using namespace brUGE::os;

extern bool g_needToStartApp;
extern brUGE::render::ERenderAPIType g_renderAPI;

namespace brUGE
{
	const char* const g_engineName = "black and red Unicorn Graphics Engine";

	DEFINE_SINGLETON(Engine)

	bool	Engine::m_isWorking = true;
	HANDLE	Engine::m_hEvent	= NULL;
	uint	Engine::m_maxFPS	= 60;  //-- ToDo: move to config.

	//------------------------------------------
	DWORD WINAPI Engine::_eventGenerator(LPVOID /*lpParameter*/)
	{
		while (m_isWorking)
		{
			::Sleep(1000 / m_maxFPS);
			SetEvent(m_hEvent);
		}
		SetEvent(m_hEvent);
		return 0;
	}

	//------------------------------------------
	Engine::Engine() : 
		m_logManager("log", true, false),
		m_console(new Console()),
		m_watchersPanel(new WatchersPanel()),
		m_timingPanel(new TimingPanel())
	{
		//-- init COM-system. That is necessary for DirectInput.
		CoInitialize(NULL);

		m_timer.start();

		//-- register console commands.
		REGISTER_CONSOLE_METHOD("sys_setMaxFPS", _setMaxFPS, Engine);
		REGISTER_CONSOLE_METHOD("exit",			 _exit,		 Engine);

		//-- register watchers.
		REGISTER_RO_MEMBER_WATCHER("FPS", float, Engine, m_fps);
	}

	//------------------------------------------
	Engine::~Engine()
	{
		// удaляемся не кидая при этом исключений. :)
		shutdown();

		// деинициализация ком подсистемы. Необходимо для DirectInput
		CoUninitialize();
	}

	//------------------------------------------
	int Engine::run()
	{
		uint64 newTime  = m_timer.time();
		uint64 prevTime = m_timer.time();

		float dt = 0;
		MSG msg	= { 0 };
		Thread thread;

		m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		thread.createThread(Engine::_eventGenerator);
		thread.setPriority(THREAD_PRIORITY_TIME_CRITICAL);

		//-- do main cycle.
		while (m_isWorking)
		{
			// clamp engine tick time.
			newTime = m_timer.time();
			dt = (newTime - prevTime) * 0.001f;
			prevTime = newTime;

			// check queue of messages.
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					m_isWorking = false;
					return 0;
				}
				else
				{
					TranslateMessage (&msg);
					DispatchMessage (&msg);
				}
			}

			//if (m_renderSys->getHWnd() == GetActiveWindow())
			//{
			m_timingPanel->start();
			{
				//-- update timing panel.
				m_timingPanel->update(dt);

				m_inputSystem.update();

				m_renderSys.beginFrame();
				m_gameWorld.beginUpdate(dt);

				m_animEngine.animate(dt);
				m_physicWorld.simulateDynamics(dt);

				//-- ToDo: some old stuff.
				updateFrame(dt);

				m_renderWorld.resolveVisibility();

				//m_collisionWorld.update(dt);

				m_gameWorld.endUpdate();

				//-- ToDo: some old stuff.
				drawFrame(dt);

				m_renderSys.endFrame();
				
				TIME_MEASURER_START_EX("CPU wait")
					WaitForSingleObject(m_hEvent, INFINITE);
				TIME_MEASURER_STOP_EX()
			}
			m_timingPanel->stop();
			//}
		}
		return 0;
	}

	//------------------------------------------
	void Engine::stop()
	{
		m_isWorking = false;
	}
	
	//------------------------------------------
	bool Engine::_initRenderSystem(ERenderAPIType api, const VideoMode& videoMode)
	{
		DWORD dwWindowFlags = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
		dwWindowFlags |= (videoMode.fullScreen) ? WS_POPUP : WS_OVERLAPPEDWINDOW;

		m_hWnd = m_mainWindow.createWnd(g_engineName, videoMode.width, videoMode.height, dwWindowFlags);
		if (!m_hWnd)
		{
			ERROR_MSG("Can't create main render window.");
			return false;
		}
		
		//-- adjust client window size.
		if (!videoMode.fullScreen)
		{
			RECT windowRect = {0, 0, videoMode.width, videoMode.height};

			//-- find the total size of the window, including menu and some other window stuff,
			//-- needed to have the client rect as width x height.
			AdjustWindowRectEx(&windowRect, dwWindowFlags, GetMenu(m_hWnd) != NULL, NULL);
			
			//-- change the window size.
			MoveWindow(m_hWnd, 0, 0,
				windowRect.right - windowRect.left,	windowRect.bottom - windowRect.top,	FALSE
				);
	
			ShowWindow(m_hWnd, SW_SHOW);
		}

		return m_renderSys.init(api, m_hWnd, videoMode);
	}
	
	//------------------------------------------
	void Engine::_releaseRenderSystem()
	{
		m_renderSys.shutDown();
		m_mainWindow.kill();
	}

	//------------------------------------------
	void Engine::init(HINSTANCE hInstance, IDemo* demo)
	{
		m_hInstance = hInstance;

		//-- show config window...
		INFO_MSG("Init menu ... ");
		if (m_configDialog.display())
		{
			// ToDo: save config.
		}
		else
		{
			m_isWorking = false;
			return;
		}
		INFO_MSG("Init menu ... completed.");
		
		//-- terminate application.
		if (!g_needToStartApp)
		{
			m_isWorking = false;
			return;
		}

		//-- register render window.
		INFO_MSG("Register render window class ... ");
		if (!m_mainWindow.init(m_hInstance, g_engineName, g_mainWndProc))
		{
			BR_EXCEPT("Can't register render window class.");
		}
		INFO_MSG("Register main window class ... completed.");
		
		//--
		ConError(g_engineName);

		INFO_MSG("Init resource system ... ");
		if (!m_resManager.init())
		{
			BR_EXCEPT("Can't init resource system.");
		}
		INFO_MSG("Init resource system ... completed.");

		//-- init render sub-system.
		INFO_MSG("Init render system ... ");		

		// ToDo: fix this stuff.
		m_videoMode.bpp = 32;
		m_videoMode.depth = 16;
		m_videoMode.frequancy = 60;

		if (!_initRenderSystem(g_renderAPI, m_videoMode))
		{
			BR_EXCEPT("Can't init render system.");
		}

		INFO_MSG("Init render system ... completed.");

		//-- init console drawing mode.
		m_console->initDrawingMode(m_videoMode);
		
		//-- init timing panel.
		INFO_MSG("Init timing panel ... ");
		if (!m_timingPanel->init())
		{
			BR_EXCEPT("Can't init timing panel.");
		}
		INFO_MSG("Init timing panel ... completed.");

		//-- init watchers panel.
		INFO_MSG("Init watchers panel ... ");
		if (!m_watchersPanel->init())
		{
			BR_EXCEPT("Can't init watchers panel.");
		}
		INFO_MSG("Init watchers panel ... completed.");

		//-- init input sub-system.
		INFO_MSG("Init input system ... ");
		if (!m_inputSystem.init(m_hWnd, hInstance))
		{
			BR_EXCEPT("Can't init input system.");
		}
		m_inputSystem.setListeners(this, this);
		INFO_MSG("Init input system ... completed.");

/*
		//-- init sound sub-system.
		LOG("Initialization sound system ... ");
		soundSystem = new SoundSystem();
		if(!soundSystem->initialize())
			throw brException("Initialization sound system ... failed");
		soundSystem->setVolume(5.5f);
		ssManager = soundSystem->createSSManager("");
		LOG("Initialization sound system ... completed.");
*/
		if (!m_physicWorld.init())
		{
			BR_EXCEPT("Can't init physic world.");
		}

		if (!m_renderWorld.init())
		{
			BR_EXCEPT("Can't init render world.");
		}

		if (!m_animEngine.init())
		{
			BR_EXCEPT("Can't init animation engine.");
		}

		if (!m_gameWorld.init())
		{
			BR_EXCEPT("Can't init game world.");
		}

		//-- init demo.
		INFO_MSG("Init demo ... ");
		m_demo.reset(demo);
		if (!m_demo->init())
		{
			BR_EXCEPT("Can't load a demo.");
		}
		INFO_MSG("Init demo ... completed.");

		INFO_MSG("Starting Message Loop...");
	}

	//------------------------------------------
	void Engine::shutdown()
	{
		//-- release demo first, because it may contain render resources.
		if (m_demo.get())
		{
			m_demo->shutdown();
			m_demo.reset();
		}
		
		//-- shutdown console.
		if (m_console.get())
		{
			m_console.reset();
		}

		//-- shutdown watchers panel.
		if (m_watchersPanel.get())
		{
			m_watchersPanel->destroy();
			m_watchersPanel.reset();
		}

		//-- shutdown timing panel.
		if (m_timingPanel.get())
		{
			m_timingPanel->destroy();
			m_timingPanel.reset();
		}

		m_gameWorld.fini();
		m_animEngine.fini();
		m_renderWorld.fini();
		m_physicWorld.fini();

		m_resManager.shutdown();

		//-- release render system.
		_releaseRenderSystem();
	}

	//------------------------------------------
	void Engine::handleMouseClick(const MouseEvent& me)
	{
		m_demo->handleMouseClick(me);
	}

	//------------------------------------------
	void Engine::handleMouseMove(const MouseAxisEvent& me)
	{
		m_demo->handleMouseMove(me);
	}

	//------------------------------------------
	void Engine::handleKeyboardEvent(const KeyboardEvent& ke)
	{
		if (ke.keyCode == DIK_GRAVE && ke.state == KS_DOWN)
		{
			m_console->visible(!m_console->visible());
			return;
		}
		else if (ke.keyCode == DIK_F1 && ke.state == KS_DOWN)
		{
			m_watchersPanel->visible(!m_watchersPanel->visible());
			return;
		}
		else if (ke.keyCode == DIK_F2 && ke.state == KS_DOWN)
		{
			m_timingPanel->visible(!m_timingPanel->visible());
			return;
		}

		if (m_console->visible() && m_console->handleKey(ke.keyCode, ke.state, ke.text))
			return;

		m_demo->handleKeyboardEvent(ke);
	}

	//------------------------------------------
	void Engine::updateFrame(float dt)
	{
		SCOPED_TIME_MEASURER_EX("update")
	
		m_demo->update(dt);
		m_watchersPanel->update(dt);
	}

	//-- ToDo: reconsider. Old stuff.
	//------------------------------------------
	void Engine::drawFrame(float dt)
	{
		SCOPED_TIME_MEASURER_EX("draw")

		m_demo->render(dt);
		m_console->draw();
		m_watchersPanel->draw(dt);
		
		//-- timing panel.
		{
			SCOPED_TIME_MEASURER_EX("timingPanel draw")
			m_timingPanel->draw(dt);
		}
	}
	
	//------------------------------------------
	void Engine::_fps()
	{
		static float currentFPS = 0;
		static int64 firstTime = m_timer.time();

		int64 temp = m_timer.time() - firstTime;
		if( temp >= 1000 )
		{
			m_fps = currentFPS - (float(temp) - 1000.0f) * currentFPS * 0.001f;
			currentFPS = 0;
			firstTime = m_timer.time();
		}

		++currentFPS;
	}

	//------------------------------------------
	int Engine::_exit()
	{
		ConPrint("Bye!");
		m_isWorking = false;
		return 0;
	}

	//------------------------------------------
	int Engine::_setMaxFPS(int fps)
	{
		m_maxFPS = fps;
		return 0;
	}
	
}/*end namespace brUGE*/
