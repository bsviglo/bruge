#include "Editor.hpp"
#include "console/Console.h"
#include "math/Matrix4x4.hpp"
#include "math/math_funcs.hpp"
#include "render/render_common.h"
#include "engine/Engine.h"
#include "render/render_world.hpp"
#include "render/animation_engine.hpp"
#include "render/physic_world.hpp"
#include "render/DebugDrawer.h"
#include "render/light_manager.hpp"
#include "render/game_world.hpp"
#include "os/FileSystem.h"
#include "gui/imgui.h"

using namespace brUGE::os;

//-- start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{

}
//--------------------------------------------------------------------------------------------------
//-- end unnamed namespace.

//--------------------------------------------------------------------------------------------------
Editor::Editor() : m_sunLight(CONST_INVALID_HANDLE), m_gameObj(CONST_INVALID_HANDLE), m_guiActive(true),
	m_numFrames(0), m_looped(false), m_stepped(false), m_activeSkinModel(false), m_curFrame(0), m_animCtrl(CONST_INVALID_HANDLE)
{

}

//--------------------------------------------------------------------------------------------------
Editor::~Editor()
{

}

//--------------------------------------------------------------------------------------------------
bool Editor::init()
{
	Engine&	   engine     = Engine::instance();
	GameWorld& gameWorld  = engine.gameWorld();

	//-- add light.
	{
		m_sunAngles.set(0.0f, 60.0f);

		mat4f sunCam;
		sunCam.setRotateX(degToRad(m_sunAngles.y));
		sunCam.postRotateY(degToRad(m_sunAngles.x));

		DirectionLight light;
		light.m_dir   = sunCam.applyToUnitAxis(mat4f::Z_AXIS);
		light.m_color = Color(1,1,1,1);

		m_sunLight = engine.renderWorld().lightsManager().addDirLight(light);
	}

	//-- add plane
	{
		mat4f mat;
		mat.setScale(1.0f, 10.0f, 1.0f);

		//-- add plane.
		gameWorld.addGameObj("resources/models/plane.xml", &mat);
	}

	//-- add test barrel
	{
		mat4f mat;
		mat.setIdentity();
		m_gameObj = gameWorld.addGameObj("resources/models/barrel.xml", &mat);
	}

	m_xyz.set(degToRad(45.0f), 0.0f, 5.0f);

	m_target.setRotateX(m_xyz.x);
	m_target.postRotateY(m_xyz.y);

	m_source.setTranslation(vec3f(0.0f, 0.0f, -m_xyz.z));
	m_source.postMultiply(m_target);

	Projection proj;
	proj.fov	  = 60.0f;
	proj.nearDist = 0.1f;
	proj.farDist  = 100.0f;

	m_camera = new CursorCamera(proj);
	m_camera->source(&m_source);
	m_camera->target(&m_target);
	m_camera->update(true, 0.0f);

	engine.renderWorld().setCamera(m_camera);

	//-- create UI.
	m_ui.reset(new UI(*this));

	return true;
}

//--------------------------------------------------------------------------------------------------
bool Editor::loadGameObj(const std::string& name)
{
	mat4f transform;
	transform.setIdentity();

	//-- release previous game object before loading a new one.
	if (m_gameObj != CONST_INVALID_HANDLE)
	{
		Engine::instance().gameWorld().delGameObj(m_gameObj);
	}

	m_objName = name;
	//transform.postRotateX(degToRad(-90.0f));

	std::string objName = "resources/models/" + name + ".xml";
	m_gameObj = Engine::instance().gameWorld().addGameObj(objName.c_str(), &transform);

	//-- update status.
	m_animCtrl        = Engine::instance().gameWorld().getGameObj(m_gameObj)->animCtrl();
	m_activeSkinModel = (m_animCtrl != CONST_INVALID_HANDLE);

	if (m_animCtrl != CONST_INVALID_HANDLE)
	{
		loadAnimation("initial");
	}
	
	return true;
}

//--------------------------------------------------------------------------------------------------
bool Editor::loadAnimation(const std::string& name)
{
	bool looped = (name == "initial") ? false : m_looped;

	m_animName = m_objName+ "/" + name;

	Engine::instance().animationEngine().stopAnim(m_animCtrl);
	Engine::instance().animationEngine().playAnim(m_animCtrl, m_animName.c_str(), looped);

	m_curFrame  = 0;
	m_numFrames = Engine::instance().animationEngine().getAnim(m_animName.c_str())->numFrames();

	return true;
}

//--------------------------------------------------------------------------------------------------
void Editor::shutdown()
{

}

