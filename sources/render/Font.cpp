#include "Font.h"
#include "render_system.hpp"
#include "IRenderDevice.h"
#include "ITexture.h"
#include "IBuffer.h"
#include "IShader.h"
#include "state_objects.h"
#include "os/FileSystem.h"
#include "loader/ResourcesManager.h"
#include "utils/Data.hpp"

#include "ft2build.h"
#include FT_FREETYPE_H

// additional offset in the texture between glyphs.
#define CHAR_OFFSET		2
#define MAX_CHAR_SIZE	1024

namespace brUGE
{
namespace render
{
	
	//------------------------------------------
	Font::Font(const char* fontName, const utils::ROData& fontData, uint size, vec2ui glyphRange)
		: m_fontName(fontName)
	{
		m_fontDesc.size			= size;
		m_fontDesc.glyphRange	= glyphRange;
		m_fontDesc.height		= 0;
		m_fontDesc.width		= 0;

		//-- ToDo:
		m_glyphMap.resize(glyphRange.y + 1);

		_createFontTex(fontData);
		_setupRender();
	}
	
	//------------------------------------------
	Font::~Font()
	{

	}
	
	//------------------------------------------
	vec2ui Font::getStringDim(const std::string& text) const
	{
		vec2ui dim(0, m_fontDesc.height);
		for (uint i = 0; i < text.length(); ++i)
			dim[0] += m_glyphMap[text[i]].horiAdvance;

		return dim;
	}
	
	//------------------------------------------
	void Font::print2D(const vec2f& pos, const Color& color, const std::string& text)
	{
		_fillBuffer(pos, color, text);
		_draw(text.length());
	}
	
	//------------------------------------------
	void Font::print2D(const vec2f& pos, const Color& color, const char* text, ...)
	{
		va_list args;
		char buffer[2048]; // 2 kbytes.
		va_start(args, text);
		_vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, text, args);
		std::string result = buffer;
		va_end(args);
		
		_fillBuffer(pos, color, result);
		_draw(result.length());		
	}
	
	//------------------------------------------
	void Font::beginDraw(bool enableScissor)
	{
		rd()->setDepthStencilState(m_stateDS, 0);
		rd()->setRasterizerState(enableScissor ? m_stateR_scissor : m_stateR);
		rd()->setBlendState(m_stateB, NULL, 0xffffffff);
		
		rd()->setVertexLayout(m_vl);
		rd()->setVertexBuffer(0, m_vb.get());
		rd()->setShader(m_shader.get());

		m_shader->setSampler("fontSampl", m_stateS);
		m_shader->setTexture("fontTex", m_texture.get());
	}
	
	//------------------------------------------
	void Font::draw2D(const vec2f& pos, const Color& color, const char* text, ...)
	{
		va_list args;
		char buffer[2048]; //-- 2 kbytes.
		va_start(args, text);
		_vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, text, args);
		std::string result = buffer;
		va_end(args);

