#include "Engine.h"
#include "IDemo.h"
#include "Exception.h"
#include "render/IRenderDevice.h"
#include "gui/ui_system.hpp"

#include "render/render_system.hpp"
#include "render/render_world.hpp"
#include "render/animation_engine.hpp"
#include "scene/game_world.hpp"

#include "Universe.hpp"

//--
#include "input/input_system.hpp"
#include "loader/ResourcesManager.h"
#include "physics/physic_world.hpp"
#include "render/render_system.hpp"
#include "render/mesh_manager.hpp"
#include "render/light_manager.hpp"
#include "render/culling_system.hpp"
#include "render/shadow_manager.hpp"

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

	//------------------------------------------------------------------------------------------------------------------
	Engine::Engine() 
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	Engine::~Engine()
	{
		//-- reverse de-initialization
		for (auto s = m_systemInitOrder.rbegin(); s != m_systemInitOrder.rend(); ++s)
			m_systems[*s].reset();

		SDL_Quit();
	}

	//------------------------------------------------------------------------------------------------------------------
	bool Engine::init(HINSTANCE, IDemo* demo)
	{
		bool ok = true;

		//-- init systems
		auto& fileSystem		= create<FileSystem>();
		auto& resourceSystem	= create<ResourceSystem>();
		auto& inputSystem		= create<InputSystem>();
		auto& physicsSystem		= create<PhysicsSystem>();
		auto& animationSystem	= create<AnimationSystem>();
		auto& uiSystem			= create<ui::UISystem()>();

		auto& renderSystem		= create<render::RenderSystem>();
		auto& cameraSystem		= create<render::CameraSystem>();
		auto& cullingSystem		= create<render::CullingSystem>();
		auto& meshSystem		= create<render::MeshSystem>();
		auto& shadowSystem		= create<render::ShadowSystem>();
		auto& lightSystem		= create<render::LightSystem>();

		//-- actual order dependent init
		for (auto typeID : m_systemInitOrder)
		{
			ok &= m_systems[typeID]->init();
		}

		//-- init universe
		m_universe = std::make_unique<Universe>();


		//-- ToDo: consider moving SDL init + window management into separate system.
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
		
		return ok;
	}

	//------------------------------------------------------------------------------------------------------------------
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

			//-- ToDo: consider moving this stuff into WindowSystem
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
			for (auto& context : m_universe->contexts())
			{
				process(*context.get());
			}
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------------------------
	void Engine::process(Universe::Context& context)
	{
		//-- ToDo: process each individual context of the universe. Each context may be associated with one world.
		//--	   So here we may have number of different world to be processed.


	}

	//------------------------------------------------------------------------------------------------------------------
	void Engine::stop()
	{
		isRunning = false;
	}

	//------------------------------------------------------------------------------------------------------------------
	void Engine::handleMouseButtonEvent(const SDL_MouseButtonEvent& e)
	{
		m_uiSystem->handleMouseButtonEvent(e);
		m_gameWorld->handleMouseButtonEvent(e);
		m_demo->handleMouseButtonEvent(e);
	}

	//------------------------------------------------------------------------------------------------------------------
	void Engine::handleMouseMotionEvent(const SDL_MouseMotionEvent& e)
	{
		m_uiSystem->handleMouseMotionEvent(e);
		m_gameWorld->handleMouseMotionEvent(e);
		m_demo->handleMouseMotionEvent(e);
	}

	//------------------------------------------------------------------------------------------------------------------
	void Engine::handleMouseWheelEvent(const SDL_MouseWheelEvent& e)
	{
		m_uiSystem->handleMouseWheelEvent(e);
		m_gameWorld->handleMouseWheelEvent(e);
		m_demo->handleMouseWheelEvent(e);
	}

	//------------------------------------------------------------------------------------------------------------------
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

	//------------------------------------------------------------------------------------------------------------------
	void Engine::handleTextInputEvent(const SDL_TextInputEvent& e)
	{
		m_uiSystem->handleTextInputEvent(e);
	}

	//------------------------------------------------------------------------------------------------------------------
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
