#include "engine/Engine.h"
#include "render/render_system.hpp"
#include "../win32Res/resource.h"

using namespace brUGE;
using namespace brUGE::render;

bool g_needToStartApp	   = true;
ERenderAPIType g_renderAPI = RENDER_API_DX10;

// start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	HWND g_resolutionsBox  = NULL;
	HWND g_renderApiBox	   = NULL;
	HWND g_modeBox		   = NULL;
	HWND g_antialiasingBox = NULL;
	
	//--
	struct DisplayResolution
	{
		uint  width;
		uint  height;
		uint  bpp;
	};
	
	//--
	std::string toStr(const DisplayResolution& res)
	{
		return utils::makeStr("%dx%dx%d", res.width, res.height, res.bpp);
	}
	
	//--
	DisplayResolution dResolutions[] = 
	{
		{ 640,  480, 32},
		{ 800,  600, 32},
		{1024,  600, 32},
		{1024,  768, 32},
		{1366,  768, 32},
		{1280, 1024, 32},
		{1440,  900, 32},
		{1680, 1050, 32}
	};
	
	template<typename Type, size_t size>
	size_t array_size(Type (&) [size]) { return size; }
}
//--------------------------------------------------------------------------------------------------
// end unnamed namespace.


namespace brUGE
{
	
	//------------------------------------------
	EngineConfigDialog::EngineConfigDialog()
	{

	}
	
	//------------------------------------------
	EngineConfigDialog::~EngineConfigDialog()
	{

	}
	
	//------------------------------------------
	bool EngineConfigDialog::display()
	{
		// Display dialog
		// Don't return to caller until dialog dismissed
		int res = 0;

		res = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG1),
			NULL, (DLGPROC)os::g_mainDlgProc);

		if (res == -1)
		{
			int winError = GetLastError();
			char* errDesc;
			int i;

			errDesc = new char[255];
			// Try windows errors first
			i = FormatMessage(
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, winError,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) errDesc, 255, NULL
				);

			BR_EXCEPT_F("%d %s", winError, errDesc);
		}

		if (res) return true;
		else	 return false;
	}

namespace os
{
	
	//------------------------------------------
	LRESULT WINAPI g_mainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam,	LPARAM lParam)
	{
		//-- handle messages.
		switch (uMsg)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		return 0;
	}
	
	//------------------------------------------
	LRESULT WINAPI g_mainDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
	{
		switch (uMsg)
		{
		case WM_INITDIALOG:
			{
				g_renderApiBox	  = GetDlgItem(hDlg, IDC_COMBO1);
				g_resolutionsBox  = GetDlgItem(hDlg, IDC_COMBO2);
				g_modeBox		  = GetDlgItem(hDlg, IDC_COMBO3);
				g_antialiasingBox = GetDlgItem(hDlg, IDC_COMBO4);

				SendMessage(g_renderApiBox,	   CB_RESETCONTENT, 0, 0); 
				SendMessage(g_resolutionsBox,  CB_RESETCONTENT, 0, 0); 
				SendMessage(g_modeBox,		   CB_RESETCONTENT, 0, 0); 
				SendMessage(g_antialiasingBox, CB_RESETCONTENT, 0, 0); 
				
				// выбор рендера.
				SendMessage(g_renderApiBox, CB_ADDSTRING, 0, (LPARAM)"OpenGL 3.*");
				SendMessage(g_renderApiBox, CB_ADDSTRING, 0, (LPARAM)"Direct3D 10");

				// выбор разрешения экрана.
				for( uint i = 0; i < array_size(dResolutions); ++i)
					SendMessage(g_resolutionsBox, CB_ADDSTRING, 0, (LPARAM)toStr(dResolutions[i]).c_str());

				// выбор режима: полноэкранные или оконный.
				SendMessage(g_modeBox, CB_ADDSTRING, 0, (LPARAM)"windowed");
				SendMessage(g_modeBox, CB_ADDSTRING, 0, (LPARAM)"full-screen");

				// выбор режима для удаление aliasing-a.
				SendMessage(g_antialiasingBox, CB_ADDSTRING, 0, (LPARAM)"off");

				SendMessage(g_renderApiBox,	   CB_SETCURSEL, 1, (LPARAM)"Direct3D 10"); 
				SendMessage(g_resolutionsBox,  CB_SETCURSEL, 0, (LPARAM)toStr(dResolutions[0]).c_str()); 
				SendMessage(g_modeBox,		   CB_SETCURSEL, 0, (LPARAM)"windowed"); 
				SendMessage(g_antialiasingBox, CB_SETCURSEL, 0, (LPARAM)"off"); 

				return TRUE;
			}
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDOK:
				{
					render::VideoMode videoMode;
					
					uint index = (uint)SendMessage(g_resolutionsBox, CB_GETCURSEL, 0, 0);
					videoMode.width	 = dResolutions[index].width;
					videoMode.height = dResolutions[index].height;
					videoMode.bpp	 = dResolutions[index].bpp;
					videoMode.depth	 = 24;

					index = (uint)SendMessage(g_renderApiBox, CB_GETCURSEL, 0, 0);
					g_renderAPI = static_cast<render::ERenderAPIType>(index);

					videoMode.fullScreen = ((uint)SendMessage(g_modeBox, CB_GETCURSEL, 0, 0)) != 0;

					index = (uint)SendMessage(g_antialiasingBox, CB_GETCURSEL, 0, 0);

					// TODO: обрабатывать выбор режима антиалиасинга.

					Engine::instance().setVideoMode(videoMode);
					
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;
				}
			case IDCANCEL: 
				g_needToStartApp = FALSE;
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			return FALSE;
		}

		return FALSE;
	}

} // os
} // brUGE
