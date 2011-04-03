#pragma once

#include "IRenderDevice.h"

namespace brUGE
{
namespace render
{

	//----------------------------------------------------------------------------------------------
	struct RasterizerStateDesc
	{
		//-- culling mode.
		enum ECullMode
		{
			CULL_NOTHING,
			CULL_FRONT,
			CULL_BACK
		};

		//-- filling mode.
		enum EFillMode
		{
			FILL_WIREFRAME,
			FILL_SOLID
		};

		RasterizerStateDesc()
			:	fillMode(FILL_SOLID),
				cullMode(CULL_NOTHING),
				frontCounterClockwise(true),
				depthBiasFactor(0.0f),
				depthBiasUnits(0.0f),
				scissorEnable(false),
				multisampleEnable(false),
				antialiasedLineEnable(false) {}

		EFillMode	fillMode;
		ECullMode	cullMode;
		bool		frontCounterClockwise;
		
		//-- represents depth bias and slope scaled depth bias respectively.
		float		depthBiasFactor;
		float		depthBiasUnits;

		bool		scissorEnable;
		bool		multisampleEnable;
		bool		antialiasedLineEnable;
	};


	//----------------------------------------------------------------------------------------------
	struct DepthStencilStateDesc
	{
		//-- comparison function for stencil and depth buffer.
		enum ECompareFunc
		{
			COMPARE_FUNC_NEVER = 0,
			COMPARE_FUNC_ALWAYS,
			COMPARE_FUNC_EQUAL,
			COMPARE_FUNC_NOT_EQUAL,
			COMPARE_FUNC_LESS,
			COMPARE_FUNC_LESS_EQUAL,
			COMPARE_FUNC_GREATER,
			COMPARE_FUNC_GREATER_EQUAL
		};

		//-- stencil operations.
		enum EStencilOp
		{
			STENCIL_OP_KEEP	= 0,
			STENCIL_OP_ZERO,
			STENCIL_OP_REPLACE,
			STENCIL_OP_INCR,
			STENCIL_OP_DECR,
			STENCIL_OP_INVERT,
			STENCIL_OP_INCR_WRAP,
			STENCIL_OP_DECR_WRAP
		};
		
		//--
		struct Face
		{
			Face()
				:	stencilFailOp(STENCIL_OP_KEEP),
					stencilPassDepthFailOp(STENCIL_OP_KEEP),
					stencilPassDepthPassOp(STENCIL_OP_KEEP),
					stencilFunc(COMPARE_FUNC_ALWAYS) {}

			EStencilOp	 stencilFailOp;
			EStencilOp	 stencilPassDepthFailOp;
			EStencilOp	 stencilPassDepthPassOp;
			ECompareFunc stencilFunc;
		};

		DepthStencilStateDesc()
			:	depthEnable(true),
				depthWriteMask(true),
				depthFunc(COMPARE_FUNC_LESS),
				stencilEnable(false),
				stencilReadMask(0xff),
				stencilWriteMask(0xff) {}

		bool		 depthEnable;
		bool		 depthWriteMask;
		ECompareFunc depthFunc;

		bool		 stencilEnable;
		byte		 stencilReadMask;
		byte		 stencilWriteMask;
		Face		 frontFace;
		Face		 backFace;
	};
	

	//----------------------------------------------------------------------------------------------
	struct BlendStateDesc
	{
		//-- define data of source and receiver.
		enum EBlendFactor
		{
			BLEND_FACTOR_ZERO = 0,
			BLEND_FACTOR_ONE,
			BLEND_FACTOR_SRC_COLOR,
			BLEND_FACTOR_INV_SRC_COLOR,
			BLEND_FACTOR_SRC_ALPHA,
			BLEND_FACTOR_INV_SRC_ALPHA,
			BLEND_FACTOR_DEST_ALPHA,
			BLEND_FACTOR_INV_DEST_ALPHA,
			BLEND_FACTOR_DEST_COLOR,
			BLEND_FACTOR_INV_DEST_COLOR

			// ToDo: to be completed.

		};

		//-- define blending operations for RGB- and(or) Alpha-component.
		enum EBlendOp
		{
			BLEND_OP_ADD = 0,
			BLEND_OP_SUBTRACT,
			BLEND_OP_REV_SUBTRACT,
			BLEND_OP_MIN,
			BLEND_OP_MAX
		};

		BlendStateDesc()
			:	srcBlend(BLEND_FACTOR_ONE),
				destBlend(BLEND_FACTOR_ZERO),
				blendOp(BLEND_OP_ADD),
				srcBlendAlpha(BLEND_FACTOR_ONE),
				destBlendAlpha(BLEND_FACTOR_ZERO),
				blendAlphaOp(BLEND_OP_ADD)
		{
			for (uint i = 0; i < IRenderDevice::MAX_MRTS; ++i)
			{
				blendEnable[i]		 = false;
				colorWriteMask[i][0] = true;
				colorWriteMask[i][1] = true;
				colorWriteMask[i][2] = true;
				colorWriteMask[i][3] = true;
			}
		}

		bool			blendEnable[IRenderDevice::MAX_MRTS];
		EBlendFactor	srcBlend;
		EBlendFactor	destBlend;
		EBlendOp		blendOp;
		EBlendFactor	srcBlendAlpha;
		EBlendFactor	destBlendAlpha;
		EBlendOp		blendAlphaOp;
		bool			colorWriteMask[IRenderDevice::MAX_MRTS][4];
	};


	//----------------------------------------------------------------------------------------------
	struct SamplerStateDesc
	{
		//-- types of texture filters.
		enum ETexFilter
		{
			FILTER_NEAREST,
			FILTER_BILINEAR,
			FILTER_TRILINEAR,
			FILTER_BILINEAR_ANISO,
			FILTER_TRILINEAR_ANISO
		};
		
		//-- texture data addressing types.
		enum ETexAddressMode
		{
			// ToDo: to be confirmed by OpenGL 3.*
			ADRESS_MODE_WRAP,
			ADRESS_MODE_MIRROR,
			ADRESS_MODE_CLAMP,
			ADRESS_MODE_BORDER,
			ADRESS_MODE_MIRROR_ONCE
		};
		
		//-- comparison functions.
		enum ETexCompareFunc
		{
			COMPARE_FUNC_LEQUAL,
			COMPARE_FUNC_GEQUAL,
			COMPARE_FUNC_LESS,
			COMPARE_FUNC_GREATER,
			COMPARE_FUNC_EQUAL,
			COMPARE_FUNC_NOTEQUAL,
			COMPARE_FUNC_ALWAYS,
			COMPARE_FUNC_NEVER
		};

		SamplerStateDesc()
			:	minMagFilter(FILTER_NEAREST),
				wrapS(ADRESS_MODE_CLAMP),
				wrapT(ADRESS_MODE_CLAMP),
				wrapR(ADRESS_MODE_CLAMP),
				loadBias(0.0f),
				maxAnisotropy(1),
				compareFunc(COMPARE_FUNC_NEVER),
				borderColour(Color(0.0f, 0.0f, 0.0f, 0.0f)),
				minLoad(0.0f),
				maxLoad(1000000.0f/*FLT_MAX*/) {} // TODO: продумать откуда взять константу FLT_MAX.

		ETexFilter		minMagFilter;
		ETexAddressMode	wrapS;
		ETexAddressMode	wrapT;
		ETexAddressMode	wrapR;
		float			loadBias;
		uint			maxAnisotropy;
		ETexCompareFunc	compareFunc;
		Color			borderColour;
		float			minLoad;
		float			maxLoad;
	};

} // render
} // brUGE
