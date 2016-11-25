#include "Player.hpp"
#include "console/Console.h"
#include "control/InputManager.h"
#include "math/Matrix4x4.hpp"
#include "math/math_funcs.hpp"
#include "render/render_common.h"
#include "engine/Engine.h"
#include "render/render_world.hpp"
#include "render/animation_engine.hpp"
#include "render/physic_world.hpp"
#include "render/DebugDrawer.h"

using namespace brUGE;
using namespace brUGE::math;
using namespace brUGE::utils;
using namespace brUGE::render;
using namespace brUGE::physic;

//--------------------------------------------------------------------------------------------------
Player::Player() : m_speed(10.f), m_yawDiff(0.0f), m_posDiff(0,0,0), m_cameraZoom(15.0f), m_walking(false)
{
	m_source.setTranslation(0.0f, m_cameraZoom, 0.0f);
	m_target.setRotateX(degToRad(70.0f));
	m_target.postRotateY(degToRad(90.0f));

	Projection proj;
	proj.fov	  = 90.0f;
	proj.nearDist = 0.5f;
	proj.farDist  = 50.0f;

	m_camera = new CursorCamera(proj);

	m_camera->source(&m_source);
	m_camera->target(&m_target);

	Engine::instance().renderWorld().setCamera(m_camera);
}

//--------------------------------------------------------------------------------------------------
Player::~Player()
{

}

//--------------------------------------------------------------------------------------------------
bool Player::load(const ROData& inData, Handle objID, const mat4f* orient /* = NULL */)
{
	bool success = IPlayerObj::load(inData, objID, orient);

	//-- loop idle animation.
	if (success)
	{
		Engine::instance().animationEngine().playAnim(m_animCtrl, "player/idle", true);
		m_walking = false;
	}

	return success;
}

//--------------------------------------------------------------------------------------------------
bool Player::save(WOData& outData)
{
	return IPlayerObj::save(outData);
}

//--------------------------------------------------------------------------------------------------
void Player::receiveEvent(const GameEvent& event)
{
	ConWarning("Event %s was received with damage %f", event.m_name, *(float*)event.m_data);
}

//--------------------------------------------------------------------------------------------------
void Player::beginUpdate(float dt)
{
	moveByKey(dt);

	//-- update player transform.
	mat4f& world = m_transform.m_worldMat;
	{
		world.preRotateZ(-m_yawDiff);
		world.postTranslation(m_posDiff);
	}

	DebugDrawer::instance().drawCoordAxis(world, 1.0f);

	//-- update camera.
	{
		m_source.setTranslation(world.applyToOrigin());
		m_source.preTranslation(0.0f, m_cameraZoom, 0.0f);

		m_target.postRotateY(-m_yawDiff);

		m_camera->update(true, dt);
	}

	//-- show site laser.
	{
		vec3f dir   = world.applyToUnitAxis(mat4f::X_AXIS).getNormalized();
		vec3f start = world.applyToOrigin() + vec3f(0.0f, 0.25f, 0.0f) + dir.scale(1.0f);
		vec3f end   = start + dir.scale(50.0f);

		vec3f collision;
		if (Engine::instance().physicWorld().collide(collision, start, end))
		{
			DebugDrawer::instance().drawLine(start, collision, Color(0,0,1,1));
			DebugDrawer::instance().drawLine(collision, end, Color(0,1,0,1));
		}
		else
		{
			DebugDrawer::instance().drawLine(start, end, Color(1,0,0,1));
		}
	}

	if (m_posDiff.length())
	{
		if (!m_walking)
		{
			Engine::instance().animationEngine().stopAnim(m_animCtrl);
			Engine::instance().animationEngine().playAnim(m_animCtrl, "player/walk", true);
			m_walking = true;
		}
	}
	else
	{
		if (m_walking)
		{
			Engine::instance().animationEngine().stopAnim(m_animCtrl);
			Engine::instance().animationEngine().playAnim(m_animCtrl, "player/idle", true);
			m_walking = false;
		}
	}

	//-- reset diff accumulators.
	m_posDiff.setZero();
	m_yawDiff = 0.0f;
}

//--------------------------------------------------------------------------------------------------
bool Player::handleMouseClick(const MouseEvent& me)
{
	if (me.button == MB_LEFT_BUTTON && me.isDown)
	{
		const PhysicWorld& physWorld = Engine::instance().physicWorld();

		//-- prepare collision ray.
		vec3f dir   = m_transform.m_worldMat.applyToUnitAxis(mat4f::X_AXIS).getNormalized();
		vec3f start = m_transform.m_worldMat.applyToOrigin() + vec3f(0.0f, 0.25f, 0.0f) + dir.scale(1.0f);
		vec3f end   = start + dir.scale(50.0f);

		mat4f localMat;
		Node* node   = nullptr;
		Handle objID = CONST_INVALID_HANDLE;

		if (physWorld.collide(localMat, node, objID, start, end))
		{
			IGameObj* obj = Engine::instance().gameWorld().getGameObj(objID);
			if (obj && obj->physObj())
			{
				obj->physObj()->addImpulse(dir.scale(5.0f), localMat.applyToOrigin());

				//-- send event about damage.
				//-- send event about attack.
				{
					float damage = 10.0f;

					GameEvent event;
					event.m_name = "hit";
					event.m_data = &damage;

					obj->receiveEvent(event);
				}
			}
		}
	}
	return false;
}

//--------------------------------------------------------------------------------------------------
bool Player::handleMouseMove(const MouseAxisEvent& mae)
{
	moveByMouse(mae.relX, mae.relY, mae.relZ);

	m_cameraZoom -= mae.relZ * 0.01;
	m_cameraZoom  = clamp(6.0f, m_cameraZoom, 15.0f);

	return true;
}

//--------------------------------------------------------------------------------------------------
bool Player::handleKeyboardEvent(const KeyboardEvent& /*ke*/)
{
	return false;
}

//--------------------------------------------------------------------------------------------------
void Player::moveByMouse(float dx, float /*dy*/, float /*dz*/)
{
	float mouseAccel = 5.0f;
	float mouseSens  = 0.0125f;

	dx = max(-mouseAccel, dx);
	dx = min(dx, mouseAccel);

	m_yawDiff = -dx * mouseSens;
}

//--------------------------------------------------------------------------------------------------
void Player::moveByKey(float dt)
{
	if (Console::instance().visible()) return;

	InputManager& im = InputManager::instance();

	//-- calculate multiplier and adjust it by time.
	float multiplier = im.isModifierDown(KM_SHIFT) ? 2.0f : 1.0f;
	multiplier *= dt;

	if (im.isKeyDown(DIK_W)) move  (+multiplier * m_speed);	//W
	if (im.isKeyDown(DIK_S)) move  (-multiplier * m_speed);	//S
	if (im.isKeyDown(DIK_D)) strafe(+multiplier * m_speed);	//A
	if (im.isKeyDown(DIK_A)) strafe(-multiplier * m_speed);	//D
}

//--------------------------------------------------------------------------------------------------
void Player::move(float value)
{
	m_posDiff += m_transform.m_worldMat.applyToUnitAxis(mat4f::X_AXIS).scale(value);
}

//--------------------------------------------------------------------------------------------------
void Player::strafe(float value)
{
	m_posDiff += m_transform.m_worldMat.applyToUnitAxis(mat4f::Y_AXIS).scale(value);	
}