//--------------------------------------------------------------------------------------------------
void Editor::update(float dt)
{
	//-- update camera.
	if (!m_guiActive)
	{
		m_target.setRotateX(m_xyz.x);
		m_target.postRotateY(m_xyz.y);

		m_source.setTranslation(vec3f(0.0f, 0.0f, -m_xyz.z));
		m_source.postMultiply(m_target);

		m_camera->update(true, dt);
	}

	//-- update UI.
	m_ui->update();

	{
		LightsManager& lm = Engine::instance().renderWorld().lightsManager();

		mat4f sunCam;
		sunCam.setRotateX(degToRad(m_sunAngles.y));
		sunCam.postRotateY(degToRad(m_sunAngles.x));

		DirectionLight l = lm.getDirLight(m_sunLight);
		l.m_dir = sunCam.applyToUnitAxis(mat4f::Z_AXIS);
		lm.updateDirLight(m_sunLight, l);
	}
}

//--------------------------------------------------------------------------------------------------
void Editor::render(float dt)
{
	mat4f origin;
	origin.setIdentity();
	DebugDrawer::instance().drawCoordAxis(origin, 0.5f);

	origin.setTranslation(0.0f, 0.5f, 2.0f);
	DebugDrawer::instance().drawBox(vec3f(0.2f, 1.0f, 0.2f), origin, Color(1,0,0,1));

	origin.setTranslation(0.0f, 1.0f, 2.5f);
	DebugDrawer::instance().drawBox(vec3f(0.2f, 2.0f, 0.2f), origin, Color(0,1,0,1));

	origin.setTranslation(0.5f, 0.5f, 2.0f);
	DebugDrawer::instance().drawCylinderY(0.1f, 1.0f, origin, Color(1,0,0,1));

	origin.setTranslation(0.5f, 1.0f, 2.5f);
	DebugDrawer::instance().drawCylinderY(0.1f, 2.0f, origin, Color(0,1,0,1));

	origin.setTranslation(1.5f, 0.5f, 2.0f);
	DebugDrawer::instance().drawCapsuleY(0.1f, 1.0f, origin, Color(1,0,0,1));

	static float angle = 0.0f;
	angle += dt;
	origin.setTranslation(1.5f, 1.0f, 2.5f);
	origin.preRotateZ(angle);
	DebugDrawer::instance().drawCapsuleY(0.1f, 2.0f, origin, Color(0,1,0,1));

	origin.setTranslation(2.5f, 0.5f, 2.0f);
	DebugDrawer::instance().drawCapsuleY(0.1f, 1.0f, origin, Color(1,0,0,0.5f), DebugDrawer::DRAW_TRANSPARENT);

	origin.setTranslation(2.5f, 1.0f, 2.5f);
	DebugDrawer::instance().drawCapsuleY(0.1f, 2.0f, origin, Color(0,1,0,0.5f), DebugDrawer::DRAW_TRANSPARENT);

	DebugDrawer::instance().drawLine(vec3f(1.0f, 0.0f, 2.0f), vec3f(1.0f, 1.0f, 2.0f), Color(1,0,0,1));
	DebugDrawer::instance().drawLine(vec3f(1.0f, 0.0f, 2.5f), vec3f(1.0f, 2.0f, 2.5f), Color(0,1,0,1));

	origin.setTranslation(-2.5f, 0.5f, 2.0f);
	DebugDrawer::instance().drawSphere(0.5f, origin, Color(1,0,0,1));

	origin.setTranslation(-2.5f, 1.0f, 3.0f);
	DebugDrawer::instance().drawSphere(1.0f, origin, Color(0,1,0,1));
}

//--------------------------------------------------------------------------------------------------
bool Editor::handleMouseButtonEvent(const SDL_MouseButtonEvent& e)
{
	if (e.button == SDL_BUTTON_RIGHT)
		m_guiActive = (e.state != SDL_PRESSED); 

	return false;
}

//--------------------------------------------------------------------------------------------------
bool Editor::handleMouseMotionEvent(const SDL_MouseMotionEvent& e)
{
	if (m_guiActive)
		return true;

	float mouseAccel = 5.0f;
	float mouseSens  = 0.0125f;

	m_xyz.x += clamp<float>(-mouseAccel, e.yrel, mouseAccel) * mouseSens;
	m_xyz.y += clamp<float>(-mouseAccel, e.xrel, mouseAccel) * mouseSens;

	//-- clamp angles.
	m_xyz.x = clamp<float>(0.1f, m_xyz.x, (float)+PI * 0.5f);

	if (m_xyz.y < -PI_2)	m_xyz.y -= -PI_2;
	if (m_xyz.y > +PI_2)	m_xyz.y -= +PI_2;

	return true;
}

//--------------------------------------------------------------------------------------------------
bool Editor::handleMouseWheelEvent(const SDL_MouseWheelEvent& e)
{
	if (m_guiActive)
		return true;

	m_xyz.z -= e.y;
	m_xyz.z = clamp<float>(1.0f, m_xyz.z, 100.0f);

	return true;
}

