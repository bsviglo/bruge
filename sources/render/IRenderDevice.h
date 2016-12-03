#pragma once

#include "render_common.h"
#include "ITexture.h"
#include "IBuffer.h"
#include "IShader.h"
#include "Color.h"

namespace brUGE
{
namespace render
{
	class  RenderSystem;
	struct DepthStencilStateDesc;
	struct RasterizerStateDesc;
	struct BlendStateDesc;
	struct SamplerStateDesc;

	//--
	enum EAttributeType
	{
		TYPE_BYTE  = 0,
		TYPE_SHORT = 1,
		TYPE_INT   = 2,
		TYPE_UINT  = 3,
		TYPE_FLOAT = 4,
		TYPE_UNKNOWN
	};

	//--
	enum EAttributeSemantic
	{
		SEMANTIC_POSITION  = 0,
		SEMANTIC_TEXCOORD  = 1,
		SEMANTIC_TEXCOORD0 = 1,
		SEMANTIC_TEXCOORD1 = 2,
		SEMANTIC_TEXCOORD2 = 3,
		SEMANTIC_NORMAL    = 4,
		SEMANTIC_TANGENT   = 5,
		SEMANTIC_BINORMAL  = 6,
		SEMANTIC_COLOR	   = 7,
		SEMANTIC_UNKNOWN
	};

	//-- input data description to vertex shader.
	struct VertexDesc 
	{
		uint			   stream;
		EAttributeSemantic semantic;
		EAttributeType     type;
		uint			   size;
	};
	
	//--
	enum EPrimitiveTopology
	{
		PRIM_TOPOLOGY_TRIANGLE_LIST,
		PRIM_TOPOLOGY_TRIANGLE_STRIP,
		PRIM_TOPOLOGY_LINE_LIST,
		PRIM_TOPOLOGY_LINE_STRIP
	};

	//--
	enum EClearFlags
	{
		CLEAR_COLOR	  = 1 << 0,
		CLEAR_DEPTH	  = 1 << 1,
		CLEAR_STENCIL = 1 << 2,
		CLEAR_ALL	  = CLEAR_COLOR | CLEAR_DEPTH | CLEAR_STENCIL
	};
	
	//
	//----------------------------------------------------------------------------------------------
	class IRenderDevice
	{
		friend class RenderSystem;

	public:
		
		//-- global constants.
		//-- Note: don't change without reconsideration.
		enum
		{
			MAX_VERTEX_STREAMS	=  4, //-- max vertex streams.
			MAX_MRTS			=  8, //-- max render targets.
			MAX_TEXTURE_UNITS	= 16,
			MAX_SAMPLER_STATES	= 16
		};

	public:
		bool			init(HWND hWindow, const VideoMode& videoMode);
		void			shutDown() { doShutDown(); }
		
		//-- frame buffer operations.
		void			setViewPort(uint x, uint y, uint width, uint height) { doSetViewPort(x, y, width, height); }
		void			setScissorRect(uint x, uint y, uint width, uint height) { doSetScissorRect(x, y, width, height); }
		void			swapBuffers();
		void			resetToDefaults();
		
		//-- set of clear methods.
		void			clear(uint clearFlags, const Color& color, float depth, uint8 stencil);
		void			clearColorRT(ITexture* rt, const Color& color);
		void			clearDepthStencilRT(uint clearFlags, ITexture* dsrt, float depth, uint8 stencil);

		//-- access to main frame buffer.
		ITexture*		getMainColorRT() { return m_mainColorRT.get(); }
		ITexture*		getMainDepthRT() { return m_mainDepthRT.get(); }

		//-- resource copying.
		void			copyTexture(ITexture* src, ITexture* dst) { doCopyTexture(src, dst); }
		
		//-- vertex operations.
		void			setVertexLayout(VertexLayoutID layout) { m_curVertLayout = layout; }
		void			setVertexBuffer(uint slot, IBuffer* buffer, uint offset = 0);
		void			setVertexBuffers(uint startSlot, IBuffer** buffers, uint count, uint offset = 0);
		void			setIndexBuffer(IBuffer* buffer) { m_curIB = buffer; }

		//-- render target operations.
		void			setRenderTarget(ITexture* colorRT, ITexture* depthRT);
		void			setRenderTargets(ITexture** colorRTs, uint numTargets, ITexture* depthRT);
		void			backToMainFrameBuffer();
		
		//-- drawing operations.
		void			draw(EPrimitiveTopology topology, uint first, uint count);
		void			drawIndexed(EPrimitiveTopology topology, uint first, uint baseVertex, uint count);
		void			drawInstanced(EPrimitiveTopology topology, uint first, uint count, uint instanceCount);
		void			drawIndexedInstanced(EPrimitiveTopology topology, uint first, uint baseVertex, uint count, uint instanceCount);

		//-- state objects.
		void			setRasterizerState(RasterizerStateID state);
		void			setDepthStencilState(DepthStencilStateID state, uint stencilRef);
		void			setBlendState(BlendStateID state, const float blendFactor[4], uint sampleMask);

		//-- shader.
		void			setShader(IShader* shader) { m_curShader = shader; }
		Ptr<IShader>	createShader(const char* src, const ShaderMacro* macros = NULL, uint count = 0);
		void			setShaderIncludes(IShaderInclude* si) { doSetShaderIncludes(si); }

		//-- vertex layout.
		VertexLayoutID createVertexLayout(const VertexDesc* vd, uint count, const IShader& shader)
			{ return doCreateVertexLayout(vd, count, shader); }

		//-- textures.
		Ptr<ITexture> createTexture(const ITexture::Desc& desc, const ITexture::Data* data = NULL, uint size = 0)
			{ return doCreateTexture(desc, data, size);	}

