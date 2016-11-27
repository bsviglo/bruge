#include "Engine.h"
#include "IDemo.h"
#include "Exception.h"
#include "utils/Thread.h"
#include "render/IRenderDevice.h"
#include "gui/imgui.h"

#include "render/render_system.hpp"
#include "render/render_world.hpp"
#include "render/animation_engine.hpp"
#include "render/game_world.hpp"
#include "render/physic_world.hpp"

#include "SDL/SDL.h"
#include "SDL/SDL_syswm.h"

using namespace brUGE;
using namespace brUGE::render;
using namespace brUGE::physic;
using namespace brUGE::utils;
using namespace brUGE::os;
using namespace brUGE::math;

extern bool g_needToStartApp;
extern brUGE::render::ERenderAPIType g_renderAPI;

namespace brUGE
{
	const char* const g_engineName = "black and red Unicorn Graphics Engine";

	DEFINE_SINGLETON(Engine)

	bool	Engine::m_isRunning = true;
	uint	Engine::m_maxFPS	= 60;  //-- ToDo: move to config.

	//------------------------------------------
	Engine::Engine() : 
		m_logManager("log", true, false),
		m_console(new Console()),
		m_watchersPanel(new WatchersPanel()),
		m_timingPanel(new TimingPanel()),
		m_gameWorld(new GameWorld()),
		m_animEngine(new AnimationEngine()),
		m_renderWorld(new RenderWorld()),
		m_resManager(new ResourcesManager()),
		m_physicWorld(new PhysicWorld())
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

