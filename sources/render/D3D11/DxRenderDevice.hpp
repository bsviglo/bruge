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
		
		ID3DInclude*						shaderInclude() { return m_shaderIncludes.get(); }
		ID3D11SamplerState*					getSamplerState(SamplerStateID id) { return m_dxSamplerStates[id]; }
		static DXDevice&					device() { return m_dxDevice; }	

	protected:
		virtual bool						doInit(HWND hWindow, const VideoMode& videoMode);
		virtual void						doShutDown(); 

		virtual void						doSwapBuffers();
		virtual void						doResetToDefaults();
		virtual void						doSetViewPort(uint x, uint y, uint width, uint height);
		virtual void						doSetScissorRect(uint x, uint y, uint width, uint height);

		virtual void						doCopyTexture(ITexture* src, ITexture* dst);

		virtual void						doClear(uint clearFlags, const Color& color, float depth, uint8 stencil);
		virtual void						doClearColorRT(ITexture* crt, const Color& color);
		virtual void						doClearDepthStencilRT(uint clearFlags, ITexture* dsrt, float depth, uint8 stencil);

		virtual void						doDraw(EPrimitiveTopology topology, uint first, uint count);
		virtual void						doDrawIndexed(EPrimitiveTopology topology, uint first, uint baseVertex, uint count);
		virtual void						doDrawInstanced(EPrimitiveTopology topology, uint first, uint count, uint instanceCount);
		virtual void						doDrawIndexedInstanced(EPrimitiveTopology topology, uint first, uint baseVertex, uint count, uint instanceCount);

		virtual std::shared_ptr<IBuffer>	doCreateBuffer(IBuffer::EType type, const void* data, uint elemCount, uint elemSize, IBuffer::EUsage usage, IBuffer::ECPUAccess access);
		virtual std::shared_ptr<ITexture>	doCreateTexture(const ITexture::Desc& desc, const ITexture::Data* data, uint size);
		virtual std::shared_ptr<IShader>	doCreateShader(const char* fsData, const char* vsData, const char* gsData, const ShaderMacro* macros, uint mCount);
		virtual void						doSetShaderIncludes(IShaderInclude* si);

		virtual DepthStencilStateID			doCreateDepthStencilState(const DepthStencilStateDesc& desc);
		virtual RasterizerStateID			doCreateRasterizedState(const RasterizerStateDesc& desc);
		virtual BlendStateID				doCreateBlendState(const BlendStateDesc& desc);
		virtual SamplerStateID				doCreateSamplerState(const SamplerStateDesc& desc);
		virtual VertexLayoutID				doCreateVertexLayout(const VertexDesc* vd, uint count, const IShader& shader);

	private:
		void _drawCommon(EPrimitiveTopology topology, bool indexed = true);

	private:
		typedef std::vector<ComPtr<ID3D11DepthStencilState> > DepthStencilStates;
		typedef std::vector<ComPtr<ID3D11BlendState> >		  BlendStates;
		typedef std::vector<ComPtr<ID3D11RasterizerState> >	  RasterizerStates;
		typedef std::vector<ComPtr<ID3D11SamplerState> >	  SamplerStates;
		typedef std::vector<ComPtr<ID3D11InputLayout> >		  InputLayouts;

		ComPtr<IDXGISwapChain>							m_dxgiSwapChain;
		DepthStencilStates  							m_dxDepthStates;
		BlendStates		   								m_dxBlendStates;
		RasterizerStates								m_dxRasterStates;
		SamplerStates									m_dxSamplerStates;
		InputLayouts									m_dxVertLayouts;
		
		std::array<ID3D11RenderTargetView*, MAX_MRTS>	m_dxCurColorRTs;
		ID3D11DepthStencilView* 						m_dxCurDepthRT;
		D3D11_VIEWPORT									m_dxCurViewPort;

		std::array<ID3D11Buffer*, MAX_VERTEX_STREAMS>	m_dxCurVBStreams;
		std::array<UINT, MAX_VERTEX_STREAMS>		 	m_dxCurVBStreamsStrides;
		std::array<UINT, MAX_VERTEX_STREAMS>		 	m_dxCurVBStreamsOffsets;

		std::unique_ptr<DXShaderIncludes>				m_shaderIncludes;

		static DXDevice									m_dxDevice;
	};
	

	//-- shortcut.
	inline DXDevice& dxDevice() { return DXRenderDevice::device(); }


	//----------------------------------------------------------------------------------------------
	struct DXRasterizerState
	{
		bool create(const RasterizerStateDesc& desc);
		ComPtr<ID3D11RasterizerState> state;
	};


	//----------------------------------------------------------------------------------------------
	struct DXDepthStencilState
	{
		bool create(const DepthStencilStateDesc& desc);
		ComPtr<ID3D11DepthStencilState> state;
	};


	//----------------------------------------------------------------------------------------------
	struct DXBlendState
	{
		bool create(const BlendStateDesc& desc);
		ComPtr<ID3D11BlendState> state;
	};


	//----------------------------------------------------------------------------------------------
	struct DXSamplerState
	{
		bool create(const SamplerStateDesc& desc);
		ComPtr<ID3D11SamplerState> state;
	};

} // render
} // brUGE
