#include "Editor.hpp"
#include "math/Matrix4x4.hpp"
#include "math/math_funcs.hpp"
#include "render/render_common.h"
#include "engine/Engine.h"
#include "render/render_world.hpp"
#include "render/animation_engine.hpp"
#include "render/DebugDrawer.h"
#include "render/light_manager.hpp"
#include "scene/game_world.hpp"
#include "physics/physic_world.hpp"
#include "os/FileSystem.h"
#include "gui/imgui/imgui.h"

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
		m_sunAngles.set(0.0f, degToRad(60.0f));

		mat4f sunCam;
		sunCam.setRotateX(m_sunAngles.y);
		sunCam.postRotateY(m_sunAngles.x);

		DirectionLight light;
		light.m_dir   = sunCam.applyToUnitAxis(mat4f::Z_AXIS);
		light.m_color = Color(1,1,1,1);

		m_sunLight = engine.renderWorld().lightsManager().addDirLight(light);
	}

	//-- add plane
	{
		mat4f mat;
		mat.setIdentity();

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

	m_cursorCamera = std::make_shared<CursorCamera>(proj);
	m_cursorCamera->source(&m_source);
	m_cursorCamera->target(&m_target);
	m_cursorCamera->update(true, 0.0f);

	m_freeCamera = std::make_shared<FreeCamera>(proj);
	m_freeCamera->init(m_source.applyToOrigin());
	m_freeCamera->update(true, 0.0f);

	//-- set active camera.
	switchCamera();

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

		m_activeCamera->update(true, dt);
	}

	//-- update UI.
	m_ui->update();

	{
		LightsManager& lm = Engine::instance().renderWorld().lightsManager();

		mat4f sunCam;
		sunCam.setRotateX(m_sunAngles.y);
		sunCam.postRotateY(m_sunAngles.x);

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
	DebugDrawer::instance().drawCylinder(0.1f, 0.5f, origin, Color(1,0,0,1));

	origin.setTranslation(0.5f, 1.0f, 2.5f);
	DebugDrawer::instance().drawCylinder(0.1f, 1.0f, origin, Color(0,1,0,1));

	origin.setTranslation(1.5f, 0.5f, 2.0f);
	DebugDrawer::instance().drawCapsule(0.1f, 0.5f, origin, Color(1,0,0,1));

	static float angle = 0.0f;
	angle += dt;
	origin.setTranslation(1.5f, 1.0f, 2.5f);
	origin.preRotateZ(angle);
	DebugDrawer::instance().drawCapsule(0.1f, 1.0f, origin, Color(0,1,0,1));

	origin.setTranslation(2.5f, 0.5f, 2.0f);
	DebugDrawer::instance().drawCapsule(0.1f, 0.5f, origin, Color(1,0,0,0.5f), DebugDrawer::DRAW_TRANSPARENT);

	origin.setTranslation(2.5f, 1.0f, 2.5f);
	DebugDrawer::instance().drawCapsule(0.1f, 1.0f, origin, Color(0,1,0,0.5f), DebugDrawer::DRAW_TRANSPARENT);

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

	m_activeCamera->updateMouse(e.xrel, e.yrel, 0);

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
void Editor::switchCamera()
{
	if (m_activeCamera == m_cursorCamera)
	{
		m_activeCamera = m_freeCamera;
	}
	else
	{
		m_activeCamera = m_cursorCamera;
	}

	Engine::instance().renderWorld().setCamera(m_activeCamera);
}

//--------------------------------------------------------------------------------------------------
Editor::UI::UI(Editor& editor) : m_self(editor)
{

}

//--------------------------------------------------------------------------------------------------
Editor::UI::~UI()
{

}

//--------------------------------------------------------------------------------------------------
void Editor::UI::update()
{
#if 0
	displayImguiDemo();
#endif

	auto& animEngine = Engine::instance().animationEngine();

	ImGui::SetNextWindowSize(ImVec2(300, 0), ImGuiSetCond_FirstUseEver);

	if (!ImGui::Begin("Editor", &m_self.m_guiActive))
	{
		ImGui::End();
	}
	else
	{
		if (ImGui::CollapsingHeader("Models"))
		{
			if (ImGui::Button("Load Model..."))
			{
				ImGui::OpenPopup("Load Model");

				m_gameObjsList.m_list.clear();
				FileSystem::instance().getFilesInDir("..\\resources\\models", m_gameObjsList.m_list, "xml", true);
			}

			displayGameObjs();
		}

		if (ImGui::CollapsingHeader("Animations"))
		{
			if (ImGui::Button("Load Animation...") && m_self.m_activeSkinModel)
			{
				ImGui::OpenPopup("Load Animation");

				m_animList.m_list.clear();
				FileSystem::instance().getFilesInDir("..\\resources\\models\\" + m_self.m_objName, m_animList.m_list, "animation", true);
			}

			displayAnimation();

			if (ImGui::Checkbox("looped", &m_self.m_looped) && m_self.m_activeSkinModel)
			{
				animEngine.stopAnim(m_self.m_animCtrl);
				animEngine.playAnim(m_self.m_animCtrl, m_self.m_animName.c_str(), m_self.m_looped);
			}

			if (ImGui::Checkbox("stepped", &m_self.m_stepped) && m_self.m_activeSkinModel)
			{
				if (m_self.m_stepped)
					animEngine.pauseAnim(m_self.m_animCtrl);
				else
					animEngine.continueAnim(m_self.m_animCtrl);
			}

			if (ImGui::SliderInt("frames", &m_self.m_curFrame, 0, m_self.m_numFrames) && m_self.m_activeSkinModel && m_self.m_stepped)
			{
				animEngine.goToAnim(m_self.m_animCtrl, m_self.m_curFrame);
			}
		}

		if (ImGui::CollapsingHeader("Sun"))
		{
			ImGui::SliderAngle("sun yaw angle", &m_self.m_sunAngles.x, 0.0f, 360.0f);
			ImGui::SliderAngle("sun pitch angle", &m_self.m_sunAngles.y, 0.0f, 90.0f);
		}

		if (ImGui::CollapsingHeader("Camera"))
		{
			if (ImGui::Button("switch camera"))
			{
				m_self.switchCamera();
			}
		}

		if (ImGui::CollapsingHeader("Material"))
		{
		}

		if (ImGui::CollapsingHeader("Physics"))
		{
		}

		ImGui::End();
	}
}

//--------------------------------------------------------------------------------------------------
void Editor::UI::displayGameObjs()
{
	if (ImGui::BeginPopup("Load Model"))
	{
		for (const auto& modelName : m_gameObjsList.m_list)
		{
			if (ImGui::Selectable(modelName.c_str()))
			{
				m_self.loadGameObj(modelName);
			}
		}

		ImGui::EndPopup();
	}
}

//--------------------------------------------------------------------------------------------------
void Editor::UI::displayAnimation()
{
	if (ImGui::BeginPopup("Load Animation"))
	{
		for (const auto& animName : m_animList.m_list)
		{
			if (ImGui::Selectable(animName.c_str()))
			{
				m_self.loadAnimation(animName);
			}
		}

		ImGui::EndPopup();
	}
}

void Editor::UI::displayImguiDemo()
{
	static bool show_test_window = true;
	static bool show_another_window = false;
	static ImVec4 clear_col = ImColor(114, 144, 154);

	// 1. Show a simple window
	// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
	{
		static float f = 0.0f;
		ImGui::Text("Hello, world!");
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
		ImGui::ColorEdit3("clear color", (float*)&clear_col);
		if (ImGui::Button("Test Window")) show_test_window ^= 1;
		if (ImGui::Button("Another Window")) show_another_window ^= 1;
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	// 2. Show another simple window, this time using an explicit Begin/End pair
	if (show_another_window)
	{
		ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Another Window", &show_another_window);
		ImGui::Text("Hello");
		ImGui::End();
	}

	// 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if (show_test_window)
	{
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);     // Normally user code doesn't need/want to call it because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
		ImGui::ShowTestWindow(&show_test_window);
	}
}

