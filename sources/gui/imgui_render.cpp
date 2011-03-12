#include "imgui_render.hpp"
#include "render/render_system.hpp"
#include "os/FileSystem.h"
#include "loader/ResourcesManager.h"
#include <cassert>

using namespace brUGE;
using namespace brUGE::render;
using namespace brUGE::math;
using namespace brUGE::utils;
using namespace brUGE::os;


//-- start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	const uint  g_tempCoordCount	= 100;
	const uint  g_circleVertsCount  = 3 * 4;
	float		g_circleVerts[g_circleVertsCount * 2];

	typedef brUGE::render::imguiRender::GuiVertex GuiVertex;

	//----------------------------------------------------------------------------------------------
	void gatherPolygonVertices(
		std::vector<GuiVertex>& outVertices,
		std::vector<float>& tempNormals, std::vector<float>& tempCoords,
		const float* coords, uint numCoords, float r, uint col)
	{
		//-- to find problem as soon as possible.
		assert(numCoords < 250);

		//tempCoords.resize (numCoords);
		//tempNormals.resize(numCoords);

		for (uint i = 0, j = numCoords - 1; i < numCoords; j = i++)
		{
			const float* v0 = &coords[j*2];
			const float* v1 = &coords[i*2];
			float dx = v1[0] - v0[0];
			float dy = v1[1] - v0[1];
			float d = sqrtf(dx*dx+dy*dy);
			if (d > 0)
			{
				d = 1.0f/d;
				dx *= d;
				dy *= d;
			}
			tempNormals[j*2+0] = dy;
			tempNormals[j*2+1] = -dx;
		}

		for (unsigned i = 0, j = numCoords - 1; i < numCoords; j = i++)
		{
			float dlx0 = tempNormals[j*2+0];
			float dly0 = tempNormals[j*2+1];
			float dlx1 = tempNormals[i*2+0];
			float dly1 = tempNormals[i*2+1];
			float dmx = (dlx0 + dlx1) * 0.5f;
			float dmy = (dly0 + dly1) * 0.5f;
			float	dmr2 = dmx*dmx + dmy*dmy;
			if (dmr2 > 0.000001f)
			{
				float	scale = 1.0f / dmr2;
				if (scale > 10.0f) scale = 10.0f;
				dmx *= scale;
				dmy *= scale;
			}
			tempCoords[i*2+0] = coords[i*2+0]+dmx*r;
			tempCoords[i*2+1] = coords[i*2+1]+dmy*r;
		}

		Color colTrans;
		colTrans.setABGR(col);
		colTrans.a = 0.0f;

		Color colOrigin;
		colOrigin.setABGR(col);

		for (uint i = 0, j = numCoords - 1; i < numCoords; j = i++)
		{
			outVertices.push_back(GuiVertex(vec3f(coords[i * 2 + 0],		coords[i * 2 + 1],		 0.0f), colOrigin));
			outVertices.push_back(GuiVertex(vec3f(coords[j * 2 + 0],		coords[j * 2 + 1],		 0.0f), colOrigin));

			outVertices.push_back(GuiVertex(vec3f(tempCoords[j * 2 + 0],	tempCoords[j * 2 + 1],   0.0f), colTrans));
			outVertices.push_back(GuiVertex(vec3f(tempCoords[j * 2 + 0],	tempCoords[j * 2 + 1],   0.0f), colTrans));
			outVertices.push_back(GuiVertex(vec3f(tempCoords[i * 2 + 0],	tempCoords[i * 2 + 1],   0.0f), colTrans));

			outVertices.push_back(GuiVertex(vec3f(coords[i * 2 + 0],		coords[i * 2 + 1],		 0.0f), colOrigin));
		}

		for (uint i = 2; i < numCoords; ++i)
		{
			outVertices.push_back(GuiVertex(vec3f(coords[0 + 0],			coords[0 + 1],			 0.0f),	colOrigin));
			outVertices.push_back(GuiVertex(vec3f(coords[(i - 1) * 2 + 0],	coords[(i - 1) * 2 + 1], 0.0f), colOrigin));
			outVertices.push_back(GuiVertex(vec3f(coords[i * 2 + 0],		coords[i * 2 + 1],		 0.0f),	colOrigin));
		}
	}

	//----------------------------------------------------------------------------------------------
	void gatherRectVertices(
		std::vector<GuiVertex>& outVertices,
		std::vector<float>& tempNormals, std::vector<float>& tempCoords,
		float x, float y, float w, float h, float fth, unsigned int col)
	{
		float verts[4*2] =
		{
			x +     0.5f, y +     0.5f,
			x + w - 0.5f, y +     0.5f,
			x + w - 0.5f, y + h - 0.5f,
			x + 0.5f    , y + h - 0.5f,
		};
		gatherPolygonVertices(outVertices, tempNormals, tempCoords, verts, 4, fth, col);
	}

	//----------------------------------------------------------------------------------------------
	void gatherRoundedRectVertices(
		std::vector<GuiVertex>& outVertices,
		std::vector<float>& tempNormals, std::vector<float>& tempCoords,
		float x, float y, float w, float h, float r, float fth, unsigned int col)
	{
		const uint n = g_circleVertsCount / 4;
		float verts[(n + 1) * 4 * 2];

		const float* cverts = g_circleVerts;
		float* v = verts;

		for (unsigned i = 0; i <= n; ++i)
		{
			*v++ = x+w-r + cverts[i*2]*r;
			*v++ = y+h-r + cverts[i*2+1]*r;
		}

		for (unsigned i = n; i <= n*2; ++i)
		{
			*v++ = x+r + cverts[i*2]*r;
			*v++ = y+h-r + cverts[i*2+1]*r;
		}

		for (unsigned i = n*2; i <= n*3; ++i)
		{
			*v++ = x+r + cverts[i*2]*r;
			*v++ = y+r + cverts[i*2+1]*r;
		}

		for (unsigned i = n*3; i < n*4; ++i)
		{
			*v++ = x+w-r + cverts[i*2]*r;
			*v++ = y+r + cverts[i*2+1]*r;
		}
		*v++ = x+w-r + cverts[0]*r;
		*v++ = y+r + cverts[1]*r;

		gatherPolygonVertices(outVertices, tempNormals, tempCoords, verts, (n+1)*4, fth, col);
	}

	//----------------------------------------------------------------------------------------------
	void gatherLineVertices(
		std::vector<GuiVertex>& outVertices,
		std::vector<float>& tempNormals, std::vector<float>& tempCoords,
		float x0, float y0, float x1, float y1, float r, float fth, unsigned int col)
	{
		float dx = x1-x0;
		float dy = y1-y0;
		float d = sqrtf(dx*dx+dy*dy);
		if (d > 0.0001f)
		{
			d = 1.0f/d;
			dx *= d;
			dy *= d;
		}
		float nx = dy;
		float ny = -dx;
		float verts[4*2];
		r -= fth;
		r *= 0.5f;
		if (r < 0.01f) r = 0.01f;
		dx *= r;
		dy *= r;
		nx *= r;
		ny *= r;

		verts[0] = x0-dx-nx;
		verts[1] = y0-dy-ny;

		verts[2] = x0-dx+nx;
		verts[3] = y0-dy+ny;

		verts[4] = x1+dx+nx;
		verts[5] = y1+dy+ny;

		verts[6] = x1+dx-nx;
		verts[7] = y1+dy-ny;

		gatherPolygonVertices(outVertices, tempNormals, tempCoords, verts, 4, fth, col);
	}

}
//--------------------------------------------------------------------------------------------------
//-- end unnamed namespace.