		SDL_Quit();
	}

	//------------------------------------------
	void Engine::init(HINSTANCE hInstance, IDemo* demo)
	{
		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER);

		//-- ToDo: load this values from config
		m_videoMode = VideoMode(1024, 768);

		SDL_Window* window = SDL_CreateWindow(
			g_engineName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_videoMode.width, m_videoMode.height, 0);

		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);
		if (SDL_GetWindowWMInfo(window, &info))
		{
			m_hWnd = info.info.win.window;
		}

		m_hInstance = hInstance;
		
		//--
		ConError(g_engineName);

		if (!m_resManager->init())
		{
			BR_EXCEPT("Can't init resource system.");
		}
		INFO_MSG("Init resource system ... completed.");

		//-- init render sub-system.	
		if (!m_renderSys.init(g_renderAPI, m_hWnd, m_videoMode))
		{
			BR_EXCEPT("Can't init render system.");
		}
		INFO_MSG("Init render system ... completed.");

		//-- init console drawing mode.
		m_console->initDrawingMode(m_videoMode);
		
		//-- init timing panel.
		if (!m_timingPanel->init())
		{
			BR_EXCEPT("Can't init timing panel.");
		}
		INFO_MSG("Init timing panel ... completed.");

		//-- init watchers panel.
		if (!m_watchersPanel->init())
		{
			BR_EXCEPT("Can't init watchers panel.");
		}
		INFO_MSG("Init watchers panel ... completed.");

		//-- init input sub-system.
		if (!m_inputSystem.init(m_hWnd, hInstance))
		{
			BR_EXCEPT("Can't init input system.");
		}
		m_inputSystem.setListeners(this, this);
		INFO_MSG("Init input system ... completed.");

		if (!m_physicWorld->init())
		{
			BR_EXCEPT("Can't init physic world.");
		}

		if (!m_renderWorld->init())
		{
			BR_EXCEPT("Can't init render world.");
		}

		if (!m_animEngine->init())
		{
			BR_EXCEPT("Can't init animation engine.");
		}

		if (!m_gameWorld->init())
		{
			BR_EXCEPT("Can't init game world.");
		}

		//-- init demo.
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
		
		m_console.reset();
		m_watchersPanel.reset();
		m_timingPanel.reset();
		m_gameWorld.reset();
		m_animEngine.reset();
		m_renderWorld.reset();
		m_physicWorld.reset();
		m_resManager.reset();

		m_renderSys.shutDown();
	}

	//------------------------------------------
	int Engine::run()
	{
		uint64 newTime = SDL_GetPerformanceCounter();
		uint64 prevTime = SDL_GetPerformanceCounter();

		float dt = 0;
		MSG msg = { 0 };

		//-- do main cycle.
		while (m_isRunning)
		{
			// clamp engine tick time.
			newTime = SDL_GetPerformanceCounter();
			dt = (static_cast<float>(newTime - prevTime) / SDL_GetPerformanceFrequency());
			prevTime = newTime;

			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_QUIT)
				{
					m_isRunning = false;
					return 0;
				}
			}

			m_timingPanel->start();
			{
				//-- do tick.
				{
					SCOPED_TIME_MEASURER_EX("update")

					//-- update timing panel.
					m_timingPanel->update(dt);

				//-- update input.
				{
					SCOPED_TIME_MEASURER_EX("input")
					m_inputSystem.update();
				}

				//-- start updating imgui input.
				imguiBeginFrame(
					m_imguiInput.mx, m_imguiInput.my, m_imguiInput.button, m_imguiInput.scroll
				);

				m_gameWorld->beginUpdate(dt);

				//-- do pre-animation.
				{
					SCOPED_TIME_MEASURER_EX("pre-animation")
						m_animEngine->preAnimate(dt);
				}

				{
					SCOPED_TIME_MEASURER_EX("animation")
						m_animEngine->animate();
				}

				//-- simulate physics.
				{
					SCOPED_TIME_MEASURER_EX("physic")
						m_physicWorld->simulateDynamics(dt);
				}

				{
					SCOPED_TIME_MEASURER_EX("post-animation")
						m_animEngine->postAnimate();
				}

				m_renderWorld->update(dt);

				//-- ToDo: some old stuff.
				updateFrame(dt);

				//m_collisionWorld.update(dt);

				m_gameWorld->endUpdate();
				}

				//-- do draw.
				{
					SCOPED_TIME_MEASURER_EX("draw")

					m_renderSys.beginFrame();
					m_renderWorld->draw();
					//-- ToDo: some old stuff.
					drawFrame(dt);

					m_renderSys.endFrame();
				}

				//-- finish updating imgui input.
				imguiEndFrame();
				m_imguiInput.scroll = 0;
			}
			m_timingPanel->stop();
		}

		return 0;
	}

	//------------------------------------------
	void Engine::stop()
	{
		m_isRunning = false;
	}

	//------------------------------------------
	void Engine::handleMouseClick(const MouseEvent& me)
	{
		if		(me.button == MB_LEFT_BUTTON  && me.isDown) m_imguiInput.button = IMGUI_MBUT_LEFT;
		else if (me.button == MB_RIGHT_BUTTON && me.isDown) m_imguiInput.button = IMGUI_MBUT_RIGHT;
		else												m_imguiInput.button = 0;

		m_gameWorld->handleMouseClick(me);
		m_demo->handleMouseClick(me);
	}

	//------------------------------------------
	void Engine::handleMouseMove(const MouseAxisEvent& mae)
	{
		m_imguiInput.mx		= mae.absX;
		m_imguiInput.my		= Engine::instance().getVideoMode().height - mae.absY;
		m_imguiInput.scroll	= -static_cast<int>(clamp<float>(-100, mae.relZ, +100) / 5);

		m_gameWorld->handleMouseMove(mae);
		m_demo->handleMouseMove(mae);
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

		m_gameWorld->handleKeyboardEvent(ke);
		m_demo->handleKeyboardEvent(ke);
	}

	//------------------------------------------
	void Engine::updateFrame(float dt)
	{
		m_demo->update(dt);
		m_watchersPanel->update(dt);
	}

	//-- ToDo: reconsider. Old stuff.
	//------------------------------------------
	void Engine::drawFrame(float dt)
	{
		m_demo->render(dt);
		m_console->draw();
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
		m_isRunning = false;
		return 0;
	}

	//------------------------------------------
	int Engine::_setMaxFPS(int fps)
	{
		m_maxFPS = fps;
		return 0;
	}
	
}/*end namespace brUGE*/
