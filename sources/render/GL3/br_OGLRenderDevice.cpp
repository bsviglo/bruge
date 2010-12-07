#include "br_OGLRenderDevice.h"
#include "br_OGLSwapChain.h"
#include "br_OGLVertexLayout.h"
#include "br_GLBuffer.h"
#include "br_OGLFont.h"
#include "br_GLCgShadingProgram.h"
#include "br_GLStateObjects.h"

namespace brUGE
{
namespace render
{
	
	//------------------------------------------
	brOGLRenderDevice::brOGLRenderDevice()
		: swapChain_(NULL)
	{
		
	}
	
	//------------------------------------------
	brOGLRenderDevice::~brOGLRenderDevice()
	{

	}
	
	//------------------------------------------
	void brOGLRenderDevice::init(HWND hWindow, const brVideoMode &videoMode)
	{
		swapChain_ = std::auto_ptr<brOGLSwapChain>(new brOGLSwapChain(hWindow, videoMode));

		//if (!_checkExtensions())
		//	throw brOGLException("brOGLRenderContext", "OpenGL version is less than 3.0");

		// Вывести инфу о OpenGL?
#if 0
		FATAL_F("OpenGL version %s", glGetString(GL_VERSION));
		WARNING("Extensions:");
		char* token = strtok((char*)glGetString(GL_EXTENSIONS), " "); 
		while ( token != NULL )
		{
			LOG(token);
			token = strtok(NULL, " ");
		}
#endif

	}
	
	//------------------------------------------
	void brOGLRenderDevice::shutDown()
	{
		// TODO: удаление всемозможных данных.
	}
	
	//------------------------------------------
	brPtr<ibrFont> brOGLRenderDevice::createFontObject()
	{
		return new brOGLFont;
	}

	//------------------------------------------
	void brOGLRenderDevice::swapBuffers()
	{
		swapChain_->swapBuffers();
	}
	
	//------------------------------------------
	void brOGLRenderDevice::setBuffersClearValues(
		const brColourF& colour,
		float depth,
		uint8 stencil)
	{
		glClearColor(colour.r, colour.g, colour.b, colour.a);			
		glClearDepth(depth);
		glClearStencil(stencil);
	}

	//------------------------------------------
	void brOGLRenderDevice::clearBuffers(
		bool colour /* = true */,
		bool depth /* = true */,
		bool stencil /* = true */)
	{
		GLbitfield flags = 0;

		if (colour)	 flags  = GL_COLOR_BUFFER_BIT;
		if (depth)	 flags |= GL_DEPTH_BUFFER_BIT;
		if (stencil) flags |= GL_STENCIL_BUFFER_BIT;
	
		if (flags)
			glClear(flags);
	}
	
	//------------------------------------------
	bool brOGLRenderDevice::_checkExtensions() const
	{
		if (!GLEW_VERSION_3_0)
			return false;

		return true;
	}
	
	//------------------------------------------
	bool brOGLRenderDevice::_isVertexBufferBound() const
	{
		for (uint i = 0; i < MAX_VERTEX_INPUT_SLOTS; ++i)
			if (vertexBuffers_[i])
				return true;
		
		return false;
	}
	
	//------------------------------------------
	void brOGLRenderDevice::setVertexLayout(const brPtr<const ibrVertexLayout>& layout)
	{
		brPtr<const brOGLVertexLayout> oglLayout = NULL;
		if (layout)
		{
			oglLayout = layout.castDown<const brOGLVertexLayout>();
			#ifdef _DEBUG
			if (!oglLayout)
				throw brException("brOGLRenderDevice::setVertexLayout", " dynamic_cast() return NULL.");
			#endif // _DEBUG
		}

		glClearErrors();
		if (!oglLayout)
		{
			for (uint i = 0; i < MAX_VERTEX_ATTRIBS; ++i)
				glDisableVertexAttribArray(i);
		}
		else
		{
			for (uint i = 0; i < MAX_VERTEX_ATTRIBS; ++i)
			{
				if (oglLayout->activated_[i])
					glEnableVertexAttribArray(i);
				else
					glDisableVertexAttribArray(i);
			}
		}

		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
			throw brException("brOGLRenderDevice::setVertexLayout", "An OpenGL error has occurred.");
		
		vertexLayout_ = oglLayout;
	}

