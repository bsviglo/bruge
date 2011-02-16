#include "IRenderDevice.h"

namespace brUGE
{
namespace render
{
	//------------------------------------------
	bool IRenderDevice::init(HWND hWindow, const VideoMode& videoMode)
	{
		// reset data.
		backToMainFrameBuffer();

		return doInit(hWindow, videoMode);
	}
	
	//------------------------------------------
	void IRenderDevice::resetToDefaults()
	{
		doResetToDefaults();
	}

	//------------------------------------------
	void IRenderDevice::clear(uint clearFlags, const Color& color, float depth, uint8 stencil)
	{
		doClear(clearFlags, color, depth, stencil);
	}

	//------------------------------------------
	void IRenderDevice::clearColorRT(ITexture* crt, const Color& color)
	{
		assert(crt != 0 && "render target is not a valid texture.");
		assert(crt->getDesc().bindFalgs & ITexture::BIND_RENDER_TARGET && "texture must have BIND_RENDER_TARGET flag.");
		
		doClearColorRT(crt, color);		
	}

	//------------------------------------------
	void IRenderDevice::doClearDepthStencilRT(uint clearFlags, ITexture* dsrt, float depth, uint8 stencil)
	{
		assert(dsrt != 0 && "render target is not a valid texture.");
		assert(dsrt->getDesc().bindFalgs & ITexture::BIND_DEPTH_STENCIL && "texture must have BIND_DEPTH_STENCIL flag.");

		doClearDepthStencilRT(clearFlags, dsrt, depth, stencil);
	}

	//------------------------------------------
	void IRenderDevice::setVertexBuffer(uint slot, IBuffer* buffer, uint offset)
	{
		assert(buffer != 0 && "vertex buffer is not a valid buffer.");
		assert(buffer->getType() == IBuffer::TYPE_VERTEX && "buffer have to have TYPE_VERTEX type.");
		assert(slot < MAX_VERTEX_STREAMS && "invalid vertex buffer slot.");

		m_curVBStreams[slot].buffer = buffer;
		m_curVBStreams[slot].offset = offset;
		m_curVBStreamsCount = 1;
	}

	//------------------------------------------
	void IRenderDevice::setVertexBuffers(uint startSlot, IBuffer** buffers, uint count, uint offset/* = 0*/)
	{
		assert(buffers != 0 && "vertex buffers are not a valid buffers.");
		assert(startSlot < MAX_VERTEX_STREAMS && startSlot + count < MAX_VERTEX_STREAMS && "invalid buffer range.");

		for (uint i = 0; i < count; ++i)
		{
			IBuffer* buffer = buffers[i];

			assert(buffer != 0 && "vertex buffer is not a valid buffer.");
			assert(buffer->getType() == IBuffer::TYPE_VERTEX && "buffer have to have TYPE_VERTEX type.");

			m_curVBStreams[startSlot + i].buffer = buffer;
			m_curVBStreams[startSlot + i].offset = offset;
		}
		m_curVBStreamsCount = count;
	}

	//------------------------------------------
	void IRenderDevice::setRenderTarget(ITexture* colorRT, ITexture* depthRT)
	{
		for (uint i = 0; i < MAX_MRTS; ++i) m_curRTs.colors[i] = NULL;

		m_curRTs.colors[0] = colorRT;
		m_curRTs.num	   = 1;
		m_curRTs.depth	   = depthRT;

		m_isRTsChangeStateDirty = true;
	}

	//------------------------------------------
	void IRenderDevice::setRenderTargets(ITexture** colorRTs, uint numRTs, ITexture* depthRT)
	{
		for (uint i = 0; i < MAX_MRTS; ++i) m_curRTs.colors[i] = NULL;

		for (uint i = 0; i < numRTs; ++i)
			m_curRTs.colors[i] = colorRTs[i];

		m_curRTs.num   = numRTs;
		m_curRTs.depth = depthRT;
		
		m_useMainRTs = false;
		m_isRTsChangeStateDirty = true;
	}

