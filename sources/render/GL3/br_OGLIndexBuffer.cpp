#include "br_OGLIndexBuffer.h"
#include "br_OGLBufferManager.h"

namespace brUGE
{
	namespace render
	{

		//------------------------------------------
		brOGLIndexBuffer::brOGLIndexBuffer(uint indexCount, ebrIndexType indexType,
											ibrBuffer::ebrUsageFlag usage, const void* data)
			: ibrIndexBuffer(indexCount, indexType),
			  bufferId_(0)
		{
			if (indexCount <= 0)
				throw brException("brOGLIndexBuffer::brOGLIndexBuffer", "Invalid <indexCount> parameter.");

			GLenum oglUsage = brOGLBufferManager::getOGLUsage(usage);

			if (type_ == IT_BIT32)
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount_ * 32, data, oglUsage);
			else
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount_ * 16, data, oglUsage);

			GLenum error = glGetError();          
			if (error != GL_NO_ERROR)
				throw brException("brOGLIndexBuffer::brOGLIndexBuffer", "Failed to create index buffer." );      
		}

		//------------------------------------------	
		brOGLIndexBuffer::~brOGLIndexBuffer()
		{
			glDeleteBuffers(1, &bufferId_);
			bufferId_ = 0;
		}

		//------------------------------------------
		void* brOGLIndexBuffer::map(ebrAccessFlag access)
		{
			GLenum oglAccess = brOGLBufferManager::getOGLAccess(access);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId_);
			GLvoid *pointer = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, oglAccess);
			if (!pointer) 
			{
				glGetError(); // GL_OUT_OF_MEMORY
				throw brException("brOGLIndexBuffer::map", "An OpenGL error has occured."); 
			}
			return static_cast<void*>(pointer);
		}

		//------------------------------------------
		void brOGLIndexBuffer::unMap()
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId_);
			if (!glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER))
			{
#ifdef _DEBUG
				GLenum Error = glGetError();
				if (Error != GL_NO_ERROR)
					throw brException("brOGLIndexBuffer::unMap", "An OpenGL error has occured.");
				else
					throw brException("brOGLIndexBuffer::unMap", "Failed to unmap buffer.");
#endif // _DEBUG
			}
		}

		//------------------------------------------
		bool brOGLIndexBuffer::isMapped() const
		{
			glClearErrors();
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId_);
			GLint mapped;
			glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_MAPPED, &mapped);
			GLenum error = glGetError();
			if (error != GL_NO_ERROR)
				throw brException("brOGLIndexBuffer::isMapped", "An OpenGL error has occured.");

			return mapped != GL_FALSE;
		}

		//------------------------------------------
		void* brOGLIndexBuffer::getMapPointer() const
		{
			glClearErrors();
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId_);
			GLvoid *pointer;
			glGetBufferPointerv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_MAP_POINTER, &pointer);
			GLenum error = glGetError();
			if (error != GL_NO_ERROR)
				throw brException("brOGLIndexBuffer::getMapPointer", "An OpenGL error has occured.");

			return pointer;
		}

		//------------------------------------------
		uint brOGLIndexBuffer::getSize() const 
		{
			glClearErrors();
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId_);
			GLint size;
			glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
			GLenum error = glGetError();
			if (error != GL_NO_ERROR)
				throw brException("brOGLIndexBuffer::getSize", "An OpenGL error has occured.");

			return size;
		}
	}
}