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

		// ����� ������� ������ � ���������� ���������� ������������� ������, �.�.
		// ����������� ��� ���� ������, � ����� ��������������� ������������, �������
		// ������ ����������� � �������. ��������� ������������ � ������ ����� ������ �
		// ��������� ��������� �������� ��������� ������ ������.
		//------------------------------------------
		Camera(const render::Projection& projection);
		virtual ~Camera() { }
		
		// ������� ���������� ������ �����, ��� ��������� ��� ����������
		// ������ ��� ������ � ����������� ���� ���������� � ���� ������.
		// �������� updateIput ������� � ��� ����� �� ������������ ���� ������������.
		//------------------------------------------
		virtual void update(bool /*updateInput*/, float /*dt*/) {}

		// ������� ��� ��������� ��������� ����.
		// ����������: ������� ���������� ������ ��� ��������� ��������� ������� ����. 
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
