#include "loader/TextureLoader.h"
#include "utils/Data.hpp"
#include "render/render_system.hpp"
#include "render/IRenderDevice.h"
#include "IL/il.h"

using namespace brUGE::math;
using namespace brUGE::render;
using namespace brUGE::utils;

// start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	//------------------------------------------
	void getSurfaceInfo(uint width,	uint height, ITexture::EFormat format,
		uint* pNumBytes, uint* pRowBytes, uint* pNumRows)
	{
		uint numBytes = 0;
		uint rowBytes = 0;
		uint numRows  = 0;

		// From the DXSDK docs:
		//
		//     When computing DXTn compressed sizes for non-square textures, the 
		//     following formula should be used at each mipmap level:
		//
		//         max(1, width ч 4) x max(1, height ч 4) x 8(DXT1) or 16(DXT2-5)
		//
		//     The pitch for DXTn formats is different from what was returned in 
		//     Microsoft DirectX 7.0. It now refers the pitch of a row of blocks. 
		//     For example, if you have a width of 16, then you will have a pitch 
		//     of four blocks (4*8 for DXT1, 4*16 for DXT2-5.)"

		if (
			format == ITexture::FORMAT_BC1 ||
			format == ITexture::FORMAT_BC2 ||
			format == ITexture::FORMAT_BC3
			)
		{
			int numBlocksWide = 0;
			if( width > 0 )		numBlocksWide = max<uint>(1, width / 4);
			int numBlocksHigh = 0;
			if( height > 0 )	numBlocksHigh = max<uint>(1, height / 4);

			//int numBlocks = numBlocksWide * numBlocksHigh;
			int numBytesPerBlock = (format == ITexture::FORMAT_BC1 ? 8 : 16);

			rowBytes = numBlocksWide * numBytesPerBlock;
			numRows	 = numBlocksHigh;
		}
		else
		{
			// TODO: брать размер формата текстуры. Считаем что текстура формата RGBA8
			uint bpp = 32;
			rowBytes = ( width * bpp + 7 ) / 8; // round up to nearest byte
			numRows	 = height;
		}

		numBytes = rowBytes * numRows;
		if (pNumBytes != NULL)	*pNumBytes = numBytes;
		if (pRowBytes != NULL)	*pRowBytes = rowBytes;
		if (pNumRows  != NULL)	*pNumRows = numRows;
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
		if(ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
		{
			ERROR_MSG("Incorrect devil.dll version");
			return false;
		}

		ilInit();
		ilEnable(IL_KEEP_DXTC_DATA);

		return true;
	}
	
	//------------------------------------------
	void TextureLoader::shutdown()
	{
		ilShutDown();
	}
	
	//------------------------------------------
	Ptr<ITexture> TextureLoader::loadTex2D(const ROData& data)
	{
		ILuint	texId	= 0;
		
		ilGenImages(1, &texId);
		ilBindImage(texId);
		
		if (RenderSystem::instance().gapi() == RENDER_API_GL3)
		{
			ilEnable(IL_ORIGIN_SET);
			ilSetInteger(IL_ORIGIN_MODE, IL_ORIGIN_UPPER_LEFT);
		}
		else
		{
			ilEnable(IL_ORIGIN_SET);
			ilSetInteger(IL_ORIGIN_MODE, IL_ORIGIN_LOWER_LEFT);
		}

		if(!ilLoadL(IL_TYPE_UNKNOWN, data.ptr(), data.length()))
			return NULL;

		ITexture::Desc desc;
		desc.texType   = ITexture::TYPE_2D;
		desc.bindFalgs = ITexture::BIND_SHADER_RESOURCE;
		desc.width	   = ilGetInteger(IL_IMAGE_WIDTH);
		desc.height	   = ilGetInteger(IL_IMAGE_HEIGHT);

		// выбираем формат.
		int  bpp		= ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL);
		int  dxtc		= ilGetInteger(IL_DXTC_DATA_FORMAT);
		bool compressed	= (dxtc == IL_DXT1)	|| (dxtc == IL_DXT3) || (dxtc == IL_DXT5);
		
		// переопределить тип для OpenGL

		if (compressed)
		{
			switch (dxtc)
			{
			case IL_DXT1:	desc.format = ITexture::FORMAT_BC1; break;
			case IL_DXT3:	desc.format	= ITexture::FORMAT_BC2; break;
			case IL_DXT5:	desc.format = ITexture::FORMAT_BC3; break;
			default: 
				ERROR_MSG("Invalid compressed format.");
				return NULL;
			} 
		}
		else
		{
			switch (bpp)
			{
			case 1:		desc.format = ITexture::FORMAT_R8; break;
			case 3:		ERROR_MSG("Unsupported texture format. RGB8 must be RGBA8."); return NULL;
			case 4:		desc.format = ITexture::FORMAT_RGBA8; break;
			default:
				ERROR_MSG("Invalid texture format.");
				return NULL;
			}
		}
			
		// TODO: продумать загрузку.

		uint rowBytes = 0;
		getSurfaceInfo(desc.width, desc.height, desc.format, NULL, &rowBytes, NULL);

		std::vector<ITexture::Data> texDataVec;
		ITexture::Data texData = {ilGetData(), rowBytes, 0};
		texDataVec.push_back(texData);

		Ptr<ITexture> texture = render::rd()->createTexture(desc, &texDataVec[0], texDataVec.size());
		
		//общая загруженная видео память в килобайтах под хранение текстур.
		m_totalSize += ilGetInteger(IL_IMAGE_SIZE_OF_DATA) / 1024;
		ilDeleteImages(1, &texId);

		return texture;
	}

} // brUGE