	//------------------------------------------
	void IRenderDevice::backToMainFrameBuffer()
	{
		for (uint i = 0; i < MAX_MRTS; ++i) m_curRTs.colors[i] = NULL;

		m_curRTs.num       = 1;
		m_curRTs.colors[0] = m_mainColorRT.get();
		m_curRTs.depth     = m_mainDepthRT.get();
		
		m_useMainRTs = true;
		m_isRTsChangeStateDirty = true;
	}

	//------------------------------------------
	void IRenderDevice::setRasterizerState(RasterizerStateID state)
	{
		if (m_curRasterState != state)
			m_curRasterState = state;
	}

	//------------------------------------------
	void IRenderDevice::setDepthStencilState(DepthStencilStateID state, uint stencilRef)
	{
		if (m_curDepthState.id != state)
			m_curDepthState.id = state;

		m_curDepthState.stencilRef = stencilRef;
	}

	//------------------------------------------
	void IRenderDevice::setBlendState(BlendStateID state, const float blendFactor[4], uint sampleMask)
	{
		if (m_curBlendState.id != state)
			m_curBlendState.id = state;

		if (blendFactor)
		{
			m_curBlendState.factor[0] = blendFactor[0];
			m_curBlendState.factor[1] = blendFactor[1];
			m_curBlendState.factor[2] = blendFactor[2];
			m_curBlendState.factor[3] = blendFactor[3];
		}

		m_curBlendState.sampleMask = sampleMask;
	}
	
	//------------------------------------------
	void IRenderDevice::swapBuffers()
	{
		m_primitivesCount = 0;
		m_drawCallsCount  = 0;  
		doSwapBuffers();
	} 

	//------------------------------------------
	void IRenderDevice::draw(EPrimitiveTopology topology, uint first, uint count)
	{
		assert(count != 0 && "count must be a value > 0.");

		m_primitivesCount += count;
		++m_drawCallsCount;  
		doDraw(topology, first, count);
	}

	//------------------------------------------
	void IRenderDevice::drawIndexed(EPrimitiveTopology topology, uint first, uint count)
	{
		assert(count != 0 && "count must be a value > 0.");

		m_primitivesCount += count;
		++m_drawCallsCount;  
		doDrawIndexed(topology, first, count);
	}

	//------------------------------------------
	void IRenderDevice::drawInstanced(
		EPrimitiveTopology topology, uint first, uint count, uint instanceCount)
	{
		assert(count != 0 && "count must be a value > 0.");
		assert(instanceCount != 0 && "instanceCount must be a value > 0.");

		m_primitivesCount += count * instanceCount;
		++m_drawCallsCount;  
		doDrawInstanced(topology, first, count, instanceCount);
	}

	//------------------------------------------
	void IRenderDevice::drawIndexedInstanced(
		EPrimitiveTopology topology, uint first, uint count, uint instanceCount)
	{
		assert(count != 0 && "count must be a value > 0.");
		assert(instanceCount != 0 && "instanceCount must be a value > 0.");

		m_primitivesCount += count * instanceCount;
		++m_drawCallsCount;  
		doDrawIndexedInstanced(topology, first, count, instanceCount);
	}

	//------------------------------------------
	Ptr<IShader> IRenderDevice::createShader(
		const char *src, const ShaderMacro *macros, uint count)
	{
		std::string strSrc(src);
		bool vs = strSrc.find("_VERTEX_SHADER_")   != std::string::npos;
		bool gs = strSrc.find("_GEOMETRY_SHADER_") != std::string::npos;
		bool fs = strSrc.find("_FRAGMENT_SHADER_") != std::string::npos;

		if (!vs || !fs)
		{
			ERROR_MSG("Shader's code must have at least vertex and fragment shaders.");
			return NULL;
		}
		
		return doCreateShader(src, gs ? src : NULL, src, macros, count);
	}

} // render
} // brUGE