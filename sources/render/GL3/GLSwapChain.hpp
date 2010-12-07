#pragma once

#include "GL_common.hpp"

namespace brUGE
{
namespace render
{

	//----------------------------------------------------------------------------------------------
	class GLSwapChain
	{
	public:
		GLSwapChain(HWND hWindow, const VideoMode& videoMode);
		~GLSwapChain();

		bool init();
		bool fini();

		void swapBuffers();
		bool fullScreenState(bool isFullScreen);

	public:
		HWND		m_hWnd;	 //-- the window description.
		HDC			m_hDC;	 //-- GDI (Graphic Device Interface)	
		HGLRC		m_hGLrc; //-- OGL (OpenGL Reproduction Context)
		VideoMode	m_videoMode;
	};

} //-- render
} //-- brUGE
