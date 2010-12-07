#ifndef _BR_OGLFONT_H_
#define _BR_OGLFONT_H_

#include "br_OGLCommon.h"
#include "render/ibr_Font.h"
#include "utils/br_String.h"

namespace brUGE
{
namespace render
{
	
	// OpenGL реализация шрифта.
	//------------------------------------------
	class brOGLFont : public ibrFont
	{
	public:
		brOGLFont();
		virtual ~brOGLFont();
		
		bool buildFont(const utils::brStr &fontName, uint height = 14, uint width = 8);

		void printXY(const brColourF &color, float xPos, float yPos, const char* text, ...);
		void printXY(const brColourF &color, float xPos, float yPos, const utils::brStr &text);

		void print(const brColourF &color, const char* text,...);
		void print(const brColourF &color, const utils::brStr &text);

		uint getHeight();
		uint getWidth();
		
		// TODO: Нужны будут для перенастроки шрифта при смене разрешения окна или при переходе в 
		// полноэкранный режим.
		bool init(const brVideoMode &videoMode, HDC hdc);
		bool reinit(const brVideoMode &videoMode, HDC hdc);

	private:
		HFONT				fontHandle_;
		brVideoMode			videoMode_;	
		HDC					hDC_;
		uint base_;
		uint height_;
		int width_;
		utils::brStr fontName_;
		bool isFontBuilded_;
	};

}/*end namespace render*/
}/*end namespace brUGE*/


#endif /*_BR_OGLFONT_H_*/