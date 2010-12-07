#include "br_GLRenderDevice.h"
#include "br_GLSwapChain.h"
#include "br_GLVertexLayout.h"
#include "br_GLBuffer.h"
#include "br_GLCgShader.h"
#include "br_GLCgShadingProgram.h"
#include "br_GLStateObjects.h"
#include "br_GLTexture.h"
#include "br_GLFrameBuffer.h"
#include "utils/br_StringTokenizer.h"

namespace brUGE
{
namespace render
{
	
	//------------------------------------------
	brGLRenderDevice::brGLRenderDevice()
		:	m_swapChain(NULL),
			m_frameBuffer(NULL)
	{
		
	}
	
	//------------------------------------------
	brGLRenderDevice::~brGLRenderDevice()
	{

	}
	
	//------------------------------------------
	void brGLRenderDevice::init(HWND hWindow, const VideoMode &videoMode)
	{
		m_swapChain	= std::auto_ptr<brGLSwapChain>(new brGLSwapChain(hWindow, videoMode));

		glViewport(0, 0, videoMode.width, videoMode.height);

		// создадим фрейм буфер, для отрисовки во внеэкранный буфер.
		m_frameBuffer = std::auto_ptr<brGLFrameBuffer>();
		
#if 1 // Вывести инфу о OpenGL?
		brStr token;
		utils::StrTokenizer tokenizer((char*)glGetString(GL_EXTENSIONS), " ");

		FATAL_F("OpenGL version %s", glGetString(GL_VERSION));
		WARNING_F("Extensions %d:", tokenizer.getCountTokens());
		
		while (tokenizer.hasMoreTokens())
		{
			tokenizer.getNextToken(token);
			LOG(token.c_str());
		}
#endif

	}
	
	//------------------------------------------
	void brGLRenderDevice::shutDown()
	{
		// TODO: Продумать что подлежит удалению.
	}
	
	//------------------------------------------
	void brGLRenderDevice::swapBuffers()
	{
		m_swapChain->swapBuffers();
	}
	
	//------------------------------------------
	void brGLRenderDevice::setBuffersClearValues(
		const Color& colour,
		float depth,
		uint8 stencil)
	{
		glClearColor(colour.r, colour.g, colour.b, colour.a);			
		glClearDepth(depth);
		glClearStencil(stencil);
	}

