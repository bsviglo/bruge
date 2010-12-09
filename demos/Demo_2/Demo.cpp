#include "engine/Engine.h"

//-- because inside this file used 'using' declaration, including this file as the last, must
//-- prevent negative influence on other include files.
#include "Demo.h"

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

	//---------------------------------------------------------------------------------------------
	template<typename Type, size_t size>
	inline size_t array_size(Type (&) [size]) { return size; }
}
//-------------------------------------------------------------------------------------------------
// end unnamed namespace

//-------------------------------------------------------------------------------------------------
Demo::Demo()
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

	mat4f mat;
	mat.setIdentity();

	Engine::instance().renderWorld().setCamera(m_camera);
	//Engine::instance().gameWorld().addGameObj("resources/models/cow.xml", &mat);

	//-- test
	for (uint i = 0; i < 10; ++i)
	{
		for (uint j = 0; j < 10; ++j)
		{
			for (uint k = 0; k < 10; ++k)
			{
				mat.setTranslation(i * 5.f, k * 5.f, j * 5.f);
				Engine::instance().gameWorld().addGameObj("resources/models/metalbox1.xml", &mat);
			}
		}
	}

	for (uint i = 0; i < 8; ++i)
	{
		for (uint j = 0; j < 8; ++j)
		{
			for (uint k = 0; k < 8; ++k)
			{
				mat.setTranslation(i * 3.f, k * 3.f, j * 3.f);
				mat.postTranslation(15, 10, 15);
				Engine::instance().gameWorld().addGameObj("resources/models/exp_barrel.xml", &mat);
			}
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
void Demo::shutdown()
{

}

//-------------------------------------------------------------------------------------------------
void Demo::update(float dt)
{
	m_camera->update(true, dt);
}

//-------------------------------------------------------------------------------------------------
void Demo::render(float dt)
{
	mat4f indentity;
	indentity.setIdentity();
	DebugDrawer::instance().drawCoordAxis(indentity);

	//-- update debug drawer.
	DebugDrawer::instance().draw(m_camera->viewProjMatrix(), dt);
}

//-------------------------------------------------------------------------------------------------
bool Demo::handleMouseClick(const MouseEvent& /*me*/)
{
	return false;
}

//-------------------------------------------------------------------------------------------------
bool Demo::handleMouseMove(const MouseAxisEvent& mae)
{
	m_camera->updateMouse(mae.relX, mae.relY, mae.relZ);
	return true;
}

//-------------------------------------------------------------------------------------------------
bool Demo::handleKeyboardEvent(const KeyboardEvent& /*ke*/)
{
	return false;
}
