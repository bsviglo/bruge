#include "Engine.h"
#include "IDemo.h"
#include "Exception.h"
#include "render/IRenderDevice.h"
#include "gui/ui_system.hpp"

#include "render/render_system.hpp"
#include "render/render_world.hpp"
#include "render/animation_engine.hpp"
#include "scene/game_world.hpp"
#include "physics/physic_world.hpp"

#include "SDL/SDL.h"
#include "SDL/SDL_syswm.h"

using namespace brUGE;
using namespace brUGE::render;
using namespace brUGE::physics;
using namespace brUGE::utils;
using namespace brUGE::os;
using namespace brUGE::math;

namespace
{
	bool				isRunning			= true;
	bool				g_needToStartApp	= true;
	ERenderAPIType		g_renderAPI			= RENDER_API_D3D11;
	const char* const	g_engineName		= "black and red Unicorn Graphics Engine";
}

namespace brUGE
{

	class RootSystem : public ISystem
	{

	};


	DEFINE_SINGLETON(Engine)

	//--------------------------------------------------------------------------------------------------
	Engine::Engine() : 
		m_logManager("log", true, false),
		m_watchersPanel(new WatchersPanel()),
		m_timingPanel(new TimingPanel()),
		m_gameWorld(new GameWorld()),
		m_animEngine(new AnimationEngine()),
		m_renderWorld(new RenderWorld()),
		m_resManager(new ResourcesManager()),
		m_physicWorld(new PhysicsWorld()),
		m_uiSystem(new ui::System())
	{
		//-- register console commands.
		REGISTER_CONSOLE_METHOD("exit",			 _exit,		 Engine);
	}

	//--------------------------------------------------------------------------------------------------
	Engine::~Engine()
	{
		shutdown();
		SDL_Quit();
	}

	//--------------------------------------------------------------------------------------------------
	bool Engine::init(HINSTANCE, IDemo* demo)
	{
		//-- init system.
		auto& rootSystem = create<RootSystem>();
		{
			auto& resourceSystem	= create<ResourceSystem>();
			auto& physicsSystem		= create<PhysicsSystem>();
			auto& animationSystem	= create<AnimationSystem>();
			auto& renderSystem		= create<render::RenderSystem>();

			rootSystem.child(resourceSystem);
			rootSystem.child(physicsSystem);
			rootSystem.child(animationSystem);
			rootSystem.child(renderSystem);
			{
				auto& cameraSystem	= create<render::CameraSystem>();
				auto& cullingSystem = create<render::CullingSystem>();
				auto& meshSystem	= create<render::MeshSystem>();
				auto& shadowSystem	= create<render::ShadowSystem>();
				auto& lightSystem	= create<render::LightSystem>();

				renderSystem.child(cameraSystem);
				renderSystem.child(cullingSystem);
				renderSystem.child(meshSystem);
				renderSystem.child(shadowSystem);
				renderSystem.child(lightSystem);
				renderSystem.link(resourceSystem);

				meshSystem.link(cullingSystem);
				meshSystem.link(cameraSystem);
				meshSystem.link(resourceSystem);

				shadowSystem.link(cullingSystem);
				shadowSystem.link(cameraSystem);
				shadowSystem.link(resourceSystem);
				shadowSystem.link(meshSystem);
				shadowSystem.link(lightSystem);		
			}
		}
		
		//-- initialize hierarchically preserving order
		rootSystem.init();

		//--

		{
	
		}



		//-- ToDo: old


		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER);

		//-- ToDo: load this values from config
		m_videoMode = VideoMode(1024, 768);

