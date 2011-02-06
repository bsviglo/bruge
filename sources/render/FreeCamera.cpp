#include "FreeCamera.h"
#include "control/InputManager.h"
#include "console/Console.h"
#include "console/WatchersPanel.h"
#include "math/Vector3.h"

namespace brUGE
{

	//------------------------------------------
	FreeCamera::FreeCamera(const render::Projection *projection /*= NULL*/)
		:	render::Camera(projection),
			m_speed(10.0f),
			m_mouseSens(0.25f),
			m_mouseAccel(20.0f),
			m_vertFov(60.0f),
			m_pitch(0.0f),
			m_yaw(0.0f),
			m_updateInput(false),
			m_drawDebug(true),
			m_pos(0.0f, 0.0f, 0.0f)
	{
		// ����������� ���������� �������.
		REGISTER_CONSOLE_METHOD("fc_sens", _setSensitivity, FreeCamera);
		REGISTER_CONSOLE_METHOD("fc_accel", _setAcceleration, FreeCamera);
		REGISTER_CONSOLE_METHOD("fc_speed", _setSpeed, FreeCamera);

		//-- register watchers.
		REGISTER_RO_MEMBER_WATCHER("camera position", vec3f, FreeCamera, m_pos);
		REGISTER_RO_MEMBER_WATCHER("camera yaw", float, FreeCamera, m_yaw);
		REGISTER_RO_MEMBER_WATCHER("camera pitch", float, FreeCamera, m_pitch);
	}
	
	//------------------------------------------
	void FreeCamera::init(const vec3f& pos)
	{
		m_pos = pos;			
	}

	// 
	//------------------------------------------
	void FreeCamera::update(bool updateInput, float dt)
	{
		m_updateInput = updateInput;
		
		if (m_drawDebug)		drawViewFrustum();
		if (!m_updateInput)		return;

		_moveByKey(dt);
		_setEuler(m_pos, math::degToRad(m_pitch), math::degToRad(m_yaw), 0.0f);
	}

	// 
	//------------------------------------------
	void FreeCamera::updateMouse(float dx, float dy, float /*dz*/)
	{
		if (!m_updateInput)	return;
		
		// ����������� �� ��������� ���� �� ���� X � Y.
		dx = math::max(-m_mouseAccel, dx);
		dx = math::min(dx, m_mouseAccel);

		dy = math::max(-m_mouseAccel, dy);
		dy = math::min(dy, m_mouseAccel);
		
		m_pitch -= dy * m_mouseSens;
		m_yaw   -= dx * m_mouseSens;
		
		m_pitch = math::max(-m_vertFov, m_pitch);
		m_pitch = math::min(m_pitch, m_vertFov);
	}
	
	// 
	//------------------------------------------
	void FreeCamera::_moveByKey(float dt)
	{
		if (Console::instance().visible()) return;

		InputManager& im = InputManager::instance();
		
		float multiplier = 1.0f;
		if (im.isModifierDown(KM_SHIFT))	multiplier = 2.0f;

		//-- adjust by time.
		multiplier *= dt;

		if (im.isKeyDown(DIK_W))	_move	( multiplier * m_speed);	//W
		if (im.isKeyDown(DIK_S))	_move	(-multiplier * m_speed);	//S
		if (im.isKeyDown(DIK_D))	_strafe	( multiplier * m_speed);	//A
		if (im.isKeyDown(DIK_A))	_strafe	(-multiplier * m_speed);	//D
		if (im.isKeyDown(DIK_E))	_throw	( multiplier * m_speed);	//Q
		if (im.isKeyDown(DIK_Q))	_throw	(-multiplier * m_speed);	//E
	}

	//------------------------------------------
	void FreeCamera::_move(float value)
	{
		m_pos += direction().scale(value);
	}
	
	//------------------------------------------
	void FreeCamera::_strafe(float value)
	{
		m_pos += side().scale(value);	
	}

	//------------------------------------------
	void FreeCamera::_throw(float value)
	{
		m_pos += up().scale(value);	
	}
	
	// ���������� �������. 
	//------------------------------------------

	//------------------------------------------
	int FreeCamera::_setSensitivity(float sens)
	{
		if (sens == 0)	ConWarning("%.3f", m_mouseSens);
		else			m_mouseSens = sens;
		return 0;
	}
	
	//------------------------------------------
	int FreeCamera::_setAcceleration(float accl)
	{
		if (accl == 0)	ConWarning("%.3f", m_mouseAccel);
		else			m_mouseAccel = accl;
		return 0;
	}

	//------------------------------------------
	int FreeCamera::_setSpeed(float speed)
	{
		if (speed == 0)	ConWarning("%.3f", m_speed);
		else			m_speed = speed;
		return 0;
	}

} // brUGE