namespace brUGE
{
namespace render
{
	//----------------------------------------------------------------------------------------------
	imguiRender::imguiRender() : m_scissor(false)
	{

	}

	//----------------------------------------------------------------------------------------------
	imguiRender::~imguiRender()
	{

	}

	//----------------------------------------------------------------------------------------------
	bool imguiRender::init()
	{
		for (int i = 0; i < g_circleVertsCount; ++i)
		{
			float a = (float)i/(float)g_circleVertsCount * PI*2;
			g_circleVerts[i*2+0] = cosf(a);
			g_circleVerts[i*2+1] = sinf(a);
		}

		m_vertices.reserve(1000);
		m_texDrawOps.reserve(100);

		m_tempCoords.resize(200);
		m_tempNormals.resize(200);

		return _setupRender();
	}

	//----------------------------------------------------------------------------------------------
	bool imguiRender::fini()
	{
		m_vb.reset();
		m_font.reset();
		m_material.reset();

		return true;
	}

	//----------------------------------------------------------------------------------------------
	void imguiRender::draw()
	{
		const imguiGfxCmd* q = imguiGetRenderQueue();
		int nq = imguiGetRenderQueueSize();

		const float s = 1.0f/8.0f;

		//-- disable scissor.
		for (int i = 0; i < nq; ++i)
		{
			const imguiGfxCmd& cmd = q[i];

			if (cmd.type == IMGUI_GFXCMD_RECT)
			{
				if (cmd.rect.r == 0)
				{
					gatherRectVertices(
						m_vertices, m_tempNormals, m_tempCoords,
						(float)cmd.rect.x*s+0.5f, (float)cmd.rect.y*s+0.5f,
						(float)cmd.rect.w*s-1,    (float)cmd.rect.h*s-1,
						1.0f, cmd.col
						);
				}
				else
				{
					gatherRoundedRectVertices(
						m_vertices, m_tempNormals, m_tempCoords,
						(float)cmd.rect.x*s+0.5f, (float)cmd.rect.y*s+0.5f,
						(float)cmd.rect.w*s-1,    (float)cmd.rect.h*s-1,
						(float)cmd.rect.r*s, 1.0f, cmd.col
						);
				}
			}
			else if (cmd.type == IMGUI_GFXCMD_LINE)
			{
				gatherLineVertices(
					m_vertices, m_tempNormals, m_tempCoords,
					cmd.line.x0*s, cmd.line.y0*s, cmd.line.x1*s, cmd.line.y1*s,
					cmd.line.r*s, 1.0f, cmd.col
					);
			}
			else if (cmd.type == IMGUI_GFXCMD_TRIANGLE)
			{
				if (cmd.flags == 1)
				{
					const float verts[3*2] =
					{
						(float)cmd.rect.x*s+0.5f, (float)cmd.rect.y*s+0.5f,
						(float)cmd.rect.x*s+0.5f+(float)cmd.rect.w*s-1, (float)cmd.rect.y*s+0.5f+(float)cmd.rect.h*s/2-0.5f,
						(float)cmd.rect.x*s+0.5f, (float)cmd.rect.y*s+0.5f+(float)cmd.rect.h*s-1,
					};
					gatherPolygonVertices(
						m_vertices, m_tempNormals, m_tempCoords,
						verts, 3, 1.0f, cmd.col
						);
				}
				if (cmd.flags == 2)
				{
					const float verts[3*2] =
					{
						(float)cmd.rect.x*s+0.5f, (float)cmd.rect.y*s+0.5f+(float)cmd.rect.h*s-1,
						(float)cmd.rect.x*s+0.5f+(float)cmd.rect.w*s/2-0.5f, (float)cmd.rect.y*s+0.5f,
						(float)cmd.rect.x*s+0.5f+(float)cmd.rect.w*s-1, (float)cmd.rect.y*s+0.5f+(float)cmd.rect.h*s-1,
					};
					gatherPolygonVertices(
						m_vertices, m_tempNormals, m_tempCoords,
						verts, 3, 1.0f, cmd.col
						);
				}
			}
			else if (cmd.type == IMGUI_GFXCMD_TEXT)
			{
				vec2ui dims = m_font->getStringDim(cmd.text.text);
				vec2f  pos(cmd.text.x, (rs().screenRes().height - cmd.text.y) + dims.y * 0.5f);

				if		(cmd.text.align == IMGUI_ALIGN_CENTER) pos.x -= dims.x / 2;
				else if (cmd.text.align == IMGUI_ALIGN_RIGHT)  pos.x -= dims.x;

				//-- batch draw text operations.
				m_texDrawOps.push_back(
					TextDrawOperation(pos, cmd.text.text, Color(cmd.col))
					);
			}
			else if (cmd.type == IMGUI_GFXCMD_SCISSOR)
			{
				//-- do real draw
				//-- 1. polygons
				//-- 2. text
				//-- 3. clear polygons vector and draw text operations.
				_doDraw();
				
				if (cmd.flags)
				{
					m_scissor = true;
					rd()->setScissorRect(
						cmd.rect.x, rs().screenRes().height - (cmd.rect.y + cmd.rect.h),
						cmd.rect.w, cmd.rect.h
						);
				}
				else
				{
					m_scissor = false;
				}
			}
		}

		//-- do final draw.
		_doDraw();
	}

