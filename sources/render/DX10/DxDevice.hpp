#pragma once

#include "Dx_common.hpp"
#include <string>

namespace brUGE
{
namespace render
{
	// Proxy to ID3D10Device.
	//----------------------------------------------------------------------------------------------
	class DXDevice
	{
	public:
		DXDevice(ID3D10Device* device = NULL);
		~DXDevice();

		ID3D10Device* operator = (ID3D10Device* device);
		inline ID3D10Device* operator -> () const
		{
			assert(m_device.get());
			return m_device.get();
		}

		// reset object.
		void reset();
		inline ID3D10Device* get() const { return m_device.get(); }
		inline bool isValid() const	{ return m_device.isValid(); }
		
		// There are errors?
		inline bool hasError() const
		{
			ID3D10InfoQueue* pInfoQueue = NULL; 
			if (SUCCEEDED(m_device->QueryInterface(__uuidof(ID3D10InfoQueue), (LPVOID*)&pInfoQueue)))
				if (pInfoQueue->GetNumStoredMessages() > 0)
					return true;
			return false;
		}
		
		// returns error description.
		const std::string getErrorDesc(HRESULT lastResult = NO_ERROR) const;

	private:
		ComPtr<ID3D10Device> m_device;
	};

} // render
} // brUGE
