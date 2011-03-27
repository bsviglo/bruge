#pragma once

#include "prerequisites.hpp"
#include "input_listener.h"
#include "MouseDevice.h"
#include "KeyboardDevice.h"
#include "utils/Singleton.h"

// TODO: 1) ����������� ��������� ��������� ������ ���������� (exclusiveMode � MouseDevice).
//		 3) ��������� ���������������� ��������� � ������.
//

namespace brUGE
{

	class KeyboardDevice;
	class MouseDevice;

	//
	// 
	//------------------------------------------------------------
	class InputManager : public utils::Singleton<InputManager>
	{
	public:
		InputManager();
		~InputManager();
		
		bool init(HWND hWnd, HINSTANCE hInstance);
		bool update();
		void setListeners(IKeyboardListener* kl, IMouseListener* ml);

		bool isKeyDown(uchar key) { return m_keyboardDevice.isKeyDown(key); }
		bool isButtonDown(uchar button) { return m_mouseDevice.isButtonDown(button); }
		bool isModifierDown(uchar modifier) { return m_keyboardDevice.isModifiersDown(modifier); }
	
	private:
		LPDIRECTINPUT8 m_dxInput;
		KeyboardDevice m_keyboardDevice;
		MouseDevice	   m_mouseDevice;
	};

} // brUGE