	//------------------------------------------
	void brGLRenderDevice::clearBuffers(
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
	bool brGLRenderDevice::_isVertexBufferBound() const
	{
		for (uint i = 0; i < MAX_VERTEX_INPUT_SLOTS; ++i)
			if (m_vertexBuffers[i])
				return true;
		
		return false;
	}
	
	//------------------------------------------
	void brGLRenderDevice::setVertexLayout(const Ptr<const ibrVertexLayout>& layout)
	{
		Ptr<const brGLVertexLayout> glLayout = layout.castDown<const brGLVertexLayout>();
#ifdef _DEBUG
			if (!glLayout && layout)
				BR_EXCEPT("brGLRenderDevice::setVertexLayout", " dynamic_cast() return NULL.");
#endif // _DEBUG

		GL_CLEAR_ERRORS
		
		//  TODO: хранить кеш аттрибутов: аттр - вкл\выкл. Что бы активировоть только изменившиеся атрибуты.

		if (!glLayout)
		{
			for (uint i = 0; i < MAX_VERTEX_ATTRIBS; ++i)
				glDisableVertexAttribArray(i);
		}
		else
		{
			for (uint i = 0; i < MAX_VERTEX_ATTRIBS; ++i)
			{
				if (glLayout->m_activated[i])	glEnableVertexAttribArray(i);
				else							glDisableVertexAttribArray(i);
			}
		}

		GL_THROW_ERRORS
		
		m_vertexLayout = glLayout;
	}

	//------------------------------------------
	void brGLRenderDevice::setVertexBuffer(
		uint inputSlot,
		const Ptr<const ibrVertexBuffer>& buffer,
		uint offset)
	{
		Ptr<const brGLVertexBuffer> glBuffer = buffer.castDown<const brGLVertexBuffer>();
#ifdef _DEBUG
			if (!glBuffer && buffer)
				BR_EXCEPT("brOGLRenderDevice::setVertexBuffer", " dynamic_cast() return NULL.");
#endif // _DEBUG

#ifdef _DEBUG
		if (inputSlot > MAX_VERTEX_INPUT_SLOTS)
			BR_EXCEPT("brOGLRenderDevice::setVertexBuffer", "Invalid <inputSlot> parameter.");

		if (!m_vertexLayout && glBuffer)	
			BR_EXCEPT("brOGLRenderDevice::setVertexBuffer", "Invalid current vertex layout.");
#endif // _DEBUG

		GL_CLEAR_ERRORS

		if (!glBuffer)
		{
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		else
		{
			const brGLVertexLayout::AttribMap& attribMap = m_vertexLayout->m_inputSlots[inputSlot];
#ifdef _DEBUG
			if (attribMap.size() == 0)
				BR_EXCEPT_F("brOGLRenderDevice::setVertexBuffer",
					"Invalid <inputSlot> index = %d for current vertex layout.", inputSlot);
#endif // _DEBUG

			glBindBuffer(GL_ARRAY_BUFFER, glBuffer->getBufferId());
			for (brGLVertexLayout::AttribMapConstIter iter = attribMap.begin(); iter != attribMap.end(); ++iter)
			{
				glVertexAttribPointer(
					iter->first,
					iter->second.size,
					iter->second.type,
					iter->second.normalized, 
					m_vertexLayout->m_strides[inputSlot],
					iter->second.offset + offset
					);
			}
		}

		GL_THROW_ERRORS

		m_vertexBuffers[inputSlot] = glBuffer;
	}

	//------------------------------------------
	void brGLRenderDevice::setIndexBuffer(
		const Ptr<const ibrIndexBuffer>& buffer)
	{
		Ptr<const brGLIndexBuffer> glBuffer = buffer.castDown<const brGLIndexBuffer>();
#ifdef _DEBUG
		if (!glBuffer && buffer)
			BR_EXCEPT("brGLRenderDevice::setIndexBuffer", "dynamic_cast() return NULL.");
#endif // _DEBUG

		GL_CLEAR_ERRORS

		if (!glBuffer)	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		else 			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->getBufferId());

		GL_THROW_ERRORS

		m_indexBuffer = glBuffer;
	}

	//------------------------------------------
	void brGLRenderDevice::setShadingProgram(
		const Ptr<const ibrShadingProgram>& program)
	{
		Ptr<const brGLCgShadingProgram> glProgram = program.castDown<const brGLCgShadingProgram>();
#ifdef _DEBUG
		if (!glProgram && program)
			BR_EXCEPT("brGLRenderDevice::setShadingProgram", " dynamic_cast() return NULL.");
#endif // _DEBUG

		// Это программа уже выбрана текущей.
		if (m_shadingProgram == glProgram)
			return; 
		else if (m_shadingProgram)
		{
			// TODO: провести оптимизацию на включение/выключение профайлов Cg.
			m_shadingProgram->disable();
		}
		
		if (glProgram)
			glProgram->enable();

		m_shadingProgram = glProgram;
	}

	//------------------------------------------
	void brGLRenderDevice::setPrimitiveTopology(ebrPrimitiveTopology topology)
	{
		switch (topology)
		{
		case PT_TRIANGLES:		m_primitiveTopology = GL_TRIANGLES;			break;
		case PT_LINE_LIST:		m_primitiveTopology = GL_LINE_LOOP;			break;
		case PT_LINE_STRIP:		m_primitiveTopology = GL_LINE_STRIP;		break;
		case PT_TRIANGLE_LIST:	m_primitiveTopology = GL_TRIANGLE_FAN;		break;
		case PT_TRIANGLE_STRIP:	m_primitiveTopology = GL_TRIANGLE_STRIP;	break;
		default:
			BR_EXCEPT("brOGLRenderDevice::setPrimitiveTopology", "Invalid primitive topology.");
		}
	}
	
	//------------------------------------------
	void brGLRenderDevice::setRenderTargets(
		uint numTargets,
		const Ptr<ibrTexture>* renderTargets,
		const Ptr<ibrTexture>& depthStencilTarget)
	{
		if (!numTargets && !depthStencilTarget)
		{
			m_frameBuffer->activate(true);
		}
		else
		{
			m_frameBuffer->activate(false);
			m_frameBuffer->attachColors(
				numTargets,
				reinterpret_cast<const Ptr<brGLTexture>*>(renderTargets)
				);

			m_frameBuffer->attachDepthStencil(depthStencilTarget.castDown<brGLTexture>());
			m_frameBuffer->checkStatus();
		}
	}
	
	//------------------------------------------
	void brGLRenderDevice::drawIndexed(uint first, uint count)
	{
#ifdef _DEBUG
		if (!_isVertexBufferBound())
			BR_EXCEPT("brGLRenderDevice::drawIndexed",
				"At least one vertex buffer must be bound to perform the draw call." );

		if (!m_indexBuffer)
			BR_EXCEPT("brGLRenderDevice::drawIndexed",
				"Index buffer must be bound to perform the draw call.");

		if (!m_shadingProgram)
			BR_EXCEPT("brGLRenderDevice::drawIndexed",
				"Shading program must be bound to perform the draw call.");
#endif // _DEBUG

		GL_CLEAR_ERRORS

		glDrawElements(
			m_primitiveTopology,
			count,
			(m_indexBuffer->getIndexType() == ibrIndexBuffer::IT_BIT16) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
			reinterpret_cast<const GLvoid*>(first * m_indexBuffer->getIndexSize())
			);

		GL_THROW_ERRORS
	}
	
	//------------------------------------------
	void brGLRenderDevice::setRasterizerState(
		const Ptr<const ibrRasterizerState>& state)
	{
		Ptr<const brGLRasterizerState> glState = state.castDown<const brGLRasterizerState>();
#ifdef _DEBUG
		if (!glState)
			BR_EXCEPT("brGLRenderDevice::setRasterizedState", " dynamic_cast() return NULL.");
#endif // _DEBUG
		
		// TODO: Подставлять предыдущий стейт, т.е. таким образом что бы новый стейт заменил только нужные стейты
		//		 которые изменились.
		glState->apply(NULL);
	}
	
	//------------------------------------------
	void brGLRenderDevice::setDepthStencilState(
		const Ptr<const ibrDepthStencilState>& state,
		uint stencilRef)
	{
		Ptr<const brGLDepthStencilState> glState = state.castDown<const brGLDepthStencilState>();
#ifdef _DEBUG
		if (!glState)
			BR_EXCEPT("brGLRenderDevice::setDepthStencilState", " dynamic_cast() return NULL.");
#endif // _DEBUG

		// TODO: Подставлять предыдущий стейт, т.е. таким образом что бы новый стейт заменил только нужные стейты
		//		 которые изменились.
		glState->apply(stencilRef, NULL);
	}
	
	//------------------------------------------
	void brGLRenderDevice::setBlendState(
		const Ptr<const ibrBlendState>& state)
	{
		Ptr<const brGLBlendState> glState = state.castDown<const brGLBlendState>();
#ifdef _DEBUG
		if (!glState)
			BR_EXCEPT("brGLRenderDevice::setBlendState", " dynamic_cast() return NULL.");
#endif // _DEBUG

		// TODO: Подставлять предыдущий стейт, т.е. таким образом что бы новый стейт заменил только нужные стейты
		//		 которые изменились.
		glState->apply(NULL);
	}
	
	//------------------------------------------
	Ptr<ibrVertexBuffer> brGLRenderDevice::createVertexBuffer(
		uint vertexSize,
		uint vertexCount,
		void* data,
		ibrBuffer::ebrUsageFlag usage /* = ibrBuffer::UF_DEFAULT */,
		ibrBuffer::ebrCPUAccessFlag	cpuAccess /* = ibrBuffer::CPU_AF_NONE */)
	{
		return new brGLVertexBuffer(vertexCount, vertexSize, usage, cpuAccess, data);
	}

	//------------------------------------------		
	Ptr<ibrIndexBuffer> brGLRenderDevice::createIndexBuffer(
		ibrIndexBuffer::ebrIndexType indexType,
		uint indexCount,
		void* data,
		ibrBuffer::ebrUsageFlag usage /* = ibrBuffer::UF_DEFAULT */,
		ibrBuffer::ebrCPUAccessFlag	cpuAccess /* = ibrBuffer::CPU_AF_NONE */)
	{
		return new brGLIndexBuffer(indexCount, indexType, usage, cpuAccess, data);
	}

	//------------------------------------------
	Ptr<ibrVertexLayout> brGLRenderDevice::createVertexLayout(
		const brVertexDesc *vd,
		uint count,
		const Ptr<ibrShadingProgram>& shader )
	{
		return new brGLVertexLayout(vd, count, shader);
	}

	//------------------------------------------
	Ptr<ibrDepthStencilState> brGLRenderDevice::createDepthStencilState(
		const ibrDepthStencilState::Desc_s& desc)
	{
		return new brGLDepthStencilState(desc);
	}

	//------------------------------------------
	Ptr<ibrRasterizerState> brGLRenderDevice::createRasterizedState(
		const ibrRasterizerState::Desc_s& desc)
	{
		return new brGLRasterizerState(desc);
	}

	//------------------------------------------
	Ptr<ibrBlendState> brGLRenderDevice::createBlendState(
		const ibrBlendState::Desc_s& desc)
	{
		return new brGLBlendState(desc);
	}
	
	//------------------------------------------
	Ptr<ibrSamplerState> brGLRenderDevice::createSamplerState(
		const ibrSamplerState::Desc_s& desc)
	{
		return new brGLSamplerState(desc);
	}

	//------------------------------------------
	Ptr<ibrVertexShader> brGLRenderDevice::createVertexShader(
		ibrShader::ebrSourceType sourceType,
		const brStr& sourceStr,
		const brStr& entryPoint,
		const brStr& args
		)
	{
		return new brGLCgVertexShader(sourceType, sourceStr, entryPoint, args);
	}

	//------------------------------------------
	Ptr<ibrFragmentShader> brGLRenderDevice::createFragmentShader(
		ibrShader::ebrSourceType sourceType,
		const brStr& sourceStr,
		const brStr& entryPoint,
		const brStr& args
		)
	{
		return new brGLCgFragmentShader(sourceType, sourceStr, entryPoint, args);
	}

	//------------------------------------------
	Ptr<ibrGeometryShader> brGLRenderDevice::createGeometryShader(
		ibrShader::ebrSourceType sourceType,
		const brStr& sourceStr,
		const brStr& entryPoint,
		const brStr& args
		)
	{
		return new brGLCgGeometryShader(sourceType, sourceStr, entryPoint, args);
	}

	//------------------------------------------
	Ptr<ibrShadingProgram> brGLRenderDevice::createShadingProgram()
	{
		return new brGLCgShadingProgram;
	}

	
	//------------------------------------------
	Ptr<ibrTexture> brGLRenderDevice::createTexture(
		const ibrTexture::Desc_s& desc,
		const void* data /* = NULL */)
	{
		return new brGLTexture(desc, data);
	}

} // render
} // brUGE