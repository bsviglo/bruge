#pragma once

#include "prerequisites.h"
#include "imgui.h"
#include "render/Color.h"
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
		bool init();
		bool fini();

		void draw();

	private:

		Font			   m_font;
		RasterizerStateID  m_rState_enable_scissor;
		RasterizerStateID  m_rState_disable_scissor;


		//-- some useful temporary variables.
		std::vector<GuiVertex>		   m_vertices;
		std::vector<TextDrawOperation> m_texDrawOps;
		std::vector<float> m_tempCoords;
		std::vector<float> m_tempNormals;
		std::vector<float> m_circleVerts;

		//-- render section.
		Ptr<IBuffer>		m_vb;
		Ptr<IShader>		m_shader;

		VertexLayoutID		m_vl;
		DepthStencilStateID m_stateDS;
		BlendStateID		m_stateB;
		RasterizerStateID	m_stateR;
		SamplerStateID		m_stateS;
	};

} // render
} // brUGE