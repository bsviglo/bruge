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
		
		// ������� �������� ����� � ������� ����� ����� ��� � ������� �����.
		void activate(bool main = true) const;
		
		void attachColors(uint count, const Ptr<brGLTexture>* textures);
		void attachDepthStencil(const Ptr<brGLTexture>& texture);

		void checkStatus() const;

		// TODO: GL_EXT_framebuffer_multisample � GL_EXT_framebuffer_blit

		// TODO: ��������� � �������� ��������� ����������� ������ ������ � �������� ������ �������.

		// TODO: ��������� ��������� ���� ��� ��� ������� �������� � �������� ������ �������.

		// TODO: ��������� ��� ����� �������������� ��������� ���� ��� ���
		//		 ����������� ������� � �������� ������ �������.

	private:
		GLuint	m_fbo;
		uint	m_attachments;
		GLenum	m_drawBuffers[16];
	};

} //-- render
} //-- brUGE
