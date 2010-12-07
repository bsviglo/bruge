#include "DxTexture.hpp"
#include "DxDevice.hpp"
#include "DxRenderDevice.hpp"

using namespace brUGE;
using namespace brUGE::render;

// start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	//----------------------------------------------------------------------------------------------
	const UINT getDXBindFlags(uint format)
	{
		UINT bindFlags = 0;
		if (format & ITexture::BIND_DEPTH_STENCIL)	 bindFlags |= D3D10_BIND_DEPTH_STENCIL;
		if (format & ITexture::BIND_RENDER_TARGET)	 bindFlags |= D3D10_BIND_RENDER_TARGET;
		if (format & ITexture::BIND_SHADER_RESOURCE) bindFlags |= D3D10_BIND_SHADER_RESOURCE;
		return bindFlags;
	}
	
	//-- see ITexture::EFormat.
	const DXGI_FORMAT dxTexFormat[] = 
	{
		DXGI_FORMAT_UNKNOWN,

		DXGI_FORMAT_R8_UNORM,
		DXGI_FORMAT_R8G8_UNORM,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,

		DXGI_FORMAT_R16_UNORM,
		DXGI_FORMAT_R16G16_UNORM,
		DXGI_FORMAT_R16G16B16A16_UNORM,

		DXGI_FORMAT_R8_SNORM,
		DXGI_FORMAT_R8G8_SNORM,
		DXGI_FORMAT_R8G8B8A8_SNORM,

		DXGI_FORMAT_R16_SNORM,
		DXGI_FORMAT_R16G16_SNORM,
		DXGI_FORMAT_R16G16B16A16_SNORM,

		DXGI_FORMAT_R16_FLOAT,
		DXGI_FORMAT_R16G16_FLOAT,
		DXGI_FORMAT_R16G16B16A16_FLOAT,

		DXGI_FORMAT_R32_FLOAT,
		DXGI_FORMAT_R32G32_FLOAT,
		DXGI_FORMAT_R32G32B32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,

		DXGI_FORMAT_R16_SINT,
		DXGI_FORMAT_R16G16_SINT,
		DXGI_FORMAT_R16G16B16A16_SINT,

		DXGI_FORMAT_R32_SINT,
		DXGI_FORMAT_R32G32_SINT,
		DXGI_FORMAT_R32G32B32_SINT,
		DXGI_FORMAT_R32G32B32A32_SINT,

		DXGI_FORMAT_R16_UINT,
		DXGI_FORMAT_R16G16_UINT,
		DXGI_FORMAT_R16G16B16A16_UINT,

		DXGI_FORMAT_R32_UINT,
		DXGI_FORMAT_R32G32_UINT,
		DXGI_FORMAT_R32G32B32_UINT,
		DXGI_FORMAT_R32G32B32A32_UINT,

		DXGI_FORMAT_R9G9B9E5_SHAREDEXP,
		DXGI_FORMAT_R11G11B10_FLOAT,
		DXGI_FORMAT_B5G6R5_UNORM,
		DXGI_FORMAT_R10G10B10A2_UNORM,

		DXGI_FORMAT_D16_UNORM,
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		DXGI_FORMAT_D32_FLOAT,

		DXGI_FORMAT_BC1_UNORM,
		DXGI_FORMAT_BC2_UNORM,
		DXGI_FORMAT_BC3_UNORM,
		DXGI_FORMAT_BC4_UNORM,
		DXGI_FORMAT_BC5_UNORM,
	};

	//-- converts from typeful to typeless format.
	//----------------------------------------------------------------------------------------------
	DXGI_FORMAT fromTypefulToTypeless(DXGI_FORMAT typefulFormat)
	{
		switch (typefulFormat)
		{
		case DXGI_FORMAT_R8_UNORM:
		case DXGI_FORMAT_R8_SNORM:
			return DXGI_FORMAT_R8_TYPELESS;

		case DXGI_FORMAT_R8G8_UNORM:
		case DXGI_FORMAT_R8G8_SNORM:
			return DXGI_FORMAT_R8G8_TYPELESS;

		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R8G8B8A8_SINT:
		case DXGI_FORMAT_R8G8B8A8_UINT:
			return DXGI_FORMAT_R8G8B8A8_TYPELESS;

		case DXGI_FORMAT_R16_UNORM:
		case DXGI_FORMAT_R16_SNORM:
		case DXGI_FORMAT_R16_FLOAT:
		case DXGI_FORMAT_R16_UINT:
		case DXGI_FORMAT_R16_SINT:
		case DXGI_FORMAT_D16_UNORM:
			return DXGI_FORMAT_R16_TYPELESS;

		case DXGI_FORMAT_R16G16_UNORM:
		case DXGI_FORMAT_R16G16_SNORM:
		case DXGI_FORMAT_R16G16_FLOAT:
		case DXGI_FORMAT_R16G16_UINT:
		case DXGI_FORMAT_R16G16_SINT:
			return DXGI_FORMAT_R16G16_TYPELESS;

		case DXGI_FORMAT_R16G16B16A16_UNORM:
		case DXGI_FORMAT_R16G16B16A16_SNORM:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
		case DXGI_FORMAT_R16G16B16A16_UINT:
		case DXGI_FORMAT_R16G16B16A16_SINT:
			return DXGI_FORMAT_R16G16B16A16_TYPELESS;

		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R32_UINT:
		case DXGI_FORMAT_R32_SINT:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			return DXGI_FORMAT_R32_TYPELESS;

		case DXGI_FORMAT_R32G32_FLOAT:
		case DXGI_FORMAT_R32G32_UINT:
		case DXGI_FORMAT_R32G32_SINT:
			return DXGI_FORMAT_R32G32_TYPELESS;

		case DXGI_FORMAT_R32G32B32_FLOAT:
		case DXGI_FORMAT_R32G32B32_UINT:
		case DXGI_FORMAT_R32G32B32_SINT:
			return DXGI_FORMAT_R32G32B32_TYPELESS;

		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_UINT:
		case DXGI_FORMAT_R32G32B32A32_SINT:
			return DXGI_FORMAT_R32G32B32A32_TYPELESS;

		default:
			return typefulFormat;
		}
	}

	//-- tries to convert from depth format to shader resource format.
	//-- If format is not depth, do nothing.
	DXGI_FORMAT fromDepthToShaderResource(DXGI_FORMAT iFormat)
	{
		switch (iFormat)
		{
		case DXGI_FORMAT_D16_UNORM:			return DXGI_FORMAT_R16_FLOAT;
		case DXGI_FORMAT_D32_FLOAT:			return DXGI_FORMAT_R32_FLOAT;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:	return DXGI_FORMAT_UNKNOWN;

		default:
			return iFormat;
		}
	}
}
//--------------------------------------------------------------------------------------------------
// end unnamed namespace.