	//------------------------------------------
	void brOGLRenderDevice::setVertexBuffer(
		uint inputSlot,
		const brPtr<const ibrVertexBuffer>& buffer,
		uint offset)
	{
		brPtr<const brGLVertexBuffer> oglBuffer = NULL;
		if (buffer)
		{
			oglBuffer = buffer.castDown<const brGLVertexBuffer>();
			#ifdef _DEBUG
			if (!oglBuffer)
				throw brException("brOGLRenderDevice::setVertexBuffer", " dynamic_cast() return NULL.");
			#endif // _DEBUG
		}

		if (inputSlot > MAX_VERTEX_INPUT_SLOTS)
			throw brException("brOGLRenderDevice::setVertexBuffer", "Invalid <inputSlot> parameter.");

		if (!vertexLayout_ && oglBuffer)	
			throw brException("brOGLRenderDevice::setVertexBuffer", "Invalid current vertex layout.");

		glClearErrors();
		if (!oglBuffer)
		{
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		else
		{
			const brOGLVertexLayout::AttribMap& attribMap = vertexLayout_->inputSlots_[inputSlot];
			if (attribMap.size() == 0)
				throw brException("brOGLRenderDevice::setVertexBuffer",
					"Invalid <inputSlot> index for current vertex layout.");

			glBindBuffer(GL_ARRAY_BUFFER, oglBuffer->bufferId_);
			for (brOGLVertexLayout::AttribMapConstIter iter = attribMap.begin(); iter != attribMap.end(); ++iter)
			{
				glVertexAttribPointer(
					iter->first,
					iter->second.size,
					iter->second.type,
					iter->second.normalized, 
					vertexLayout_->strides_[inputSlot],
					iter->second.offset + offset
					);
			}
		}

		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
			throw brException("brOGLRenderDevice::setVertexBuffer", "An OpenGL error has occurred.");

		vertexBuffers_[inputSlot] = oglBuffer;
	}

	//------------------------------------------
	void brOGLRenderDevice::setIndexBuffer(
		const brPtr<const ibrIndexBuffer>& buffer)
	{
		brPtr<const brGLIndexBuffer> oglBuffer = NULL;
		if (buffer)
		{
			oglBuffer = buffer.castDown<const brGLIndexBuffer>();
			#ifdef _DEBUG
			if (!oglBuffer)
				throw brException("brOGLRenderDevice::setIndexBuffer", "dynamic_cast() return NULL.");
			#endif // _DEBUG
		}

		glClearErrors();
		if (!oglBuffer)
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		else 
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oglBuffer->bufferId_);

		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
			throw brException("brOGLRenderDevice::setIndexBuffer", "An OpenGL error has occurred.");

		indexBuffer_ = oglBuffer;
	}

	//------------------------------------------
	void brOGLRenderDevice::setShadingProgram(
		const brPtr<const ibrShadingProgram>& program)
	{
		brPtr<const brGLCgShadingProgram> oglProgram = NULL;
		if (program)
		{
			oglProgram = program.castDown<const brGLCgShadingProgram>();
			#ifdef _DEBUG
			if (!oglProgram)
				throw brException("brOGLRenderDevice::setShadingProgram", " dynamic_cast() return NULL.");
			#endif // _DEBUG
		}

		// Это программа уже выбрана текущей.
		if (shadingProgram_ == oglProgram)
			return; 
		else if (shadingProgram_)
		{
			// TODO: провести оптимизацию на включение/выключение профайлов Cg.
			shadingProgram_->disable();
		}
		
		if (oglProgram)
			oglProgram->enable();

		shadingProgram_ = oglProgram;
	}

