#ifndef _BR_GLSWAPCHAIN_H_
#define _BR_GLSWAPCHAIN_H_

#include "br_GLCommon.h"

namespace brUGE
{
namespace render
{
	// 
	//------------------------------------------
	class brGLSwapChain
	{
	public:
		brGLSwapChain(HWND hWindow, const VideoMode& videoMode);
		~brGLSwapChain();

		void swapBuffers();
		bool fullScreenState(bool state);

	public:
		HWND			m_hWnd;	 // ���������� ����	
		HDC				m_hDC;	 // �������� ���������� GDI (Graphic Device Interface)	
		HGLRC			m_hGLrc; // �������� ��������������� OGL (OpenGL Reproduction Context)
		VideoMode		m_videoMode;
	};

} // render
} // brUGE

#endif /*_BR_OGLSWAPCHAIN_H_*/