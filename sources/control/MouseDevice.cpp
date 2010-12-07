#include "MouseDevice.h"
#include "utils/LogManager.h"

namespace brUGE
{

	//------------------------------------------
	MouseDevice::MouseDevice()
		: m_hWnd(NULL), m_buttons(0), m_listener(NULL), m_dxMouse(NULL), m_exclusiveMode(false) // true
	{
		m_relAxis[0] = 0.0f; m_relAxis[1] = 0.0f; m_relAxis[2] = 0.0f;
		m_absAxis[0] = 0.0f; m_absAxis[1] = 0.0f; m_absAxis[2] = 0.0f;
	}

	//------------------------------------------
	MouseDevice::~MouseDevice()
	{	
		if (m_dxMouse)
		{
			m_dxMouse->Unacquire();
			m_dxMouse->Release();
			m_dxMouse = NULL;
		}
	}

	//------------------------------------------
	bool MouseDevice::init(LPDIRECTINPUT8 &dxInput, HWND hWnd)
	{
		// —охран€ем так как нам он понадобитьс€ при конвертировании мышиных координат.
		m_hWnd = hWnd;

		if( FAILED(dxInput->CreateDevice(GUID_SysMouse, &m_dxMouse, NULL)) )
		{
			m_dxMouse = NULL;
			ERROR_MSG("Can't create mouse device.");
			return false;
		}

		if( FAILED(m_dxMouse->SetDataFormat(&c_dfDIMouse2)) )
		{
			m_dxMouse->Release();
			m_dxMouse = NULL;
			ERROR_MSG("Can't set a data format to mouse device.");
			return false;
		}
	
		if( FAILED(m_dxMouse->SetCooperativeLevel(hWnd,
			DISCL_BACKGROUND | ((m_exclusiveMode) ? DISCL_EXCLUSIVE : DISCL_NONEXCLUSIVE))) )
		{
			m_dxMouse->Release();
			m_dxMouse = NULL;
			ERROR_MSG("Can't set a cooperation level to mouse device.");
			return false;
		}
		
		DIPROPDWORD dipdw;

		dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj        = 0;
		dipdw.diph.dwHow        = DIPH_DEVICE;
		dipdw.dwData            = MOUSE_BUFFER_SIZE;

		if( FAILED(m_dxMouse->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph )) )
		{
			m_dxMouse->Release();
			m_dxMouse = NULL;
			ERROR_MSG("Can't set properties DIPROP_BUFFERSIZE of keyboard device.");
			return false;
		}

		HRESULT hr = m_dxMouse->Acquire();
		if (FAILED(hr) && hr != DIERR_OTHERAPPHASPRIO)
		{
			m_dxMouse->Release();
			m_dxMouse = NULL;
			ERROR_MSG("Can't acquire with mouse device.");
			return false;
		}

		return true;
	}

	//------------------------------------------
	bool MouseDevice::update()
	{
		// обновл€ем относительные данные о перемещени€ курсора мыши.
		for (uint i=0; i<3; ++i)
			m_relAxis[i] = 0.0f;

		DIDEVICEOBJECTDATA diBuff[MOUSE_BUFFER_SIZE];
		DWORD entries = MOUSE_BUFFER_SIZE;

		HRESULT hr = m_dxMouse->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), diBuff, &entries, 0 );
		// ѕровер€ем успешность получени€ данных от устройства мыши,
		// если получение было не успешно, и было св€зано с поторей устройства,
		// пытаемс€ захватить устройство и после захвата пытаемс€ снова получить данные,
		// если же и в этот раз получили ошибку, то выкидываем исключение о невозмонжности подключитьс€ к устройству.
		if( hr != DI_OK )
		{
			hr = m_dxMouse->Acquire();
			while( hr == DIERR_INPUTLOST ) 
				hr = m_dxMouse->Acquire();

			hr = m_dxMouse->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), diBuff, &entries, 0 );

			if (FAILED(hr))
			{
				ERROR_MSG("can't acquire mouse device during update state.");
				return false;
			}
		}

		bool axesMoved = false;
		
		for( uint i = 0; i < entries; ++i )
		{
			switch( diBuff[i].dwOfs )
			{
			case DIMOFS_BUTTON0: _doMouseClick(0, diBuff[i]); break;
			case DIMOFS_BUTTON1: _doMouseClick(1, diBuff[i]); break;
			case DIMOFS_BUTTON2: _doMouseClick(2, diBuff[i]); break;
			case DIMOFS_BUTTON3: _doMouseClick(3, diBuff[i]); break;
			case DIMOFS_BUTTON4: _doMouseClick(4, diBuff[i]); break;	
			case DIMOFS_BUTTON5: _doMouseClick(5, diBuff[i]); break;
			case DIMOFS_BUTTON6: _doMouseClick(6, diBuff[i]); break;	
			case DIMOFS_BUTTON7: _doMouseClick(7, diBuff[i]); break;	
			case DIMOFS_X:
				m_relAxis[0] += static_cast<signed int>(diBuff[i].dwData);
				axesMoved = true;
				break;
			case DIMOFS_Y:
				m_relAxis[1] += static_cast<signed int>(diBuff[i].dwData);
				axesMoved = true;
				break;
			case DIMOFS_Z:
				m_relAxis[2] += static_cast<signed int>(diBuff[i].dwData);
				axesMoved = true;
				break;
			default: break;
			}
		}

		if( axesMoved )
		{
			if( !m_exclusiveMode )
			{
				POINT point;
				GetCursorPos(&point);
				ScreenToClient(m_hWnd, &point);
				m_absAxis[0] = static_cast<float>(point.x);
				m_absAxis[1] = static_cast<float>(point.y);
			}
			else
			{
				m_absAxis[0] += m_relAxis[0];
				m_absAxis[1] += m_relAxis[1];
			}
			m_absAxis[2] += m_relAxis[2];

			_doMouseMove(diBuff[entries-1].dwTimeStamp);
		}

		return true;
	}

	// 
	//------------------------------------------
	void MouseDevice::_doMouseClick(uint button, DIDEVICEOBJECTDATA &data)
	{
		if( data.dwData & 0x80 )
		{
			m_buttons |= 1 << button; // включить флаг.
			if( m_listener)
				m_listener->handleMouseClick( MouseEvent(static_cast<uchar>(button), data.dwTimeStamp, true));
		}
		else
		{
			m_buttons &= ~(1 << button); // отключить флаг.
			if( m_listener )
				m_listener->handleMouseClick( MouseEvent(static_cast<uchar>(button), data.dwTimeStamp, false));
		}
	}
	
	// 
	//------------------------------------------
	void MouseDevice::_doMouseMove(uint /*timeStamp*/)
	{
		if ( m_listener )
			m_listener->handleMouseMove(
				MouseAxisEvent(m_relAxis[0], m_relAxis[1], m_relAxis[2],
				   				 m_absAxis[0], m_absAxis[1], m_absAxis[2]));
	}
}