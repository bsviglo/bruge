#pragma once

#include "GL_common.hpp"
#include "render/IBuffer.h"

namespace brUGE
{
namespace render
{	
	
	//--
	//-- http://www.opengl.org/registry/specs/ARB/map_buffer_range.txt
	//----------------------------------------------------------------------------------------------
	class GLBuffer : public IBuffer
	{
	public:
		GLBuffer(EType type, EUsage usage, ECPUAccess cpuAccess);
		virtual ~GLBuffer();
		
		virtual void* doMap(EAccess access);
		virtual void  doUnmap();
		
		bool init(const void* data, uint elemCount, uint elemSize);
		GLuint getBufferId() const { return m_ID; }
		GLenum getBufferTarget() const { return m_target; }

	private:
		GLuint m_ID;
		GLenum m_target;
	};


} //-- render
} //-- brUGE