	//------------------------------------------
	void brOGLRenderDevice::setPrimitiveTopology(ebrPrimitiveTopology topology)
	{
		switch (topology)
		{
		case PT_TRIANGLES:
			primitiveTopology_ = GL_TRIANGLES;
			break;
		case PT_LINE_LIST:
			primitiveTopology_ = GL_LINE_LOOP;
			break;
		case PT_LINE_STRIP:
			primitiveTopology_ = GL_LINE_STRIP;
			break;
		case PT_TRIANGLE_LIST:
			primitiveTopology_ = GL_TRIANGLE_FAN;
			break;
		case PT_TRIANGLE_STRIP:
			primitiveTopology_ = GL_TRIANGLE_STRIP;
			break;
		default:
			throw brException("brOGLRenderDevice::setPrimitiveTopology", "Invalid primitive topology.");
		}
	}
	
	//------------------------------------------
	void brOGLRenderDevice::drawIndexed(uint first, uint count)
	{
		if (!_isVertexBufferBound())
			throw brException("brOGLRenderDevice::draw",
				"At least one vertex buffer must be bound to perform the draw call." );

		if (!indexBuffer_)
			throw brException("brOGLRenderDevice::draw",
				"Index buffer must be bound to perform the draw call.");

		if (!shadingProgram_)
			throw brException("brOGLRenderDevice::draw",
			"shading program must be bound to perform the draw call.");

		glClearErrors();

		glDrawElements(
			primitiveTopology_,
			count,
			(indexBuffer_->getIndexType() == ibrIndexBuffer::IT_BIT16) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
			reinterpret_cast<const GLvoid*>(first * indexBuffer_->getIndexSize())
			);

		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
			throw brException("brOGLRenderDevice::draw", "An OpenGL error has occurred.");
	}
	
	//------------------------------------------
	void brOGLRenderDevice::setRasterizerState(
		const brPtr<const ibrRasterizerState>& state)
	{
		brPtr<const brGLRasterizerState> oglState = state.castDown<const brGLRasterizerState>();
#ifdef _DEBUG
			if (!oglState)
				throw brException("brOGLRenderDevice::setRasterizedState", " dynamic_cast() return NULL.");
#endif // _DEBUG
		
		// TODO: Подставлять предыдущий стейт, т.е. таким образом что бы новый стейт заменил только нужные стейты
		//		 которые изменились.
		oglState->apply(NULL);
	}
	
	//------------------------------------------
	void brOGLRenderDevice::setDepthStencilState(
		const brPtr<const ibrDepthStencilState>& state,
		uint stencilRef)
	{
		brPtr<const brGLDepthStencilState> oglState = state.castDown<const brGLDepthStencilState>();
#ifdef _DEBUG
		if (!oglState)
			throw brException("brOGLRenderDevice::setDepthStencilState", " dynamic_cast() return NULL.");
#endif // _DEBUG

		// TODO: Подставлять предыдущий стейт, т.е. таким образом что бы новый стейт заменил только нужные стейты
		//		 которые изменились.
		oglState->apply(stencilRef, NULL);
	}
	
	//------------------------------------------
	void brOGLRenderDevice::setBlendState(
		const brPtr<const ibrBlendState>& state)
	{
		brPtr<const brGLBlendState> oglState = state.castDown<const brGLBlendState>();
#ifdef _DEBUG
		if (!oglState)
			throw brException("brOGLRenderDevice::setBlendState", " dynamic_cast() return NULL.");
#endif // _DEBUG

		// TODO: Подставлять предыдущий стейт, т.е. таким образом что бы новый стейт заменил только нужные стейты
		//		 которые изменились.
		oglState->apply(NULL);
	}


} // render
} // brUGE