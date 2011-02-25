#pragma once

#include "prerequisites.h"
#include "utils/LogManager.h"

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#	define NOMINMAX
#endif
#include <windows.h>

namespace brUGE
{
	using utils::Ptr;

namespace render
{
	//-- predeclaration.
	class Camera;
	class DebugDrawer;
	class RenderSystem;

	class IRenderDevice;
	class IBuffer;
	class IShader;
	class ITexture;
	struct VertexDesc;
	struct ShaderMacro;

//-- ToDo: delete.
#define INVALID_ID -1

	typedef int VertexLayoutID;
	typedef int DepthStencilStateID;
	typedef int RasterizerStateID;
	typedef int BlendStateID;
	typedef int SamplerStateID;
	
	//-- render API type.
	enum ERenderAPIType
	{
		RENDER_API_GL3,
		RENDER_API_DX10,
	};

	//-- multi-sampling info.
	struct MultiSampling
	{
		MultiSampling() : m_count(1), m_quality(0) { } 
		MultiSampling(uint count, uint quality) : m_count(count), m_quality(quality) { } 

		uint m_count;
		uint m_quality;
	};

	//-- video mode info.
	struct VideoMode
	{
		uint			width;
		uint			height;
		uint			bpp;
		uint			depth;
		uint			stencil;
		uint			frequancy;
		MultiSampling	multiSampling;
		bool			vSync;
		bool			fullScreen;
	};

/*
	//-- back buffer descriptor.
	struct BackBufferDesc
	{
		bool			  hasColor;
		bool			  hasDepthStencil;
		ITexture::EFormat colorFormat;
		ITexture::EFormat depthStencilFormat;	
	};
*/

	//------------------------------------------
	struct Projection
	{
		float	fov;
		float	nearDist;
		float	farDist;			
	};
	
	//------------------------------------------
	struct ScreenResolution
	{
		float width;
		float height;
	};

} // render
} // brUGE
