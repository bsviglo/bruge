#include "br_GLFont.h"
#include "render/br_Color.h"

#include <stdarg.h> 
#include <stdio.h>

#define BR_FONT_PRINT_BUFFER_SIZE 1024

namespace brUGE
{
	using utils::brStr;

namespace render
{
	
	brGLFont::brGLFont()
		: isFontBuilded_(false)
	{
		
	}

	brGLFont::~brGLFont()
	{
		glDeleteLists(base_, 96);       
	}

	//------------------------------------------
	bool brGLFont::buildFont(const utils::brStr &fontName, uint height/* = 14*/, uint width/* = 8*/)
	{
		if (isFontBuilded_)
			return false;

		// получаем номер первого списка из 96 
		base_ = glGenLists(96);

		if (!base_)
			throw brException("brOGLFont::buildFont", "Can't generate glGenLists(96).");

		height_ = height;
		width_	= width;
		fontName_ = fontName;

		// создаём шрифт
		fontHandle_ = CreateFont (	height_,						// Высота шрифта
									width_,							// ширина шрифта
									0,								// угол отношения
									0,								// угол наклона
									FW_NORMAL,						// толщина шрифта
									FALSE,							// курсив или нет
									FALSE,							// подчёркнут или нет	
									FALSE,							// зачёркнут или нет
									ANSI_CHARSET,					// идентификатор набора символов	
									OUT_TT_PRECIS,					// точность вывода
									CLIP_DEFAULT_PRECIS,			// точность отсечения
									ANTIALIASED_QUALITY,			// качество вывода
									FF_DONTCARE | DEFAULT_PITCH,	// семейство и шаг
									fontName_.c_str());						// название шрифта
		
		if (fontHandle_ == NULL)
			throw brException("brOGLFont::buildFont", "Can't create font handle.");

		// выбираем наш шрифт текущим
		SelectObject(hDC_, fontHandle_);

		// строим шрифт (т.е. записываем символы шрифта в списки, по одному на каждый список)
		wglUseFontBitmaps(hDC_, 32, 96, base_);

		GetCharWidth32 (hDC_, 'A', 'A', &width_);
		
		return (isFontBuilded_ = true);
	}

	//------------------------------------------
	bool brGLFont::init(const VideoMode &videoMode, HDC hDC)
	{
		hDC_ = hDC;
		videoMode_ = videoMode;
		return true;
	}

	//------------------------------------------
	bool brGLFont::reinit(const VideoMode &videoMode, HDC hDC)
	{
		hDC_ = hDC;
		videoMode_ = videoMode;
		if (isFontBuilded_)
		{
			isFontBuilded_ = false;
			return buildFont(fontName_, height_, width_);
		}
		return true;
	}
	
	//------------------------------------------
	void brGLFont::printXY(const brColourF &color, float xPos, float yPos, const char* text, ...)
	{
		// выбираем наш шрифт текущим
		SelectObject(hDC_, fontHandle_);

		char    TEXT[BR_FONT_PRINT_BUFFER_SIZE];      // Место для нашей строки
		va_list    ap;          // Указатель на список аргументов

		// Если нет текста
		if (!text){
			return;            // Ничего не делать
		}

		va_start(ap, text);           // Разбор строки переменных
		vsnprintf_s(TEXT, BR_FONT_PRINT_BUFFER_SIZE, _TRUNCATE, text, ap);	  // И конвертирование символов в реальные коды
		va_end(ap);                   // Результат помещается в строку TEXT

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0, videoMode_.width, 0, videoMode_.height, -1, 1); 

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		
		glRasterPos2f(xPos, yPos);

		glPushAttrib(GL_CURRENT_BIT);
		glColor4fv(color.pointer());

		glPushAttrib(GL_LIST_BIT);      
	
		glListBase(base_ - 32);
		glCallLists((GLsizei)strlen(TEXT), GL_UNSIGNED_BYTE, TEXT);
		
		glPopAttrib();
		glPopAttrib();

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	}
	
	//------------------------------------------
	void brGLFont::print(const brColourF &color, const char *text, ...)
	{
		// выбираем наш шрифт текущим
		SelectObject(hDC_, fontHandle_);

		char    TEXT[BR_FONT_PRINT_BUFFER_SIZE];      // Место для нашей строки
		va_list    ap;          // Указатель на список аргументов

		if (text == NULL)      // Если нет текста
		{
			return;            // Ничего не делать
		}

		va_start(ap, text);           // Разбор строки переменных
		vsnprintf_s(TEXT, BR_FONT_PRINT_BUFFER_SIZE, _TRUNCATE, text, ap);	  // И конвертирование символов в реальные коды
		va_end(ap);                   // Результат помещается в строку TEXT

		glPushAttrib(GL_CURRENT_BIT);
		glColor4fv(color.pointer());

		glPushAttrib(GL_LIST_BIT);      
		
		glListBase(base_ - 32);
		glCallLists((GLsizei)strlen(TEXT), GL_UNSIGNED_BYTE, TEXT);
		
		glPopAttrib();
		glPopAttrib();
	}
	
	//------------------------------------------
	void brGLFont::print(const brColourF &color, const brStr &text)
	{
		// выбираем наш шрифт текущим
		SelectObject(hDC_, fontHandle_);

		glPushAttrib(GL_CURRENT_BIT);
		glColor4fv(color.pointer());

		glPushAttrib(GL_LIST_BIT);      
		
		glListBase(base_ - 32);
		glCallLists((GLsizei)text.length(), GL_UNSIGNED_BYTE, text.c_str());
		
		glPopAttrib();
		glPopAttrib();	
	}
	
	//------------------------------------------
	void brGLFont::printXY(const brColourF &color, float xPos, float yPos, const brStr &text)
	{
		// выбираем наш шрифт текущим
		SelectObject(hDC_, fontHandle_);

		glMatrixMode (GL_PROJECTION);
		glPushMatrix ();
		glLoadIdentity ();
		glOrtho (0, videoMode_.width, 0, videoMode_.height, -1, 1); 

		glMatrixMode (GL_MODELVIEW);
		glPushMatrix ();
		glLoadIdentity ();
		
		glRasterPos2f (xPos, yPos);
		
		glPushAttrib(GL_CURRENT_BIT);
		glColor4fv(color.pointer());

		glPushAttrib (GL_LIST_BIT);      
		
		glListBase(base_ - 32);
		glCallLists((GLsizei)text.length(), GL_UNSIGNED_BYTE, text.c_str ());

		glPopAttrib();
		glPopAttrib();

		glMatrixMode (GL_MODELVIEW);
		glPopMatrix ();

		glMatrixMode (GL_PROJECTION);
		glPopMatrix ();
	}
	
	//------------------------------------------
	uint brGLFont::getHeight()
	{
		return height_;
	}
	
	//------------------------------------------
	uint brGLFont::getWidth()
	{
		return width_;
	}

	}/*end namespace render*/
}/*end namespace brUGE*/