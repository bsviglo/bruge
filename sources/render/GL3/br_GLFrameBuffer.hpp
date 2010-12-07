#pragma once

#include "GL_common.hpp"

namespace brUGE
{
namespace render
{
	// 
	//------------------------------------------
	class GLFrameBuffer
	{
	public:
		GLFrameBuffer();
		~GLFrameBuffer();
		
		// сделать активным вывод в главный фрейм буфер или в скрытый буфер.
		void activate(bool main = true) const;
		
		void attachColors(uint count, const Ptr<brGLTexture>* textures);
		void attachDepthStencil(const Ptr<brGLTexture>& texture);

		void checkStatus() const;

		// TODO: GL_EXT_framebuffer_multisample и GL_EXT_framebuffer_blit

		// TODO: продумать и доделать установку конкретного мипмап уровня в качестве рендер таргета.

		// TODO: продумать установку кубе мап или массива текстурв в качестве буфера глубины.

		// TODO: продумать как будет осуществляется установка кубе мап или
		//		 текстурного массива в качестве рендер таргета.

	private:
		GLuint	m_fbo;
		uint	m_attachments;
		GLenum	m_drawBuffers[16];
	};

} //-- render
} //-- brUGE
