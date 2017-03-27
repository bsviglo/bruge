#include "Camera.h"
#include "render_system.hpp"
#include "console/WatchersPanel.h"

namespace brUGE
{	
	using namespace render;

	//------------------------------------------
	Camera::Camera(const Projection& projection)
	{
		m_renderCam.m_projInfo = projection;
		
		vec3f pos(0.0f ,0.0f, 0.0f);
		ScreenResolution screen = Renderer::instance().screenRes();

		m_renderCam.m_proj.setPerspectiveProj(
			projection.fov,	screen.width / screen.height,
			projection.nearDist, projection.farDist
			);

		m_renderCam.m_view.setLookAt(pos, vec3f(0.0f, 0.0f, 1.0f), vec3f(0.0f, 1.0f, 0.0f));
		m_renderCam.m_invView = m_renderCam.m_view.getInverted();
		
		m_renderCam.m_viewProj = m_renderCam.m_view;
		m_renderCam.m_viewProj.postMultiply(m_renderCam.m_proj);
	}

	// 
	//------------------------------------------
	void Camera::setLookAt(const vec3f &pos, const vec3f &dir, const vec3f &up)
	{
		m_renderCam.m_view.setLookAt(pos, dir, up);
		m_renderCam.m_invView = m_renderCam.m_view.getInverted();

		m_renderCam.m_viewProj = m_renderCam.m_view;
		m_renderCam.m_viewProj.postMultiply(m_renderCam.m_proj);
	}

	// Note: Углы ейлера (pitch, yaw, roll) задаются в градусах.
	// Note: К вопросу как происходит формирование видовой матрицы:
	//	     T*Rz*Ry*Rx - все матрицы здесь инвертированы и нету Rz так
	//	     как вращение вокруг этой оси для не предусматривается.
	//------------------------------------------
	void Camera::setEuler(const vec3f& pos, float pitch, float yaw, float /*roll*/)
	{
		// строим обычную матрицу, затем инвертируем ее и получаем видовую матрицу.
		m_renderCam.m_invView.setRotateX(-pitch);
		m_renderCam.m_invView.postRotateY(-yaw);
		m_renderCam.m_invView.postTranslation(pos);
		m_renderCam.m_view = m_renderCam.m_invView.getInverted();
		
		m_renderCam.m_viewProj = m_renderCam.m_view;
		m_renderCam.m_viewProj.postMultiply(m_renderCam.m_proj);
	}

	//----------------------------------------------------------------------------------------------
	const vec3f& Camera::side() const
	{
		return m_renderCam.m_invView.applyToUnitAxis(0);
	}

	//----------------------------------------------------------------------------------------------
	const vec3f& Camera::up() const
	{
		return m_renderCam.m_invView.applyToUnitAxis(1);
	}

	//----------------------------------------------------------------------------------------------
	const vec3f& Camera::direction() const
	{
		return m_renderCam.m_invView.applyToUnitAxis(2);
	}

	//----------------------------------------------------------------------------------------------
	const vec3f& Camera::position() const
	{
		return m_renderCam.m_invView.applyToOrigin();
	}

} //-- brUGE