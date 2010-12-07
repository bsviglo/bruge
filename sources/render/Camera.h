#pragma once

#include "prerequisites.h"
#include "utils/Ptr.h"
#include "Frustum.h"
#include "render_common.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
#include "math/Vector4.h"
#include "math/Matrix4x4.h"

namespace brUGE
{
namespace render
{

	//
	// ����� ����� ������.
	//------------------------------------------------------------
	class Camera : public utils::RefCount
	{
	public:
		// ����� ������� ������ � ���������� ���������� ������������� ������, �.�.
		// ����������� ��� ���� ������, � ����� ��������������� ������������, �������
		// ������ ����������� � �������. ��������� ������������ � ������ ����� ������ �
		// ��������� ��������� �������� ��������� ������ ������.
		//------------------------------------------
		Camera(const Projection* projection = NULL);
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

		// ���������� ��������� �������� ��������� ������.
		//------------------------------------------
		void drawViewFrustum() const;

		const Frustum&		frustum() const				{ return m_frustum; }
		const Projection& 	projection() const			{ return m_projInfo; }
		const mat4f&		viewProjMatrix() const		{ return m_viewProjMat; }
		const mat4f&		invViewProjMatrix() const	{ return m_invViewProjMat; }

		const vec3f&		side() const				{ return m_side; }
		const vec3f&		up() const					{ return m_up; }
		const vec3f&		direction() const			{ return m_dir; }
		const vec3f&		position() const			{ return m_pos; }
		
		const mat4f&		viewMatrix() const			{ return m_viewMat; }
		const mat4f&		invViewMatrix() const		{ return m_invViewMat; }

	protected:
		// ������� ��� ���������� ������� ������� ���� ������.
		void _set(const mat4f &mat);
		void _setLookAt(const vec3f &pos, const vec3f &dir, const vec3f &up);
		void _setEuler(const vec3f &pos, float pitch, float yaw, float roll);

		// ���������� �������.
		int _printMatrix(std::string type);
	
	private:
		void _init(const Projection  *projection);

	private:
		ScreenResolution m_screen;
		Projection m_projInfo;
		Frustum	m_frustum;

		mat4f m_viewMat;
		mat4f m_invViewMat;
		mat4f m_projMat;
		mat4f m_viewProjMat;
		mat4f m_invViewProjMat;
		
		vec3f m_side;
		vec3f m_up;
		vec3f m_dir;
		vec3f m_pos;

		mutable vec3f m_frustumDirs[4];
	};

} // render
} // brUGE
