#pragma once

#include "Dx_common.hpp"
#include "DxDevice.hpp"
#include "DxShader.hpp"
#include "render/IRenderDevice.h"

namespace brUGE
{
namespace render
{

	//
	//----------------------------------------------------------------------------------------------
	class DXRenderDevice : public IRenderDevice
	{
	public:
		DXRenderDevice();
		virtual ~DXRenderDevice();
		
		ID3D10Include*				shaderInclude() { return m_shaderIncludes.get(); }
		ID3D10SamplerState*			getSamplerState(SamplerStateID id) { return m_dxSamplerStates[id]; }
		static DXDevice&			device() { return m_dxDevice; }	

	protected:
		virtual bool				doInit(HWND hWindow, const VideoMode& videoMode);
		virtual void				doShutDown(); 

		virtual void				doSwapBuffers();
		virtual void				doResetToDefaults();
		virtual void				doSetViewPort(uint x, uint y, uint width, uint height);
		virtual void				doSetScissorRect(uint x, uint y, uint width, uint height);

		virtual void				doCopyTexture(ITexture* src, ITexture* dst);

		virtual void				doClear(uint clearFlags, const Color& color, float depth, uint8 stencil);
		virtual void				doClearColorRT(ITexture* crt, const Color& color);
		virtual void				doClearDepthStencilRT(uint clearFlags, ITexture* dsrt, float depth, uint8 stencil);

		virtual void				doDraw(EPrimitiveTopology topology, uint first, uint count);
		virtual void				doDrawIndexed(EPrimitiveTopology topology, uint first, uint count);
		virtual void				doDrawInstanced(EPrimitiveTopology topology, uint first, uint count, uint instanceCount);
		virtual void				doDrawIndexedInstanced(EPrimitiveTopology topology, uint first, uint count, uint instanceCount);

		virtual Ptr<IBuffer>		doCreateBuffer(IBuffer::EType type, const void* data, uint elemCount, uint elemSize, IBuffer::EUsage usage, IBuffer::ECPUAccess access);
		virtual Ptr<ITexture>		doCreateTexture(const ITexture::Desc& desc, const ITexture::Data* data, uint size);
		virtual Ptr<IShader>		doCreateShader(const char* fsData, const char* vsData, const char* gsData, const ShaderMacro* macros, uint mCount);
		virtual void				doSetShaderIncludes(IShaderInclude* si);

		virtual DepthStencilStateID doCreateDepthStencilState(const DepthStencilStateDesc& desc);
		virtual RasterizerStateID	doCreateRasterizedState(const RasterizerStateDesc& desc);
		virtual BlendStateID		doCreateBlendState(const BlendStateDesc& desc);
		virtual SamplerStateID		doCreateSamplerState(const SamplerStateDesc& desc);
		virtual VertexLayoutID		doCreateVertexLayout(const VertexDesc* vd, uint count, const IShader& shader);

	private:
		void _drawCommon(EPrimitiveTopology topology, bool indexed = true);

	private:
		typedef std::vector<ComPtr<ID3D10DepthStencilState> > DepthStencilStates;
		typedef std::vector<ComPtr<ID3D10BlendState> >		  BlendStates;
		typedef std::vector<ComPtr<ID3D10RasterizerState> >	  RasterizerStates;
		typedef std::vector<ComPtr<ID3D10SamplerState> >	  SamplerStates;
		typedef std::vector<ComPtr<ID3D10InputLayout> >		  InputLayouts;

		ComPtr<IDXGISwapChain>			m_dxgiSwapChain;
		DepthStencilStates  			m_dxDepthStates;
		BlendStates		   				m_dxBlendStates;
		RasterizerStates				m_dxRasterStates;
		SamplerStates					m_dxSamplerStates;
		InputLayouts					m_dxVertLayouts;
		
		ID3D10RenderTargetView* 		m_dxCurColorRTs[MAX_MRTS];
		ID3D10DepthStencilView* 		m_dxCurDepthRT;
		D3D10_VIEWPORT					m_dxCurViewPort;

		ID3D10Buffer*					m_dxCurVBStreams[MAX_VERTEX_STREAMS];
		UINT		 					m_dxCurVBStreamsStrides[MAX_VERTEX_STREAMS];
		UINT		 					m_dxCurVBStreamsOffsets[MAX_VERTEX_STREAMS];

		std::auto_ptr<DXShaderIncludes> m_shaderIncludes;

		static DXDevice					m_dxDevice;
	};
	

	//-- shortcut.
	inline DXDevice& dxDevice() { return DXRenderDevice::device(); }


	//----------------------------------------------------------------------------------------------
	struct DXRasterizerState
	{
		bool create(const RasterizerStateDesc& desc);
		ComPtr<ID3D10RasterizerState> state;
	};


	//----------------------------------------------------------------------------------------------
	struct DXDepthStencilState
	{
		bool create(const DepthStencilStateDesc& desc);
		ComPtr<ID3D10DepthStencilState> state;
	};


	//----------------------------------------------------------------------------------------------
	struct DXBlendState
	{
		bool create(const BlendStateDesc& desc);
		ComPtr<ID3D10BlendState> state;
	};


	//----------------------------------------------------------------------------------------------
	struct DXSamplerState
	{
		bool create(const SamplerStateDesc& desc);
		ComPtr<ID3D10SamplerState> state;
	};

} // render
} // brUGE
