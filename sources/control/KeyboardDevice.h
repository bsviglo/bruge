#pragma once

#include "prerequisites.h"
#include "input_listener.h"

//define default version of DirectInput 8
#define DIRECTINPUT_VERSION 0x0800
#include "dinput.h"

namespace brUGE
{

	//----------------------------------------------------------------------------------------------
	class KeyboardDevice
	{
	public:

		KeyboardDevice();
		~KeyboardDevice();

		// инициализация клавиатуры, получаем хендл контролера ввода, хендл окна и размер буфера.
		bool init(LPDIRECTINPUT8& dxInput, HWND hWnd);

		// Обновление устройства производиться на каждом тике.
		bool update();
		
		void setEventCallback(IKeyboardListener *_listener) { m_listener = _listener; }
		IKeyboardListener *getEventCallback(){ return m_listener; }

		bool isKeyDown(uchar _keyCode) { return (m_keyState[_keyCode].key & 0x80) != 0; }
		bool isModifiersDown( uchar _modifiers ) { return (m_modifiers & _modifiers) != 0; }

	private:
		struct KeyState
		{
			KeyState() : key(0), timeStamp(0) { }
			uchar key;
			DWORD timeStamp;
		};

		KeyState			 m_keyState[256];
		IKeyboardListener*	 m_listener;
		uint				 m_modifiers;
		LPDIRECTINPUTDEVICE8 m_dxKeyboard;
	};

} // brUGE