namespace brUGE
{
namespace render
{
	
	//------------------------------------------
	DXTexture::DXTexture(const Desc& desc) : ITexture(desc)
	{
			
	}
	
	//------------------------------------------
	DXTexture::~DXTexture()
	{

	}

	//------------------------------------------
	bool DXTexture::init(const ITexture::Data* data, uint size)
	{
		bool success = true;
		UINT bindFlags = getDXBindFlags(m_desc.bindFalgs);

		switch (m_desc.texType)
		{
		case TYPE_1D:		success &= _createTex1D(bindFlags, data, size);		    break;
		case TYPE_1D_ARRAY: success &= _createTex1D(bindFlags, data, size);		    break;
		case TYPE_2D:		success &= _createTex2D(bindFlags, data, size, false);  break;
		case TYPE_2D_ARRAY:	success &= _createTex2D(bindFlags, data, size, false);  break;
		case TYPE_CUBE_MAP:	success &= _createTex2D(bindFlags, data, size, true);	break;
		case TYPE_3D:		success &= _createTex3D(bindFlags, data, size);			break;
		default:
			return false;
		}

		if (!success) return false;

		if (bindFlags & D3D10_BIND_RENDER_TARGET)	success &= _createRTView();
		if (bindFlags & D3D10_BIND_SHADER_RESOURCE) success &= _createSRView();
		if (bindFlags & D3D10_BIND_DEPTH_STENCIL)	success &= _createDSView();

		return success;
	}

