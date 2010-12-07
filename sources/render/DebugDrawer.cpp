#include "DebugDrawer.h"
#include "render_system.hpp"
#include "IRenderDevice.h"
#include "IBuffer.h"
#include "IShader.h"
#include "state_objects.h"
#include "console/Console.h"
#include "console/TimingPanel.h"
#include "loader/ResourcesManager.h"

namespace brUGE
{
	DEFINE_SINGLETON(render::DebugDrawer)

namespace render
{
	//---------------------------------------------------------------------------------------------
	DebugDrawer::DebugDrawer() : m_isEnabled(false)
	{

	}

	//---------------------------------------------------------------------------------------------
	bool DebugDrawer::init()
	{
		REGISTER_CONSOLE_METHOD("r_drawDebugInfo", _drawDebugInfo, DebugDrawer);
		m_vertices.reserve(1024);

		return _setupRender();
	}

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::destroy()
	{
		//-- release render resources.
		m_VB.reset();
		m_shader.reset();
	}

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::drawLine(const vec3f& start, const vec3f& end, const Color& color)
	{
		if (!m_isEnabled) return;

		m_vertices.push_back(VertDesc(start, color));
		m_vertices.push_back(VertDesc(end,	 color));
	}

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::drawCoordAxis(const mat4f& orient, float scale)
	{
		if (!m_isEnabled) return;
		
		vec3f origin = orient.applyToOrigin();

		drawLine(origin, origin + orient.applyToUnitAxis(0).getNormalized().scale(scale), Color(0,1,0));
		drawLine(origin, origin + orient.applyToUnitAxis(1).getNormalized().scale(scale), Color(0,0,1));
		drawLine(origin, origin + orient.applyToUnitAxis(2).getNormalized().scale(scale), Color(1,0,0));
	}

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::drawAABB(const AABB& aabb, const Color& color)
	{
		if (!m_isEnabled) return;

		vec3f lb1(aabb.min);
		vec3f lb2(aabb.min.x, aabb.min.y, aabb.max.z);
		vec3f lt1(aabb.min.x, aabb.max.y, aabb.min.z);
		vec3f lt2(aabb.min.x, aabb.max.y, aabb.max.z);
		vec3f rb1(aabb.max.x, aabb.min.y, aabb.min.z);
		vec3f rb2(aabb.max.x, aabb.min.y, aabb.max.z);
		vec3f rt1(aabb.max.x, aabb.max.y, aabb.min.z);
		vec3f rt2(aabb.max);
		
		//-- min corner.
		drawLine(lb1, lt1, color);
		drawLine(lb1, lb2, color);
		drawLine(lb1, rb1, color);

		drawLine(lt2, lt1, color);
		drawLine(lt2, lb2, color);
		drawLine(lt2, rt2, color);

		drawLine(rt1, lt1, color);
		drawLine(rt1, rt2, color);
		drawLine(rt1, rb1, color);

		drawLine(rb2, lb2, color);
		drawLine(rb2, rt2, color);
		drawLine(rb2, rb1, color);
	}

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::drawAABB(const AABB& aabb, const Color& color, const mat4f& transform)
	{
		if (!m_isEnabled) return;

		AABB tmp(aabb);
		tmp.transform(transform);

		drawAABB(tmp, color);
	}

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::drawOBB(const OBB& obb, const Color& color)
	{
		if (!m_isEnabled) return;

		vec3f min(-1.0f, -1.0f, -1.0f);
		vec3f max(+1.0f, +1.0f, +1.0f);

		vec3f lb1(min);
		vec3f lb2(min.x, min.y, max.z);
		vec3f lt1(min.x, max.y, min.z);
		vec3f lt2(min.x, max.y, max.z);
		vec3f rb1(max.x, min.y, min.z);
		vec3f rb2(max.x, min.y, max.z);
		vec3f rt1(max.x, max.y, min.z);
		vec3f rt2(max);

		lb1 = obb.transform().applyToPoint(lb1);
		lb2 = obb.transform().applyToPoint(lb2);
		lt1 = obb.transform().applyToPoint(lt1);
		lt2 = obb.transform().applyToPoint(lt2);
		rb1 = obb.transform().applyToPoint(rb1);
		rb2 = obb.transform().applyToPoint(rb2);
		rt1 = obb.transform().applyToPoint(rt1);
		rt2 = obb.transform().applyToPoint(rt2);

		//-- min corner.
		drawLine(lb1, lt1, color);
		drawLine(lb1, lb2, color);
		drawLine(lb1, rb1, color);

		drawLine(lt2, lt1, color);
		drawLine(lt2, lb2, color);
		drawLine(lt2, rt2, color);

		drawLine(rt1, lt1, color);
		drawLine(rt1, rt2, color);
		drawLine(rt1, rb1, color);

		drawLine(rb2, lb2, color);
		drawLine(rb2, rt2, color);
		drawLine(rb2, rb1, color);
	}

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::drawFrustum(
		const Color& color, const mat4f& orient, float fov, float aspect, float nearDist, float farDist)
	{
		if (!m_isEnabled) return;

		float tg_v = tan(math::degToRad(fov) * 0.5f);
		float tg_h = tan(math::degToRad(fov) * 0.5f * aspect);

		vec3f up_far	(0, tg_v * farDist, 0);
		vec3f side_far	(tg_h * farDist, 0, 0);
		vec3f up_near	(0, tg_v * nearDist, 0);
		vec3f side_near	(tg_h * nearDist, 0, 0);
		vec3f near_p	(0, 0, nearDist);
		vec3f far_p		(0, 0, farDist);

		vec3f v1 = orient.applyToPoint(near_p + up_near - side_near);
		vec3f v2 = orient.applyToPoint(near_p - up_near - side_near);
		vec3f v3 = orient.applyToPoint(near_p - up_near + side_near);
		vec3f v4 = orient.applyToPoint(near_p + up_near + side_near);

		vec3f v5 = orient.applyToPoint(far_p + up_far - side_far);
		vec3f v6 = orient.applyToPoint(far_p - up_far - side_far);
		vec3f v7 = orient.applyToPoint(far_p - up_far + side_far);
		vec3f v8 = orient.applyToPoint(far_p + up_far + side_far);

		// верхн€€ плоскость.
		m_vertices.push_back(VertDesc(v1, color));
		m_vertices.push_back(VertDesc(v2, color));

		m_vertices.push_back(VertDesc(v2, color));
		m_vertices.push_back(VertDesc(v3, color));

		m_vertices.push_back(VertDesc(v3, color));
		m_vertices.push_back(VertDesc(v4, color));

		m_vertices.push_back(VertDesc(v4, color));
		m_vertices.push_back(VertDesc(v1, color));

		// нижн€€ плоскость.
		m_vertices.push_back(VertDesc(v5, color));
		m_vertices.push_back(VertDesc(v6, color));

		m_vertices.push_back(VertDesc(v6, color));
		m_vertices.push_back(VertDesc(v7, color));

		m_vertices.push_back(VertDesc(v7, color));
		m_vertices.push_back(VertDesc(v8, color));

		m_vertices.push_back(VertDesc(v8, color));
		m_vertices.push_back(VertDesc(v5, color));

		// грани соедин€ющие верхнюю и нижнюю плоскоть.
		m_vertices.push_back(VertDesc(v1, color));
		m_vertices.push_back(VertDesc(v5, color));

		m_vertices.push_back(VertDesc(v2, color));
		m_vertices.push_back(VertDesc(v6, color));

		m_vertices.push_back(VertDesc(v3, color));
		m_vertices.push_back(VertDesc(v7, color));

		m_vertices.push_back(VertDesc(v4, color));
		m_vertices.push_back(VertDesc(v8, color));

		/*
		mat4f transf;
		transf.setPerspectiveProj(fov, aspect, nearDist, farDist);
		transf.invert();
		transf = orient;
		//transf.postMultiply(orient);
		
		vec3f v1 = transf.applyToPoint(vec3f(-1.f, 1.f, 1.f));
		vec3f v2 = transf.applyToPoint(vec3f(-1.f, 1.f,-1.f));
		vec3f v3 = transf.applyToPoint(vec3f( 1.f, 1.f,-1.f));
		vec3f v4 = transf.applyToPoint(vec3f( 1.f, 1.f, 1.f));

		vec3f v5 = transf.applyToPoint(vec3f(-1.f,-1.f, 1.f));
		vec3f v6 = transf.applyToPoint(vec3f(-1.f,-1.f,-1.f));
		vec3f v7 = transf.applyToPoint(vec3f( 1.f,-1.f,-1.f));
		vec3f v8 = transf.applyToPoint(vec3f( 1.f,-1.f, 1.f));
		
		// верхн€€ плоскость.
		m_vertices.push_back(VertDesc(v1, color));
		m_vertices.push_back(VertDesc(v2, color));

		m_vertices.push_back(VertDesc(v2, color));
		m_vertices.push_back(VertDesc(v3, color));

		m_vertices.push_back(VertDesc(v3, color));
		m_vertices.push_back(VertDesc(v4, color));

		m_vertices.push_back(VertDesc(v4, color));
		m_vertices.push_back(VertDesc(v1, color));

		// нижн€€ плоскость.
		m_vertices.push_back(VertDesc(v5, color));
		m_vertices.push_back(VertDesc(v6, color));

		m_vertices.push_back(VertDesc(v6, color));
		m_vertices.push_back(VertDesc(v7, color));

		m_vertices.push_back(VertDesc(v7, color));
		m_vertices.push_back(VertDesc(v8, color));

		m_vertices.push_back(VertDesc(v8, color));
		m_vertices.push_back(VertDesc(v5, color));

		// грани соедин€ющие верхнюю и нижнюю плоскоть.
		m_vertices.push_back(VertDesc(v1, color));
		m_vertices.push_back(VertDesc(v5, color));

		m_vertices.push_back(VertDesc(v2, color));
		m_vertices.push_back(VertDesc(v6, color));

		m_vertices.push_back(VertDesc(v3, color));
		m_vertices.push_back(VertDesc(v7, color));

		m_vertices.push_back(VertDesc(v4, color));
		m_vertices.push_back(VertDesc(v8, color));
		*/
	}

