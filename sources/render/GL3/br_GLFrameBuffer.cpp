#include "br_GLFrameBuffer.h"
#include "br_GLTexture.h"

namespace brUGE
{
namespace render
{
	
	//------------------------------------------
	brGLFrameBuffer::brGLFrameBuffer()
		:	m_fbo(0),
			m_attachments(0)
	{
		for (uint i = 0; i < 16; ++i)
			m_drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;

		glClearErrors();
		glGenFramebuffers(1, &m_fbo);

		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
			throw brException("brGLFrameBuffer::brGLFrameBuffer", "Failed to create framebuffer object.");
	}
	
	//------------------------------------------
	brGLFrameBuffer::~brGLFrameBuffer()
	{
		glClearErrors();
		glDeleteFramebuffers(1, &m_fbo);

		if (glGetError() != GL_NO_ERROR)
			FATAL("brGLFrameBuffer::~brGLFrameBuffer: Failed to delete framebuffer object.");
	}
	
	//------------------------------------------
	void brGLFrameBuffer::activate(bool main /* = true */) const
	{
		if (main)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDrawBuffer(GL_BACK);
		}
		else
			glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	}

	//------------------------------------------
	void brGLFrameBuffer::attachColors(
		uint count,
		const Ptr<brGLTexture>* textures)
	{
#ifdef _DEBUG
		glClearErrors();
#endif // _DEBUG

		// By default the draw-buffer and read-buffer for an FBO is GL_COLOR_ATTACHMENT0_EXT, meaning 
		// that the driver will expect there to be a color buffer attached to render to.
		// Since we don’t have a color attachment the framebuffer will be considered incomplete.
		// Consequently, we must inform the driver that	we do not wish to render to the color buffer.
		// We do this with a call to set the draw-buffer and read-buffer to GL_NONE.
		// And then restore it to default state.
		if (!count)
		{
			glDrawBuffer(GL_NONE); 
			glReadBuffer(GL_NONE);

			// Очистить все слоты, занятые предыдущим вызовом.
			for (uint i = 0; i < m_attachments; ++i)
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, 0, 0);

			m_attachments = 0;			
			return; // выход.
		}
		else
		{
			glDrawBuffer(GL_COLOR_ATTACHMENT0); 
			glReadBuffer(GL_COLOR_ATTACHMENT0); 
		}
		
		// TODO: завершить для всех типов текстур.
		
		for (uint i = 0; i < count; ++i)
		{
			brGLTexture* texture = textures[i].get();
			switch (texture->getTexTarget())
			{
			case GL_TEXTURE_1D:
				glFramebufferTexture1D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
					GL_TEXTURE_1D, texture->getTexId(), 0);
				break;
			case GL_TEXTURE_1D_ARRAY:
				throw brException("brGLFrameBuffer::attachColors", "Texture target is yet not implemented.");
			case GL_TEXTURE_2D:
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
					GL_TEXTURE_2D, texture->getTexId(), 0);
				break;
			case GL_TEXTURE_2D_ARRAY:
			case GL_TEXTURE_3D:
			case GL_TEXTURE_CUBE_MAP:
				throw brException("brGLFrameBuffer::attachColors", "Texture target is yet not implemented.");
			default:
				throw brException("brGLFrameBuffer::attachColors", "Invalid texture target.");
			}
		}
		
#ifdef _DEBUG
		if (glGetError() != GL_NO_ERROR)
			throw brException("brGLFrameBuffer::attachColors", "An OpenGL error has occurred.");
#endif // _DEBUG
		
		// Очистить все слоты, которые использовались при предыдущем вызове, но сейчас не используются.
		for (uint i = 0; i < m_attachments - count; ++i)
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, 0, 0);
		
		m_attachments = count;

		// активируем цветовые буфера для MRT.
		glDrawBuffers(m_attachments, m_drawBuffers);
	}
	
	//------------------------------------------
	void brGLFrameBuffer::attachDepthStencil(
		const Ptr<brGLTexture>& texture)
	{
#ifdef _DEBUG
		glClearErrors();
#endif // _DEBUG

		if (texture)
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_TEXTURE_2D,
				GL_DEPTH_STENCIL_ATTACHMENT, texture->getTexId(), 0);			
		else
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_TEXTURE_2D,
				GL_DEPTH_STENCIL_ATTACHMENT, 0, 0);

#ifdef _DEBUG
		if (glGetError() != GL_NO_ERROR)
			throw brException("brGLFrameBuffer::attachDepthStencil", "An OpenGL error has occurred.");
#endif // _DEBUG
	}
	
	//------------------------------------------
	void brGLFrameBuffer::checkStatus() const
	{
#ifdef _DEBUG
		GLenum code = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		switch (code)
		{
		case GL_FRAMEBUFFER_COMPLETE:
			// все ОК.
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			throw brException("brGLFrameBuffer::checkStatus", "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT.");
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			throw brException("brGLFrameBuffer::checkStatus", "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT.");
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			throw brException("brGLFrameBuffer::checkStatus", "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT.");
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
			throw brException("brGLFrameBuffer::checkStatus", "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT.");
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			throw brException("brGLFrameBuffer::checkStatus", "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER.");
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			throw brException("brGLFrameBuffer::checkStatus", "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER.");
		case GL_FRAMEBUFFER_UNSUPPORTED:
			throw brException("brGLFrameBuffer::checkStatus", "GL_FRAMEBUFFER_UNSUPPORTED.");
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			throw brException("brGLFrameBuffer::checkStatus", "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE.");
		default:
			throw brException("brGLFrameBuffer::checkStatus", "Unknown framebuffer status.");
		}
#endif // _DEBUG
	}

} // render
} // brUGE