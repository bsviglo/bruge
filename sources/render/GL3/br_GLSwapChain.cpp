#include "br_GLSwapChain.h"

namespace brUGE
{
namespace render
{
	
	//------------------------------------------
	brGLSwapChain::brGLSwapChain(
		HWND hWindow,
		const VideoMode& videoMode
		)
		:	m_hWnd(hWindow),
			m_videoMode(videoMode)
	{
		if (m_videoMode.fullScreen)
		{
			fullScreenState(m_videoMode.fullScreen);
		}

		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd , 0 , sizeof( PIXELFORMATDESCRIPTOR ));

		pfd.nSize			=   sizeof(pfd);				// ������ ���������
		pfd.nVersion		=   1;							// ������ ���������
		pfd.dwFlags			=   PFD_SUPPORT_OPENGL |		// ��������� OpenGL
								PFD_DOUBLEBUFFER |			// ������������ ������� �����������
								PFD_DRAW_TO_WINDOW;			// �������� � ����
		pfd.iPixelType		=	PFD_TYPE_RGBA;				// ������������ ����� ������ RGBA
		pfd.cStencilBits	=	(byte)m_videoMode.stencil;	// ������ � ����� ������� ���������
		pfd.cColorBits		=	(byte)m_videoMode.bpp;		// bpp (bits per pixel)
		pfd.cDepthBits		=	(byte)m_videoMode.depth;	// ������ � ����� Z-�������

		m_hDC = ::GetDC(m_hWnd);

		// �������� �������� ���������� ��� ������ �������� 
		int PixelFormat = ChoosePixelFormat(m_hDC, &pfd);

		// ���� ������� �� ����� ������ ��� ������ ��������
		if( !PixelFormat )
			BR_EXCEPT("brGLSwapChain::brGLSwapChain", "Appropriate pixel format is not founded.");

		// ���� �� ���������� ���������� ������ ��������
		if( !SetPixelFormat(m_hDC, PixelFormat, &pfd) )
			BR_EXCEPT("brGLSwapChain::brGLSwapChain", "Can't set pixel format.");

#if 1 // OpenGL 2.1
		// ������ �������� ��������������� OpenGL
		m_hGLrc = wglCreateContext(m_hDC);
#else // OpenGL 3.0

		HGLRC tempContext = wglCreateContext(m_hDC);
		wglMakeCurrent(m_hDC,tempContext);

		int formatAttributes[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
			WGL_CONTEXT_MINOR_VERSION_ARB, 0,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
			0
		};

		PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
		wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) wglGetProcAddress("wglCreateContextAttribsARB");

		if (wglCreateContextAttribsARB == NULL)
		{
			wglDeleteContext(tempContext);
			BR_EXCEPT("brGLSwapChain::brGLSwapChain", "OpenGL 3.0 is not supported.");
		}

		// ������ �������� ��������������� OpenGL
		m_hGLrc = wglCreateContextAttribsARB(m_hDC, 0, formatAttributes);

		if (!m_hGLrc)
		{
			wglDeleteContext(tempContext);
			BR_EXCEPT("brGLSwapChain::brGLSwapChain", "Can't create a GL rendering context.");
		}

		wglDeleteContext(tempContext);
#endif

		// ���� �� ������� ��� �������
		if( m_hGLrc == NULL )
			BR_EXCEPT("brSwapChain::brSwapChain", "Can't create OpenGL Reproduction Context (GLRC).");

		// ������ �������� ��������������� �������
		if( !wglMakeCurrent(m_hDC, m_hGLrc) )
		{
			wglDeleteContext(m_hGLrc);
			BR_EXCEPT("brSwapChain::brSwapChain", "Can't make context current.");
		}
		
		// �������������� GLEW.
		if (glewInit() != GLEW_OK)
			BR_EXCEPT("brGLSwapChain::brGLSwapChain", "Can't initialize glew.");
	}
	
	//------------------------------------------
	brGLSwapChain::~brGLSwapChain()
	{
		// ������ ��� �������� ��������������� �� �������
		if( ::wglGetCurrentContext() != NULL )
			::wglMakeCurrent( NULL , NULL ); 

		// ���� ��� �������� ��������������� ����������, �� ������� ���
		if( m_hGLrc != NULL )
		{
			//�������� ��������� ��������������� OGL
			::wglDeleteContext( m_hGLrc ); 
			m_hGLrc = NULL; 
		}
	}
	
	//------------------------------------------
	void brGLSwapChain::swapBuffers()
	{
		::SwapBuffers(m_hDC);
	}
	
	//------------------------------------------
	bool brGLSwapChain::fullScreenState(bool state)
	{
		DEVMODE devmode;
		memset(&devmode, 0, sizeof(DEVMODE));

		devmode.dmSize				=	sizeof(DEVMODE);		// ������ ����� ���������	
		devmode.dmFields			=	DM_BITSPERPEL |			// ���� ��� ����������� ��������� BPP (bits per pixel) 
										DM_PELSWIDTH |			// ���� ��� ����������� ��������� ������ ����
										DM_PELSHEIGHT |			// ���� ��� ����������� ��������� ������ ����
										DM_DISPLAYFREQUENCY;	// ���� ��� ����������� ��������� ������� ���������� ������
		devmode.dmBitsPerPel		=	m_videoMode.bpp;		// ��������� bpp
		devmode.dmPelsWidth			=	m_videoMode.width;		// ��������� ������ ����
		devmode.dmPelsHeight		=	m_videoMode.height;		// ��������� ������ ����
		devmode.dmDisplayFrequency	=	m_videoMode.frequancy;	// ��������� ������� ���������� ������

		// ������� ������� � ������������� �����
		HRESULT fullscreen = DISP_CHANGE_FAILED;
		if (state)
		{
			fullscreen = ChangeDisplaySettings(&devmode, CDS_FULLSCREEN);
		}
		else
		{
			fullscreen = ChangeDisplaySettings(&devmode, 0);
		}

		// ���� �� ������� ������� � ������������� �����
		if(fullscreen != DISP_CHANGE_SUCCESSFUL)
			BR_EXCEPT("brGLSwapChain::fullScreenState",
				"Problems with transitions between full screen and windowed mode.");

		return true;
	}

} // render
} // brUGE