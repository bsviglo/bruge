#include "br_GLTexture.h"
#include "br_GLStateObjects.h"

namespace brUGE
{
namespace render
{
	
	//------------------------------------------
	brGLTexture::brGLTexture(
		const ibrTexture::Desc_s& desc,
		const void* data /* = NULL */
		)
		:	m_texId(0),
			m_texTarget(_getTexTarget(desc.texType)),
			ibrTexture(desc)
	{
		GL_CLEAR_ERRORS

		glGenTextures(1, &m_texId);
		glBindTexture(m_texTarget, m_texId);

		GL_THROW_ERRORS
		
		m_texFormat = _getTexFormat(desc.format);
	
		if (m_texFormat.compressed)
			_createCompressedTex(data);
		else
			_createTex(data);
	}
	
	//------------------------------------------
	brGLTexture::~brGLTexture()
	{
		GL_CLEAR_ERRORS

		glDeleteTextures(1, &m_texId);
		
		GL_LOG_ERRORS
	}
	
	//------------------------------------------
	void brGLTexture::generateMipmap()
	{
		GL_CLEAR_ERRORS

		// TODO: генерация мип-мап уровней.
		//glGenerateMipmap();

		GL_THROW_ERRORS
	}
	
	// TODO: Сделать возможность содавать не все мип-мап уровни, а только определенное число.
	//------------------------------------------
	void brGLTexture::_createTex(const void* data)
	{
		GL_CLEAR_ERRORS

		// создаем текстуру у которой есть текстурные данные или у которой их нет, но не нужно
		// генерировать мипмап уровни.
		if (data || (!data && m_desc.mipLevels == 1))
		{
			switch (m_desc.texType)
			{
			case TT_TEXTURE1D:
				glTexImage1D(GL_TEXTURE_1D, 0, m_texFormat.intFormat,
					m_desc.width, 0, m_texFormat.extFormat, m_texFormat.type, data);
				break;
			case TT_TEXTURE1D_ARRAY:
				BR_EXCEPT("brGLTexture::_createTex", "TT_TEXTURE1D_ARRAY not implemented.");
			case TT_TEXTURE2D:
				//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glTexImage2D(GL_TEXTURE_2D,	0, m_texFormat.intFormat,
					m_desc.width, m_desc.height, 0, m_texFormat.extFormat,
					m_texFormat.type, data);
				break;
			case TT_TEXTURE2D_ARRAY:
				BR_EXCEPT("brGLTexture::_createTex", "TT_TEXTURE2D_ARRAY not implemented.");
			case TT_TEXTURE3D:
				glTexImage3D(GL_TEXTURE_3D,	0, m_texFormat.intFormat,
					m_desc.width, m_desc.height, m_desc.depth, 0, m_texFormat.extFormat,
					m_texFormat.type, data);
				break;
			case TT_TEXTURE_CUBE_MAP:
				BR_EXCEPT("brGLTexture::_createTex", "TT_TEXTURE_CUBE_MAP not implemented.");
			default:
				BR_EXCEPT("brGLTexture::_createTex", "Invalid texture type.");
			}
			
			if (m_desc.mipLevels != 1)
				generateMipmap();

			GL_THROW_ERRORS

		}
		// нет текстурных данных, но нужно генерировать память под мипмап уровни.
		else
		{
			if (m_desc.texType != TT_TEXTURE1D_ARRAY &&
				m_desc.texType != TT_TEXTURE2D_ARRAY &&
				m_desc.texType != TT_TEXTURE_CUBE_MAP)
			{
				GLint level		= 0;
				GLint width		= m_desc.width;
				GLint height	= m_desc.height;
				GLint depth		= m_desc.depth;

				for (;;)
				{
					switch (m_desc.texType)
					{
					case TT_TEXTURE1D:
						glTexImage1D(GL_TEXTURE_1D, level, m_texFormat.intFormat,
							m_desc.width, 0, m_texFormat.extFormat, m_texFormat.type, NULL);
						break;
					case TT_TEXTURE2D:
						glTexImage2D(GL_TEXTURE_2D,	level, m_texFormat.intFormat,
							m_desc.width, m_desc.height, 0, m_texFormat.extFormat,
							m_texFormat.type, NULL);
						//glTextureImage2DEXT(m_texId, GL_TEXTURE_2D,	level, m_texFormat.intFormat,
						//	m_desc.width, m_desc.height, 0, m_texFormat.extFormat,
						//	m_texFormat.type, NULL);
						break;
					case TT_TEXTURE3D:
						glTexImage3D(GL_TEXTURE_3D,	level, m_texFormat.intFormat,
							m_desc.width, m_desc.height, m_desc.depth, 0, m_texFormat.extFormat,
							m_texFormat.type, NULL);
						break;
					default:
						BR_EXCEPT("brGLTexture::_createTex", "Invalid texture type.");
					}
					
					GL_THROW_ERRORS

					if (width == 1 && height == 1 && depth == 1)
						break;

					if (width	> 1)	 width	>>= 1;
					if (height	> 1)	 height >>= 1;
					if (depth	> 1)	 depth	>>= 1;
				}
			}
			else
			{
				// TODO: дописать генерацию мип-мап уровней для текстурных массивов и кубы мап.

				throw brException("brGLTexture::_createTex", "Texture array types not yet implemented.");
			}
		}
	}

