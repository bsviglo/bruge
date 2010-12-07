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

		pfd.nSize			=   sizeof(pfd);				// размер структуры
		pfd.nVersion		=   1;							// версия структкры
		pfd.dwFlags			=   PFD_SUPPORT_OPENGL |		// поддержка OpenGL
								PFD_DOUBLEBUFFER |			// использовать двойную буферизацию
								PFD_DRAW_TO_WINDOW;			// рисовать в окно
		pfd.iPixelType		=	PFD_TYPE_RGBA;				// использовать режим цветов RGBA
		pfd.cStencilBits	=	(byte)m_videoMode.stencil;	// размер в битах буффера трафарета
		pfd.cColorBits		=	(byte)m_videoMode.bpp;		// bpp (bits per pixel)
		pfd.cDepthBits		=	(byte)m_videoMode.depth;	// размер в битах Z-буффера

		m_hDC = ::GetDC(m_hWnd);

		// выбираем наиболее подходящий нам формат пикселей 
		int PixelFormat = ChoosePixelFormat(m_hDC, &pfd);

		// если система не нашла нужный нам формат пикселей
		if( !PixelFormat )
			BR_EXCEPT("brGLSwapChain::brGLSwapChain", "Appropriate pixel format is not founded.");

		// если не получилось установить формат пикселей
		if( !SetPixelFormat(m_hDC, PixelFormat, &pfd) )
			BR_EXCEPT("brGLSwapChain::brGLSwapChain", "Can't set pixel format.");

#if 1 // OpenGL 2.1
		// создаём контекст воспроизведения OpenGL
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

		// создаём контекст воспроизведения OpenGL
		m_hGLrc = wglCreateContextAttribsARB(m_hDC, 0, formatAttributes);

		if (!m_hGLrc)
		{
			wglDeleteContext(tempContext);
			BR_EXCEPT("brGLSwapChain::brGLSwapChain", "Can't create a GL rendering context.");
		}

		wglDeleteContext(tempContext);
#endif

		// если не удалось его создать
		if( m_hGLrc == NULL )
			BR_EXCEPT("brSwapChain::brSwapChain", "Can't create OpenGL Reproduction Context (GLRC).");

		// делаем контекст воспроизведения текущим
		if( !wglMakeCurrent(m_hDC, m_hGLrc) )
		{
			wglDeleteContext(m_hGLrc);
			BR_EXCEPT("brSwapChain::brSwapChain", "Can't make context current.");
		}
		
		// инициализируем GLEW.
		if (glewInit() != GLEW_OK)
			BR_EXCEPT("brGLSwapChain::brGLSwapChain", "Can't initialize glew.");
	}
	
	//------------------------------------------
	brGLSwapChain::~brGLSwapChain()
	{
		// Делаем наш контекст воспроизведения не текущим
		if( ::wglGetCurrentContext() != NULL )
			::wglMakeCurrent( NULL , NULL ); 

		// Если наш контекст воспроизведения существует, то удаляем его
		if( m_hGLrc != NULL )
		{
			//удаление контекста воспроизведения OGL
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

		devmode.dmSize				=	sizeof(DEVMODE);		// размер нашей структуры	
		devmode.dmFields			=	DM_BITSPERPEL |			// флаг для возможности изменения BPP (bits per pixel) 
										DM_PELSWIDTH |			// флаг для возможности изменения ширины окна
										DM_PELSHEIGHT |			// флаг для возможности изменения длинны окна
										DM_DISPLAYFREQUENCY;	// флаг для возможности изменения частоты обновления экрана
		devmode.dmBitsPerPel		=	m_videoMode.bpp;		// установка bpp
		devmode.dmPelsWidth			=	m_videoMode.width;		// установка ширины окна
		devmode.dmPelsHeight		=	m_videoMode.height;		// установка длинны окна
		devmode.dmDisplayFrequency	=	m_videoMode.frequancy;	// установка частоты обновления экрана

		// пробуем перейти в полноэкранный режим
		HRESULT fullscreen = DISP_CHANGE_FAILED;
		if (state)
		{
			fullscreen = ChangeDisplaySettings(&devmode, CDS_FULLSCREEN);
		}
		else
		{
			fullscreen = ChangeDisplaySettings(&devmode, 0);
		}

		// если не удалось перейти в полноэкранный режим
		if(fullscreen != DISP_CHANGE_SUCCESSFUL)
			BR_EXCEPT("brGLSwapChain::fullScreenState",
				"Problems with transitions between full screen and windowed mode.");

		return true;
	}

} // render
} // brUGE