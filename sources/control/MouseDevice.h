#pragma once

#include "prerequisites.hpp"
#include "input_listener.h"

namespace brUGE
{

	//----------------------------------------------------------------------------------------------
	class MouseDevice
	{
	public:
		MouseDevice();
		~MouseDevice();
		
		// ������������� ����, �������� ����� ���������� �����, ����� ���� � ������ ������.
		bool init(LPDIRECTINPUT8& dxInput, HWND hWnd);

		// ���������� ���������� ������������� �� ������ ����.
		bool update();

		void setEventCallback(IMouseListener* listener) { this->m_listener = listener; }
		IMouseListener *getEventCallback() { return m_listener; }

		bool isButtonDown(uint button) { return (m_buttons & (1 << button)) != 0; };

	private:
		void _doMouseClick(uint button, DIDEVICEOBJECTDATA &data);
		void _doMouseMove(uint timeStamp);

	private:
		HWND				 m_hWnd;
		IMouseListener*		 m_listener;
		bool				 m_exclusiveMode;
		uint				 m_buttons; // 32 ������ ������. �� ��� 8 ������� ����� �������� �� ��������� ������.
		float				 m_relAxis[3];
		float				 m_absAxis[3];
		LPDIRECTINPUTDEVICE8 m_dxMouse;	
	};

} // brUGE