//--------------------------------------------------------------------------------------------------
bool Editor::handleKeyboardEvent(const SDL_KeyboardEvent& /*e*/)
{
	return false;
}

//--------------------------------------------------------------------------------------------------
Editor::UI::UI(Editor& editor) : m_self(editor), m_scroll(0)
{

}

//--------------------------------------------------------------------------------------------------
Editor::UI::~UI()
{

}

//--------------------------------------------------------------------------------------------------
void Editor::UI::update()
{
	AnimationEngine& animEngine = Engine::instance().animationEngine();

	uint width	= rs().screenRes().width;
	uint height = rs().screenRes().height;

	imguiBeginScrollArea("Shadows", width-300-10, 10, 300, height-20, &m_scroll);
	{
		imguiSeparatorLine();
		imguiLabel("Models:");
		{
			imguiIndent();
			if (imguiButton("Load"))
			{
				m_gameObjsList.m_enabled = !m_gameObjsList.m_enabled;
				if (m_gameObjsList.m_enabled)
				{
					m_gameObjsList.m_list.clear();
					FileSystem::instance().getFilesInDir(
						"..\\resources\\models", m_gameObjsList.m_list, "xml", true
						);
				}
			}
			imguiUnindent();
		}

		imguiSeparatorLine();
		imguiLabel("Animations:");
		{
			imguiIndent();
			if (imguiButton("Load", m_self.m_activeSkinModel))
			{
				m_animList.m_enabled = !m_animList.m_enabled;
				if (m_animList.m_enabled)
				{
					m_animList.m_list.clear();
					FileSystem::instance().getFilesInDir(
						"..\\resources\\models\\" + m_self.m_objName, m_animList.m_list, "animation", true
						);
				}
			}
			if (imguiCheck("looped", &m_self.m_looped, m_self.m_activeSkinModel))
			{
				animEngine.stopAnim(m_self.m_animCtrl);
				animEngine.playAnim(m_self.m_animCtrl, m_self.m_animName.c_str(), m_self.m_looped);
			}

			if (imguiCheck("stepped", &m_self.m_stepped, m_self.m_activeSkinModel))
			{
				if (m_self.m_stepped)
					animEngine.pauseAnim(m_self.m_animCtrl);
				else
					animEngine.continueAnim(m_self.m_animCtrl);
			}

			if (imguiSlider("frames", &m_self.m_curFrame, 0.0f, m_self.m_numFrames, 1.0f, m_self.m_activeSkinModel && m_self.m_stepped))
			{
				animEngine.goToAnim(m_self.m_animCtrl, m_self.m_curFrame);
			}

			imguiUnindent();
		}

		imguiSeparatorLine();
		imguiLabel("Sun:");
		{
			imguiIndent();
			imguiSlider("sun yaw angle", &m_self.m_sunAngles.x, 0.0f, 360.0f, 0.5f);
			imguiSlider("sun pitch angle", &m_self.m_sunAngles.y, 0.0f, 90.0f, 0.5f);
			imguiUnindent();
		}

		imguiLabel("Material:");
		{
			imguiIndent();
			imguiUnindent();
		}
		imguiLabel("Physics:");
		{
			static float mass = 0.0f;

			imguiIndent();
			imguiButton("choose node...");
			imguiSlider("mass, kg", &mass, 0.0f, 500.0f, 0.1f);
			imguiButton("choose shape type...");
			{
				imguiIndent();
				imguiUnindent();
			}
			imguiUnindent();
		}
	}
	imguiEndScrollArea();

	//-- display combo box.
	displayGameObjs();
	displayAnimation();
}

//--------------------------------------------------------------------------------------------------
void Editor::UI::displayGameObjs()
{
	if (m_gameObjsList.m_enabled)
	{
		uint width	= rs().screenRes().width;
		uint height = rs().screenRes().height;

		imguiBeginScrollArea("Effects", width-520, height-10-250, 200, 250, &m_gameObjsList.m_scroll);

		for (uint i = 0; i < m_gameObjsList.m_list.size(); ++i)
		{
			const char* name = m_gameObjsList.m_list[i].c_str();

			if (imguiItem(name))
			{
				m_self.loadGameObj(name);
				m_gameObjsList.m_enabled = false;
			}
		}
		imguiEndScrollArea();
	}
}

//--------------------------------------------------------------------------------------------------
void Editor::UI::displayAnimation()
{
	if (m_animList.m_enabled)
	{
		uint width	= rs().screenRes().width;
		uint height = rs().screenRes().height;

		imguiBeginScrollArea("Effects", width-520, height-10-250, 200, 250, &m_animList.m_scroll);

		for (uint i = 0; i < m_animList.m_list.size(); ++i)
		{
			const char* name = m_animList.m_list[i].c_str();

			if (imguiItem(name))
			{
				m_self.loadAnimation(name);
				m_animList.m_enabled = false;
			}
		}
		imguiEndScrollArea();
	}
}

