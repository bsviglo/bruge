#include "DebugDrawer.h"
#include "Camera.h"
#include "IRenderDevice.h"
#include "IBuffer.h"
#include "IShader.h"
#include "state_objects.h"
#include "os/FileSystem.h"
#include "console/TimingPanel.h"
#include "loader/ResourcesManager.h"
#include "gui/imgui/imgui.h"

using namespace brUGE::os;
using namespace brUGE::math;

namespace brUGE
{
	DEFINE_SINGLETON(render::DebugDrawer)

namespace render
{
	//---------------------------------------------------------------------------------------------
	DebugDrawer::DebugDrawer() : m_isEnabled(true)
	{

	}

	//---------------------------------------------------------------------------------------------
	DebugDrawer::~DebugDrawer()
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

			m_pVB = m_VB.get();
		}

		//-- load wire material.
		{
			RODataPtr file = FileSystem::instance().readFile("resources/materials/debug_wire.mtl");
			if (!file || !(m_wireMaterial = rs().materials().createMaterial(*file)))
			{
				return false;
			}

			//-- create rops for drawing.
			{
				RenderOp op;
				op.m_primTopolpgy = PRIM_TOPOLOGY_LINE_LIST;
				op.m_VBs		  = &m_pVB;
				op.m_VBCount	  = 1;
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
						
			m_meshes[MT_BOX]	    = rm.loadMesh("system/meshes/box.mesh");
			m_meshes[MT_CYLINDER]   = rm.loadMesh("system/meshes/cylinder.mesh");
			m_meshes[MT_SPHERE]     = rm.loadMesh("system/meshes/sphere.mesh");
			m_meshes[MT_HEMISPHERE] = rm.loadMesh("system/meshes/hemisphere.mesh");

			//-- check result.
			for (uint i = 0; i < MT_COUNT; ++i)
			{
				if (!m_meshes[i])
				{
					ERROR_MSG("Failed to load one of the system models.");
					return false;
				}
			}

			//-- load solid material.
			RODataPtr file = FileSystem::instance().readFile("resources/materials/debug_solid.mtl");
			if (!file || !(m_solidMaterial = rs().materials().createMaterial(*file)))
			{
				return false;
			}
		}

		return true;
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

		// ������� ���������.
		m_vertices.push_back(VertDesc(v1, color.toVec4()));
		m_vertices.push_back(VertDesc(v2, color.toVec4()));

		m_vertices.push_back(VertDesc(v2, color.toVec4()));
		m_vertices.push_back(VertDesc(v3, color.toVec4()));

		m_vertices.push_back(VertDesc(v3, color.toVec4()));
		m_vertices.push_back(VertDesc(v4, color.toVec4()));

		m_vertices.push_back(VertDesc(v4, color.toVec4()));
		m_vertices.push_back(VertDesc(v1, color.toVec4()));

		// ������ ���������.
		m_vertices.push_back(VertDesc(v5, color.toVec4()));
		m_vertices.push_back(VertDesc(v6, color.toVec4()));

		m_vertices.push_back(VertDesc(v6, color.toVec4()));
		m_vertices.push_back(VertDesc(v7, color.toVec4()));

		m_vertices.push_back(VertDesc(v7, color.toVec4()));
		m_vertices.push_back(VertDesc(v8, color.toVec4()));

		m_vertices.push_back(VertDesc(v8, color.toVec4()));
		m_vertices.push_back(VertDesc(v5, color.toVec4()));

		// ����� ����������� ������� � ������ ��������.
		m_vertices.push_back(VertDesc(v1, color.toVec4()));
		m_vertices.push_back(VertDesc(v5, color.toVec4()));

		m_vertices.push_back(VertDesc(v2, color.toVec4()));
		m_vertices.push_back(VertDesc(v6, color.toVec4()));

		m_vertices.push_back(VertDesc(v3, color.toVec4()));
		m_vertices.push_back(VertDesc(v7, color.toVec4()));

		m_vertices.push_back(VertDesc(v4, color.toVec4()));
		m_vertices.push_back(VertDesc(v8, color.toVec4()));

		//------------------------------------------------------------------------------------------
		/*
		mat4f transf;
		transf.setPerspectiveProj(fov, aspect, nearDist, farDist);
		transf.preMultiply(orient);
		transf.invert();
		
		vec4f v1 = transf.applyToPoint(vec4f(-1.f, 1.f, 1.f, 1.0f));
		vec4f v2 = transf.applyToPoint(vec4f(-1.f, 1.f, 0.f, 1.0f));
		vec4f v3 = transf.applyToPoint(vec4f( 1.f, 1.f, 0.f, 1.0f));
		vec4f v4 = transf.applyToPoint(vec4f( 1.f, 1.f, 1.f, 1.0f));

		vec4f v5 = transf.applyToPoint(vec4f(-1.f,-1.f, 1.f, 1.0f));
		vec4f v6 = transf.applyToPoint(vec4f(-1.f,-1.f, 0.f, 1.0f));
		vec4f v7 = transf.applyToPoint(vec4f( 1.f,-1.f, 0.f, 1.0f));
		vec4f v8 = transf.applyToPoint(vec4f( 1.f,-1.f, 1.f, 1.0f));
		
		// ������� ���������.
		m_vertices.push_back(VertDesc(v1, color));
		m_vertices.push_back(VertDesc(v2, color));

		m_vertices.push_back(VertDesc(v2, color));
		m_vertices.push_back(VertDesc(v3, color));

		m_vertices.push_back(VertDesc(v3, color));
		m_vertices.push_back(VertDesc(v4, color));

		m_vertices.push_back(VertDesc(v4, color));
		m_vertices.push_back(VertDesc(v1, color));

		// ������ ���������.
		m_vertices.push_back(VertDesc(v5, color));
		m_vertices.push_back(VertDesc(v6, color));

		m_vertices.push_back(VertDesc(v6, color));
		m_vertices.push_back(VertDesc(v7, color));

		m_vertices.push_back(VertDesc(v7, color));
		m_vertices.push_back(VertDesc(v8, color));

		m_vertices.push_back(VertDesc(v8, color));
		m_vertices.push_back(VertDesc(v5, color));

		// ����� ����������� ������� � ������ ��������.
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
	void DebugDrawer::drawBox(const vec3f& size, const mat4f& world, const Color& color, EDrawType drawType)
	{
		if (!m_isEnabled) return;

		MeshInstance instance(world, color);
		instance.m_world.preScale(size);

		m_meshCaches[MT_BOX][drawType].push_back(instance);
	}

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::drawCylinder(float radius, float halfHeight, const mat4f& world, const Color& color, EDrawType drawType)
	{
		if (!m_isEnabled) return;

		MeshInstance instance(world, color);
		instance.m_world.preScale(radius, halfHeight * 2.0f, radius);

		m_meshCaches[MT_CYLINDER][drawType].push_back(instance);
	}

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::drawSphere(float radius, const mat4f& world, const Color& color, EDrawType drawType)
	{
		if (!m_isEnabled) return;

		MeshInstance instance(world, color);
		instance.m_world.preScale(radius, radius, radius);

		m_meshCaches[MT_SPHERE][drawType].push_back(instance);
	}

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::drawHemiSphere(float radius, const mat4f& world, const Color& color, EDrawType drawType)
	{
		if (!m_isEnabled) return;

		MeshInstance instance(world, color);
		instance.m_world.preScale(radius, radius, radius);

		m_meshCaches[MT_HEMISPHERE][drawType].push_back(instance);
	}

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::drawCapsule(float radius, float halfHeight, const mat4f& world, const Color& color, EDrawType drawType)
	{
		if (!m_isEnabled) return;

		//-- top sphere
		mat4f topSphereMat = world;
		topSphereMat.preTranslation(vec3f(0.0f, halfHeight, 0.0f));
		drawSphere(radius, topSphereMat, color, drawType);

		//-- bottom sphere
		mat4f bottomSphere = world;
		bottomSphere.preTranslation(vec3f(0.0f, -halfHeight, 0.0f));
		drawSphere(radius, bottomSphere, color, drawType);

		//-- cylinder
		drawCylinder(radius, halfHeight, world, color, drawType);
	}

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

		if (void* vb = m_VB->map<void>(IBuffer::ACCESS_WRITE_DISCARD))
		{
			memcpy(vb, &m_vertices[0], sizeof(VertDesc) * m_vertices.size());
			m_VB->unmap();
		}

		m_pVB = m_VB.get();
		m_wireROPs[0].m_VBs = &m_pVB;
	}

	//---------------------------------------------------------------------------------------------
	void DebugDrawer::draw()
	{
		if (!m_isEnabled) return;
		
		//-- 1. do solid geometry drawing.
		{
			//-- gather all render operations for the whole set of render passes.
			RenderOps rops;
			for (uint pass = 0; pass < DT_COUNT; ++pass)
			{
				//-- iterate over the hole set of mesh types.
				for (uint type = 0; type < MT_COUNT; ++type)
				{
					const MeshesCache& cache = m_meshCaches[type][pass];

					//-- check cache capacity.
					if (!cache.empty())
					{
						//-- ToDo: reconsider.
						m_meshes[type]->gatherROPs(RenderSystem::PASS_Z_ONLY, false, rops);

						rops.back().m_material		= m_solidMaterial->renderFx();
						rops.back().m_instanceTB	= m_instancingTB.get();
						rops.back().m_instanceSize  = sizeof(MeshInstance);
						rops.back().m_instanceCount = cache.size();
						rops.back().m_instanceData  = &cache.front();
					}
				}

				//-- retrieve desired render pass and draw it.
				const RenderSystem::EPassType renderPasses[] = {
					RenderSystem::PASS_DEBUG_SOLID,
					RenderSystem::PASS_DEBUG_TRANSPARENT,
					RenderSystem::PASS_DEBUG_OVERRIDE
				};

				rs().beginPass(renderPasses[pass]);
				rs().addROPs(rops);
				rs().endPass();

				//-- prepare rops for the next pass.
				rops.clear();
			}

			//-- clear caches.
			for (uint i = 0; i < MT_COUNT; ++i)
				for (uint j = 0; j < DT_COUNT; ++j)
					m_meshCaches[i][j].clear();
		}
		
		//-- 2. do wire geometry drawing.
		if (!m_vertices.empty())
		{
			//-- load vertices into GPU's VB.
			_swapBuffers();

			//-- update some rop's information.
			m_wireROPs[0].m_indicesCount = m_vertices.size();

			rs().beginPass(RenderSystem::PASS_DEBUG_WIRE);
			rs().addROPs(m_wireROPs);
			rs().endPass();

			//-- clear caches.
			m_vertices.clear();
		}
		
		//-- ToDo: reconsider.
		//-- 3. do text drawing.
		{
			if (!m_textDataVec.empty())
			{
				ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
				ImGui::Begin("Debug Drawer", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs);
				ImGui::GetWindowDrawList()->PushClipRectFullScreen();

				for (const auto& data : m_textDataVec)
				{
					vec4f projPos = rs().camera()->m_viewProj.applyToPoint(data.m_pos.toVec4());
					if (!almostZero(projPos.w) && projPos.w > 0)
					{
						vec2f clipPos(projPos.x / projPos.w, projPos.y / projPos.w);
						vec2f curPos(
							(0.5f * (1.0f + clipPos.x)) * rs().screenRes().width,
							(0.5f * (1.0f - clipPos.y)) * rs().screenRes().height
						);

						ImGui::GetWindowDrawList()->AddText(
							ImVec2(curPos.x, curPos.y),
							ImGui::ColorConvertFloat4ToU32(ImVec4(data.m_color.r, data.m_color.g, data.m_color.b, data.m_color.a)),
							data.m_text.c_str()
							);
					}
				}

				ImGui::GetWindowDrawList()->PopClipRect();
				ImGui::End();
				ImGui::PopStyleColor();
			}

			//-- clear text data list.
			m_textDataVec.clear();
		}
	}

} // render
} // brUGE