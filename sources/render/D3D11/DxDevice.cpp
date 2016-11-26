#include "DxDevice.hpp"

namespace brUGE
{
namespace render
{
	
	//------------------------------------------
	DXDevice::DXDevice(ID3D11Device* device /* = NULL */) :	m_device(device)
	{

	}
	
	//------------------------------------------
	DXDevice::~DXDevice()
	{

	}
	
	//------------------------------------------
	ID3D11Device* DXDevice::operator = (ID3D11Device* device)
	{
		m_device = device;
		return m_device.get();
	}
	
	//------------------------------------------
	void DXDevice::reset()
	{
		m_device.reset();
	}
	
	//------------------------------------------
	const std::string DXDevice::getErrorDesc(HRESULT lastResult /* = NO_ERROR */) const
	{
		if (!m_device) return "NULL device";

		std::string res;

		switch (lastResult)
		{
		case NO_ERROR:
			break;
		case E_INVALIDARG:
			res += "invalid parameters were passed.\n";
			break;
		default:
			assert(false && "unknown HRESULT."); // unknown HRESULT
		}

		ID3D11InfoQueue* pInfoQueue = NULL; 
		HRESULT hr = m_device->QueryInterface(__uuidof(ID3D11InfoQueue), (LPVOID*)&pInfoQueue);

		if (SUCCEEDED(hr))
		{
			uint numStoredMessages = pInfoQueue->GetNumStoredMessages();
			for (uint i = 0; i < numStoredMessages; ++i)
			{
				SIZE_T messageLength = 0;
				hr = pInfoQueue->GetMessage(i, NULL, &messageLength);
				D3D11_MESSAGE* pMessage = (D3D11_MESSAGE*)malloc(messageLength);
				hr = pInfoQueue->GetMessage(i, pMessage, &messageLength);
				res += pMessage->pDescription + std::string("\n");
				free(pMessage);
			}
			// clear messages queue
			pInfoQueue->ClearStoredMessages();
		}

		return res;
	}

} // render
} // brUGE