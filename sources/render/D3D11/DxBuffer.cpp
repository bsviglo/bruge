#include "DxBuffer.hpp"
#include "DxDevice.hpp"
#include "DxRenderDevice.hpp"

// start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	//-- see IBuffer::EUsage.
	const D3D11_USAGE dxUsage[] = 
	{
		D3D11_USAGE_DEFAULT,
		D3D11_USAGE_IMMUTABLE,
		D3D11_USAGE_DYNAMIC,
		D3D11_USAGE_STAGING
	};
	
	//-- see IBuffer::EAccess.
	const D3D11_MAP dxAccess[] = 
	{
		D3D11_MAP_READ,
		D3D11_MAP_WRITE,
		D3D11_MAP_READ_WRITE,
		D3D11_MAP_WRITE_DISCARD,
		D3D11_MAP_WRITE_NO_OVERWRITE
	};
	
	//-- see IBuffer::ECPUAccess.
	const UINT dxCPUAccess[] = 
	{
		0,
		D3D11_CPU_ACCESS_READ,
		D3D11_CPU_ACCESS_WRITE,
		D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE
	};
	
	//-- see IBuffer::EType.
	const D3D11_BIND_FLAG dxBindFlag[] = 
	{
		D3D11_BIND_VERTEX_BUFFER,
		D3D11_BIND_INDEX_BUFFER,
		D3D11_BIND_CONSTANT_BUFFER,
		D3D11_BIND_SHADER_RESOURCE
	};
	
	//-- texture buffer element format.
	const DXGI_FORMAT dxFormats[] =
	{
		DXGI_FORMAT_R32_FLOAT,
		DXGI_FORMAT_R32G32_FLOAT,
		DXGI_FORMAT_R32G32B32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT
	};

}
//--------------------------------------------------------------------------------------------------
// end unnamed namespace.


namespace brUGE
{
namespace render
{

	//------------------------------------------	
	DXBuffer::DXBuffer(EType type, EUsage usage, ECPUAccess cpuAccess)
		: IBuffer(type, usage, cpuAccess), m_bufferResource(nullptr)
	{
		
	}
	
	//------------------------------------------
	DXBuffer::~DXBuffer()
	{

	}
	
	//------------------------------------------
	bool DXBuffer::init(const void* data, uint elemCount, uint elemSize)
	{
		m_elemSize  = elemSize;
		m_elemCount = elemCount;

		D3D11_BUFFER_DESC desc;
		desc.ByteWidth		= elemSize * m_elemCount;
		desc.BindFlags		= dxBindFlag[m_type];
		desc.CPUAccessFlags	= dxCPUAccess[m_cpuAccess];
		desc.Usage			= dxUsage[m_usage];
		desc.MiscFlags		= 0;
			
		HRESULT hr = S_FALSE;
		
		// create structure D3D11_SUBRESOURCE_DATA only if we have initial data.
		if (data)
		{
			D3D11_SUBRESOURCE_DATA resData;
			resData.pSysMem = data;
			resData.SysMemPitch = 0;
			resData.SysMemSlicePitch = 0;

			hr = dxDevice()->CreateBuffer(&desc, &resData, &m_buffer); 
		}
		else
		{
			hr = dxDevice()->CreateBuffer(&desc, NULL, &m_buffer); 
		}
		
		if (FAILED(hr) || dxDevice().hasError())
		{
			ERROR_MSG("Failed to create buffer object.\nDesc: " + dxDevice().getErrorDesc());
			return false;
		}

		//-- set the base texture we'll use in the render system.
		hr = m_buffer->QueryInterface(IID_ID3D11Resource, (void **)&m_bufferResource);
		if (FAILED(hr) || dxDevice().hasError())
		{
			ERROR_MSG("Failed to create buffer object.\nDesc: " + dxDevice().getErrorDesc());
			return false;
		}

		if (m_type == TYPE_TEXTURE)
		{
			assert((!(m_elemSize % 4) && m_elemSize > 0 && m_elemSize <= 16)
				&& "m_elemSize must be aligned to 4 bytes.");

			D3D11_SHADER_RESOURCE_VIEW_DESC desc;
			desc.Format = dxFormats[(m_elemSize / 4) - 1];
			desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			desc.Buffer.ElementOffset = 0;
			desc.Buffer.ElementWidth  = m_elemCount;

			hr = dxDevice()->CreateShaderResourceView(m_buffer, &desc, &m_SRView);
			if (FAILED(hr) || dxDevice().hasError())
			{
				ERROR_MSG("Failed to create shader resource view.\n Desc: " + dxDevice().getErrorDesc());
				return false;
			}
		}
		
		return true;
	}
	
	//------------------------------------------
	void* DXBuffer::doMap(EAccess access)
	{
		D3D11_MAPPED_SUBRESOURCE data = { nullptr, 0 , 0 };

#ifdef _DEBUG
		HRESULT hr =
#endif // _DEBUG

		dxDevice().immediateContext()->Map(m_bufferResource, 0, dxAccess[access], 0, &data);

#ifdef _DEBUG
		if (FAILED(hr)) ERROR_MSG("Failed mapping buffer %d.", this);
#endif // _DEBUG

		//-- ToDo: take into account data.RowPitch value
		return data.pData;
	}
	
	//------------------------------------------
	void DXBuffer::doUnmap()
	{
		dxDevice().immediateContext()->Unmap(m_bufferResource, 0);
	}	

} // render
} // brUGE