#include "Zombie.hpp" 
#include "console/Console.h"
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


//-- start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	//----------------------------------------------------------------------------------------------
	int random(int maxValue)
	{
		return int((rand() / float(RAND_MAX + 1)) * maxValue);
	}
}
//--------------------------------------------------------------------------------------------------
//-- end unnamed namespace.


//--------------------------------------------------------------------------------------------------
Zombie::Zombie() : m_state(STATE_IDLE), m_speed(1.0f), m_health(100.0f), m_attackTimeout(0.0f), m_hitTimeout(0.0f)
{

}

//--------------------------------------------------------------------------------------------------
Zombie::~Zombie()
{

}

//--------------------------------------------------------------------------------------------------
bool Zombie::load(const ROData& inData, Handle objID, const mat4f* orient)
{
	bool success = IGameObj::load(inData, objID, orient);

	//-- loop idle animation.
	if (success)
	{
		Engine::instance().animationEngine().playAnim(m_animCtrl, "zfat/idle", true);
		m_state = STATE_IDLE;
	}

	m_originMat = m_transform.m_worldMat;
	m_accumPos.setZero();
	m_accumYaw  = 0.0f;

	return success;
}

//--------------------------------------------------------------------------------------------------
bool Zombie::save(WOData& outData)
{
	return IGameObj::save(outData);
}

//--------------------------------------------------------------------------------------------------
void Zombie::receiveEvent(const GameEvent& event)
{
	if (strcmp(event.m_name, "hit") == 0)
	{
		//-- zombie already dead.
		if (m_state == STATE_DEAD)
			return;

		m_health -= *static_cast<float*>(event.m_data);
		if (m_health <= 0.0f)
		{
			//-- play death animation.
			Engine::instance().animationEngine().stopAnim(m_animCtrl);
			Engine::instance().animationEngine().playAnim(m_animCtrl, "zfat/idle", true);
			m_state = STATE_DEAD;
		}
		else
		{
			//-- play hit pain.
			Engine::instance().animationEngine().stopAnim(m_animCtrl);
			Engine::instance().animationEngine().playAnim(m_animCtrl, "zfat/pain", true);
			m_hitTimeout = 0.5f;

			m_state = STATE_COOLDOWN;
		}
	}
}

//--------------------------------------------------------------------------------------------------
float signAngle(const vec3f& v1, const vec3f& v2)
{
	vec3f c     = v1.cross(v2);
	float angle = std::atan2(c.length(), v1.dot(v2));
	
	return c.dot(vec3f(1.f,0.f,0.f)) < 0.0f ? -angle : angle;
}


//--------------------------------------------------------------------------------------------------
void Zombie::beginUpdate(float dt)
{
	DebugDrawer::instance().drawCoordAxis(m_transform.m_worldMat, 1.0f);

	//-- zombie is dead.
	if (m_state == STATE_DEAD)
		return;

	//-- retrieve player position.
	const mat4f& playerMat = Engine::instance().gameWorld().getPlayer()->worldPos();

	//-- calculate direction vector.
	vec3f selfDir = m_originMat.applyToUnitAxis(mat4f::X_AXIS).getNormalized();
	vec3f dir     = playerMat.applyToOrigin() - m_transform.m_worldMat.applyToOrigin();
	float dist    = dir.length();
	dir.normalize();

	//-- if zombie is still in hit cool down, wait why it comes to oneself.
	m_hitTimeout = max(m_hitTimeout - dt, 0.0f);
	if (almostZero(m_hitTimeout))
	{
		float yaw1 = atan2(dir.x * selfDir.z - selfDir.x * dir.z, dir.x * selfDir.x + dir.y * selfDir.y);
		//float yaw2 = atan2(dir.cross(selfDir).length(), dir.dot(selfDir));
		//float yaw3 = signAngle(dir, selfDir);

		//if (!almostZero(yaw3, 0.01f))
		//	m_transform.m_worldMat.preRotateZ(yaw3);
		m_accumYaw = yaw1;

		//INFO_MSG("angle %f %f %f", yaw1, yaw2, yaw3);

		//-- move zombie to a new position and start playing walk animation.
		if (dist > 2.5f)
		{
			m_accumPos += dir.scale(m_speed * dt);
			//vec3f move = dir.scale(m_speed * dt);
			//m_transform.m_worldMat.postTranslation(move);

			if (m_state != STATE_WALK)
			{
				const char* walkAnims[] = {"zfat/walk1", "zfat/walk2", "zfat/walk3", "zfat/walk4"};

				Engine::instance().animationEngine().stopAnim(m_animCtrl);
				Engine::instance().animationEngine().playAnim(m_animCtrl, walkAnims[random(3)], true);
				m_state = STATE_WALK;
			}
		}
		//-- zombie is close enough to hit the player.
		else
		{
			m_attackTimeout = max(m_attackTimeout - dt, 0.0f);
			if (almostZero(m_attackTimeout))
			{
				m_state = STATE_IDLE;

				//-- play attack animation.
				if (m_state != STATE_ATTACK)
				{
					Engine::instance().animationEngine().stopAnim(m_animCtrl);
					Engine::instance().animationEngine().playAnim(m_animCtrl, "zfat/attack", true);
					m_state = STATE_ATTACK;
				}

				//-- send event about attack.
				{
					float damage = 10.0f;

					GameEvent event;
					event.m_name = "hit";
					event.m_data = &damage;

					Engine::instance().gameWorld().getPlayer()->receiveEvent(event);

					m_attackTimeout = 1.0f;
				}

			}
		}
	}
	
	//-- reconstruct world pos.
	m_transform.m_worldMat = m_originMat;
	m_transform.m_worldMat.preRotateZ(m_accumYaw);
	m_transform.m_worldMat.postTranslation(m_accumPos);
}
