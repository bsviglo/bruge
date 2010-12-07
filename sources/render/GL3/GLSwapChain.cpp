#include "GLSwapChain.hpp"

namespace brUGE
{
namespace render
{
	
	//----------------------------------------------------------------------------------------------
	GLSwapChain::GLSwapChain(HWND hWindow, const VideoMode& videoMode)
		: m_hWnd(hWindow), m_videoMode(videoMode)
	{

	}
	
	//----------------------------------------------------------------------------------------------
	GLSwapChain::~GLSwapChain()
	{

	}
	
	//----------------------------------------------------------------------------------------------
	bool GLSwapChain::init()
	{
		if (m_videoMode.fullScreen)
		{
			fullScreenState(m_videoMode.fullScreen);
		}

		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd , 0 , sizeof(PIXELFORMATDESCRIPTOR));

		pfd.nSize		 = sizeof(pfd);				//-- size of the structure.
		pfd.nVersion	 = 1;						//-- the structure version.
		pfd.dwFlags		 = PFD_SUPPORT_OPENGL |		//-- support OpenGL
						   PFD_DOUBLEBUFFER |		//-- using the double buffering mode.
						   PFD_DRAW_TO_WINDOW;		//-- draw into the window.
		pfd.iPixelType	 = PFD_TYPE_RGBA;			//-- use RGBA color palette.
		pfd.cStencilBits = m_videoMode.stencil;		//-- size in bits of the stencil buffer.
		pfd.cColorBits	 = m_videoMode.bpp;			//-- bpp (bits per pixel)
		pfd.cDepthBits	 = m_videoMode.depth;		//-- size in bits of the z-buffer.

		m_hDC = ::GetDC(m_hWnd);

		//-- choose the most appropriate pixel format.
		int pixelFormat = ChoosePixelFormat(m_hDC, &pfd);

		if (!pixelFormat)
		{
			ERROR_MSG("Appropriate pixel format is not founded.");
			return false;
		}

		//-- try to set desired pixel format as a current.
		if (!SetPixelFormat(m_hDC, pixelFormat, &pfd))
		{
			ERROR_MSG("Can't set pixel format.");
			return false;
		}

		//-- Note: OpenGL 3 and above are created with help of the dummy context.
		
		//-- 1. Try to create dummy OpenGL context.
		HGLRC dummyContext = wglCreateContext(m_hDC);
		wglMakeCurrent(m_hDC,dummyContext);
		
		//-- 2. Try to initialize GLEW.
		if (glewInit() != GLEW_OK)
		{
			wglDeleteContext(dummyContext);
			ERROR_MSG("Can't initialize glew.");
			return false;
		}
		
		//-- 3. Preparation of the format attributes.
		int formatAttributes[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
			WGL_CONTEXT_MINOR_VERSION_ARB, 3,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
			0
		};

		//-- 4. try to create the real OpenGL reproduction context.
		m_hGLrc = wglCreateContextAttribsARB(m_hDC, 0, formatAttributes);
		if (m_hGLrc == NULL)
		{
			wglDeleteContext(dummyContext);
			ERROR_MSG("Can't create OpenGL Reproduction Context (GLRC).");
			return false;
		}
		
		//-- 5. Delete the dummy context.
		wglDeleteContext(dummyContext);
		dummyContext = NULL;

		//-- 6. make the real OpenGL context as a current.
		if (!wglMakeCurrent(m_hDC, m_hGLrc))
		{
			wglDeleteContext(m_hGLrc);
			ERROR_MSG("Can't make context current.");
			return false;
		}

		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool GLSwapChain::fini()
	{
		//-- make the current context as a not current.
		if (wglGetCurrentContext() != NULL)
		{
			wglMakeCurrent(NULL, NULL); 
		}

		//-- delete the OpenGL reproduction context if it is exists.
		if (m_hGLrc != NULL)
		{
			wglDeleteContext(m_hGLrc); 
			m_hGLrc = NULL; 
		}

		return true;
	}

	//----------------------------------------------------------------------------------------------
	void GLSwapChain::swapBuffers()
	{
		SwapBuffers(m_hDC);
	}
	
	//----------------------------------------------------------------------------------------------
	bool GLSwapChain::fullScreenState(bool isFullScreen)
	{
		DEVMODE devmode;
		memset(&devmode, 0, sizeof(DEVMODE));

		devmode.dmSize				=	sizeof(DEVMODE);		//-- size of the structure.
		devmode.dmFields			=	DM_BITSPERPEL |			//-- flag for the possibility to change BPP (bits per pixel) 
										DM_PELSWIDTH |			//-- flag for the possibility to change the windows width.
										DM_PELSHEIGHT |			//-- flag for the possibility to change the windows height.
										DM_DISPLAYFREQUENCY;	//-- flag for the possibility to change the windows update frequency.
		devmode.dmBitsPerPel		=	m_videoMode.bpp;		//-- setup the bpp.
		devmode.dmPelsWidth			=	m_videoMode.width;		//-- setup the window width.
		devmode.dmPelsHeight		=	m_videoMode.height;		//-- setup the window height.
		devmode.dmDisplayFrequency	=	m_videoMode.frequancy;	//-- setup the screen update frequency.

		//-- try to change the full screen mode.
		HRESULT fullscreen = DISP_CHANGE_FAILED;
		if (isFullScreen)
		{
			fullscreen = ChangeDisplaySettings(&devmode, CDS_FULLSCREEN);
		}
		else
		{
			fullscreen = ChangeDisplaySettings(&devmode, 0);
		}

		//-- the transition between full screen and the windowed mode is failed.
		if (fullscreen != DISP_CHANGE_SUCCESSFUL)
		{
			ERROR_MSG("Problems with transitions between full screen and windowed mode.");
			return false;
		}

		return true;
	}

} //-- render
} //-- brUGE