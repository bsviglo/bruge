#include "GLBuffer.hpp"

// start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{

	//-- see IBuffer::EType.
	const GLenum glType[] =
	{
		GL_ARRAY_BUFFER,
		GL_ELEMENT_ARRAY_BUFFER,
		GL_UNIFORM_BUFFER,
		GL_TEXTURE_BUFFER
	};
	
	//-- see IBuffer::EAccess.
	const GLbitfield glAccess[] =
	{
		GL_MAP_READ_BIT,
		GL_MAP_WRITE_BIT,
		GL_MAP_READ_BIT  | GL_MAP_WRITE_BIT,
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT,
		GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT
	};
	
	//-- buffer usage type.
	const GLenum glUsage[] = 
	{
		GL_DYNAMIC_DRAW, 
		GL_STREAM_DRAW,	
		GL_STATIC_DRAW,
		GL_INVALID_ENUM
	};

}
//--------------------------------------------------------------------------------------------------
// end unnamed namespace.


namespace brUGE
{
namespace render
{

	//----------------------------------------------------------------------------------------------
	GLBuffer::GLBuffer(EType type, EUsage usage, ECPUAccess cpuAccess)
		: IBuffer(type, usage, cpuAccess), m_target(glType[type]), m_ID(0)
	{

	}
	
	//----------------------------------------------------------------------------------------------
	GLBuffer::~GLBuffer()
	{
#ifdef _DEBUG
		GLint mapped;
		glGetBufferParameteriv(m_target, GL_BUFFER_MAPPED, &mapped);
		if (mapped)
		{
			glUnmapBuffer(m_target);
			glBindBuffer(m_target, 0);
			WARNING_MSG("Buffer [%d] was implicitly unmapped before deletion.", this);
		}
#endif // _DEBUG

		GL_CLEAR_ERRORS

		glDeleteBuffers(1, &m_ID);
			
		GL_LOG_ERRORS
	}
	
	//----------------------------------------------------------------------------------------------
	bool GLBuffer::init(const void* data, uint elemCount, uint elemSize)
	{
		GL_CLEAR_ERRORS

		glGenBuffers(1, &m_ID);
		glBindBuffer(m_target, m_ID);

		if (m_ID == 0)
		{
			GL_LOG_ERRORS
			return false;
		}

		glBufferData(m_target, elemCount * elemSize, data, glUsage[m_usage]);

		GL_LOG_ERRORS
		
		return true;
	}
	
	//----------------------------------------------------------------------------------------------
	void* GLBuffer::doMap(EAccess access)
	{
		GL_CLEAR_ERRORS

		GLvoid* pointer =
			glMapNamedBufferRangeEXT(m_ID, 0, m_elemCount * m_elemSize, glAccess[access]
			);

		GL_LOG_ERRORS

		return static_cast<void*>(pointer);
	}
	
	//----------------------------------------------------------------------------------------------
	void GLBuffer::doUnmap()
	{
		GL_CLEAR_ERRORS

		glUnmapNamedBufferEXT(m_ID);

		GL_LOG_ERRORS
	}

} //-- render
} //-- brUGE