	//------------------------------------------
	void brGLTexture::_createCompressedTex(const void* data)
	{
		GL_CLEAR_ERRORS

		GLsizei size = _getMemorySize(m_desc.width, m_desc.height, m_desc.format);

		// создаем текстуру у которой есть текстурные данные или у которой их нет, но не нужно
		// генерировать мипмап уровни.
		if (data || (!data && m_desc.mipLevels == 1))
		{
			switch (m_desc.texType)
			{
			case TT_TEXTURE1D:
				glCompressedTexImage1D(GL_TEXTURE_1D, 0, m_texFormat.intFormat,
					m_desc.width, 0, size, data);
				break;
			case TT_TEXTURE1D_ARRAY:
				BR_EXCEPT("brGLTexture::_createCompressedTex", "TT_TEXTURE1D_ARRAY not implemented.");
			case TT_TEXTURE2D:
				glCompressedTexImage2D(GL_TEXTURE_2D, 0, m_texFormat.intFormat,
					m_desc.width, m_desc.height, 0, size, data);
				break;
			case TT_TEXTURE2D_ARRAY:
				BR_EXCEPT("brGLTexture::_createCompressedTex", "TT_TEXTURE2D_ARRAY not implemented.");
			case TT_TEXTURE3D:
				glCompressedTexImage3D(GL_TEXTURE_3D, 0, m_texFormat.intFormat,
					m_desc.width, m_desc.height, m_desc.depth, 0, size, data);
				break;
			case TT_TEXTURE_CUBE_MAP:
				BR_EXCEPT("brGLTexture::_createCompressedTex", "TT_TEXTURE_CUBE_MAP not implemented.");
			default:
				BR_EXCEPT("brGLTexture::_createCompressedTex", "Invalid texture type.");
			}

			if (m_desc.mipLevels != 1)
				generateMipmap();

			GL_THROW_ERRORS

		}
		else
		{
			if (m_desc.texType != TT_TEXTURE1D_ARRAY &&
				m_desc.texType != TT_TEXTURE2D_ARRAY &&
				m_desc.texType != TT_TEXTURE_CUBE_MAP)
			{
				GLint level		= 0;
				GLint width		= m_desc.width;
				GLint height	= m_desc.height;
				GLint depth		= m_desc.depth;
				
				for (;;)
				{
					switch (m_desc.texType)
					{
					case TT_TEXTURE1D:
						glCompressedTexImage1D(GL_TEXTURE_1D, level, m_texFormat.intFormat,
							m_desc.width, 0, size, NULL);
						break;
					case TT_TEXTURE2D:
						glCompressedTexImage2D(GL_TEXTURE_2D, level, m_texFormat.intFormat,
							m_desc.width, m_desc.height, 0, size, NULL);
						break;
					case TT_TEXTURE3D:
						glCompressedTexImage3D(GL_TEXTURE_3D, level, m_texFormat.intFormat,
							m_desc.width, m_desc.height, m_desc.depth, 0, size, NULL);
						break;
					default:
						BR_EXCEPT("brGLTexture::_createCompressedTex", "Invalid texture type.");
					}

					GL_THROW_ERRORS
					
					if (width == 1 && height == 1 && depth == 1)
						break;

					if (width	> 1)	 width	>>= 1;
					if (height	> 1)	 height >>= 1;
					if (depth	> 1)	 depth	>>= 1;

					size = _getMemorySize(width, height, m_desc.format);
				}
			}
			else
			{
				// TODO: дописать генерацию мип-мап уровней для текстурных массивов и кубы мап.

				BR_EXCEPT("brGLTexture::_createTex", "Texture array types not yet implemented.");
			}
		}
	}

