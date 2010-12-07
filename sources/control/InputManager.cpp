#include "InputManager.h"
#include "MouseDevice.h"
#include "KeyboardDevice.h"
#include "utils/LogManager.h"

namespace brUGE
{

	DEFINE_SINGLETON(InputManager)

	// 
	//------------------------------------------
	InputManager::InputManager() : m_dxInput(NULL)
	{

	}

	// 
	//------------------------------------------
	InputManager::~InputManager()
	{
		if (m_dxInput)
		{
			m_dxInput->Release();
			m_dxInput = NULL;
		}
	}

	// �������� Direct Input ����������� � ��� ����������:
	// ���������� � ����.
	//------------------------------------------
	bool InputManager::init(HWND hWnd, HINSTANCE hInstance)
	{
		HRESULT result;
		//�������� ������������ ��������� �����
		result = ::DirectInput8Create(hInstance, DIRECTINPUT_VERSION,
			IID_IDirectInput8, (void**)&m_dxInput, NULL);

		if(FAILED(result))
		{
			m_dxInput = NULL;
			ERROR_MSG("Can't create Direct Input device.");
			return false;
		}

		// �������� ���������� ���������� � ����
		if (!m_keyboardDevice.init(m_dxInput, hWnd))
		{
			ERROR_MSG("Can't create keyboard device.");
			return false;
		}

		if (!m_mouseDevice.init(m_dxInput, hWnd))
		{
			ERROR_MSG("Can't create mouse device.");
			return false;
		}

		return true;
	}
	
	// 
	//------------------------------------------
	bool InputManager::update()
	{
		bool success = true;
		success &= m_keyboardDevice.update();
		success &= m_mouseDevice.update();
		return success;
	}

	// 
	//------------------------------------------
	void InputManager::setListeners(IKeyboardListener* kl, IMouseListener* ml)
	{
		m_keyboardDevice.setEventCallback(kl);
		m_mouseDevice.setEventCallback(ml);
	}
	
}/*namespace brUGE*/