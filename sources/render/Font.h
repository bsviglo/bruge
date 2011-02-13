#pragma once

#include "prerequisites.h"
#include "render_common.h"
#include "Color.h"
#include "math/Vector2.h"
#include <map>
#include <string>

// ToDo: add Kerning.

namespace brUGE
{
namespace render
{
	// 
	//---------------------------------------------------------------------------------------------
	class Font : public utils::RefCount
	{
	public:
		struct Desc
		{
			uint	size;
			vec2ui	glyphRange;
			uint	width;
			uint	height;
		};

	public:
		
		//-- fontName	- the name of font used in the engine.
		//-- fontData	- the content of desired *.ttf file.
		//-- size		- the size of the font.
		//-- glyphRange - symbols range.
		//-----------------------------------------------------------------------------------------
		Font(const char* fontName, const utils::ROData& fontData, uint size, vec2ui glyphRange);
		~Font();

		//-- beginDraw() just setups the render states, and the draw2D() performs updating vertex
		//-- buffer and the actual draw operation (e.g. DIP).
		//-- Note: useful in places where you need call more times the function print2D.
		//--	   This couple of functions do this task more effectively.
		//-----------------------------------------------------------------------------------------
		void beginDraw(bool enableScissor = false);
		void draw2D(const vec2f& pos, const Color& color, const std::string& text);
		void draw2D(const vec2f& pos, const Color& color, const char* text, ...);
		void endDraw();
		
		//-- performs immediate drawing with state changes. 
		//-----------------------------------------------------------------------------------------
		void print2D(const vec2f& pos, const Color& color, const std::string& text);
		void print2D(const vec2f& pos, const Color& color, const char* text, ...);
		
		//-- returns size of string in pixel, if we draw it with this font.
		//-----------------------------------------------------------------------------------------
		vec2ui getStringDim(const std::string& text) const;

		void getDesc(Desc& desc) { desc = m_fontDesc; }

	private:
		void _createFontTex	(const utils::ROData& data);
		void _setupRender	();
		void _fillBuffer	(const vec2f& pos, const Color& color, const std::string& text);
		void _draw			(uint count);

	private:
		struct GlyphDesc
		{
			vec2f bottomLeft;
			vec2f topRight;
			float horiAdvance;
		};

		struct VertPTC
		{
			vec3f pos;
			vec2f texCoord;
			vec4f color;
		};

		//typedef std::map<uint, GlyphDesc> GlyphMap;
		typedef std::vector<GlyphDesc> GlyphMap;

		mutable GlyphMap	m_glyphMap;
		Desc				m_fontDesc;
		std::string			m_fontName;
		
		Ptr<ITexture>		m_texture;
		Ptr<IBuffer>		m_vb;
		Ptr<IShader>		m_shader;

		VertexLayoutID		m_vl;
		DepthStencilStateID m_stateDS;
		BlendStateID		m_stateB;
		RasterizerStateID	m_stateR;
		RasterizerStateID	m_stateR_scissor;
		SamplerStateID		m_stateS;
	};

} // render
} // brUGE
