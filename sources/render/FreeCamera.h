#pragma once

#include "prerequisites.h"
#include "Camera.h"

namespace brUGE
{
	//
	// ����� ����������� ��������� ���������(��������������) ������.
	// �� �������� ���������� � ���������������� �����: ��������� ���� � ������� � 
	// �� �� ������ ���������� ����������.
	//----------------------------------------------------------------------------------------------
	class FreeCamera : public render::Camera
	{
	public:
		FreeCamera(const render::Projection *projection = NULL);
		virtual ~FreeCamera() { }
		
		// ������ Camera.
		// ����������: ������������ ��� ���������� ������ ������ � ���
		//			   ������������ ��������� ���������� �� ���������� ����������.
		virtual void update(bool updateInput, float dt);

		// ������� ��� ��������� ��������� ����.
		// ����������: ������� ���������� ������ ��� ��������� ��������� ������� ����. 
		//------------------------------------------
		virtual void updateMouse(float dx, float dy, float dz);
		
		// ������������������� ������.
		//------------------------------------------
		void init(const vec3f& position);

		void setSensitivity(float sens)			{ m_mouseSens = sens; }
		void setSpeed(float speed)				{ m_speed = speed; }
		void setMouseAcceleration(float accel)	{ m_mouseAccel = accel; }

	private:
		void _moveByKey(float dt);
		void _move(float value);										
		void _strafe(float value);
		void _throw(float value);

	private:
		bool	m_updateInput;
		bool	m_drawDebug;
		float	m_speed;				
		float	m_mouseSens;
		float	m_mouseAccel;
		float	m_vertFov;
		
		float   m_pitch;
		float	m_yaw;
		vec3f	m_pos;
	};

} // brUGE