	//----------------------------------------------------------------------------------------------
	bool imguiRender::_setupRender()
	{
		m_vertices.reserve(16348);

		//-- create geometry vertex buffer.
		{
			m_vb = rd()->createBuffer(IBuffer::TYPE_VERTEX, NULL, 16348, sizeof(GuiVertex),
				IBuffer::USAGE_DYNAMIC, IBuffer::CPU_ACCESS_WRITE);
		}

		//-- load material.
		{
			RODataPtr file = FileSystem::instance().readFile("resources/materials/gui.mtl");
			if (!file || !(m_material = rs().materials()->createMaterial(*file)))
			{
				return false;
			}

			//-- create rops for drawing.
			{
				RenderOp op;
				op.m_primTopolpgy = PRIM_TOPOLOGY_TRIANGLE_LIST;
				op.m_mainVB		  = &*m_vb;
				op.m_indicesCount = 0;
				op.m_material	  = m_material->renderFx();

				m_geomROPs.push_back(op);
			}
		}

		//-- create render states.
		{
			DepthStencilStateDesc dsDesc;
			dsDesc.depthEnable = false;
			m_stateDS = render::rd()->createDepthStencilState(dsDesc);

			RasterizerStateDesc rDesc;
			m_stateR = render::rd()->createRasterizedState(rDesc);

			rDesc.scissorEnable = true;
			m_stateR_scissor = render::rd()->createRasterizedState(rDesc);

			BlendStateDesc bDesc;
			bDesc.blendEnable[0] = true;
			bDesc.srcBlend		 = BlendStateDesc::BLEND_FACTOR_SRC_ALPHA;
			bDesc.destBlend		 = BlendStateDesc::BLEND_FACTOR_INV_SRC_ALPHA;
			bDesc.blendOp		 = BlendStateDesc::BLEND_OP_ADD;
			m_stateB = render::rd()->createBlendState(bDesc);
		}

		//-- font.
		{
			m_font = ResourcesManager::instance().loadFont("system/font/VeraMono", 12, vec2ui(32, 127));
			if (!m_font.isValid())
				return false;
		}

		return true;
	}