	//------------------------------------------
	bool DXTexture::init(const ComPtr<ID3D10Texture2D>& tex2D)
	{
		assert(m_desc.texType == TYPE_2D);
		
		//-- just copy already created texture.
		m_tex2D = tex2D;

		//-- set the base texture we'll use in the render system.
		HRESULT hr = m_tex2D->QueryInterface(IID_ID3D10Resource, (void **)&m_tex);
		if (FAILED(hr) || dxDevice().hasError())
		{
			ERROR_MSG("Failed to create 2D texture.\n Desc: " + dxDevice().getErrorDesc());
			return false;
		}

		bool success   = true;
		UINT bindFlags = getDXBindFlags(m_desc.bindFalgs);

		if (bindFlags & D3D10_BIND_RENDER_TARGET)	success &= _createRTView();
		if (bindFlags & D3D10_BIND_SHADER_RESOURCE) success &= _createSRView();
		if (bindFlags & D3D10_BIND_DEPTH_STENCIL)	success &= _createDSView();

		return success;
	}
	
	//------------------------------------------
	void DXTexture::doGenerateMipmaps()
	{
		if (m_SRView)
			dxDevice()->GenerateMips(m_SRView.get());
	}
	
	//------------------------------------------
	bool DXTexture::_createTex1D(UINT bindFlags, const Data* data, uint size)
	{
		D3D10_TEXTURE1D_DESC dxDesc;
		dxDesc.Width			= m_desc.width;
		dxDesc.ArraySize		= m_desc.arraySize;
		dxDesc.BindFlags		= bindFlags;
		dxDesc.CPUAccessFlags	= 0; // CPU access is not required.
		dxDesc.Format			= fromTypefulToTypeless(dxTexFormat[m_desc.format]);
		dxDesc.MipLevels		= m_desc.mipLevels;
		dxDesc.Usage			= D3D10_USAGE_DEFAULT; // default usage?
		dxDesc.MiscFlags		= (m_desc.mipLevels == 1) ? 0 : D3D10_RESOURCE_MISC_GENERATE_MIPS;
		
		std::vector<D3D10_SUBRESOURCE_DATA> dxData;
		if (size > 0)
		{
			dxData.resize(size);
			for (uint i = 0; i < size; ++i)
			{
				dxData[i].pSysMem			= data[i].mem;
				dxData[i].SysMemPitch		= 0;
				dxData[i].SysMemSlicePitch	= 0;
			}
		}

		HRESULT hr = dxDevice()->CreateTexture1D(&dxDesc, (size) ? &dxData[0] : NULL, &m_tex1D);
		if (FAILED(hr) || dxDevice().hasError())
		{
			ERROR_MSG("Failed to create 1D texture.\nDesc: " + dxDevice().getErrorDesc());
			return false;
		}

		// set the base texture we'll use in the render system.
		hr = m_tex1D->QueryInterface(IID_ID3D10Resource, (void **)&m_tex);
		if (FAILED(hr) || dxDevice().hasError())
		{
			ERROR_MSG("Failed to create 1D texture.\n Desc: " + dxDevice().getErrorDesc());
			return false;
		}

		return true;
	}
	
