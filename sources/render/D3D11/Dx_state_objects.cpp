#include "DxRenderDevice.hpp"
#include "DxDevice.hpp"
#include "render/state_objects.h"

// start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	//------------------------------------------
	UINT8 dxConvertWriteMask(const bool vec[])
	{
		UINT8 mask = 0;
		if (vec[0]) mask |= D3D11_COLOR_WRITE_ENABLE_RED;
		if (vec[1]) mask |= D3D11_COLOR_WRITE_ENABLE_GREEN;
		if (vec[2]) mask |= D3D11_COLOR_WRITE_ENABLE_BLUE;
		if (vec[3]) mask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;
		return mask;
	}

	//--
	const D3D11_FILL_MODE dxFillMode[] = {D3D11_FILL_WIREFRAME, D3D11_FILL_SOLID};

	//--
	const D3D11_CULL_MODE dxCullMode[] = {D3D11_CULL_NONE, D3D11_CULL_FRONT, D3D11_CULL_BACK};

	//--
	const D3D11_STENCIL_OP dxStencilOp[] = 
	{
		D3D11_STENCIL_OP_KEEP,
		D3D11_STENCIL_OP_ZERO,
		D3D11_STENCIL_OP_REPLACE,
		D3D11_STENCIL_OP_INCR,
		D3D11_STENCIL_OP_DECR,
		D3D11_STENCIL_OP_INVERT,
		D3D11_STENCIL_OP_INCR_SAT,
		D3D11_STENCIL_OP_DECR_SAT
	};
	
	//--
	const D3D11_COMPARISON_FUNC dxComparisonFunc[] = 
	{
		D3D11_COMPARISON_NEVER,
		D3D11_COMPARISON_ALWAYS,
		D3D11_COMPARISON_EQUAL,
		D3D11_COMPARISON_NOT_EQUAL,
		D3D11_COMPARISON_LESS,
		D3D11_COMPARISON_LESS_EQUAL,
		D3D11_COMPARISON_GREATER,
		D3D11_COMPARISON_GREATER_EQUAL
	};
	
	//--
	const D3D11_BLEND dxBlend[] = 
	{
		D3D11_BLEND_ZERO,
		D3D11_BLEND_ONE,
		D3D11_BLEND_SRC_COLOR,
		D3D11_BLEND_INV_SRC_COLOR,
		D3D11_BLEND_SRC_ALPHA,
		D3D11_BLEND_INV_SRC_ALPHA,
		D3D11_BLEND_DEST_ALPHA,
		D3D11_BLEND_INV_DEST_ALPHA,
		D3D11_BLEND_DEST_COLOR,
		D3D11_BLEND_INV_DEST_COLOR
	};

	//--
	const D3D11_BLEND_OP dxBlendOp[] = 
	{
		D3D11_BLEND_OP_ADD,
		D3D11_BLEND_OP_SUBTRACT,
		D3D11_BLEND_OP_REV_SUBTRACT,
		D3D11_BLEND_OP_MIN,
		D3D11_BLEND_OP_MAX
	};

	//--
	const D3D11_FILTER dxTexFilter[] = 
	{
		D3D11_FILTER_MIN_MAG_MIP_POINT,
		D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_FILTER_ANISOTROPIC,
		D3D11_FILTER_ANISOTROPIC
	};

	//--
	const D3D11_TEXTURE_ADDRESS_MODE dxTexAddressMode[] = 
	{
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_MIRROR,
		D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_BORDER,
		D3D11_TEXTURE_ADDRESS_MIRROR_ONCE
	};
	
	//--
	const D3D11_COMPARISON_FUNC dxTexComparisonFunc[] = 
	{
		D3D11_COMPARISON_NEVER,
		D3D11_COMPARISON_ALWAYS,
		D3D11_COMPARISON_EQUAL,
		D3D11_COMPARISON_NOT_EQUAL,
		D3D11_COMPARISON_LESS,
		D3D11_COMPARISON_LESS_EQUAL,
		D3D11_COMPARISON_GREATER,
		D3D11_COMPARISON_GREATER_EQUAL
	};
}
//--------------------------------------------------------------------------------------------------
// end unnamed namespace.

namespace brUGE
{
namespace render
{
	
	//------------------------------------------	
	bool DXRasterizerState::create(const RasterizerStateDesc& desc)
	{
		D3D11_RASTERIZER_DESC dxDesc;
		
		dxDesc.FillMode = dxFillMode[desc.fillMode];
		dxDesc.CullMode = dxCullMode[desc.cullMode];
		dxDesc.FrontCounterClockwise = desc.frontCounterClockwise;
		dxDesc.ScissorEnable		 = desc.scissorEnable;
		dxDesc.MultisampleEnable	 = desc.multisampleEnable;
		dxDesc.DepthBias			 = desc.depthBiasFactor;
		dxDesc.DepthBiasClamp		 = 0.0f;
		dxDesc.SlopeScaledDepthBias	 = desc.depthBiasUnits;
		dxDesc.AntialiasedLineEnable = desc.antialiasedLineEnable;
		
		// ToDo: to be completed.

		// while defaults.
		dxDesc.DepthClipEnable		 = FALSE;
		
		HRESULT hr = dxDevice()->CreateRasterizerState(&dxDesc, &state);

		if (FAILED(hr) || dxDevice().hasError())
		{
			ERROR_MSG("Can't create rasterizer state.\nDesc: " + dxDevice().getErrorDesc());
			return false;
		}
		return true;
	}
	
