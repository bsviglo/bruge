#include "engine/Engine.h"
#include "render/decal_manager.hpp"
#include "gui/imgui.h"

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
	uint random(uint maxValue)
	{
		return uint((rand() / float(RAND_MAX + 1)) * maxValue);
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
	//-- init camera.
	{
		Projection proj;
		proj.fov	  = 60.0f;
		proj.nearDist = 0.1f;
		proj.farDist  = 250.0f;

		m_camera = new FreeCamera(&proj);
	}

	Engine&			 engine     = Engine::instance();
	GameWorld&		 gameWorld  = engine.gameWorld();
	AnimationEngine& animEngine = engine.animationEngine();

	mat4f mat;
	mat.setIdentity();

	engine.renderWorld().setCamera(m_camera);
	
	//-- plane.
	gameWorld.addGameObj("resources/models/plane.xml", &mat);

	//-- test
	for (uint i = 0; i < 10; ++i)
	{
		for (uint j = 0; j < 10; ++j)
		{
			for (uint k = 0; k < 10; ++k)
			{
				mat.setTranslation(i * 5.f, k * 5.f, j * 5.f);
				mat.postTranslation(-15, 25, -15);
				gameWorld.addGameObj("resources/models/metalbox1.xml", &mat);
			}
		}
	}

	for (uint i = 0; i < 5; ++i)
	{
		for (uint j = 0; j < 5; ++j)
		{
			for (uint k = 0; k < 5; ++k)
			{
				mat.setTranslation(i * 3.f, k * 3.f, j * 3.f);
				mat.postTranslation(-5, 50, -5);
				gameWorld.addGameObj("resources/models/exp_barrel.xml", &mat);
			}
		}
	}


	//mat.setScale(0.1f, 0.1f, 0.1f);
	mat.setIdentity();
	mat.postRotateX(degToRad(-90.0f));
	mat.postTranslation(-25.0f, 0.5f, 0.0f);
	//mat.setIdentity();
	Handle playerID  = gameWorld.addGameObj("resources/models/player.xml", &mat);
	IGameObj* player = gameWorld.getGameObj(playerID);

	//-- loop idle animation.
	animEngine.playAnim(player->animCtrl(), "idle", true);

	/*
	const char* anims[] = 
	{
		"idle", "run", "pain", "fall", "crouch", "cheer", "grab_a", "grab_b", "jump"
	};

	for (uint i = 0; i < 10; ++i)
	{
		for (uint j = 0; j < 10; ++j)
		{
			mat.setIdentity();
			mat.postRotateX(degToRad(-90.0f));
			mat.postTranslation(i * 4, 0, 4 * j);

			Handle playerID  = gameWorld.addGameObj("resources/models/player.xml", &mat);
			IGameObj* player = gameWorld.getGameObj(playerID);

			//-- loop idle animation.
			animEngine.playAnim(player->animCtrl(),	anims[random(array_size(anims))] , true);
		}
	}
	*/

	rs().postProcessing()->enable("ssaa.pp");

	return true;
}

//-------------------------------------------------------------------------------------------------
void Demo::shutdown()
{

}

//-------------------------------------------------------------------------------------------------
void Demo::update(float dt)
{
	m_imguiActive = InputManager::instance().isKeyDown(DIK_LCONTROL);

	//-- draw gui.
	//gui();

	if (!m_imguiActive)
	{
		m_camera->update(true, dt);
	}

	for (uint i = 0; i < m_collisionDescs.size(); ++i)
	{
		m_collisions[i] = m_collisionDescs[i].first;
		m_collisions[i].postMultiply(m_collisionDescs[i].second->matrix());
	}

	if (!m_imguiActive)
	{
		if (m_cursorVisible)
		{
			m_cursorVisible = false;
			ShowCursor(FALSE);
		}

		//-- set cursor on the center of the window.
		ScreenResolution sr = render::rs().screenRes();
		SetCursorPos(sr.width / 2, sr.height / 2);
	}
	else
	{
		if (!m_cursorVisible)
		{
			m_cursorVisible = true;
			ShowCursor(TRUE);
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

	m_imguiActive = InputManager::instance().isKeyDown(DIK_LCONTROL);

	if (imguiBeginScrollArea("Properties", width-250-10, height-10-450, 250, 450, &propScroll))
		m_imguiActive = true;

	for (uint i = 0; i < 20; ++i)
	{
		imguiSeparatorLine();
		imguiSlider("Slider", &slider[i], 0.0f, 20.0f, 0.01f);
	}

	if (imguiCheck("Show Log", showLog))
		showLog = !showLog;
	if (imguiCheck("Show Tools", showTools))
		showTools = !showTools;

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

	for (uint i = 0; i < m_collisions.size(); ++i)
		DebugDrawer::instance().drawCoordAxis(m_collisions[i], 0.125f);

	mat4f mat;
	for (uint i = 0; i < 4; ++i)
	{
		mat.setTranslation(i * 3.f, 3.f, 3.f);
		mat.postTranslation(-5, 10, -5);

		DebugDrawer::instance().drawCylinderY(1.5f, 2.0f, mat, Color(1,1,0,1));
		DebugDrawer::instance().drawCoordAxis(mat, 0.25f);
	}

	for (uint i = 0; i < 4; ++i)
	{
		mat.setTranslation(13.f, i * 3.f, 3.f);
		mat.postTranslation(-5, 10, -5);

		DebugDrawer::instance().drawSphere(1.5f, mat, Color(1,1,0,1));
		DebugDrawer::instance().drawCoordAxis(mat, 0.25f);
	}

	for (uint i = 0; i < 4; ++i)
	{
		mat.setTranslation(23.f, 13.f, i * 3.f);
		mat.postTranslation(-5, 10, -5);

		DebugDrawer::instance().drawBox(vec3f(1,2,3), mat, Color(1,1,0,1));
		DebugDrawer::instance().drawCoordAxis(mat, 0.25f);
	}
}

//-------------------------------------------------------------------------------------------------
bool Demo::handleMouseClick(const MouseEvent& me)
{
	if (m_imguiActive) return true;

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

	return false;
}

//-------------------------------------------------------------------------------------------------
bool Demo::handleMouseMove(const MouseAxisEvent& mae)
{
	if (m_imguiActive) return true;

	m_camera->updateMouse(mae.relX, mae.relY, mae.relZ);
	return true;
}

//-------------------------------------------------------------------------------------------------
bool Demo::handleKeyboardEvent(const KeyboardEvent& /*ke*/)
{
	return false;
}