	//------------------------------------------
	bool DXTexture::_createTex2D(UINT bindFlags, const Data* data, uint size, bool cubeMap)
	{
		D3D10_TEXTURE2D_DESC dxDesc;
		dxDesc.Width				= m_desc.width;
		dxDesc.Height				= m_desc.height;
		dxDesc.ArraySize			= m_desc.arraySize;
		dxDesc.BindFlags			= bindFlags;
		dxDesc.CPUAccessFlags		= 0; // CPU access is not required.
		dxDesc.Format				= fromTypefulToTypeless(dxTexFormat[m_desc.format]);
		dxDesc.MipLevels			= m_desc.mipLevels;
		dxDesc.Usage				= D3D10_USAGE_DEFAULT; // default usage?
		dxDesc.MiscFlags			= (m_desc.mipLevels == 1) ? 0 : D3D10_RESOURCE_MISC_GENERATE_MIPS;
		dxDesc.SampleDesc.Count		= 1;
		dxDesc.SampleDesc.Quality	= 0;

		if (cubeMap)
		{
			dxDesc.MiscFlags		|= D3D10_RESOURCE_MISC_TEXTURECUBE;
			dxDesc.ArraySize		= 6;
		}
		
		std::vector<D3D10_SUBRESOURCE_DATA> dxData;
		if (size > 0)
		{
			dxData.resize(size);
			for (uint i = 0; i < size; ++i)
			{
				dxData[i].pSysMem			= data[i].mem;
				dxData[i].SysMemPitch		= data[i].memPitch;
				dxData[i].SysMemSlicePitch	= 0;
			}
		}
				
		HRESULT hr = dxDevice()->CreateTexture2D(&dxDesc, (size) ? &dxData[0] : NULL, &m_tex2D);
		if (FAILED(hr) || dxDevice().hasError())
		{
			ERROR_MSG("Failed to create 2D texture.\nDesc: " + dxDevice().getErrorDesc());
			return false;
		}

		// set the base texture we'll use in the render system
		hr = m_tex2D->QueryInterface(IID_ID3D10Resource, (void **)&m_tex);
		if (FAILED(hr) || dxDevice().hasError())
		{
			ERROR_MSG("Failed to create 2D texture.\nDesc: " + dxDevice().getErrorDesc());
			return false;
		}

		return true;
	}
	
	//------------------------------------------
	bool DXTexture::_createTex3D(UINT bindFlags, const Data* data, uint size)
	{
		D3D10_TEXTURE3D_DESC dxDesc;
		dxDesc.Width			= m_desc.width;
		dxDesc.Height			= m_desc.height;
		dxDesc.Depth			= m_desc.depth;
		dxDesc.BindFlags		= bindFlags;
		dxDesc.CPUAccessFlags	= 0; // CPU access is not required.
		dxDesc.Format			= fromTypefulToTypeless(dxTexFormat[m_desc.format]);
		dxDesc.MipLevels		= m_desc.mipLevels;
		dxDesc.Usage			= D3D10_USAGE_DEFAULT; // default usage?
		dxDesc.MiscFlags		= (m_desc.mipLevels == 1) ? 0 : D3D10_RESOURCE_MISC_GENERATE_MIPS;
		
		std::vector<D3D10_SUBRESOURCE_DATA> dxData;
		if (size > 0)
		{
			dxData.resize(size);
			for (uint i = 0; i < size; ++i)
			{
				dxData[i].pSysMem			= data[i].mem;
				dxData[i].SysMemPitch		= data[i].memPitch;
				dxData[i].SysMemSlicePitch	= data[i].memSlicePitch;
			}
		}

		HRESULT hr = dxDevice()->CreateTexture3D(&dxDesc, (size) ? &dxData[0] : NULL, &m_tex3D);
		if (FAILED(hr) || dxDevice().hasError())
		{
			ERROR_MSG("Failed to create 3D texture.\nDesc: " + dxDevice().getErrorDesc());
			return false;
		}

		// set the base texture we'll use in the render system
		hr = m_tex3D->QueryInterface(IID_ID3D10Resource, (void **)&m_tex);
		if (FAILED(hr) || dxDevice().hasError())
		{
			ERROR_MSG("Failed to create 3D texture.\nDesc: " + dxDevice().getErrorDesc());
			return false;
		}

		return true;
	}
	
