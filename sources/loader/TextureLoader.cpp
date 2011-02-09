#include "loader/TextureLoader.h"
#include "utils/Data.hpp"
#include "render/render_system.hpp"
#include "render/IRenderDevice.h"

using namespace brUGE;
using namespace brUGE::math;
using namespace brUGE::render;
using namespace brUGE::utils;

// start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{

//-- to guaranty one byte aligned packing.
#pragma pack (push, 1)

	//----------------------------------------------------------------------------------------------
	struct DDSPixelFormat
	{
		uint32 m_size;
		uint32 m_flags;
		uint32 m_fourCC;
		uint32 m_bpp;
		uint32 m_RBitMask;
		uint32 m_GBitMask;
		uint32 m_BBitMask;
		uint32 m_ABitMask;
	};

	//----------------------------------------------------------------------------------------------
	struct DDSCaps
	{
		uint32 m_caps;
		uint32 m_caps2;
		uint32 m_caps3;
		uint32 m_caps4;
	};

	//----------------------------------------------------------------------------------------------
	struct DDSColorKey
	{
		uint32 m_lowVal;
		uint32 m_highVal;
	};

	//----------------------------------------------------------------------------------------------
	struct DDSHeader
	{
		uint32			m_size;
		uint32			m_flags;
		uint32			m_height;
		uint32			m_width;
		uint32			m_pitch;
		uint32			m_depth;
		uint32			m_mipMapCount;
		uint32			m_alphaBitDepth;
		uint32			m_reserved;
		uint32			m_surface;
		DDSColorKey		m_ckDescOverlay;
		DDSColorKey		m_ckDestBlt;
		DDSColorKey		m_ckSrcOverlay;
		DDSColorKey		m_ckSrcBlt;
		DDSPixelFormat	m_format;
		DDSCaps			m_caps;
		uint32			m_textureStage;
	};

#pragma	pack (pop)

	//----------------------------------------------------------------------------------------------
	enum EDDSInfo
	{
		//-- bit flags for header
		DDS_CAPS				= 0x00000001,
		DDS_HEIGHT				= 0x00000002,
		DDS_WIDTH				= 0x00000004,
		DDS_PITCH				= 0x00000008,
		DDS_PIXELFORMAT			= 0x00001000,
		DDS_MIPMAPCOUNT			= 0x00020000,
		DDS_LINEARSIZE			= 0x00080000,
		DDS_DEPTH				= 0x00800000,

		//-- flags for pixel formats
		DDS_ALPHA_PIXELS		= 0x00000001,
		DDS_ALPHA				= 0x00000002,
		DDS_FOURCC				= 0x00000004,
		DDS_RGB					= 0x00000040,
		DDS_RGBA				= 0x00000041,

		//-- flags for complex caps
		DDS_COMPLEX				= 0x00000008,
		DDS_TEXTURE				= 0x00001000,
		DDS_MIPMAP				= 0x00400000,

		//-- flags for cubemaps
		DDS_CUBEMAP				= 0x00000200,
		DDS_CUBEMAP_POSITIVEX	= 0x00000400,
		DDS_CUBEMAP_NEGATIVEX	= 0x00000800,
		DDS_CUBEMAP_POSITIVEY	= 0x00001000,
		DDS_CUBEMAP_NEGATIVEY	= 0x00002000,
		DDS_CUBEMAP_POSITIVEZ	= 0x00004000,
		DDS_CUBEMAP_NEGATIVEZ	= 0x00008000,
		DDS_VOLUME				= 0x00200000
	};

	const uint32 FOURCC_DXT1 = 0x31545844; //-- DXT1 or BC1
	const uint32 FOURCC_DXT3 = 0x33545844; //-- DXT3 or BC2
	const uint32 FOURCC_DXT5 = 0x35545844; //-- DXT5 or BC3
	const uint32 FOURCC_ATI1 = 0x31495441; //-- ATI1
	const uint32 FOURCC_ATI2 = 0x32495441; //-- ATI2 (AKA 3Dc)

	//----------------------------------------------------------------------------------------------
	struct SurfaceInfo
	{
		uint32 m_numBytes;
		uint32 m_rowBytes;
		uint32 m_numRows;
	};

	//----------------------------------------------------------------------------------------------
	SurfaceInfo surfaceInfo(uint32 width, uint32 height, ITexture::EFormat format)
	{
		assert(width != 0);
		assert(height != 0);

		SurfaceInfo sInfo = {0,0,0};

		//-- From the DXSDK docs:
		//--
		//--   When computing DXTn compressed sizes for non-square textures, the 
		//--   following formula should be used at each mipmap level:
		//--
		//--      max(1, width / 4) x max(1, height / 4) x 8(DXT1) or 16(DXT2-5)
		//--
		//--   The pitch for DXTn formats is different from what was returned in 
		//--   Microsoft DirectX 7.0. It now refers the pitch of a row of blocks. 
		//--   For example, if you have a width of 16, then you will have a pitch 
		//--   of four blocks (4*8 for DXT1, 4*16 for DXT2-5.)"

		if (
			format == ITexture::FORMAT_BC1 ||
			format == ITexture::FORMAT_BC2 ||
			format == ITexture::FORMAT_BC3
			)
		{
			uint numBlocksWide    = max<uint>(1, width / 4);
			uint numBlocksHight   = max<uint>(1, height / 4);
			uint numBytesPerBlock = (format == ITexture::FORMAT_BC1 ? 8 : 16);

			sInfo.m_rowBytes = numBlocksWide * numBytesPerBlock;
			sInfo.m_numRows	 = numBlocksHight;
		}
		else
		{
			// TODO:
			uint bpp = 32;
			sInfo.m_rowBytes = (width * bpp + 7 ) / 8; // round up to nearest byte
			sInfo.m_numRows	 = height;
		}

		sInfo.m_numBytes = sInfo.m_rowBytes * sInfo.m_numRows;
		
		return sInfo;
	}
}
//--------------------------------------------------------------------------------------------------
// end unnamed namespace.