	//----------------------------------------------------------------------------------------------
	void imguiRender::_swapBuffers()
	{
		if (void* vb = m_vb->map<void>(IBuffer::ACCESS_WRITE_DISCARD))
		{
			memcpy(vb, &m_vertices[0], sizeof(GuiVertex) * m_vertices.size());
			m_vb->unmap();
		}

		m_geomROPs[0].m_indicesCount = m_vertices.size();
	}

	//----------------------------------------------------------------------------------------------
	void imguiRender::_doDraw()
	{
		//-- draw polygons.
		if (!m_vertices.empty())
		{
			_swapBuffers();
			rd()->setDepthStencilState(m_stateDS, 0);
			rd()->setBlendState(m_stateB, NULL, 0xffffffff);
			rd()->setRasterizerState(m_scissor ? m_stateR_scissor : m_stateR);

			rs().addImmediateRenderOps(m_geomROPs);

			m_vertices.clear();
		}
		
		//-- draw text.
		if (!m_texDrawOps.empty())
		{
			m_font->beginDraw(m_scissor);
			for (uint i = 0; i < m_texDrawOps.size(); ++i)
			{
				TextDrawOperation& op = m_texDrawOps[i];
				m_font->draw2D(op.m_pos, op.m_color, op.m_text);
			}
			m_font->endDraw();

			m_texDrawOps.clear();
		}
	}

} // render
} // brUGE