	//------------------------------------------
	bool DXTexture::_createSRView()
	{
		D3D10_SHADER_RESOURCE_VIEW_DESC dxDesc;
		dxDesc.Format = fromDepthToShaderResource(dxTexFormat[m_desc.format]);

		switch (m_desc.texType)
		{
		case ITexture::TYPE_1D:
			{
				D3D10_TEXTURE1D_DESC desc;
				m_tex1D->GetDesc(&desc);

				dxDesc.ViewDimension					= D3D10_SRV_DIMENSION_TEXTURE1D;
				dxDesc.Texture1D.MostDetailedMip		= 0;
				dxDesc.Texture1D.MipLevels				= desc.MipLevels;
			}
			break;
		case ITexture::TYPE_1D_ARRAY:
			{
				D3D10_TEXTURE1D_DESC desc;
				m_tex1D->GetDesc(&desc);

				dxDesc.ViewDimension					= D3D10_SRV_DIMENSION_TEXTURE1DARRAY;
				dxDesc.Texture1DArray.MostDetailedMip	= 0;
				dxDesc.Texture1DArray.MipLevels			= desc.MipLevels;
				dxDesc.Texture1DArray.FirstArraySlice	= 0;
				dxDesc.Texture1DArray.ArraySize			= desc.ArraySize;
			}
			break;
		case ITexture::TYPE_2D:
			{
				D3D10_TEXTURE2D_DESC desc;
				m_tex2D->GetDesc(&desc);

				dxDesc.ViewDimension					= D3D10_SRV_DIMENSION_TEXTURE2D;
				dxDesc.Texture2D.MostDetailedMip		= 0;
				dxDesc.Texture2D.MipLevels				= desc.MipLevels;
			}
			break;
		case ITexture::TYPE_2D_ARRAY:
			{
				D3D10_TEXTURE2D_DESC desc;
				m_tex2D->GetDesc(&desc);

				dxDesc.ViewDimension					= D3D10_SRV_DIMENSION_TEXTURE2DARRAY;
				dxDesc.Texture2DArray.MostDetailedMip	= 0;
				dxDesc.Texture2DArray.MipLevels			= desc.MipLevels;
				dxDesc.Texture2DArray.FirstArraySlice	= 0;
				dxDesc.Texture2DArray.ArraySize			= desc.ArraySize;
			}
			break;
		case ITexture::TYPE_CUBE_MAP:
			{
				D3D10_TEXTURE2D_DESC desc;
				m_tex2D->GetDesc(&desc);

				dxDesc.ViewDimension					= D3D10_SRV_DIMENSION_TEXTURECUBE;
				dxDesc.Texture2D.MostDetailedMip		= 0;
				dxDesc.Texture2D.MipLevels				= desc.MipLevels;
			}
			break;
		case ITexture::TYPE_3D:
			{
				D3D10_TEXTURE3D_DESC desc;
				m_tex3D->GetDesc(&desc);

				dxDesc.ViewDimension					= D3D10_SRV_DIMENSION_TEXTURE3D;
				dxDesc.Texture3D.MostDetailedMip		= 0;
				dxDesc.Texture3D.MipLevels				= desc.MipLevels;
			}
			break;
		}

		HRESULT hr = dxDevice()->CreateShaderResourceView(m_tex, &dxDesc, &m_SRView);
		if (FAILED(hr) || dxDevice().hasError())
		{
			ERROR_MSG("Failed to create shader resource view.\n Desc: " + dxDevice().getErrorDesc());
			return false;
		}

		return true;
	}

