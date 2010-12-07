#pragma once

#include "render_common.h"

namespace brUGE
{
namespace render
{

	//----------------------------------------------------------------------------------------------
	class ITexture : public utils::RefCount
	{
	public:

		//-- texture format.
		enum EFormat
		{
			FORMAT_NONE	= 0,

			// Unsigned formats
			FORMAT_R8,
			FORMAT_RG8,
			FORMAT_RGBA8,
			FORMAT_RGBA8_sRGB,

			FORMAT_R16,
			FORMAT_RG16,
			FORMAT_RGBA16,

			// Signed formats
			FORMAT_R8S,
			FORMAT_RG8S,
			FORMAT_RGBA8S,

			FORMAT_R16S,
			FORMAT_RG16S,
			FORMAT_RGBA16S,

			// Float formats
			FORMAT_R16F,
			FORMAT_RG16F,
			FORMAT_RGBA16F,

			FORMAT_R32F,
			FORMAT_RG32F,
			FORMAT_RGB32F,
			FORMAT_RGBA32F,

			// Signed integer formats
			FORMAT_R16I,
			FORMAT_RG16I,
			FORMAT_RGB16I,
			FORMAT_RGBA16I,

			FORMAT_R32I,
			FORMAT_RG32I,
			FORMAT_RGBA32I,

			// Unsigned integer formats
			FORMAT_R16UI,
			FORMAT_RG16UI,
			FORMAT_RGBA16UI,

			FORMAT_R32UI,
			FORMAT_RG32UI,
			FORMAT_RGB32UI,
			FORMAT_RGBA32UI,

			// Packed formats
			FORMAT_RGB9E5,
			FORMAT_RG11B10F,
			FORMAT_RGB565,
			FORMAT_RGB10A2,

			// Depth formats
			FORMAT_D16,
			FORMAT_D24,
			FORMAT_D24S8,
			FORMAT_D32F,

			// Compressed formats
			// DX10 - GL 3.*
			// BC1	  DXT1				
			// BC2	  DXT3
			// BC3	  DXT5
			// BC4	  RGTC1
			// BC5	  RGTC2
			FORMAT_BC1,
			FORMAT_BC2,
			FORMAT_BC3,
			FORMAT_BC4,
			FORMAT_BC5,
		};
		
		//-- mode of usage texture in the pipeline.
		enum EBindFlags
		{
			BIND_SHADER_RESOURCE = 1 << 0,
			BIND_RENDER_TARGET	 = 1 << 1,
			BIND_DEPTH_STENCIL	 = 1 << 2
		};	

		//-- texture type.
		enum EType
		{
			TYPE_1D = 0,
			TYPE_1D_ARRAY,
			TYPE_2D,
			TYPE_2D_ARRAY,
			TYPE_CUBE_MAP,
			TYPE_3D
		};
		
		//--
		struct Data
		{
			const void* mem;
			uint memPitch;
			uint memSlicePitch;
		};
		
		//
		// TODO: Возможно еще требует обдумывания.
		//
		// Note: Если mipLevels == 1 это значит нам не надо MipMap-уровни.
		//		 Если mipLevels == 0 - значит генерируем весь набор вплоть до 1x1.
		// Только для компрессированных текстур:
		//		 Если mipLevels > 1  - генерируем столько MipMap-уровней сколько указано.
		struct Desc
		{
			Desc() : width(1), height(1), depth(1), arraySize(1), mipLevels(1) {}
			
			//-- describe multi-sampling properties.
			struct SampleDesc
			{
				SampleDesc() : count(1), quality(0) {}

				uint count;
				uint quality;
			};
					
			uint width;
			uint height;
			uint depth;
			uint arraySize;
			uint mipLevels;
			uint bindFalgs;
			EType texType;
			EFormat format;
			SampleDesc sample;
		};

	public:
		const Desc& getDesc() const  { return m_desc; }
		EType getType() const { return m_desc.texType; }
		void  generateMipmaps() { doGenerateMipmaps(); }
			
	protected:
		ITexture(const Desc& desc) : m_desc(desc) {}
		virtual ~ITexture() {}

		virtual void doGenerateMipmaps() = 0;
	
	protected:
		Desc m_desc;
	};

} // render
} // brUGE
