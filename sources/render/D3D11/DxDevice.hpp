#pragma once

#include "Dx_common.hpp"
#include <string>

namespace brUGE
{
namespace render
{
	// Proxy to ID3D11Device.
	//----------------------------------------------------------------------------------------------
	class DXDevice
	{
	public:
		DXDevice(ID3D11Device* device = NULL);
		~DXDevice();

		ID3D11Device* operator = (ID3D11Device* device);
		inline ID3D11Device* operator -> () const
		{
			assert(m_device.get());
			return m_device.get();
		}

		inline ID3D11DeviceContext* immediateContext()
		{
			assert(m_device.get());
			ID3D11DeviceContext* context = nullptr;
			m_device->GetImmediateContext(&context);
			assert(context);
			return context;
		}

		// reset object.
		void reset();
		inline ID3D11Device* get() const { return m_device.get(); }
		inline bool isValid() const	{ return m_device.isValid(); }
		
		// There are errors?
		inline bool hasError() const
		{
			ID3D11InfoQueue* pInfoQueue = NULL; 
			if (SUCCEEDED(m_device->QueryInterface(__uuidof(ID3D11InfoQueue), (LPVOID*)&pInfoQueue)))
				if (pInfoQueue->GetNumStoredMessages() > 0)
					return true;
			return false;
		}
		
		// returns error description.
		const std::string getErrorDesc(HRESULT lastResult = NO_ERROR) const;

	private:
		ComPtr<ID3D11Device> m_device;
	};

} // render
} // brUGE