		//-- buffers.
		Ptr<IBuffer> createBuffer(
			IBuffer::EType type, const void* data = NULL, uint elemCount = 0, uint elemSize = 1,
			IBuffer::EUsage usage = IBuffer::USAGE_DEFAULT,
			IBuffer::ECPUAccess	cpuAccess = IBuffer::CPU_ACCESS_NONE)
			{ return doCreateBuffer(type, data, elemCount, elemSize, usage, cpuAccess); }

		//-- state objects.
		DepthStencilStateID createDepthStencilState(const DepthStencilStateDesc& desc) { return doCreateDepthStencilState(desc); }
		RasterizerStateID	createRasterizedState  (const RasterizerStateDesc& desc)   { return doCreateRasterizedState(desc); }
		BlendStateID		createBlendState	   (const BlendStateDesc& desc)		   { return doCreateBlendState(desc); }
		SamplerStateID		createSamplerState	   (const SamplerStateDesc& desc)	   { return doCreateSamplerState(desc); }	 
		
	protected:
		IRenderDevice()
			: m_curShader(NULL), m_curIB(NULL), m_useMainRTs(true), m_curVBStreamsCount(0),
			  m_curVertLayout(-1), m_curRasterState(-1), m_isRTsChangeStateDirty(true) { }
		virtual ~IRenderDevice() {}
	
		virtual bool				doInit(HWND hWindow, const VideoMode& videoMode) = 0;
		virtual void				doShutDown() = 0; 

		virtual void				doSetViewPort(uint x, uint y, uint width, uint height) = 0;
		virtual void				doSetScissorRect(uint x, uint y, uint width, uint height) = 0;
		virtual void				doSwapBuffers() = 0;
		virtual void				doResetToDefaults() = 0;

		virtual void				doCopyTexture(ITexture* src, ITexture* dst) = 0;

		virtual void				doDraw(EPrimitiveTopology topology, uint first, uint count) = 0;
		virtual void				doDrawIndexed(EPrimitiveTopology topology, uint first, uint baseVertex, uint count) = 0;
		virtual void				doDrawInstanced(EPrimitiveTopology topology, uint first, uint count, uint instanceCount) = 0;
		virtual void				doDrawIndexedInstanced(EPrimitiveTopology topology, uint first, uint baseVertex, uint count, uint instanceCount) = 0;

		virtual void				doClear(uint clearFlags, const Color& color, float depth, uint8 stencil) = 0;
		virtual void				doClearColorRT(ITexture* crt, const Color& color) = 0;
		virtual void				doClearDepthStencilRT(uint clearFlags, ITexture* dsrt, float depth, uint8 stencil) = 0;

		virtual Ptr<IBuffer>		doCreateBuffer(IBuffer::EType type, const void* data, uint elemCount, uint elemSize, IBuffer::EUsage usage, IBuffer::ECPUAccess access) = 0;
		virtual Ptr<ITexture>		doCreateTexture(const ITexture::Desc& desc, const ITexture::Data* data, uint size) = 0;
		virtual Ptr<IShader>		doCreateShader(const char* vs, const char* gs, const char* fs, const ShaderMacro* macros, uint count) = 0;
		virtual void				doSetShaderIncludes(IShaderInclude* si) = 0;

		virtual DepthStencilStateID doCreateDepthStencilState(const DepthStencilStateDesc& desc) = 0;
		virtual RasterizerStateID	doCreateRasterizedState(const RasterizerStateDesc& desc) = 0;
		virtual BlendStateID		doCreateBlendState(const BlendStateDesc& desc) = 0;
		virtual SamplerStateID		doCreateSamplerState(const SamplerStateDesc& desc) = 0;
		virtual VertexLayoutID		doCreateVertexLayout(const VertexDesc* vd, uint count, const IShader& shader) = 0;
				
		struct DepthStencilStateEx
		{
			DepthStencilStateEx() : id(-1), stencilRef(0) {}

			DepthStencilStateID id;
			uint				stencilRef;
		};

		struct BlendStateEx
		{
			BlendStateEx() : id(-1), sampleMask(0xffffffff)
				{ factor[0] = factor[1] = factor[2] = factor[3] = 0.0f; }

			BlendStateID id;
			float		 factor[4];
			uint		 sampleMask;
		};

		//-- cur states.
		VertexLayoutID		m_curVertLayout;
		DepthStencilStateEx m_curDepthState;
		RasterizerStateID	m_curRasterState;
		BlendStateEx		m_curBlendState;

		//-- vertex buffer stream.
		struct VertexBufferStream
		{
			VertexBufferStream() : buffer(NULL), offset(0) {}

			IBuffer* buffer;
			uint	 offset;
		};
		
		//-- render targets.
		struct RenderTarget
		{
			RenderTarget() : num(0), depth(NULL)
			{
				reset();
			}

			void reset()
			{
				for (uint i = 0; i < MAX_MRTS; ++i)
				{
					colors[i] = NULL;
				}
			}

			ITexture* colors[MAX_MRTS];
			uint	  num;
			ITexture* depth;
		};
	
		VertexBufferStream  m_curVBStreams[MAX_VERTEX_STREAMS];
		uint				m_curVBStreamsCount;
		
		bool				m_isRTsChangeStateDirty;
		bool				m_useMainRTs;
		RenderTarget		m_curRTs;
		IBuffer*			m_curIB;
		IShader*			m_curShader;
		
		//-- main frame buffer textures.
		Ptr<ITexture>		m_mainColorRT;
		Ptr<ITexture>		m_mainDepthRT;

		HWND				m_hWnd;
		VideoMode			m_videoMode;

		RenderStatistics	m_statistics;		
	};

} // render
} // brUGE

