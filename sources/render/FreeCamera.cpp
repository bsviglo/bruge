#include "FreeCamera.h"
#include "console/WatchersPanel.h"
#include "SDL/SDL_keyboard.h"

namespace brUGE
{

	//------------------------------------------
	FreeCamera::FreeCamera(const render::Projection& projection)
		:	Camera(projection),
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
		//-- console functions and variables.
		REGISTER_CONSOLE_MEMBER_VALUE("fc_sens",  float, m_mouseSens,  FreeCamera);
		REGISTER_CONSOLE_MEMBER_VALUE("fc_accel", float, m_mouseAccel, FreeCamera);
		REGISTER_CONSOLE_MEMBER_VALUE("fc_speed", float, m_speed,	   FreeCamera);

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
		
		if (!m_updateInput)		return;

		moveByKey(dt);
		setEuler(m_pos, math::degToRad(m_pitch), math::degToRad(m_yaw), 0.0f);
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
	void FreeCamera::moveByKey(float dt)
	{
		//-- calculate multiplier and adjust it by time.
		float multiplier = (SDL_GetModState() & KMOD_SHIFT) ? 2.0f : 1.0f;
		multiplier *= dt;

		const auto* keyState = SDL_GetKeyboardState(nullptr);

		if (keyState[SDL_SCANCODE_W]) move  (+multiplier * m_speed);	//W
		if (keyState[SDL_SCANCODE_S]) move  (-multiplier * m_speed);	//S
		if (keyState[SDL_SCANCODE_D]) strafe(+multiplier * m_speed);	//A
		if (keyState[SDL_SCANCODE_A]) strafe(-multiplier * m_speed);	//D
		if (keyState[SDL_SCANCODE_E]) rise  (+multiplier * m_speed);	//Q
		if (keyState[SDL_SCANCODE_Q]) rise  (-multiplier * m_speed);	//E
	}

	//------------------------------------------
	void FreeCamera::move(float value)
	{
		m_pos += direction().scale(value);
	}
	
	//------------------------------------------
	void FreeCamera::strafe(float value)
	{
		m_pos += side().scale(value);	
	}

	//------------------------------------------
	void FreeCamera::rise(float value)
	{
		m_pos += up().scale(value);	
	}
	
} //-- brUGE