	//------------------------------------------
	bool DXTexture::_createRTView()
	{
		D3D10_RENDER_TARGET_VIEW_DESC dxDesc;
		dxDesc.Format = fromDepthToShaderResource(dxTexFormat[m_desc.format]);

		switch (m_desc.texType)
		{
		case ITexture::TYPE_1D:
			{
				dxDesc.ViewDimension					= D3D10_RTV_DIMENSION_TEXTURE1D;
				dxDesc.Texture1D.MipSlice				= 0;
			}
			break;
		case ITexture::TYPE_1D_ARRAY:
			{
				D3D10_TEXTURE1D_DESC desc;
				m_tex1D->GetDesc(&desc);

				dxDesc.ViewDimension					= D3D10_RTV_DIMENSION_TEXTURE1DARRAY;
				dxDesc.Texture1DArray.MipSlice			= 0;
				dxDesc.Texture1DArray.FirstArraySlice	= 0;
				dxDesc.Texture1DArray.ArraySize			= desc.ArraySize;
			}
			break;
		case ITexture::TYPE_2D:
			{
				dxDesc.ViewDimension					= D3D10_RTV_DIMENSION_TEXTURE2D;
				dxDesc.Texture2D.MipSlice				= 0;
			}
			break;
		case ITexture::TYPE_2D_ARRAY:
			{
				D3D10_TEXTURE2D_DESC desc;
				m_tex2D->GetDesc(&desc);

				dxDesc.ViewDimension					= D3D10_RTV_DIMENSION_TEXTURE2DARRAY;
				dxDesc.Texture2DArray.MipSlice			= 0;
				dxDesc.Texture2DArray.FirstArraySlice	= 0;
				dxDesc.Texture2DArray.ArraySize			= desc.ArraySize;
			}
			break;
		case ITexture::TYPE_3D:
			{
				D3D10_TEXTURE3D_DESC desc;
				m_tex3D->GetDesc(&desc);

				dxDesc.ViewDimension					= D3D10_RTV_DIMENSION_TEXTURE3D;
				dxDesc.Texture3D.FirstWSlice			= 0;
				dxDesc.Texture3D.MipSlice				= 0;
				dxDesc.Texture3D.WSize					= desc.Depth;
			}
			break;
		}

		HRESULT hr = dxDevice()->CreateRenderTargetView(m_tex, &dxDesc, &m_RTView);
		if (FAILED(hr) || dxDevice().hasError())
		{
			ERROR_MSG("Failed to create render target view.\n Desc: " + dxDevice().getErrorDesc());
			return false;
		}

		return true;
	}
	
	//------------------------------------------
	bool DXTexture::_createDSView()
	{
		D3D10_DEPTH_STENCIL_VIEW_DESC dxDesc;
		dxDesc.Format = dxTexFormat[m_desc.format];
		
		switch (m_desc.texType)
		{
		case ITexture::TYPE_1D:
			{
				dxDesc.ViewDimension					= D3D10_DSV_DIMENSION_TEXTURE1D;
				dxDesc.Texture1D.MipSlice				= 0;
			}
			break;
		case ITexture::TYPE_1D_ARRAY:
			{
				D3D10_TEXTURE1D_DESC desc;
				m_tex1D->GetDesc(&desc);

				dxDesc.ViewDimension					= D3D10_DSV_DIMENSION_TEXTURE1DARRAY;
				dxDesc.Texture1DArray.MipSlice			= 0;
				dxDesc.Texture1DArray.FirstArraySlice	= 0;
				dxDesc.Texture1DArray.ArraySize			= desc.ArraySize;
			}
			break;
		case ITexture::TYPE_2D:
			{
				dxDesc.ViewDimension					= D3D10_DSV_DIMENSION_TEXTURE2D;
				dxDesc.Texture2D.MipSlice				= 0;
			}
			break;
		case ITexture::TYPE_2D_ARRAY:
			{
				D3D10_TEXTURE2D_DESC desc;
				m_tex2D->GetDesc(&desc);

				dxDesc.ViewDimension					= D3D10_DSV_DIMENSION_TEXTURE2DARRAY;
				dxDesc.Texture2DArray.MipSlice			= 0;
				dxDesc.Texture2DArray.FirstArraySlice	= 0;
				dxDesc.Texture2DArray.ArraySize			= desc.ArraySize;
			}
			break;
		}

		HRESULT hr = dxDevice()->CreateDepthStencilView(m_tex, &dxDesc, &m_DSView);
		if (FAILED(hr) || dxDevice().hasError())
		{
			ERROR_MSG("Failed to create depth-stencil view.\nDesc: " + dxDevice().getErrorDesc());
			return false;
		}

		return true;
	}

} // render
} // brUGE