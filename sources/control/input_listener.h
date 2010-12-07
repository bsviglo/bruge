#pragma once

#include "prerequisites.h"

//define default version of DirectInput 8
#define DIRECTINPUT_VERSION 0x0800
#include "dinput.h"

namespace brUGE
{
		
	#define KEYBOARD_BUFFER_SIZE 17
	#define MOUSE_BUFFER_SIZE 64
	
	
	// —труктура содержить информацию о перемещении указател€ мыши,
	// она содержит как относительные так и абсолютные перемещени€ вдоль
	// трех осей, из которых Z - означает колесо мыши.
	// »спользуетс€ при генерации handleMouseMove() сообщени€ от мыши.
	//------------------------------------------
	struct MouseAxisEvent
	{
		MouseAxisEvent(float relX, float relY, float relZ, float absX, float absY, float absZ)
			: relX(relX), relY(relY), relZ(relZ), absX(absX), absY(absY), absZ(absZ) { }

		float relX, relY, relZ;
		float absX, absY, absZ;
	};

	// —труктура содержит информацию о нажатой кнопке мыши,
	// а также времени ее нажати€ и ее состо€нии.
	// »спользуетс€ при генерации handleMouseClick() сообщени€ от мыши.
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

	// ѕеречисление модификаторов ввода.
	enum KeyboardModifier
	{
		KM_SHIFT = 1,
		KM_CTRL  = 1 << 1,
		KM_ALT   = 1 << 2
	};

	// ѕеречисление возможных состо€ний клавиш. 
	// KS_DOWN		- нажата	
	// KS_PRESSED	- удерживаетс€ нажатой
	// KS_UP		- отпущена
	// ≈сли неважно зажата или только приозошло нажатие клавиши, но важен факт происход€щего действи€,
	// то можно просто воспользоватьс€ этим перечислением как булевой переменной
	enum KeyState
	{
		KS_UP		= 0,
		KS_DOWN		= 1,
		KS_PRESSED	= 2
	};

	// —труктура событи€ произошедшего на клавиатуре, отсылаетс€ слушателю дл€ анализа.
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
	
	// Ќаследуетс€ от двух интерфейсов и представл€ет их общий интерфейс.
	// ”добно когда необходимо обрабатывать все входные устройства.
	//------------------------------------------
	class IInputListener : public IKeyboardListener,
						   public IMouseListener
	{

	};

} // brUGE
