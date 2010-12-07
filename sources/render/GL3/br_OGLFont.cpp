#include "br_OGLFont.h"
#include "render/br_Color.h"

#include <stdarg.h> 
#include <stdio.h>

#define BR_FONT_PRINT_BUFFER_SIZE 1024

namespace brUGE
{
	using utils::brStr;

namespace render
{
	
	brOGLFont::brOGLFont()
		: isFontBuilded_(false)
	{
		
	}

	brOGLFont::~brOGLFont()
	{
		glDeleteLists(base_, 96);       
	}

	//------------------------------------------
	bool brOGLFont::buildFont(const utils::brStr &fontName, uint height/* = 14*/, uint width/* = 8*/)
	{
		if (isFontBuilded_)
			return false;

		// �������� ����� ������� ������ �� 96 
		base_ = glGenLists(96);

		if (!base_)
			throw brException("brOGLFont::buildFont", "Can't generate glGenLists(96).");

		height_ = height;
		width_	= width;
		fontName_ = fontName;

		// ������ �����
		fontHandle_ = CreateFont (	height_,						// ������ ������
									width_,							// ������ ������
									0,								// ���� ���������
									0,								// ���� �������
									FW_NORMAL,						// ������� ������
									FALSE,							// ������ ��� ���
									FALSE,							// ���������� ��� ���	
									FALSE,							// ��������� ��� ���
									ANSI_CHARSET,					// ������������� ������ ��������	
									OUT_TT_PRECIS,					// �������� ������
									CLIP_DEFAULT_PRECIS,			// �������� ���������
									ANTIALIASED_QUALITY,			// �������� ������
									FF_DONTCARE | DEFAULT_PITCH,	// ��������� � ���
									fontName_.c_str());						// �������� ������
		
		if (fontHandle_ == NULL)
			throw brException("brOGLFont::buildFont", "Can't create font handle.");

		// �������� ��� ����� �������
		SelectObject(hDC_, fontHandle_);

		// ������ ����� (�.�. ���������� ������� ������ � ������, �� ������ �� ������ ������)
		wglUseFontBitmaps(hDC_, 32, 96, base_);

		GetCharWidth32 (hDC_, 'A', 'A', &width_);
		
		return (isFontBuilded_ = true);
	}

	//------------------------------------------
	bool brOGLFont::init(const brVideoMode &videoMode, HDC hDC)
	{
		hDC_ = hDC;
		videoMode_ = videoMode;
		return true;
	}

	//------------------------------------------
	bool brOGLFont::reinit(const brVideoMode &videoMode, HDC hDC)
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
	void brOGLFont::printXY(const brColourF &color, float xPos, float yPos, const char* text, ...)
	{
		// �������� ��� ����� �������
		SelectObject(hDC_, fontHandle_);

		char    TEXT[BR_FONT_PRINT_BUFFER_SIZE];      // ����� ��� ����� ������
		va_list    ap;          // ��������� �� ������ ����������

		// ���� ��� ������
		if (!text){
			return;            // ������ �� ������
		}

		va_start(ap, text);           // ������ ������ ����������
		vsnprintf_s(TEXT, BR_FONT_PRINT_BUFFER_SIZE, _TRUNCATE, text, ap);	  // � ��������������� �������� � �������� ����
		va_end(ap);                   // ��������� ���������� � ������ TEXT

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
	void brOGLFont::print(const brColourF &color, const char *text, ...)
	{
		// �������� ��� ����� �������
		SelectObject(hDC_, fontHandle_);

		char    TEXT[BR_FONT_PRINT_BUFFER_SIZE];      // ����� ��� ����� ������
		va_list    ap;          // ��������� �� ������ ����������

		if (text == NULL)      // ���� ��� ������
		{
			return;            // ������ �� ������
		}

		va_start(ap, text);           // ������ ������ ����������
		vsnprintf_s(TEXT, BR_FONT_PRINT_BUFFER_SIZE, _TRUNCATE, text, ap);	  // � ��������������� �������� � �������� ����
		va_end(ap);                   // ��������� ���������� � ������ TEXT

		glPushAttrib(GL_CURRENT_BIT);
		glColor4fv(color.pointer());

		glPushAttrib(GL_LIST_BIT);      
		
		glListBase(base_ - 32);
		glCallLists((GLsizei)strlen(TEXT), GL_UNSIGNED_BYTE, TEXT);
		
		glPopAttrib();
		glPopAttrib();
	}
	
	//------------------------------------------
	void brOGLFont::print(const brColourF &color, const brStr &text)
	{
		// �������� ��� ����� �������
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
	void brOGLFont::printXY(const brColourF &color, float xPos, float yPos, const brStr &text)
	{
		// �������� ��� ����� �������
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
	uint brOGLFont::getHeight()
	{
		return height_;
	}
	
	//------------------------------------------
	uint brOGLFont::getWidth()
	{
		return width_;
	}

	}/*end namespace render*/
}/*end namespace brUGE*/