	//------------------------------------------
	/*static*/ uint brGLTexture::_getMemorySize(
		uint width, uint height, ebrTexFormat format)
	{
		switch (format)
		{
			case TF_BC1_UNORM:
			case TF_BC1_UNORM_sRGB:
			case TF_BC4_SNORM:
			case TF_BC4_UNORM:
				return ((width + 3) / 4) * ((height + 3) / 4) * 8;
			case TF_BC2_UNORM:
			case TF_BC2_UNORM_sRGB:
			case TF_BC3_UNORM:
			case TF_BC3_UNORM_sRGB:
			case TF_BC5_SNORM:
			case TF_BC5_UNORM:
				return ((width + 3) / 4) * ((height + 3) / 4) * 16;
			default:
				BR_EXCEPT("brGLTexture::_getMemorySize", "Invalid texture type.");
		}
	}
	
	//------------------------------------------
	/*static*/ GLenum brGLTexture::_getTexTarget(
		ibrTexture::ebrTexType type)
	{
		switch (type)
		{
		case TT_TEXTURE1D:			return GL_TEXTURE_1D;
		case TT_TEXTURE1D_ARRAY:	return GL_TEXTURE_1D_ARRAY;
		case TT_TEXTURE2D:			return GL_TEXTURE_2D;
		case TT_TEXTURE2D_ARRAY:	return GL_TEXTURE_2D_ARRAY;
		case TT_TEXTURE3D:			return GL_TEXTURE_3D;
		case TT_TEXTURE_CUBE_MAP:	return GL_TEXTURE_CUBE_MAP;
		default:
			BR_EXCEPT("brGLTexture::_getTexTarget", "Invalid texture target.");
		}
	}
	
