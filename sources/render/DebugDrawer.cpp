#include "DebugDrawer.h"
#include "Camera.h"
#include "IRenderDevice.h"
#include "IBuffer.h"
#include "IShader.h"
#include "state_objects.h"
#include "os/FileSystem.h"
#include "console/Console.h"
#include "console/TimingPanel.h"
#include "loader/ResourcesManager.h"

using namespace brUGE::os;
using namespace brUGE::math;

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
		REGISTER_CONSOLE_MEMBER_VALUE("r_drawDebugInfo", bool, m_isEnabled, DebugDrawer);
		m_vertices.reserve(1024);

		//-- create wire geometry vertex buffer.
		{
			m_VB = rd()->createBuffer(IBuffer::TYPE_VERTEX, NULL, 2048, sizeof(VertDesc),
				IBuffer::USAGE_DYNAMIC, IBuffer::CPU_ACCESS_WRITE);
		}

		//-- load wire material.
		{
			RODataPtr file = FileSystem::instance().readFile("resources/materials/debug_wire.mtl");
			if (!file || !(m_wireMaterial = rs().materials()->createMaterial(*file)))
			{
				return false;
			}

			//-- create rops for drawing.
			{
				RenderOp op;
				op.m_primTopolpgy = PRIM_TOPOLOGY_LINE_LIST;
				op.m_mainVB		  = &*m_VB;
				op.m_indicesCount = 0;
				op.m_material	  = m_wireMaterial->renderFx();

				m_wireROPs.push_back(op);
			}
		}

		//-- create solid geometry instancing buffer
		{
			m_instancingTB = rd()->createBuffer(
				IBuffer::TYPE_TEXTURE, NULL, 512 * sizeof(MeshInstance) / sizeof(vec4f),
				sizeof(vec4f), IBuffer::USAGE_DYNAMIC, IBuffer::CPU_ACCESS_WRITE
				);
		}

		//-- load all standard system meshes.
		{
			ResourcesManager& rm = ResourcesManager::instance();
						
			m_meshes[MT_BOX]	  = rm.loadMesh("system/meshes/box.obj");
			m_meshes[MT_CYLINDER] = rm.loadMesh("system/meshes/cylinder.obj");
			m_meshes[MT_SPHERE]   = rm.loadMesh("system/meshes/sphere.obj");

			//-- check result.
			for (uint i = 0; i < MT_COUNT; ++i)
			{
				if (!m_meshes[i].isValid())
				{
					ERROR_MSG("Failed to load one of the system models.");
					return false;
				}
			}
		}

		//-- font.
		{
			m_font = ResourcesManager::instance().loadFont("system/font/VeraMono", 12, vec2ui(32, 127));
			if (!m_font.isValid())
				return false;
		}

		return true;
	}

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::destroy()
	{
		//-- release render resources.

		//-- wire geometry.
		m_VB.reset();
		m_wireMaterial.reset();

		//-- solid geometry.
		m_instancingTB.reset();
		for (uint i = 0; i < MT_COUNT; ++i)
			m_meshes[i].reset();

		//-- text rendering.
		m_font.reset();
	}

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::drawLine(const vec3f& start, const vec3f& end, const Color& color)
	{
		if (!m_isEnabled) return;

		m_vertices.push_back(VertDesc(start, color.toVec4()));
		m_vertices.push_back(VertDesc(end,	 color.toVec4()));
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

		vec3f lb1(aabb.m_min);
		vec3f lb2(aabb.m_min.x, aabb.m_min.y, aabb.m_max.z);
		vec3f lt1(aabb.m_min.x, aabb.m_max.y, aabb.m_min.z);
		vec3f lt2(aabb.m_min.x, aabb.m_max.y, aabb.m_max.z);
		vec3f rb1(aabb.m_max.x, aabb.m_min.y, aabb.m_min.z);
		vec3f rb2(aabb.m_max.x, aabb.m_min.y, aabb.m_max.z);
		vec3f rt1(aabb.m_max.x, aabb.m_max.y, aabb.m_min.z);
		vec3f rt2(aabb.m_max);
		
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
		m_vertices.push_back(VertDesc(v1, color.toVec4()));
		m_vertices.push_back(VertDesc(v2, color.toVec4()));

		m_vertices.push_back(VertDesc(v2, color.toVec4()));
		m_vertices.push_back(VertDesc(v3, color.toVec4()));

		m_vertices.push_back(VertDesc(v3, color.toVec4()));
		m_vertices.push_back(VertDesc(v4, color.toVec4()));

		m_vertices.push_back(VertDesc(v4, color.toVec4()));
		m_vertices.push_back(VertDesc(v1, color.toVec4()));

		// нижн€€ плоскость.
		m_vertices.push_back(VertDesc(v5, color.toVec4()));
		m_vertices.push_back(VertDesc(v6, color.toVec4()));

		m_vertices.push_back(VertDesc(v6, color.toVec4()));
		m_vertices.push_back(VertDesc(v7, color.toVec4()));

		m_vertices.push_back(VertDesc(v7, color.toVec4()));
		m_vertices.push_back(VertDesc(v8, color.toVec4()));

		m_vertices.push_back(VertDesc(v8, color.toVec4()));
		m_vertices.push_back(VertDesc(v5, color.toVec4()));

		// грани соедин€ющие верхнюю и нижнюю плоскоть.
		m_vertices.push_back(VertDesc(v1, color.toVec4()));
		m_vertices.push_back(VertDesc(v5, color.toVec4()));

		m_vertices.push_back(VertDesc(v2, color.toVec4()));
		m_vertices.push_back(VertDesc(v6, color.toVec4()));

		m_vertices.push_back(VertDesc(v3, color.toVec4()));
		m_vertices.push_back(VertDesc(v7, color.toVec4()));

		m_vertices.push_back(VertDesc(v4, color.toVec4()));
		m_vertices.push_back(VertDesc(v8, color.toVec4()));

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
	void DebugDrawer::drawText2D(const char* text, const vec3f& pos, const Color& color)
	{
		if (!m_isEnabled) return;

		TextData data;
		data.m_pos	 = pos;
		data.m_text  = text;
		data.m_color = color;

		m_textDataVec.push_back(data);
	}

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::drawBox(const vec3f& size, const mat4f& world, const Color& color)
	{
		if (!m_isEnabled) return;

		MeshInstance instance(world, color);
		instance.m_world.preScale(size);

		m_meshCaches[MT_BOX].push_back(instance);
	}

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::drawCylinderY(float radius, float height, const mat4f& world, const Color& color)
	{
		if (!m_isEnabled) return;

		MeshInstance instance(world, color);
		instance.m_world.preScale(vec3f(radius, height, radius));

		m_meshCaches[MT_CYLINDER].push_back(instance);
	}

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::drawCylinderX(float radius, float height, const mat4f& world, const Color& color)
	{
		if (!m_isEnabled) return;

		MeshInstance instance(world, color);
		instance.m_world.postRotateZ(degToRad(90.0f));
		instance.m_world.preScale(vec3f(radius, height, radius));

		m_meshCaches[MT_CYLINDER].push_back(instance);
	}

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::drawCylinderZ(float radius, float height, const mat4f& world, const Color& color)
	{
		if (!m_isEnabled) return;

		MeshInstance instance(world, color);
		instance.m_world.postRotateX(degToRad(90.0f));
		instance.m_world.preScale(vec3f(radius, height, radius));

		m_meshCaches[MT_CYLINDER].push_back(instance);
	}

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::drawSphere(float radius, const mat4f& world, const Color& color)
	{
		if (!m_isEnabled) return;

		MeshInstance instance(world, color);
		instance.m_world.preScale(radius, radius, radius);

		m_meshCaches[MT_SPHERE].push_back(instance);
	}

	/*
	//---------------------------------------------------------------------------------------------
	void DebugDrawer::drawCapsuleX(float radius, float height, const mat4f& world, const Color& color)
	{
		if (!m_isEnabled) return;

		MeshInstance instance(world, color);
		instance.m_world.preScale(radius, radius, radius);

		m_meshCaches[MT_CYLINDER].push_back(instance);
	}

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::drawCapsuleY(float radius, float height, const mat4f& world, const Color& color)
	{

	}

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::drawCapsuleZ(float radius, float height, const mat4f& world, const Color& color)
	{

	}
	*/

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::_swapBuffers()
	{
		if (m_vertices.empty()) return;

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

		m_wireROPs[0].m_mainVB = m_VB.get();
	}

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::draw()
	{
		if (!m_isEnabled) return;
		
		//-- 1. do solid geometry drawing.
		{
			//-- gather all render operations.
			RenderOps rops;
			for (uint i = 0; i < MT_COUNT; ++i)
			{
				if (m_meshCaches[i].size())
				{
					m_meshes[i]->gatherRenderOps(rops);

					rops.back().m_instanceTB	= m_instancingTB.get();
					rops.back().m_instanceSize  = sizeof(MeshInstance);
					rops.back().m_instanceCount = m_meshCaches[i].size();
					rops.back().m_instanceData  = &m_meshCaches[i].front();
				}
			}

			rs().beginPass(RenderSystem::PASS_DEBUG_SOLID);
			rs().addRenderOps(rops);
			rs().endPass();

			//-- clear caches.
			for (uint i = 0; i < MT_COUNT; ++i)
				m_meshCaches[i].clear();
		}
		
		//-- 2. do wire geometry drawing.
		{
			//-- load vertices into GPU's VB.
			_swapBuffers();

			//-- update some rop's information.
			m_wireROPs[0].m_indicesCount = m_vertices.size();

			rs().beginPass(RenderSystem::PASS_DEBUG_WIRE);
			rs().addRenderOps(m_wireROPs);
			rs().endPass();

			//-- clear caches.
			m_vertices.clear();
		}
		
		//-- ToDo: reconsider.
		//-- 3. do text drawing.
		{
			if (!m_textDataVec.empty())
			{
				m_font->beginDraw();
				for (uint i = 0; i < m_textDataVec.size(); ++i)
				{
					const TextData& data = m_textDataVec[i];

					vec4f projPos = rs().camera().viewProjMatrix().applyToPoint(data.m_pos.toVec4());
					if (!almostZero(projPos.w) && projPos.w > 0)
					{
						vec2f clipPos(projPos.x / projPos.w, projPos.y / projPos.w);
						vec2f curPos(
							(0.5f * (1.0f + clipPos.x)) * rs().screenRes().width,
							(0.5f * (1.0f - clipPos.y)) * rs().screenRes().height
							);

						m_font->draw2D(curPos, data.m_color, data.m_text);
					}
				}
				m_font->endDraw();

				//-- clear text data list.
				m_textDataVec.clear();
			}
		}
	}

} // render
} // brUGE