		_fillBuffer(pos, color, result);
		render::rd()->draw(PRIM_TOPOLOGY_TRIANGLE_STRIP, 0, result.length() * 4);
	}

	//------------------------------------------
	void Font::endDraw()
	{
		// pass
	}
	
	//------------------------------------------
	void Font::draw2D(const vec2f& pos, const Color& color, const std::string& text)
	{
		_fillBuffer(pos, color, text);
		render::rd()->draw(PRIM_TOPOLOGY_TRIANGLE_STRIP, 0, text.length() * 4);
	}
	
	//------------------------------------------
	void Font::_createFontTex(const utils::ROData& data)
	{
		FT_Library	library;
		FT_Face		face; 

		FT_Error error = FT_Init_FreeType(&library);
		if (error)
		{
			BR_EXCEPT("An error occurred during library initialization.");
		}

		error = FT_New_Memory_Face(library, (FT_Byte*)data.ptr(), data.length(), 0, &face);
		if (error)
		{
			FT_Done_FreeType(library);
			BR_EXCEPT("Error code means that the font file is broken");
		}

		// TODO: Разобраться с установлением кодировки и русскими символами.
#if 0 
		FT_CharMap charmap;
		for(int i = 0; i < face->num_charmaps; ++i)
		{
			charmap = face->charmaps[i]; 
			if (charmap->platform_id == TT_PLATFORM_MICROSOFT &&
				charmap->encoding_id == TT_MS_ID_UNICODE_CS) // если нет - используем дефолт
			{
				FT_Set_Charmap(face, charmap);
				break;
			}
		}
#endif

#if 0 // неявное задание размера глифа в пикселах.
		error = FT_Set_Char_Size(
			face,					/* handle to face object           */
			m_fontDesc.size * 64,	/* char_width in 1/64th of points  */
			m_fontDesc.size * 64,	/* char_height in 1/64th of points */
			720,					/* horizontal device resolution    */
			720						/* vertical device resolution      */
			);
#else // явное задание размера глифа в пикселах.
		error = FT_Set_Pixel_Sizes(
			face,				/* handle to face object */
			0,					/* pixel_width */
			m_fontDesc.size		/* pixel_height */ 
			); 
#endif

		if (error)
		{
			FT_Done_Face(face);
			FT_Done_FreeType(library);
			BR_EXCEPT("Failed to set char size.");
		}
		
		//-- make shortcut on glyph range.
		const vec2ui& glyphRange = m_fontDesc.glyphRange;

		uint glyphCount	 = 0;
		int	 tmpVal		 = 0;
		uint maxHeight	 = 0;
		uint maxBearingY = 0;
		uint maxWidth	 = 0;

		for (uint charIndex = glyphRange[0]; charIndex <= glyphRange[1]; ++charIndex, ++glyphCount)
		{
			FT_Load_Char(face, charIndex, FT_LOAD_RENDER);
			const FT_GlyphSlot& glyph = face->glyph;
			
			tmpVal = 2 * (glyph->bitmap.rows << 6) - glyph->metrics.horiBearingY;
			if(tmpVal > (int)maxHeight)
				maxHeight = tmpVal;
			
			tmpVal = glyph->metrics.horiBearingY;
			if(tmpVal > (int)maxBearingY)
				maxBearingY = tmpVal;
			
			tmpVal = (glyph->advance.x >> 6) + (glyph->metrics.horiBearingX >> 6);
			if(tmpVal > (int)maxWidth)
				maxWidth = tmpVal;
		}
		
		// дополняем дескриптор.
		m_fontDesc.height = maxHeight >> 6;

		// Вычисляем нужный размер текстуры.
		uint rawSize = (maxWidth + CHAR_OFFSET) * ((maxHeight >> 6) + CHAR_OFFSET) * glyphCount;
		// + обработка ситуации когда в текстуру не может вместится целый символ.
		uint sqrSize		= static_cast<uint>(sqrtf(rawSize));
		uint maxVal			= math::max(maxWidth, (maxHeight >> 6));
		uint modSqrAdd		= (sqrSize % maxVal) ? maxVal - (sqrSize % maxVal) : 0;
		uint texSideSize	= sqrSize + modSqrAdd;

		INFO_MSG("Font %s using texture size %d x %d.", m_fontName.c_str(), texSideSize, texSideSize);

		byte* imageData = new byte[texSideSize * texSideSize];

		for (uint i = 0; i < texSideSize * texSideSize; ++i)
			imageData[i] = 0x00;

		FT_Int advance		 = 0;
		float  invTexSide	 = 1.0f / texSideSize;
		byte*  glyphImageBuf = NULL;	
		uint   m			 = 0;
		uint   l			 = 0;
		uint   row			 = 0;		

		for (uint charIndex = glyphRange[0]; charIndex <= glyphRange[1]; ++charIndex)
		{
			if (FT_Load_Char(face, charIndex, FT_LOAD_RENDER))
			{
				// Не можем загрузить данный глиф, пропускаем его.
				WARNING_MSG("Info: cannot load character '%c' in font '%s'.", charIndex, m_fontName.c_str());
				continue;
			}

			// TODO: Нужен ли размер степени 2-х ?
			
			const FT_GlyphSlot& glyph = face->glyph;
			advance			= (glyph->advance.x >> 6) + (glyph->metrics.horiBearingX >> 6);
			glyphImageBuf	= glyph->bitmap.buffer;

			if (!glyphImageBuf) // проверка на валидность буфера.
			{
				if (charIndex != 32) // если это не пробел. У пробела нету буфера данных.
				{
					WARNING_MSG("Freetype returned null for character '%c' in font '%s'.",
						charIndex, m_fontName.c_str());
				}

				GlyphDesc glyphDesc;
				glyphDesc.horiAdvance = (glyph->advance.x >> 6);
				m_glyphMap[charIndex] = glyphDesc;

				continue;
			}

			int y_bearnig = (maxBearingY >> 6) - (glyph->metrics.horiBearingY >> 6);

			for(int j = 0; j < glyph->bitmap.rows; ++j)
			{
				row				= j + m + y_bearnig;
				uchar* pDest	= &imageData[(row * texSideSize) + l];

				for(int k = 0; k < glyph->bitmap.width; ++k)
					*pDest++= *glyphImageBuf++; 
			}

			GlyphDesc glyphDesc;
			glyphDesc.topRight		= vec2f(l + (glyph->advance.x >> 6), m).scale(invTexSide);
			glyphDesc.bottomLeft	= vec2f(l, m + (maxHeight >> 6)).scale(invTexSide);
			glyphDesc.horiAdvance	= (glyph->advance.x >> 6);
			
			// выбираем максимальную ширину символа. Для моношрифта она одинакова.
			if ((uint)(glyph->advance.x >> 6) > m_fontDesc.width)
				m_fontDesc.width = (glyph->advance.x >> 6);

			m_glyphMap[charIndex] = glyphDesc;

			// смещаем столбец.
			l += advance + CHAR_OFFSET;

			// переходим на новую строку.
			if ((texSideSize - 1) < (l + advance))
			{
				m += (maxHeight >> 6) + CHAR_OFFSET;
				l = 0;
			}
		}

		ITexture::Desc fontTexDesc;
		fontTexDesc.texType		= ITexture::TYPE_2D;
		fontTexDesc.format		= ITexture::FORMAT_R8;
		fontTexDesc.mipLevels	= 1;
		fontTexDesc.bindFalgs	= ITexture::BIND_SHADER_RESOURCE;
		fontTexDesc.width		= texSideSize;
		fontTexDesc.height		= texSideSize;
		
		// ToDo: Убрать явное определение размера формата текстуры.
		std::vector<ITexture::Data> texDataDesc;
		ITexture::Data data = {imageData, texSideSize * 1, 0};
		texDataDesc.push_back(data);
		m_texture = rd()->createTexture(fontTexDesc, &texDataDesc[0], texDataDesc.size());

		delete [] imageData;
		FT_Done_Face(face);
		FT_Done_FreeType(library);
	}
	
	//------------------------------------------
	void Font::_setupRender()
	{
		//-- create shader and vertex layout.
		{
			m_shader = ResourcesManager::instance().loadShader("font");
			if (!m_shader)
			{
				BR_EXCEPT("failed load shader");
			}

			VertexDesc desc[] = 
			{
				{0, TYPE_POSITION, FORMAT_FLOAT, 3},
				{0, TYPE_TEXCOORD, FORMAT_FLOAT, 2},
				{0, TYPE_COLOR,	   FORMAT_FLOAT, 4}
			};
			m_vl = render::rd()->createVertexLayout(desc, 3, *m_shader.get());
		}
		
		//-- create render states.
		{
			SamplerStateDesc samplerDesc;
			samplerDesc.minMagFilter  = SamplerStateDesc::FILTER_NEAREST;
			samplerDesc.wrapS		  = SamplerStateDesc::ADRESS_MODE_WRAP;
			samplerDesc.wrapT		  = SamplerStateDesc::ADRESS_MODE_WRAP;
			m_stateS = render::rd()->createSamplerState(samplerDesc);

			// подготовим стейты.
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

		//-- MAX_CHAR_SIZE = 1024: => 1024 * 4 * sizeof(VertPTC) = 144 kbytes.
		m_vb = render::rd()->createBuffer(IBuffer::TYPE_VERTEX, NULL, 4 * MAX_CHAR_SIZE, sizeof(VertPTC),
			IBuffer::USAGE_DYNAMIC, IBuffer::CPU_ACCESS_WRITE);
	}
	
	//------------------------------------------
	void Font::_fillBuffer(const vec2f& pos, const Color& color, const std::string& text)
	{
		/*
		Правило построения каждого тр-ка в стрипе (начиная с 3-го индекса стрипа)
		звучит так: если текущий индекс вершины нечётный по счёту, то тр-к составляется
		из тройки индексов (предпредыдущий, предыдущий, текущий индекс), если же текущий
		индекс чётный, то из тройки (предпредыдущий, текущий, предыдущий индекс).
		Таким образом, если у нас есть две полоски стрипов и их нужно склеить, нужно между
		ними вставить две вершины: первая - это последняя вершина первого стрипа,
		а вторая - это первая вершина второго стрипа. В результате такого склеивания 
		образуется 4 вырожденных треугольника (т.е. таких треугольника, хотя бы две вершины
		которых совпадают — очевидно, что такой треугольник не закрашивается).
		
		Рассмотрим пример:
		Пусть A,B,C,D - первый стрип, а E,F,G,H - второй.
		Стрип-результат их склеивания - A,B,C,D,  D,E,  E,F,G,H.
		Вырожденные тр-ки:(C,D,D), (D,E,D), (D,E,E), (E,F,E). 
		*/

		if (!text.length()) return;

		VertPTC* vb = m_vb->map<VertPTC>(IBuffer::ACCESS_WRITE_DISCARD);

		ScreenResolution screenRes = RenderSystem::instance().screenRes();

		vec3f curPos(
			(2.0f * pos.x / screenRes.width) - 1.0f,
			1.0f - (2.0f * pos.y / screenRes.height),
			0.0f
			);
		vec2f charSize(0.0f, (2.0f * m_fontDesc.height) / screenRes.height);
		float twoDivWidth = 2.0f / screenRes.width;

		// ToDo: optimization.

		for (uint i = 0, j = 0; i < text.length() && i < MAX_CHAR_SIZE; ++i, j += 4)
		{
			const GlyphDesc& glyph = m_glyphMap[text[i]];
			charSize.x = glyph.horiAdvance * twoDivWidth;
		
			// bottom left
			vb[j + 0].pos		= curPos;
			vb[j + 0].texCoord	= glyph.bottomLeft;
			vb[j + 0].color		= color.toVec4();			
			
			// upper left
			vb[j + 1].pos		= vec3f(curPos.x, curPos.y + charSize.y, curPos.z);
			vb[j + 1].texCoord	= vec2f(glyph.bottomLeft.x, glyph.topRight.y);
			vb[j + 1].color		= color.toVec4();			
			
			// bottom right
			vb[j + 2].pos		= vec3f(curPos.x + charSize.x, curPos.y, curPos.z);
			vb[j + 2].texCoord	= vec2f(glyph.topRight.x, glyph.bottomLeft.y);
			vb[j + 2].color		= color.toVec4();			

			// upper right
			vb[j + 3].pos		= vec3f(curPos.x + charSize.x, curPos.y + charSize.y, curPos.z);
			vb[j + 3].texCoord	= glyph.topRight;
			vb[j + 3].color		= color.toVec4();			
			
			curPos.x += charSize.x;
		}

		m_vb->unmap();
	}

	//------------------------------------------
	void Font::_draw(uint count)
	{
		rd()->setDepthStencilState(m_stateDS, 0);
		rd()->setRasterizerState(m_stateR);
		rd()->setBlendState(m_stateB, NULL, 0xffffffff);

		rd()->setVertexLayout(m_vl);
		rd()->setVertexBuffer(0, m_vb.get());

		rd()->setShader(m_shader.get());
		{
			m_shader->setSampler("fontSampl", m_stateS);
			m_shader->setTexture("fontTex", m_texture.get());
		}

		rd()->draw(PRIM_TOPOLOGY_TRIANGLE_STRIP, 0, count * 4);
	}

} // render
} // brUGE