	//------------------------------------------
	/*static*/ brGLTexture::GLTexFormat_s brGLTexture::_getTexFormat(
		ibrTexture::ebrTexFormat format)
	{
		switch(format)
		{
		// некомпресированные форматы.
		case ibrTexture::TF_RGBA8:			return GLTexFormat_s(GL_RGBA8,			GL_RGBA,	GL_UNSIGNED_BYTE,	false);
		case ibrTexture::TF_RGBA8ui:		return GLTexFormat_s(GL_RGBA8UI,		GL_RGBA,	GL_UNSIGNED_BYTE,	false);
		case ibrTexture::TF_RGBA8i:			return GLTexFormat_s(GL_RGBA8I,			GL_RGBA,	GL_BYTE,			false);
		case ibrTexture::TF_RGBA8_sRGB8:	return GLTexFormat_s(GL_SRGB8_ALPHA8,	GL_RGBA,	GL_UNSIGNED_BYTE,	false);

		case ibrTexture::TF_RGBA16:			return GLTexFormat_s(GL_RGBA16,			GL_RGBA,	GL_UNSIGNED_SHORT,	false);
		case ibrTexture::TF_RGBA16f:		return GLTexFormat_s(GL_RGBA16F,		GL_RGBA,	GL_FLOAT,			false);
		case ibrTexture::TF_RGBA16ui:		return GLTexFormat_s(GL_RGBA16UI,		GL_RGBA,	GL_UNSIGNED_SHORT,	false);
		case ibrTexture::TF_RGBA16i:		return GLTexFormat_s(GL_RGBA16I,		GL_RGBA,	GL_SHORT,			false);

		case ibrTexture::TF_RGBA32f:		return GLTexFormat_s(GL_RGBA32F,		GL_RGBA,	GL_FLOAT,			false);
		case ibrTexture::TF_RGBA32ui:		return GLTexFormat_s(GL_RGBA32UI,		GL_RGBA,	GL_UNSIGNED_INT,	false);
		case ibrTexture::TF_RGBA32i:		return GLTexFormat_s(GL_RGBA32I,		GL_RGBA,	GL_INT,				false);

		case ibrTexture::TF_RGB16f:			return GLTexFormat_s(GL_RGB16F,			GL_RGB,		GL_FLOAT,			false);
		case ibrTexture::TF_RGB16ui:		return GLTexFormat_s(GL_RGB16UI,		GL_RGB,		GL_UNSIGNED_SHORT,	false);
		case ibrTexture::TF_RGB16i:			return GLTexFormat_s(GL_RGB16I,			GL_RGB,		GL_SHORT,			false);

		case ibrTexture::TF_RGB32f:			return GLTexFormat_s(GL_RGB32F,			GL_RGB,		GL_FLOAT,			false);
		case ibrTexture::TF_RGB32ui:		return GLTexFormat_s(GL_RGB32UI,		GL_RGB,		GL_UNSIGNED_INT,	false);
		case ibrTexture::TF_RGB32i:			return GLTexFormat_s(GL_RGB32I,			GL_RGB,		GL_INT,				false);
		
		case ibrTexture::TF_R8:				return GLTexFormat_s(GL_R8,				GL_RED,		GL_UNSIGNED_BYTE,	false);
		case ibrTexture::TF_R8ui:			return GLTexFormat_s(GL_R8UI,			GL_RED,		GL_UNSIGNED_BYTE,	false);
		case ibrTexture::TF_R8i:			return GLTexFormat_s(GL_R8I,			GL_RED,		GL_BYTE,			false);

		case ibrTexture::TF_R16:			return GLTexFormat_s(GL_R16,			GL_RED,		GL_UNSIGNED_SHORT,	false);
		case ibrTexture::TF_R16f:			return GLTexFormat_s(GL_R16F,			GL_RED,		GL_FLOAT,			false);
		case ibrTexture::TF_R16ui:			return GLTexFormat_s(GL_R16UI,			GL_RED,		GL_UNSIGNED_SHORT,	false);
		case ibrTexture::TF_R16i:			return GLTexFormat_s(GL_R16I,			GL_RED,		GL_SHORT,			false);

		case ibrTexture::TF_R32f:			return GLTexFormat_s(GL_R32F,			GL_RED,		GL_FLOAT,			false);
		case ibrTexture::TF_R32ui:			return GLTexFormat_s(GL_R32UI,			GL_RED,		GL_UNSIGNED_INT,	false);
		case ibrTexture::TF_R32i:			return GLTexFormat_s(GL_R32I,			GL_RED,		GL_INT,				false);

		case ibrTexture::TF_RG8:			return GLTexFormat_s(GL_RG8,			GL_RG,		GL_UNSIGNED_BYTE,	false);
		case ibrTexture::TF_RG8ui:			return GLTexFormat_s(GL_RG8UI,			GL_RG,		GL_UNSIGNED_BYTE,	false);
		case ibrTexture::TF_RG8i:			return GLTexFormat_s(GL_RG8I,			GL_RG,		GL_BYTE,			false);

		case ibrTexture::TF_RG16:			return GLTexFormat_s(GL_RG16,			GL_RG,		GL_UNSIGNED_SHORT,	false);
		case ibrTexture::TF_RG16f:			return GLTexFormat_s(GL_RG16F,			GL_RG,		GL_FLOAT,			false);
		case ibrTexture::TF_RG16ui:			return GLTexFormat_s(GL_RG16UI,			GL_RG,		GL_UNSIGNED_SHORT,	false);
		case ibrTexture::TF_RG16i:			return GLTexFormat_s(GL_RG16I,			GL_RG,		GL_SHORT,			false);

		case ibrTexture::TF_RG32f:			return GLTexFormat_s(GL_RG32F,			GL_RG,		GL_FLOAT,			false);
		case ibrTexture::TF_RG32ui:			return GLTexFormat_s(GL_RG32UI,			GL_RG,		GL_UNSIGNED_INT,	false);
		case ibrTexture::TF_RG32i:			return GLTexFormat_s(GL_RG32I,			GL_RG,		GL_INT,				false);
		
		// GL_EXT_texture_shared_exponent - для OpenGL
		case ibrTexture::TF_RGB9_E5_SHAREDEXP:		return GLTexFormat_s(GL_RGB9_E5, GL_RGBA, GL_UNSIGNED_BYTE, false);


		// компресированные форматы.
		case ibrTexture::TF_BC1_UNORM:		return GLTexFormat_s(GL_COMPRESSED_RGB_S3TC_DXT1_EXT,			0, 0, true);
		case ibrTexture::TF_BC1_UNORM_sRGB:	return GLTexFormat_s(GL_COMPRESSED_SRGB_S3TC_DXT1_EXT,			0, 0, true);
		case ibrTexture::TF_BC2_UNORM:		return GLTexFormat_s(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,			0, 0, true);
		case ibrTexture::TF_BC2_UNORM_sRGB:	return GLTexFormat_s(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,	0, 0, true);
		case ibrTexture::TF_BC3_UNORM:		return GLTexFormat_s(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,			0, 0, true);
		case ibrTexture::TF_BC3_UNORM_sRGB:	return GLTexFormat_s(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,	0, 0, true);
		case ibrTexture::TF_BC4_UNORM:		return GLTexFormat_s(GL_COMPRESSED_RED_RGTC1,					0, 0, true);
		case ibrTexture::TF_BC4_SNORM:		return GLTexFormat_s(GL_COMPRESSED_SIGNED_RED_RGTC1,			0, 0, true);
		case ibrTexture::TF_BC5_UNORM:		return GLTexFormat_s(GL_COMPRESSED_RG_RGTC2,					0, 0, true);
		case ibrTexture::TF_BC5_SNORM:		return GLTexFormat_s(GL_COMPRESSED_SIGNED_RG_RGTC2,				0, 0, true);

		default:
			BR_EXCEPT("brGLTexture::_getTexFormat", "Invalid format.");
		}
	}

} // render
} // brUGE