	//---------------------------------------------------------------------------------------------
	bool DebugDrawer::_setupRender()
	{
		//-- shader.
		{
			m_shader = ResourcesManager::instance().loadShader("debug");
			if (!m_shader)
				return false;
		}
		
		//-- create vertex input and vertex buffer.
		{
			VertexDesc desc[] = 
			{
				{ 0, TYPE_POSITION, FORMAT_FLOAT, 3 },
				{ 0, TYPE_COLOR,	FORMAT_FLOAT, 4 }
			};
			m_VL = rd()->createVertexLayout(desc, 2, *m_shader.get());

			m_VB = rd()->createBuffer(IBuffer::TYPE_VERTEX, NULL, 2048, sizeof(VertDesc),
				IBuffer::USAGE_DYNAMIC, IBuffer::CPU_ACCESS_WRITE);
		}
		
		//-- create render states.
		{
			DepthStencilStateDesc dsDesc;
			dsDesc.depthEnable    = true;
			dsDesc.depthFunc	  = DepthStencilStateDesc::COMPARE_FUNC_ALWAYS;
			dsDesc.depthWriteMask = false;
			m_stateDS = rd()->createDepthStencilState(dsDesc);

			RasterizerStateDesc rDesc;
			m_stateR = rd()->createRasterizedState(rDesc);
		}
		
		return true;
	}

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::_swapBuffers()
	{
		//-- check has the GPU buffer enough memory to hold all the vertex data.
		if (m_vertices.size() > m_VB->getElemCount())
		{
			m_VB = rd()->createBuffer(IBuffer::TYPE_VERTEX, NULL, m_vertices.size() * 2,
				sizeof(VertDesc), IBuffer::USAGE_DYNAMIC, IBuffer::CPU_ACCESS_WRITE);

			ConWarning("[DebugDrawer] Reallocate the GPU vertex buffer to hold all the vertex data."
				"The new size is = %d kb", m_VB->getSize() / 1024);
		}

		void* vb = m_VB->map<void>(IBuffer::ACCESS_WRITE_DISCARD);
			memcpy(vb, &m_vertices[0], sizeof(VertDesc) * m_vertices.size());
		m_VB->unmap();		
	}

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::draw(const mat4f& viewProjMat, float /*dt*/)
	{
		SCOPED_TIME_MEASURER_EX("DebugDrawer draw")

		if (!m_isEnabled) return;
		
		//-- load vertices into GPU's VB.
		_swapBuffers();
		
		rd()->setDepthStencilState(m_stateDS, 0);
		rd()->setRasterizerState(m_stateR);
		rd()->setBlendState(NULL, NULL, 0xffffffff);

		rd()->setVertexLayout(m_VL);
		rd()->setVertexBuffer(0, m_VB.get());

		rd()->setShader(m_shader.get());
		{
			m_shader->setMat4f("g_viewProjMat", viewProjMat);
		}

		rd()->draw(PRIM_TOPOLOGY_LINE_LIST, 0, m_vertices.size());

		m_vertices.clear();
	}
	
	//---------------------------------------------------------------------------------------------
	int DebugDrawer::_drawDebugInfo(bool flag)
	{
		m_isEnabled = flag;
		return 0;
	}

} // render
} // brUGE