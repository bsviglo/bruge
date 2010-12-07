#include "br_VBO.h"

namespace brUGE
{
	namespace render
	{
		
		//------------------------------------------
		brVBO::brVBO(brVBOType type) : type_(type), id_(0), size_(0), target_(0)
		{
			if(type_ == VBOT_INDICES)
				target_ = GL_ELEMENT_ARRAY_BUFFER_ARB;
			else
				target_ = GL_ARRAY_BUFFER_ARB;
		}
	
		//------------------------------------------
		brVBO::~brVBO()
		{
			_destroy();
		}
		
		//------------------------------------------
		bool brVBO::create()
		{
			if(id_ == 0)
				glGenBuffersARB(1, &id_);

			if(id_ == 0)
			{
				WARNING_F("[Render][VBO]:creating vbo failed.");
				return false;
			}

			bind();
			unbind();

			LOG_F("[Render][VBO]:vbo #%d: created.", id_);
			return true;
		}
		
		//------------------------------------------
		void brVBO::bind() const
		{
			if(type_ == VBOT_INDICES)
				glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, id_);
			else
				glBindBufferARB(GL_ARRAY_BUFFER_ARB, id_);
			
			RENDER_GUARD_FUNC("VBO", "glBindBufferARB");
		}
		
		//------------------------------------------
		void brVBO::unbind() const
		{
			if(type_ == VBOT_INDICES)
				glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
			else
				glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

			RENDER_GUARD_FUNC("VBO", "glBindBufferARB");
		}
		
		//------------------------------------------
		void brVBO::allocate(uint32 size)
		{
			size_ = size;
			bind();
			glBufferDataARB(target_, size, 0, GL_STATIC_DRAW_ARB);
			LOG_F("[Render][VBO]:vbo #%d: memory allocated %f Kb", id_, size/1024.0f);
			RENDER_GUARD_FUNC("VBO", "glBufferDataARB");
			unbind();
		}
		
		//------------------------------------------
		void brVBO::allocateAndWriteData(const void *buf, uint32 size)
		{
			size_ = size;
			bind();
			glBufferDataARB(target_, size, buf, GL_STATIC_DRAW_ARB);
			LOG_F("[Render][VBO]:vbo #%d: memory allocated and data written %f Kb", id_, size/1024.0f);
			RENDER_GUARD_FUNC("VBO", "glBufferDataARB");
			unbind();
		}
		
		//------------------------------------------
		void brVBO::writeData(const void *buf, uint32 size, uint32 offset)
		{
			bind();
			glBufferSubDataARB(target_, offset, size, buf);
			LOG_F("[Render][VBO]:vbo #%d: data written %f Kb", id_, size/1024.0f);
			RENDER_GUARD_FUNC("VBO", "glBufferSubDataARB");
			unbind();
		}
		
		//------------------------------------------
		void brVBO::_destroy()
		{
			size_ = 0;
			if(id_){
				unbind();
				glDeleteBuffersARB(1, &id_);
				LOG_F("[Render][VBO]:vbo #%d: successfully deleted.", id_);
				RENDER_GUARD_FUNC("VBO", "glDeleteBuffersARB");
			}
		}
		
	}/*end namespace render*/
}/*end namespace brUGE*/