		SDL_Window* window = SDL_CreateWindow(
			g_engineName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			m_videoMode.width, m_videoMode.height, SDL_WINDOW_MAXIMIZED | SDL_WINDOW_BORDERLESS
		);

		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);
		if (SDL_GetWindowWMInfo(window, &info))
		{
			m_hWnd = info.info.win.window;
			int32 w = 0, h = 0;
			SDL_GetWindowSize(window, &w, &h);

			m_videoMode.width = w;
			m_videoMode.height = h;
		}
		
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

		if (!m_uiSystem->init(m_videoMode))
		{
			BR_EXCEPT("Can't init ui system.");
		}
		INFO_MSG("Init ui system ... completed.");
		
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

	//--------------------------------------------------------------------------------------------------
	void Engine::shutdown()
	{
		//-- release demo first, because it may contain render resources.
		if (m_demo.get())
		{
			m_demo->shutdown();
			m_demo.reset();
		}
		
		m_uiSystem.reset();
		m_watchersPanel.reset();
		m_timingPanel.reset();
		m_gameWorld.reset();
		m_animEngine.reset();
		m_renderWorld.reset();
		m_physicWorld.reset();
		m_resManager.reset();

		m_renderSys.shutDown();
	}

	//--------------------------------------------------------------------------------------------------
	int Engine::run()
	{
		uint64 newTime = SDL_GetPerformanceCounter();
		uint64 prevTime = SDL_GetPerformanceCounter();

		//-- do main cycle.
		while (isRunning)
		{
			// calculate engine tick time.
			newTime = SDL_GetPerformanceCounter();
			float dt = max<float>(0.0001f, (static_cast<float>(newTime - prevTime) / SDL_GetPerformanceFrequency()));
			prevTime = newTime;

			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_WINDOWEVENT)
				{
					switch (event.window.event)
					{
					case SDL_WINDOWEVENT_FOCUS_LOST:
						break;
					case SDL_WINDOWEVENT_CLOSE:
						isRunning = false;
						break;
					default:
						break;
					}
				}
				else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
				{
					handleKeyboardEvent(event.key);
				}
				else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
				{
					handleMouseButtonEvent(event.button);
				}
				else if (event.type == SDL_MOUSEMOTION)
				{
					handleMouseMotionEvent(event.motion);
				}
				else if (event.type == SDL_MOUSEWHEEL)
				{
					handleMouseWheelEvent(event.wheel);
				}
				else if (event.type == SDL_TEXTINPUT)
				{
					handleTextInputEvent(event.text);
				}
				else if (event.type == SDL_QUIT)
				{
					isRunning = false;
				}
			}

			//--
			m_uiSystem->tick(dt);

			m_timingPanel->start();
			{
				//-- do tick.
				{
					SCOPED_TIME_MEASURER_EX("update")

					//-- update timing panel.
					m_timingPanel->update(dt);

					//-- update demo module first
					m_demo->update(dt);

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
						m_physicWorld->simulate(dt);
					}

					{
						SCOPED_TIME_MEASURER_EX("post-animation")
						m_animEngine->postAnimate();
					}

					m_renderWorld->update(dt);
					m_watchersPanel->update(dt);
					m_gameWorld->endUpdate();
				}

				//-- do draw.
				{
					SCOPED_TIME_MEASURER_EX("draw")

					m_renderSys.beginFrame();
					m_renderWorld->draw();
					m_demo->render(dt);
					displayStatistics(dt);
					m_uiSystem->draw();
					m_renderSys.endFrame();
				}
			}
			m_timingPanel->stop();
		}

		return 0;
	}

	//--------------------------------------------------------------------------------------------------
	void Engine::stop()
	{
		isRunning = false;
	}

	//--------------------------------------------------------------------------------------------------
	void Engine::handleMouseButtonEvent(const SDL_MouseButtonEvent& e)
	{
		m_uiSystem->handleMouseButtonEvent(e);
		m_gameWorld->handleMouseButtonEvent(e);
		m_demo->handleMouseButtonEvent(e);
	}

	//--------------------------------------------------------------------------------------------------
	void Engine::handleMouseMotionEvent(const SDL_MouseMotionEvent& e)
	{
		m_uiSystem->handleMouseMotionEvent(e);
		m_gameWorld->handleMouseMotionEvent(e);
		m_demo->handleMouseMotionEvent(e);
	}

	//--------------------------------------------------------------------------------------------------
	void Engine::handleMouseWheelEvent(const SDL_MouseWheelEvent& e)
	{
		m_uiSystem->handleMouseWheelEvent(e);
		m_gameWorld->handleMouseWheelEvent(e);
		m_demo->handleMouseWheelEvent(e);
	}

	//--------------------------------------------------------------------------------------------------
	void Engine::handleKeyboardEvent(const SDL_KeyboardEvent& e)
	{
		//if (e.keysym.sym == SDLK_BACKQUOTE && e.state == SDL_PRESSED)
		//{
		//	m_console->visible(!m_console->visible());
		//	return;
		//}
		//else if (e.keysym.sym == SDLK_F1 && e.state == SDL_PRESSED)
		//{
		//	m_watchersPanel->visible(!m_watchersPanel->visible());
		//	return;
		//}
		if (e.keysym.sym == SDLK_F2 && e.state == SDL_PRESSED)
		{
			m_timingPanel->visible(!m_timingPanel->visible());
			return;
		}

		//-- ToDo: will be substituted with new imgui library
		//if (m_console->visible() && m_console->handleKey(ke.keyCode, ke.state, ke.text))
		//	return;

		m_uiSystem->handleKeyboardEvent(e);
		m_gameWorld->handleKeyboardEvent(e);
		m_demo->handleKeyboardEvent(e);
	}

	//--------------------------------------------------------------------------------------------------
	void Engine::handleTextInputEvent(const SDL_TextInputEvent& e)
	{
		m_uiSystem->handleTextInputEvent(e);
	}

	//--------------------------------------------------------------------------------------------------
	int Engine::_exit()
	{
		ConPrint("Bye!");
		isRunning = false;
		return 0;
	}

	//--------------------------------------------------------------------------------------------------
	void Engine::displayStatistics(float dt)
	{
		bool enabled = true;

		ImGui::SetNextWindowPos(ImVec2(10, 10));
		if (!ImGui::Begin("Overlay", &enabled, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::End();
			return;
		}
		ImGui::Text("Statistics:");
		ImGui::Separator();
		ImGui::Text("FSP | TPF : %d | %.2f ms ", static_cast<uint>(1.0f / dt), dt * 1000);
		ImGui::Text("Draw calls: %d           ", m_renderSys.statistics().drawCallsCount);
		ImGui::Text("Primitives: %d k         ", m_renderSys.statistics().primitivesCount / 1000);
		ImGui::End();
	}
	
}/*end namespace brUGE*/
