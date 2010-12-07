#include "WinApp.h"
#include "Exception.h"
#include "utils/LogManager.h"
#include "../win32Res/resource.h"
#include <assert.h>

namespace brUGE
{
namespace os
{
	//------------------------------------------
	WinApp::WinApp() : m_hInstance(NULL), m_isAlive(false)
	{

	}

	//------------------------------------------
	WinApp::~WinApp()
	{
		// мы все еще не отрегистрировали класс окна.
		assert(!m_isAlive);
	}

	//------------------------------------------
	void WinApp::kill()
	{
		// TODO: ѕофиксить это!!!
		UnregisterClass((LPCSTR)m_titleName.c_str(), m_hInstance);

		//if (UnregisterClass((LPCSTR)m_titleName.c_str(), m_hInstance) == 0)
		//	throw brException("WinApp::kill()", "Can't unregister window class.");

		m_isAlive = false;						
	}

	//------------------------------------------
	void WinApp::showWnd(HWND hWnd, int shownSpecific /* = SW_SHOW */) const
	{
		ShowWindow(hWnd, shownSpecific);
		UpdateWindow(hWnd);
	}

	//------------------------------------------
	bool WinApp::init(HINSTANCE hInstance, const std::string& titleName, WNDPROC func)
	{
		m_hInstance = hInstance;
		m_titleName = titleName;

		if (!_registerWinClass(m_hInstance, m_titleName, func))
		{
			m_isAlive = false;
			return false;
		}
		m_isAlive = true;
		return true;
	}

	//------------------------------------------
	bool WinApp::_registerWinClass(HINSTANCE hInst, const std::string& titleName, WNDPROC func)
	{
		WNDCLASS wndClass;
		wndClass.style         = 0;												// стиль окна
		wndClass.lpfnWndProc   = (WNDPROC)func;									// указатель на функцию, котора€ будет получать сообщени€ от Windows 
		wndClass.cbClsExtra    = 0;												// расширенные параметры - нам пока не нужны	
		wndClass.cbWndExtra    = 0;												
		wndClass.hInstance     = hInst;											// идентификатор приложени€	
		wndClass.hIcon         = LoadIcon (hInst, MAKEINTRESOURCE(IDI_ICON1));	// загрузка стандартной иконки
		wndClass.hCursor       = LoadCursor (NULL,IDC_ARROW);					// загрузка курсора мыши
		wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW);						// фон окна	
		wndClass.lpszMenuName  = NULL;											// меню - у нас его нет
		wndClass.lpszClassName = (LPCSTR)titleName.c_str();						// им€ класса окна

		// ѕытаемс€ зарегистрировать класс окна
		if (!RegisterClass(&wndClass))
		{
			ERROR_MSG("Can't register a windows class.");
			return false;
		}
		return true;
	}
	
	//------------------------------------------
	HWND WinApp::createWnd(const std::string& wndName, int width, int height, DWORD windowStyle) const
	{
		HWND tempHWnd = CreateWindow(m_titleName.c_str(), wndName.c_str(),
			windowStyle,
			0,							
			0,				
			width,				
			height,				
			NULL,						
			NULL,
			m_hInstance,		
			NULL);		

		if (!tempHWnd)
			BR_EXCEPT("Can't create a window.");

		return tempHWnd;
	}

} // os
} // brUGE
