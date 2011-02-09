#include "engine/Engine.h"
#include "render/decal_manager.hpp"

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

	Engine&			 engine     = Engine::instance();
	GameWorld&		 gameWorld  = engine.gameWorld();
	AnimationEngine& animEngine = engine.animationEngine();

	mat4f mat;
	mat.setIdentity();

	engine.renderWorld().setCamera(m_camera);
	
	//-- plane.
	gameWorld.addGameObj("resources/models/plane.xml", &mat);

	//-- test
	for (uint i = 0; i < 5; ++i)
	{
		for (uint j = 0; j < 5; ++j)
		{
			for (uint k = 0; k < 5; ++k)
			{
				mat.setTranslation(i * 5.f, k * 5.f, j * 5.f);
				mat.postTranslation(-15, 25, -15);
				gameWorld.addGameObj("resources/models/metalbox1.xml", &mat);
			}
		}
	}

	for (uint i = 0; i < 4; ++i)
	{
		for (uint j = 0; j < 4; ++j)
		{
			for (uint k = 0; k < 4; ++k)
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

	for (uint i = 0; i < m_collisionDescs.size(); ++i)
	{
		m_collisions[i] = m_collisionDescs[i].first;
		m_collisions[i].postMultiply(m_collisionDescs[i].second->matrix());
	}
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
	m_camera->updateMouse(mae.relX, mae.relY, mae.relZ);
	return true;
}

//-------------------------------------------------------------------------------------------------
bool Demo::handleKeyboardEvent(const KeyboardEvent& /*ke*/)
{
	return false;
}
