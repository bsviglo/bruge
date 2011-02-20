#include "Camera.h"
#include "render_system.hpp"
#include "console/Console.h"
#include "console/WatchersPanel.h"

namespace brUGE
{
namespace render
{
	
	//------------------------------------------
	Camera::Camera(const Projection *projection /*= NULL*/)
	{
		_init(projection);

		//-- register console funcs.
		REGISTER_CONSOLE_METHOD("c_matrices", _printMatrix, Camera);

		//-- register watchers.
		REGISTER_RO_MEMBER_WATCHER("camera direction", vec3f, Camera, m_dir);
	}

	//------------------------------------------
	void Camera::_init(const Projection* projection)
	{	
		if (projection)	m_projInfo = *projection;
		else			m_projInfo = RenderSystem::instance().projection();
		
		vec3f pos(0.0f ,0.0f, 0.0f);
		m_screen = RenderSystem::instance().screenRes();

		m_projMat.setPerspectiveProj(m_projInfo.fov, m_screen.width / m_screen.height,
			m_projInfo.nearDist, m_projInfo.farDist);

		m_viewMat.setLookAt(pos, vec3f(0.0f, 0.0f, 1.0f), vec3f(0.0f, 1.0f, 0.0f));
		m_invViewMat = m_viewMat.getInverted();
		m_pos = pos;
		
		m_side = m_invViewMat.applyToUnitAxis(0);
		m_up   = m_invViewMat.applyToUnitAxis(1);
		m_dir  = m_invViewMat.applyToUnitAxis(2);

		m_viewProjMat = m_viewMat;
		m_viewProjMat.postMultiply(m_projMat);
		m_invViewProjMat = m_viewProjMat.getInverted();
	}

	// 
	//------------------------------------------
	void Camera::_setLookAt(const vec3f &pos, const vec3f &dir, const vec3f &up)
	{
		m_viewMat.setLookAt(pos, dir, up);
		m_invViewMat = m_viewMat.getInverted();

		m_side = m_invViewMat.applyToUnitAxis(0);
		m_up   = m_invViewMat.applyToUnitAxis(1);
		m_dir  = m_invViewMat.applyToUnitAxis(2);
		m_pos  = m_invViewMat.applyToOrigin();

		m_viewProjMat = m_viewMat;
		m_viewProjMat.postMultiply(m_projMat);
		m_invViewProjMat = m_viewProjMat.getInverted();
	}

	// Note: Углы ейлера (pitch, yaw, roll) задаются в градусах.
	// Note: К вопросу как происходит формирование видовой матрицы:
	//	     T*Rz*Ry*Rx - все матрицы здесь инвертированы и нету Rz так
	//	     как вращение вокруг этой оси для не предусматривается.
	//------------------------------------------
	void Camera::_setEuler(const vec3f& pos, float pitch, float yaw, float /*roll*/)
	{
		// строим обычную матрицу, затем инвертируем ее и получаем видовую матрицу.
		m_invViewMat.setRotateX(-pitch);
		m_invViewMat.postRotateY(-yaw);
		m_invViewMat.postTranslation(pos);
		m_viewMat = m_invViewMat.getInverted();
		
		m_side = m_invViewMat.applyToUnitAxis(0);
		m_up   = m_invViewMat.applyToUnitAxis(1);
		m_dir  = m_invViewMat.applyToUnitAxis(2);
		m_pos  = m_invViewMat.applyToOrigin();

		m_viewProjMat = m_viewMat;
		m_viewProjMat.postMultiply(m_projMat);
		m_invViewProjMat = m_viewProjMat.getInverted();
	}
	
	//------------------------------------------
	void Camera::drawViewFrustum() const
	{
/*
		static brOGLRender *render = brOGLRender::getSingletonPtr();

		float dist = projection_.farDist - projection_.nearDist;
		float tg_v = tan(DEG_TO_RAD(projection_.fov) * 0.5);
		float tg_h = tan(DEG_TO_RAD(projection_.fov) * 0.5 * screen_.width / screen_.height);

		float v_height = dist * tg_v;
		float v_width  = dist * tg_h;

		vec3f up	= scale(up_, v_height);
		vec3f side	= scale(side_, v_width);
		vec3f far_p = pos_ + scale(dir_, dist);

		vec3f p_lt = far_p + up - side;
		vec3f p_lb = far_p - up - side;
		vec3f p_rb = far_p - up + side;
		vec3f p_rt = far_p + up + side;

		// отрисовка пирамиды видимости.
		render->renderLine(pos_, p_lt, color_Yellow);
		render->renderLine(pos_, p_lb, color_Yellow);
		render->renderLine(pos_, p_rt, color_Yellow);
		render->renderLine(pos_, p_rb, color_Yellow);
		render->wireframe_renderQuad(p_lb, p_lt, p_rt, p_rb, color_Yellow);

		// отрисовка осей координат.
		render->renderLine(pos_, pos_ + scale(dir_, 2.0f),	color_Red);
		render->renderLine(pos_, pos_ + scale(up_, 2.0f),	color_Blue);
		render->renderLine(pos_, pos_ + scale(side_, 2.0f), color_Green);
*/
	}

#define PRINT_MAT4(mat)															\
	{																			\
		ConWarning("%.3f %.3f %.3f %.3f", mat.m00, mat.m01, mat.m02, mat.m03);	\
		ConWarning("%.3f %.3f %.3f %.3f", mat.m10, mat.m11, mat.m12, mat.m13);	\
		ConWarning("%.3f %.3f %.3f %.3f", mat.m20, mat.m21, mat.m22, mat.m23);	\
		ConWarning("%.3f %.3f %.3f %.3f", mat.m30, mat.m31, mat.m32, mat.m33);	\
	}	
	
#define PRINT_VEC3(vec) ConWarning("%.3f %.3f %.3f", vec.x, vec.y, vec.z);
	
	// 
	//------------------------------------------
	int Camera::_printMatrix(std::string type)
	{
		if		(type == "view"		)	PRINT_MAT4(m_viewMat)
		else if (type == "invView"	)	PRINT_MAT4(m_invViewMat)
		else if (type == "proj"		)	PRINT_MAT4(m_projMat)
		else if (type == "viewProj"	)	PRINT_MAT4(m_viewProjMat)
		else if (type == "dir"		)	PRINT_VEC3(m_dir)
		else if (type == "side"		)	PRINT_VEC3(m_side)
		else if (type == "pos"		)	PRINT_VEC3(m_pos)
		else if (type == "up"		)	PRINT_VEC3(m_up)
		else							ConError(" * vectorType[string]{dir, side, pos, up} \n * vectorType[string]{dir, side, pos, up}");
		return 0;
	}

} // render
} // brUGE