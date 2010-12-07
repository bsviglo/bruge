#ifndef _BR_GLTEXTURE_H_
#define _BR_GLTEXTURE_H_

#include "br_GLCommon.h"
#include "render/ibr_Texture.h"

// Note: Используется расширение GL_EXT_direct_state_access. Будем надеятся, что AMD-ATI в
//		 ближайшее время добавят это расширение в свой список поддерживаемых расширений, как это
//		 случилось с расширением GL_EXT_bindable_uniform. Надеюсь ждать слишком долго не придется.:)
// 
// TODO: Так как ATI пока не поддерживает GL_EXT_direct_state_access и это расширение еще не стандарт OGL 3.*
//		 ждем его включения, после чего класс будет поправлен.
//
//glTexImage2D(GL_TEXTURE_2D,	level, m_texFormat.intFormat,
//			 m_desc.width, m_desc.height, 0, m_texFormat.extFormat,
//			 m_texFormat.type, NULL);
//
//glTextureImage2DEXT(m_texId, GL_TEXTURE_2D,	level, m_texFormat.intFormat,
//	m_desc.width, m_desc.height, 0, m_texFormat.extFormat,
//	m_texFormat.type, NULL);


namespace brUGE
{
namespace render
{
	
	//------------------------------------------
	class brGLTexture : public ibrTexture
	{
	public:
		// структура содержит отформатированный
		// формат ibrTexture::ebrTextureFormat для OpenGL 3.0.
		struct GLTexFormat_s
		{
			GLTexFormat_s() {}
			GLTexFormat_s(
				GLenum intFormat_,
				GLenum extFormat_,
				GLenum type_,
				bool compressed_
				)
				:	intFormat(intFormat_),
					extFormat(extFormat_),
					type(type_),
					compressed(compressed_) {}

			GLenum	intFormat;
			GLenum	extFormat;
			GLenum	type;
			bool	compressed;
		};

	public:
		brGLTexture(
			const ibrTexture::Desc_s& desc,
			const void* data = NULL
			);
		virtual ~brGLTexture();
		virtual void generateMipmap();

		GLuint getTexId() const { return m_texId; }
		GLenum getTexTarget() const { return m_texTarget; }

	protected:
		static GLTexFormat_s	_getTexFormat(ebrTexFormat format);
		static GLenum			_getTexTarget(ebrTexType type);
		static uint				_getMemorySize(uint width, uint height, ebrTexFormat format);

		void _createTex(const void* data);
		void _createCompressedTex(const void* data);

	protected:
		GLuint			m_texId;
		GLenum			m_texTarget;
		GLTexFormat_s	m_texFormat;
	};

} // render
} // brUGE

#endif /*_BR_GLTEXTURE_H_*/