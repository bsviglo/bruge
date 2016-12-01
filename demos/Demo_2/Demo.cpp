#include "engine/Engine.h"
#include "render/game_world.hpp"
#include "render/decal_manager.hpp"
#include "render/DebugDrawer.h"
#include "render/light_manager.hpp"
#include "render/render_world.hpp"
#include "render/animation_engine.hpp"
#include "render/physic_world.hpp"
#include "render/post_processing.hpp"
#include "render/terrain_system.hpp"
#include "gui/imgui.h"
#include "Player.hpp"
#include "Zombie.hpp"

//-- because inside this file used 'using' declaration, including this file as the last, must
//-- prevent negative influence on other include files.
#include "Demo.h"

using namespace brUGE::render;
using namespace brUGE::physic;
using namespace brUGE::utils;

// start unnamed namespace
//-------------------------------------------------------------------------------------------------
namespace
{
	//---------------------------------------------------------------------------------------------
	float random()
	{
		return rand() / float(RAND_MAX + 1);
	}

	//----------------------------------------------------------------------------------------------
	int random(int maxValue)
	{
		return int((rand() / float(RAND_MAX + 1)) * maxValue);
	}

	//---------------------------------------------------------------------------------------------
	template<typename Type, size_t size>
	inline size_t array_size(Type (&) [size]) { return size; }

}
//-------------------------------------------------------------------------------------------------
// end unnamed namespace

//-------------------------------------------------------------------------------------------------
Demo::Demo() : m_imguiActive(false), m_cursorVisible(true)
{

}

//-------------------------------------------------------------------------------------------------
Demo::~Demo()
{

}

//-------------------------------------------------------------------------------------------------
bool Demo::init()
{
	Engine&			 engine     = Engine::instance();
	GameWorld&		 gameWorld  = engine.gameWorld();

	//-- add light.
	{
		DirectionLight light;
		light.m_dir   = vec3f(0.5, -0.5, 0.5).getNormalized();
		light.m_color = Color(1,1,1,1);

		/*Handle lightID = */engine.renderWorld().lightsManager().addDirLight(light);
	}

	//--- create player object and camera.
	{
		Player* player = new Player();

		mat4f mat;
		mat.setIdentity();
		//mat.postRotateX(degToRad(-90.0f));

		gameWorld.addPlayer(player, "resources/models/player.xml", &mat);
	}
	
	//-- create another content
	{
		mat4f mat;
		mat.setIdentity();

		//-- add plane.
		gameWorld.addGameObj("resources/models/plane.xml", &mat);

		for (uint i = 0; i < 5; ++i)
		{
			for (uint j = 0; j < 5; ++j)
			{
				for (uint k = 0; k < 5; ++k)
				{
					mat.setTranslation(i * 3.f, k * 3.f, j * 3.f);
					mat.postTranslation(-5, 50, -5);
					gameWorld.addGameObj("resources/models/barrel.xml", &mat);
				}
			}
		}

		//-- test
		for (uint i = 0; i < 5; ++i)
		{
			for (uint j = 0; j < 5; ++j)
			{
				for (uint k = 0; k < 5; ++k)
				{
					mat.setTranslation(i * 5.f, k * 5.f, j * 5.f);
					mat.postTranslation(-15, 25, -15);
					gameWorld.addGameObj("resources/models/metalbox.xml", &mat);
				}
			}
		}

		for (uint i = 0; i < 100; ++i)
		{
			mat.setIdentity();
			mat.setRotateY(random() * 6.24f);
			mat.postTranslation(-random(100), 0.0f, -random(100));
			mat.postTranslation(random(100), 0, random(100));
			gameWorld.addGameObj("resources/models/palm.xml", &mat);
		}

/*
		for (uint i = 0; i < 85; ++i)
		{
			mat.setIdentity();
			mat.setRotateY(random() * 6.24f);
			mat.postTranslation(-random(100), 0.0f, -random(100));
			mat.postTranslation(random(100), 0, random(100));
			gameWorld.addGameObj("resources/models/date_palm.xml", &mat);
		}

		for (uint i = 0; i < 55; ++i)
		{
			mat.setIdentity();
			mat.setRotateY(random() * 6.24f);
			mat.postTranslation(-random(100), 0.0f, -random(100));
			mat.postTranslation(random(100), 0, random(100));
			gameWorld.addGameObj("resources/models/pole.xml", &mat);
		}

		for (uint i = 0; i < 55; ++i)
		{
			mat.setIdentity();
			mat.setRotateY(random() * 6.24f);
			mat.postTranslation(-random(100), 0.0f, -random(100));
			mat.postTranslation(random(100), 0, random(100));
			gameWorld.addGameObj("resources/models/woodGate.xml", &mat);
		}
*/

		for (uint i = 0; i < 250; ++i)
		{
			mat.setIdentity();
			//mat.postRotateX(degToRad(-90.0f));
			//mat.postRotateY(random() * 6.24f);
			mat.postTranslation(-random(100), 0.0f, -random(100));
			mat.postTranslation(random(100), 0, random(100));

			gameWorld.addGameObj(new Zombie(), "resources/models/zfat.xml", &mat);
		}
	}

	engine.renderWorld().postProcessing().enable("ssaa.pp");

	return true;
}

