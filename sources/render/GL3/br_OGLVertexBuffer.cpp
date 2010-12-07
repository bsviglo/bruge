#include "br_OGLVertexBuffer.h"
#include "br_OGLBufferManager.h"

namespace brUGE
{
	namespace render
	{

		brOGLBufferImpl::brOGLBufferImpl(
			ibrBuffer::ebrUsageFlag usage,
			GLenum bufferType
			)
			:	bufferId_(0),
				bufferType_(bufferType),
				bufferUsage_(_getOGLUsage(usage))
		{
			glClearErrors();
			glGenBuffers(1, &bufferId_);
			glBindBuffer(bufferType_, bufferId_);
			GLenum error = glGetError();
			if (error != GL_NO_ERROR)
				throw brException("brOGLBufferImpl::brOGLBufferImpl", "Failed to generate buffer object.");
		}

		brOGLBufferImpl::~brOGLBufferImpl()
		{
			glClearErrors();
			glBindBuffer(bufferType_, bufferId_);
			glDeleteBuffers(1, &bufferId_);
			if (glGetError() != GL_NO_ERROR)
				throw brException("brOGLBufferImpl::~brOGLBufferImpl", "Failed to delete buffer object.");
		}

		void* brOGLBufferImpl::mapImpl(
			ibrBuffer::ebrAccessFlag access
			)
		{
			GLenum oglAccess = _getOGLAccess(access);
			glBindBuffer(bufferType_, bufferId_);
			GLvoid *pointer = glMapBuffer(bufferType_, oglAccess);
			if (!pointer) 
			{
				glGetError(); // GL_OUT_OF_MEMORY
				throw brException("brOGLBufferImpl::mapImpl", "An OpenGL error has occurred."); 
			}
			return static_cast<void*>(pointer);
		}

		void brOGLBufferImpl::unMapImpl()
		{
			glBindBuffer(bufferType_, bufferId_);
			if (!glUnmapBuffer(bufferType_))
			{
		#ifdef _DEBUG
				GLenum Error = glGetError();
				if (Error != GL_NO_ERROR)
					throw brException("brOGLBufferImpl::unMapImpl", "An OpenGL error has occurred.");
				else
					throw brException("brOGLBufferImpl::unMapImpl", "Failed to unmap buffer.");
		#endif // _DEBUG
			}
		}

		bool brOGLBufferImpl::isMappedImpl() const
		{
			glClearErrors();
			glBindBuffer(bufferType_, bufferId_);
			GLint mapped;
			glGetBufferParameteriv(bufferType_, GL_BUFFER_MAPPED, &mapped);
			GLenum error = glGetError();
			if (error != GL_NO_ERROR)
				throw brException("brOGLBufferImpl::isMappedImpl", "An OpenGL error has occurred.");

			return mapped != GL_FALSE;
		}

		uint brOGLBufferImpl::getSizeImpl() const
		{
			glClearErrors();
			glBindBuffer(bufferType_, bufferId_);
			GLint size;
			glGetBufferParameteriv(bufferType_, GL_BUFFER_SIZE, &size);
			GLenum error = glGetError();
			if (error != GL_NO_ERROR)
				throw brException("brOGLBufferImpl::getSizeImpl", "An OpenGL error has occurred.");

			return size;
		}

		void* brOGLBufferImpl::getMapPointerImpl() const
		{
			glClearErrors();
			glBindBuffer(bufferType_, bufferId_);
			GLvoid *pointer;
			glGetBufferPointerv(bufferType_, GL_BUFFER_MAP_POINTER, &pointer);
			GLenum error = glGetError();
			if (error != GL_NO_ERROR)
				throw brException("brOGLBufferImpl::getMapPointerImpl", "An OpenGL error has occurred.");

			return pointer;
		}

		GLenum brOGLBufferImpl::_getOGLAccess(ibrBuffer::ebrAccessFlag access)
		{
			switch(access)
			{
			case ibrBuffer::AF_READ:
				return GL_READ_ONLY;
			case ibrBuffer::AF_WRITE:
				return GL_WRITE_ONLY;
			case ibrBuffer::AF_READ_WRITE:
				return GL_READ_WRITE;
			default:
				return GL_READ_WRITE;
			}
		}

