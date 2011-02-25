#pragma once

#include "prerequisites.h"
#include "Camera.h"

namespace brUGE
{
	//
	// Класс реализующий поведение свободной(спектраторской) камеры.
	// Он получает информацию о пользовательском вводе: положении мыши и клавишь и 
	// на их основе производит обновление.
	//----------------------------------------------------------------------------------------------
	class FreeCamera : public render::Camera
	{
	public:
		FreeCamera(const render::Projection *projection = NULL);
		virtual ~FreeCamera() { }
		
		// смотри Camera.
		// Примечание: Используется для обновления логики камеры и для
		//			   немедленного получения информации из устройства клавиатуры.
		virtual void update(bool updateInput, float dt);

		// Функция для обноления положения мыши.
		// Примечание: Функция вызывается только при изменении положения курсора мыши. 
		//------------------------------------------
		virtual void updateMouse(float dx, float dy, float dz);
		
		// Проинициализировать камеру.
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

