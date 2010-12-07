#include "KeyboardDevice.h"
#include "utils/LogManager.h"


// start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	// ���������� ����� ����� ������� ����� �������
	// ������� ���������� ������ ������� ��������� � ��� ��� ������� ������.
	uint g_timeLimit = 500;

	// ���������� ����� ����� �������� ������� � ����������� ���������� ���������
	// � ������� �������.  
	uint g_repetSendKeyTime = 50;

	// ���������� ����������� scan-����� � ������� ascii-���������.
	// TODO: ���������� ������� ���� ����� ��� ������-���������.!!
	uint translateScan2Ascii(uchar scanCode)
	{
		word translateResult[2];
		static HKL	 layout = GetKeyboardLayout(0);
		static uchar state[256];

		if (GetKeyboardState(state) == FALSE)
			return 0;

		uint vk = MapVirtualKeyEx(scanCode, 1, layout);

		if (FAILED(ToAsciiEx(vk, scanCode, state, (LPWORD)translateResult, 0, layout)))
			return 0;
		else
			return translateResult[0];
	}
}
//--------------------------------------------------------------------------------------------------
// end unnamed namespace.


namespace brUGE
{

	//------------------------------------------
	KeyboardDevice::KeyboardDevice() : m_dxKeyboard(NULL), m_listener(NULL), m_modifiers(0)
	{

	}

	//------------------------------------------
	KeyboardDevice::~KeyboardDevice()
	{
		if (m_dxKeyboard)
		{
			m_dxKeyboard->Unacquire();
			m_dxKeyboard->Release();
			m_dxKeyboard = NULL;
		}
	}

	// 
	//------------------------------------------
	bool KeyboardDevice::init(LPDIRECTINPUT8& dxInput, HWND hWnd)
	{
		// �������� ���������� ����������
		if (FAILED(dxInput->CreateDevice(GUID_SysKeyboard, &m_dxKeyboard, NULL)))
		{
			m_dxKeyboard = NULL;
			ERROR_MSG("Can't create keyboard device.");
			return false;
		}

		// ���������� ������ ������
		if (FAILED(m_dxKeyboard->SetDataFormat(&c_dfDIKeyboard)))
		{
			m_dxKeyboard->Release();
			m_dxKeyboard = NULL;
			ERROR_MSG("Can't set a data format to keyboard device.");
			return false;
		}

		// ��������� �������� ����������, �.�. ��� �� ����� �������������� ��������� � ������� ������������.

		// TODO: ������� ��������� ���������� ����������� ���������� � ������.!!

		if (FAILED(m_dxKeyboard->SetCooperativeLevel(hWnd, /*DISCL_FOREGROUND*/ DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)))
		{
			m_dxKeyboard->Release();
			m_dxKeyboard = NULL;
			ERROR_MSG("Can't set a cooperation level to keyboard device.");
			return false;
		}

		// �������� ������� ����������������� ������.
		DIPROPDWORD dipdw;
		dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj        = 0;
		dipdw.diph.dwHow        = DIPH_DEVICE;
		dipdw.dwData            = KEYBOARD_BUFFER_SIZE;

		if (FAILED(m_dxKeyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
		{
			m_dxKeyboard->Release();
			m_dxKeyboard = NULL;
			ERROR_MSG("Can't set properties DIPROP_BUFFERSIZE of keyboard device.");
			return false;
		}	

		// ������ ����������.
		if (FAILED(m_dxKeyboard->Acquire()))
		{
			m_dxKeyboard->Release();
			m_dxKeyboard = NULL;
			ERROR_MSG("Can't acquire with keyboard device.");
			return false;
		}

		return true;
	}

	//------------------------------------------
	bool KeyboardDevice::update()
	{
		// ���� ������� ������ �� � ������� �� 256 ���������, ������� ����� ����� �������� = 128, ���� �� �� �� = 0
		DIDEVICEOBJECTDATA data[KEYBOARD_BUFFER_SIZE];
		DWORD entries = KEYBOARD_BUFFER_SIZE;
		HRESULT result;
				
		// ������� ��� ������������ � ������ ������
		result = m_dxKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), data, &entries, 0);  

		// ���� ���������� ���� �������� � �� ���� ������, �� ����� � ����� �������� ��� ��������� �������
		if (result != DI_OK)
		{
			result = m_dxKeyboard->Acquire();
			while (result == DIERR_INPUTLOST)
			{
				result = m_dxKeyboard->Acquire();
			}
		}
		
		// ���� �� ������ ��������� �� ������� ����������.
		if (FAILED(result))
		{
			ERROR_MSG("Input System: Fatal error: problem with keyboard device");
			return false;
		}
		
		uint keyCode	= 0;
		uint timeStamp	= 0;

		// ���������� �� ���� ���������� ������, ��������� ��������� � �������� �� �����������(���������)
		for (uint i = 0; i < entries; ++i)
		{
			keyCode		= data[i].dwOfs;
			timeStamp	= data[i].dwTimeStamp;

			m_keyState[keyCode].key		= static_cast<uchar>(data[i].dwData);
			m_keyState[keyCode].timeStamp	= timeStamp;
			
			if (data[i].dwData & 0x80)
			{

				// �������� ������������ �����
				if( keyCode== DIK_LCONTROL || keyCode == DIK_RCONTROL )		m_modifiers |= KM_CTRL;
				else if( keyCode == DIK_LSHIFT || keyCode == DIK_RSHIFT )	m_modifiers |= KM_SHIFT;
				else if( keyCode == DIK_LMENU || keyCode == DIK_RMENU )		m_modifiers |= KM_ALT;

				if (m_listener)
					m_listener->handleKeyboardEvent(KeyboardEvent(static_cast<uchar>(keyCode),
						timeStamp, translateScan2Ascii(static_cast<uchar>(keyCode)), KS_DOWN));
			}
			else
			{

				// ��������� ������������ �����
				if( keyCode== DIK_LCONTROL || keyCode == DIK_RCONTROL )		m_modifiers &= ~KM_CTRL;
				else if( keyCode == DIK_LSHIFT || keyCode == DIK_RSHIFT )	m_modifiers &= ~KM_SHIFT;
				else if( keyCode == DIK_LMENU || keyCode == DIK_RMENU )		m_modifiers &= ~KM_ALT;

				if (m_listener)
					m_listener->handleKeyboardEvent(KeyboardEvent(static_cast<uchar>(keyCode), timeStamp, 0, KS_UP));
			}
		}

		// ��������� �������������� ������ ��������� ������ � ������� ������������ ������
		DWORD time = GetTickCount();

		for (uint i=0; i<256; ++i)
		{
			if ((m_keyState[i].key & 0x80) && (time - m_keyState[i].timeStamp) > g_timeLimit)
			{
				m_keyState[i].timeStamp += g_repetSendKeyTime;
				if (m_listener)
					m_listener->handleKeyboardEvent(KeyboardEvent(static_cast<uchar>(i),
						time, translateScan2Ascii(static_cast<uchar>(i)), KS_PRESSED));
			}
		}

		return true;
	}

} // brUGE