		GLenum brOGLBufferImpl::_getOGLUsage(ibrBuffer::ebrUsageFlag usage)
		{
			switch(usage)
			{
			case ibrBuffer::UF_DEFAULT:
				return	GL_STREAM_DRAW;
			case ibrBuffer::UF_DYNAMIC:
				return GL_DYNAMIC_DRAW;
			case ibrBuffer::UF_IMMUTABLE:
				return GL_STATIC_DRAW;
			default:
				return GL_STREAM_DRAW;
			}
		}

		//------------------------------------------
		brOGLVertexBuffer::brOGLVertexBuffer(
			uint vertexCount,
			uint vertexSize,
			ibrBuffer::ebrUsageFlag usage,
			const void* data
			)
			:	ibrVertexBuffer(vertexSize, vertexCount),
				brOGLBufferImpl(usage, GL_ARRAY_BUFFER)
		{
			if (vertexCount_ <= 0)
				throw brException("brOGLVertexBuffer::brOGLVertexBuffer", "Invalid <vertexCount> parameter.");
			if (vertexSize_ <= 0)
				throw brException("brOGLVertexBuffer::brOGLVertexBuffer", "Invalid <vertexSize> parameter.");

			glBufferData(bufferType_, vertexCount_ * vertexSize_, data, bufferUsage_);
			GLenum error = glGetError();          
			if (error != GL_NO_ERROR)
				throw brException("brOGLVertexBuffer::brOGLVertexBuffer", "Failed to create vertex buffer." );      
		}

		//------------------------------------------	
		brOGLVertexBuffer::~brOGLVertexBuffer()
		{
			
		}
		
		//------------------------------------------
		void* brOGLVertexBuffer::map(ebrAccessFlag access)
		{
			return mapImpl(access);
		}
		
		//------------------------------------------
		void brOGLVertexBuffer::unMap()
		{
			unMapImpl();
		}
		
		//------------------------------------------
		bool brOGLVertexBuffer::isMapped() const
		{
			return isMapped();
		}
		
		//------------------------------------------
		void* brOGLVertexBuffer::getMapPointer() const
		{
			return getMapPointerImpl();
		}
		
		//------------------------------------------
		uint brOGLVertexBuffer::getSize() const 
		{
			return getSizeImpl();
		}

		//------------------------------------------
		brOGLIndexBuffer::brOGLIndexBuffer(
			uint indexCount,
			ebrIndexType indexType,
			ibrBuffer::ebrUsageFlag usage,
			const void* data
			)
			:	ibrIndexBuffer(indexCount, indexType),
				brOGLBufferImpl(usage, GL_ELEMENT_ARRAY_BUFFER)
		{
			if (indexCount <= 0)
				throw brException("brOGLIndexBuffer::brOGLIndexBuffer", "Invalid <indexCount> parameter.");

			if (type_ == IT_BIT32)
				glBufferData(bufferType_, indexCount_ * 32, data, bufferUsage_);
			else
				glBufferData(bufferType_, indexCount_ * 16, data, bufferUsage_);

			GLenum error = glGetError();          
			if (error != GL_NO_ERROR)
				throw brException("brOGLIndexBuffer::brOGLIndexBuffer", "Failed to create index buffer." );      
		}

		//------------------------------------------	
		brOGLIndexBuffer::~brOGLIndexBuffer()
		{
			
		}

		//------------------------------------------
		void* brOGLIndexBuffer::map(ebrAccessFlag access)
		{
			return mapImpl(access);
		}

		//------------------------------------------
		void brOGLIndexBuffer::unMap()
		{
			unMapImpl();
		}

		//------------------------------------------
		bool brOGLIndexBuffer::isMapped() const
		{
			return isMappedImpl();
		}

		//------------------------------------------
		void* brOGLIndexBuffer::getMapPointer() const
		{
			return getMapPointerImpl();
		}

		//------------------------------------------
		uint brOGLIndexBuffer::getSize() const 
		{
			return getSizeImpl();
		}
	}
}