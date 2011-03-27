#pragma once

#include "prerequisites.hpp"
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#	define NOMINMAX
#endif
#include <windows.h>

namespace brUGE
{
namespace os
{

	//------------------------------------------			
	LRESULT WINAPI g_mainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam,	LPARAM lParam);
	LRESULT WINAPI g_mainDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,	LPARAM lParam);
	LRESULT WINAPI MenuWndProc(HWND	hWnd, UINT uMsg, WPARAM	wParam, LPARAM lParam);

	// 
	//------------------------------------------
	class WinApp
	{
	public:
	public:
		WinApp();
		~WinApp();

		bool init(HINSTANCE hInstance, const std::string& titleName, WNDPROC func);
		void kill();
		void showWnd(HWND hWnd, int shownSpecific = SW_SHOW) const;

		//------------------------------------------
		HWND createWnd(const std::string& wndName, int width, int height,
			DWORD windowStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE) const;

	private:
		bool _registerWinClass(HINSTANCE hInstance, const std::string& titleName, WNDPROC func);

	private:
		HINSTANCE   m_hInstance;
		std::string m_titleName;
		bool		m_isAlive;
	};

} // os
} // brUGE

