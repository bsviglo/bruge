#pragma once

#include "prerequisites.h"
#include "imgui.h"
#include "render/IBuffer.h"
#include "render/Font.h"
#include "render/Color.h"
#include "render/materials.hpp"
#include "math/Vector3.h"
#include <vector>

namespace brUGE
{
namespace render
{

	//-- Implements imgui renderer interface.
	//-- Note: This is basic initial version. In near future it will be reworked. 
	//----------------------------------------------------------------------------------------------
	class imguiRender
	{
	public:
		struct GuiVertex
		{
			GuiVertex(const vec3f& pos, const Color& color) : m_pos(pos), m_color(color) { }

			vec3f m_pos;
			Color m_color;
		};

		struct TextDrawOperation
		{
			TextDrawOperation(const vec2f& pos, const char* text, const Color& color)
				: m_pos(pos), m_text(text), m_color(color) { }

			vec2f		m_pos;
			const char* m_text;
			Color		m_color;
		};

	public:
		imguiRender();
		~imguiRender();

		bool init();
		bool fini();

		void draw();

	private:
		bool _setupRender();
		void _swapBuffers();
		void _doDraw();

	private:

		//-- some useful temporary variables.
		std::vector<GuiVertex>			m_vertices;
		std::vector<TextDrawOperation>	m_texDrawOps;
		std::vector<float>				m_tempCoords;
		std::vector<float>				m_tempNormals;

		//-- render section.
		bool			    m_scissor;
		Ptr<Font>			m_font;
		Ptr<IBuffer>		m_vb;
		Ptr<Material>		m_material;
		RenderOps			m_geomROPs;

		DepthStencilStateID m_stateDS;
		BlendStateID		m_stateB;
		RasterizerStateID	m_stateR;
		RasterizerStateID	m_stateR_scissor;
		SamplerStateID		m_stateS;
	};

} // render
} // brUGE