#pragma once

#include "prerequisites.h"

//define default version of DirectInput 8
#define DIRECTINPUT_VERSION 0x0800
#include "dinput.h"

namespace brUGE
{
		
	#define KEYBOARD_BUFFER_SIZE 17
	#define MOUSE_BUFFER_SIZE 64
	
	
	// ��������� ��������� ���������� � ����������� ��������� ����,
	// ��� �������� ��� ������������� ��� � ���������� ����������� �����
	// ���� ����, �� ������� Z - �������� ������ ����.
	// ������������ ��� ��������� handleMouseMove() ��������� �� ����.
	//------------------------------------------
	struct MouseAxisEvent
	{
		MouseAxisEvent(float relX, float relY, float relZ, float absX, float absY, float absZ)
			: relX(relX), relY(relY), relZ(relZ), absX(absX), absY(absY), absZ(absZ) { }

		float relX, relY, relZ;
		float absX, absY, absZ;
	};

	// ��������� �������� ���������� � ������� ������ ����,
	// � ����� ������� �� ������� � �� ���������.
	// ������������ ��� ��������� handleMouseClick() ��������� �� ����.
	//------------------------------------------
	struct MouseEvent
	{
		MouseEvent(uchar button, uint timeStamp, bool isDown)
			: button(button), timeStamp(timeStamp), isDown(isDown) { }
			
		bool  isDown;
		uchar button;
		uint  timeStamp;
	};
	
	//-- mouse buttons descriptor.
	enum MouseButton
	{
		MB_LEFT_BUTTON = 0,
		MB_RIGHT_BUTTON,
		MB_MIDDLE_BUTTON,
		MB_BUTTON3,
		MB_BUTTON4,
		MB_BUTTON5,
		MB_BUTTON6,
		MB_BUTTON7
	};

	// ������������ ������������� �����.
	enum KeyboardModifier
	{
		KM_SHIFT = 1,
		KM_CTRL  = 1 << 1,
		KM_ALT   = 1 << 2
	};

	// ������������ ��������� ��������� ������. 
	// KS_DOWN		- ������	
	// KS_PRESSED	- ������������ �������
	// KS_UP		- ��������
	// ���� ������� ������ ��� ������ ��������� ������� �������, �� ����� ���� ������������� ��������,
	// �� ����� ������ ��������������� ���� ������������� ��� ������� ����������
	enum KeyState
	{
		KS_UP		= 0,
		KS_DOWN		= 1,
		KS_PRESSED	= 2
	};

	// ��������� ������� ������������� �� ����������, ���������� ��������� ��� �������.
	struct KeyboardEvent
	{
		KeyboardEvent(uchar _keyCode, uint _timeStamp, uint _text, KeyState _state)
			: keyCode(_keyCode), timeStamp(_timeStamp), text(_text), state(_state) { }

		uchar	 keyCode;
		uint	 timeStamp;
		KeyState state;
		uint	 text;
	};
	
	//
	// 
	//------------------------------------------------------------
	class IMouseListener
	{
	public:
		virtual void handleMouseClick(const MouseEvent &_me) = 0;
		virtual void handleMouseMove(const MouseAxisEvent &_me) = 0;
	};

	//
	// 
	//------------------------------------------------------------
	class IKeyboardListener
	{
	public:
		virtual void handleKeyboardEvent(const KeyboardEvent &_ke) = 0;
	};
	
	// ����������� �� ���� ����������� � ������������ �� ����� ���������.
	// ������ ����� ���������� ������������ ��� ������� ����������.
	//------------------------------------------
	class IInputListener : public IKeyboardListener,
						   public IMouseListener
	{

	};

} // brUGE
