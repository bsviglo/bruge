#pragma once

#include "prerequisites.hpp"
#include "scene/game_world.hpp"
#include "utils/Data.hpp"
#include "math/Matrix4x4.hpp"
#include "math/Vector3.hpp"

//--------------------------------------------------------------------------------------------------
class Zombie : public brUGE::IGameObj
{
public:
	Zombie();
	virtual ~Zombie();

	//-- serialization functions.
	virtual bool load(const brUGE::utils::ROData& inData, Handle objID, const brUGE::mat4f* orient = NULL);
	virtual bool save(brUGE::utils::WOData& outData);

	virtual void receiveEvent(const brUGE::GameEvent& event);

	virtual void beginUpdate(float dt);
	virtual void preAnimUpdate()	{ }
	virtual void postAnimUpdate()	{ }
	virtual void endUpdate()		{ }

private:
	void updateState();

private:
	enum EState
	{
		STATE_IDLE = 0,
		STATE_COOLDOWN,
		STATE_WALK,
		STATE_ATTACK,
		STATE_DEAD
	};

	brUGE::mat4f m_originMat;
	float		 m_accumYaw;
	brUGE::vec3f m_accumPos;

	EState m_state;
	float  m_speed;
	float  m_hitTimeout;
	float  m_attackTimeout;
	float  m_health;
};