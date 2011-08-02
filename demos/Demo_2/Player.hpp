#pragma once

#include "prerequisites.hpp"
#include "math/Vector3.hpp"
#include "control/input_listener.h"
#include "utils/Data.hpp"
#include "render/game_world.hpp"
#include "render/CursorCamera.hpp"

//--------------------------------------------------------------------------------------------------
class Player : public brUGE::IPlayerObj
{
public:
	Player();
	virtual ~Player();

	//-- serialization functions.
	virtual bool load(const brUGE::utils::ROData& inData, Handle objID, const brUGE::mat4f* orient = NULL);
	virtual bool save(brUGE::utils::WOData& outData);

	virtual void receiveEvent(const brUGE::GameEvent& event);

	virtual void beginUpdate(float dt);
	virtual void preAnimUpdate()	{ }
	virtual void postAnimUpdate()	{ }
	virtual void endUpdate()		{ }

	virtual bool handleMouseClick	(const brUGE::MouseEvent& me);
	virtual bool handleMouseMove	(const brUGE::MouseAxisEvent& mae);
	virtual bool handleKeyboardEvent(const brUGE::KeyboardEvent& ke);

private:
	void moveByMouse(float dx, float dy, float dz);
	void moveByKey	(float dt);
	void move		(float value);				
	void strafe		(float value);

private:
	bool		 m_walking;
	float		 m_speed;
	float		 m_yawDiff;
	float		 m_cameraZoom;
	brUGE::vec3f m_posDiff;
	brUGE::mat4f m_localMat;

	//-- camera matrices.
	brUGE::Ptr<brUGE::CursorCamera>	m_camera;
	brUGE::mat4f					m_source;
	brUGE::mat4f					m_target;
};


//--------------------------------------------------------------------------------------------------
class Bot : public brUGE::IGameObj
{

};