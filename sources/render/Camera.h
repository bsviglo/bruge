#pragma once

#include "prerequisites.hpp"
#include "render_common.h"
#include "math/math_all.hpp"

namespace brUGE
{

	//-- Base camera.
	//----------------------------------------------------------------------------------------------
	class Camera : public utils::RefCount
	{
	public:

		//-- ToDo: rework interface.

		// Можно создать камеру с собственой настройкой перспективной камеры, т.е.
		// персонально для этой камеры, а можно воспользоваться перспективой, которая
		// задана поумолчанию в рендере. Изменение переспективы в камере видет только к
		// изменению усеченной пирамиды видимости данной камеры.
		//------------------------------------------
		Camera(const render::Projection& projection);
		virtual ~Camera() { }
		
		// Функции вызываемая каждый фрейм, она обновляет всю информацию
		// нужную для камеры и посредством этой информации и саму камеру.
		// Параметр updateIput говорит о том нужно ли обрабатывать ввод пользователя.
		//------------------------------------------
		virtual void update(bool /*updateInput*/, float /*dt*/) {}

		// Функция для обноления положения мыши.
		// Примечание: Функция вызывается только при изменении положения курсора мыши. 
		//------------------------------------------
		virtual void updateMouse(float /*dx*/, float /*dy*/, float /*dz*/) {}

		const render::RenderCamera& renderCam() const			{ return m_renderCam; } 
		const vec3f&				side() const;
		const vec3f&				up() const;
		const vec3f&				direction() const;
		const vec3f&				position() const;

	protected:
		void setLookAt(const vec3f &pos, const vec3f &dir, const vec3f &up);
		void setEuler(const vec3f &pos, float pitch, float yaw, float roll);

	private:
		render::RenderCamera m_renderCam;
	};

} //-- brUGE