//-------------------------------------------------------------------------------------------------
void Demo::shutdown()
{

}

//-------------------------------------------------------------------------------------------------
void Demo::update(float /*dt*/)
{
	m_imguiActive = SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_LCTRL];

	//-- draw gui.
	//gui();

	for (uint i = 0; i < m_collisionDescs.size(); ++i)
	{
		m_collisions[i] = m_collisionDescs[i].first;
		m_collisions[i].postMultiply(m_collisionDescs[i].second->matrix());
	}

	if (!m_imguiActive)
	{
		if (m_cursorVisible)
		{
			SDL_SetRelativeMouseMode(SDL_TRUE);
			m_cursorVisible = false;
		}
	}
	else
	{
		if (!m_cursorVisible)
		{
			SDL_SetRelativeMouseMode(SDL_FALSE);
			m_cursorVisible = true;
		}
	}

	
}

//-------------------------------------------------------------------------------------------------
void Demo::gui()
{
	static uint width = Engine::instance().getVideoMode().width;
	static uint height = Engine::instance().getVideoMode().height;
	static int  propScroll = 0;
	static bool showLog = false;
	static bool showTools = false;
	static float slider[20] = {3.5f, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

	m_imguiActive = SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_LCTRL];

	if (imguiBeginScrollArea("Properties", width-250-10, height-10-450, 250, 450, &propScroll))
		m_imguiActive = true;

	for (uint i = 0; i < 20; ++i)
	{
		imguiSeparatorLine();
		imguiSlider("Slider", &slider[i], 0.0f, 20.0f, 0.01f);
	}

	imguiCheck("Show Log", &showLog);
	imguiCheck("Show Tools", &showTools);

	imguiSeparator();
	imguiLabel("Sample");
	if (imguiButton("Hello BronX!"))
	{
	
	}

	imguiSeparator();
	imguiLabel("Input Mesh");
	imguiButton("Hello again BronX!!!");

	imguiEndScrollArea();
}

//-------------------------------------------------------------------------------------------------
void Demo::render(float /*dt*/)
{
	mat4f indentity;
	indentity.setIdentity();
	DebugDrawer::instance().drawCoordAxis(indentity);
}

//-------------------------------------------------------------------------------------------------
bool Demo::handleMouseButtonEvent(const SDL_MouseButtonEvent&)
{
	if (m_imguiActive) return true;

	/*
	if (me.button == MB_LEFT_BUTTON && me.isDown)
	{
		const PhysicWorld& physWorld = Engine::instance().physicWorld();
		mat4f localMat;
		Node* node = nullptr;
		if (physWorld.collide(localMat, node, m_camera->position(), m_camera->direction()))
		{
			DecalManager& decalManager = Engine::instance().renderWorld().decalManager();

			decalManager.addDynamicDecal(localMat, vec3f(0.25f,0.25f,0.6f), node);

			m_collisionDescs.push_back(std::make_pair(localMat, node));
			ConPrint("add new collision #%d", m_collisionDescs.size());
			m_collisions.resize(m_collisionDescs.size());
		}
	}
	*/

	return false;
}

//-------------------------------------------------------------------------------------------------
bool Demo::handleMouseMotionEvent(const SDL_MouseMotionEvent&)
{
	if (m_imguiActive) return true;

	return true;
}

//-------------------------------------------------------------------------------------------------
bool Demo::handleMouseWheelEvent(const SDL_MouseWheelEvent&)
{
	return false;
}

//-------------------------------------------------------------------------------------------------
bool Demo::handleKeyboardEvent(const SDL_KeyboardEvent&)
{
	return false;
}