namespace brUGE
{
	//------------------------------------------
	TextureLoader::TextureLoader() : m_totalSize(0)
	{

	}
	
	//------------------------------------------
	TextureLoader::~TextureLoader()
	{
	
	}
	
	//------------------------------------------
	bool TextureLoader::init()
	{
		//-- pass

		return true;
	}
	
	//------------------------------------------
	void TextureLoader::shutdown()
	{
		//-- pass
	}
	
	//------------------------------------------
	Ptr<ITexture> TextureLoader::loadTex2D(const ROData& data)
	{
		DDSHeader ddsDesc;
		char      filecode[4];

		//-- 1. verify the file is a true .dds file.
		if (!data.read(filecode) || strncmp(filecode, "DDS ", 4) != 0)
		{
			ERROR_MSG("This file is not a valid DDS image file.");
			return nullptr;
		}

		//-- 2. get the surface description.
		if (!data.read(ddsDesc))
		{
			ERROR_MSG("Can't read DDS description. Most likely this file is not a valid .dds file.");
			return nullptr;
		}

		//-- 3. get some basic info about texture...
		ITexture::Desc desc;
		desc.texType   = ITexture::TYPE_2D;
		desc.bindFalgs = ITexture::BIND_SHADER_RESOURCE;
		desc.width	   = ddsDesc.m_width;
		desc.height	   = ddsDesc.m_height;
		desc.mipLevels = (ddsDesc.m_flags & DDS_MIPMAPCOUNT) ? ddsDesc.m_mipMapCount : 1;

		//-- 4. get texture format.
		if(ddsDesc.m_format.m_flags & DDS_FOURCC)
		{
			switch(ddsDesc.m_format.m_fourCC)
			{
			case FOURCC_DXT1:	desc.format	= ITexture::FORMAT_BC1; break;
			case FOURCC_DXT3:	desc.format = ITexture::FORMAT_BC2; break;
			case FOURCC_DXT5:	desc.format	= ITexture::FORMAT_BC3; break;
			default:
				ERROR_MSG("This %d compressed format currently not supported.", ddsDesc.m_format.m_fourCC);
				return nullptr;
			}
		}
		else if(ddsDesc.m_format.m_flags & DDS_RGB)
		{
			switch(ddsDesc.m_format.m_bpp)
			{
			case 8:		desc.format = ITexture::FORMAT_R8;    break;
			case 16:	desc.format	= ITexture::FORMAT_RG8;	  break;
			case 32:	desc.format	= ITexture::FORMAT_RGBA8; break;
			case 24:
				ERROR_MSG("This %d uncompressed format not supported.", ddsDesc.m_format);
				return nullptr;
			default:
				assert(0);
				return nullptr;
			}
		}
		else
		{
			ERROR_MSG("Unsupported .dds texture format.");
			return nullptr;
		}
		
		uint offset	= data.pos();
		uint width  = desc.width;
		uint height = desc.height;

		std::vector<ITexture::Data> texDataVec;

		//-- 5. iterate over the all mip-map levels set and gather data.
		for (uint level = 0; level < desc.mipLevels && (width || height); ++level)
		{
			//-- make sure that width and height great or equal 1.
			width  = math::max(width,  uint(1));
			height = math::max(height, uint(1));

			//-- retrieve data offset params for current mip-map level.
			SurfaceInfo info = surfaceInfo(width, height, desc.format);

			//-- gather all mip-map levels data in config file.
			{
				ITexture::Data texData;
				texData.mem				= data.ptr(offset);
				texData.memPitch		= info.m_rowBytes;
				texData.memSlicePitch	= 0;

				texDataVec.push_back(texData);
			}

			//-- calculate data offset for the next mip-map level.
			offset += info.m_numBytes;

			//-- calculate size of the next mip-map level.
			width  >>= 1;
			height >>= 1;
		}

		//-- 6. now all data are prepared lets create texture.
		Ptr<ITexture> texture = render::rd()->createTexture(
			desc, &texDataVec[0], texDataVec.size()
			);
		
		return texture;
	}

} // brUGE