	//------------------------------------------
	bool DXDepthStencilState::create(const DepthStencilStateDesc& desc)
	{
		D3D11_DEPTH_STENCIL_DESC dxDesc;
		
		dxDesc.DepthEnable = desc.depthEnable;
		if (desc.depthWriteMask) dxDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		else					 dxDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

		dxDesc.DepthFunc = dxComparisonFunc[desc.depthFunc];

		// настройки буфера стенсиля.	
		dxDesc.StencilEnable    = desc.stencilEnable;
		dxDesc.StencilReadMask  = desc.stencilReadMask;
		dxDesc.StencilWriteMask = desc.stencilWriteMask;
		
		D3D11_DEPTH_STENCILOP_DESC& front =	dxDesc.FrontFace;
		front.StencilFailOp		 = dxStencilOp[desc.frontFace.stencilFailOp];
		front.StencilDepthFailOp = dxStencilOp[desc.frontFace.stencilPassDepthFailOp];
		front.StencilPassOp		 = dxStencilOp[desc.frontFace.stencilPassDepthPassOp];
		front.StencilFunc		 = dxComparisonFunc[desc.frontFace.stencilFunc];

		D3D11_DEPTH_STENCILOP_DESC& back =	dxDesc.BackFace;
		back.StencilFailOp		 = dxStencilOp[desc.backFace.stencilFailOp];
		back.StencilDepthFailOp	 = dxStencilOp[desc.backFace.stencilPassDepthFailOp];
		back.StencilPassOp		 = dxStencilOp[desc.backFace.stencilPassDepthPassOp];
		back.StencilFunc		 = dxComparisonFunc[desc.backFace.stencilFunc];
		
		HRESULT hr = dxDevice()->CreateDepthStencilState(&dxDesc, &state);

		if (FAILED(hr) || dxDevice().hasError())
		{
			ERROR_MSG("Can't create depth and stencil state.\nDesc: " + dxDevice().getErrorDesc());
			return false;
		}
		return true;
	}
	
	//------------------------------------------
	bool DXBlendState::create(const BlendStateDesc& desc)
	{
		D3D11_BLEND_DESC dxDesc;

		dxDesc.AlphaToCoverageEnable = false;
		dxDesc.IndependentBlendEnable = false;

		for (uint i = 0; i < IRenderDevice::MAX_MRTS; ++i)
		{
			auto& rtDesc = dxDesc.RenderTarget[i];

			rtDesc.SrcBlend					= dxBlend[desc.srcBlend];
			rtDesc.DestBlend				= dxBlend[desc.destBlend];
			rtDesc.BlendOp					= dxBlendOp[desc.blendOp];

			rtDesc.SrcBlendAlpha			= dxBlend[desc.srcBlendAlpha];
			rtDesc.DestBlendAlpha			= dxBlend[desc.destBlendAlpha];
			rtDesc.BlendOpAlpha				= dxBlendOp[desc.blendAlphaOp];

			rtDesc.BlendEnable				= desc.blendEnable[i];
			rtDesc.RenderTargetWriteMask	= dxConvertWriteMask(desc.colorWriteMask[i]);
		}
		
		HRESULT hr = dxDevice()->CreateBlendState(&dxDesc, &state);

		if (FAILED(hr) || dxDevice().hasError())
		{
			ERROR_MSG("Can't create blend state.\nDesc: " + dxDevice().getErrorDesc());		
			return false;
		}
		return true;
	}
	
	//------------------------------------------
	bool DXSamplerState::create(const SamplerStateDesc& desc)
	{
		D3D11_SAMPLER_DESC dxDesc;

		dxDesc.AddressU	= dxTexAddressMode[desc.wrapS];
		dxDesc.AddressV	= dxTexAddressMode[desc.wrapT];
		dxDesc.AddressW	= dxTexAddressMode[desc.wrapR];
		
		dxDesc.BorderColor[0] = desc.borderColour.r;
		dxDesc.BorderColor[1] = desc.borderColour.g;
		dxDesc.BorderColor[2] = desc.borderColour.b;
		dxDesc.BorderColor[3] = desc.borderColour.a;

		dxDesc.ComparisonFunc = dxTexComparisonFunc[desc.compareFunc];
		dxDesc.Filter		  = dxTexFilter[desc.minMagFilter];
		dxDesc.MaxAnisotropy  = desc.maxAnisotropy;
		dxDesc.MaxLOD		  = desc.maxLoad;
		dxDesc.MinLOD		  = desc.minLoad;
		dxDesc.MipLODBias	  = desc.loadBias;

		HRESULT hr = dxDevice()->CreateSamplerState(&dxDesc, &state);

		if (FAILED(hr) || dxDevice().hasError())
		{
			ERROR_MSG("Can't create sampler state.\n Desc: " + dxDevice().getErrorDesc());
			return false;
		}
		return true;
	}

} // render
} // brUGE