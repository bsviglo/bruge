#include "DxRenderDevice.hpp"
#include "DxBuffer.hpp"
#include "DxShader.hpp"

using namespace brUGE::utils;
using namespace brUGE::render;

// start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	//-- helper function, give us short write of often used conversion.
	inline DXTexture* toDxTex(const Ptr<ITexture>& iTex)
	{
		return static_cast<DXTexture*>(iTex.get());
	}

	//--
	inline UINT dxDepthStencilClearFlags(uint dsflags)
	{
		uint dxFlags = 0;
		if (dsflags & CLEAR_DEPTH)   dxFlags |= D3D10_CLEAR_DEPTH;
		if (dsflags & CLEAR_STENCIL) dxFlags |= D3D10_CLEAR_STENCIL;
		return dxFlags;
	}

	//-- see EPrimitiveTopology.
	const D3D10_PRIMITIVE_TOPOLOGY dxPrimTopology[] =
	{
		D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
		D3D10_PRIMITIVE_TOPOLOGY_LINELIST,	
		D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP		
	};
	
	//-- 
	const size_t dxVertFormatSize[] = {	1, 2, 4, 4, 4 };

	//-- see EAttributeFormat.
	const DXGI_FORMAT dxVertFormats[][4] =
	{
		DXGI_FORMAT_R8_SINT,   DXGI_FORMAT_R8G8_SINT,    DXGI_FORMAT_UNKNOWN,		  DXGI_FORMAT_R8G8B8A8_SINT,
		DXGI_FORMAT_R16_SINT,  DXGI_FORMAT_R16G16_SINT,  DXGI_FORMAT_UNKNOWN,		  DXGI_FORMAT_R16G16B16A16_SINT,
		DXGI_FORMAT_R32_SINT,  DXGI_FORMAT_R32G32_SINT,  DXGI_FORMAT_R32G32B32_SINT,  DXGI_FORMAT_R32G32B32A32_SINT,
		DXGI_FORMAT_R32_UINT,  DXGI_FORMAT_R32G32_UINT,  DXGI_FORMAT_R32G32B32_UINT,  DXGI_FORMAT_R32G32B32A32_UINT,
		DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT
	};
	
	//--
	struct DXSemantic
	{
		const char* name;
		uint index;
	};

	//-- see EAttributeType.
	const DXSemantic dxSemantics[] =
	{
		{"POSITION", 0},
		{"TEXCOORD", 0},
		{"TEXCOORD", 1},
		{"TEXCOORD", 2},
		{"NORMAL",   0},
		{"TANGENT",  0},
		{"BINORMAL", 0},
		{"COLOR",    0},
	};
}
//--------------------------------------------------------------------------------------------------
// end unnamed namespace.


namespace brUGE
{
namespace render
{
	//-- static handle to d3d10 device.
	/*static*/ DXDevice DXRenderDevice::m_dxDevice = NULL;
	
	//------------------------------------------
	DXRenderDevice::DXRenderDevice() : m_dxgiSwapChain(NULL)
	{

	}

	//------------------------------------------
	DXRenderDevice::~DXRenderDevice()
	{

	}

	//-- tries to initialize device. The initialization performs in few steps. If one of them fails,
	//-- then stops further steps and go out with the false result.
	//------------------------------------------
	bool DXRenderDevice::doInit(HWND hWindow, const VideoMode& videoMode)
	{
		//-- 1. save the video mode and the window handle.
		m_videoMode = videoMode;
		m_hWnd		= hWindow;

		HRESULT hr = S_FALSE;
	
		//-- 2. configure the swap chain.
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));

		sd.BufferCount							= 1;
		sd.BufferDesc.Width						= m_videoMode.width;
		sd.BufferDesc.Height					= m_videoMode.height;
		sd.BufferDesc.Format					= DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		sd.BufferDesc.RefreshRate.Numerator		= m_videoMode.frequancy; 
		sd.BufferDesc.RefreshRate.Denominator	= 1;
		sd.BufferUsage							= DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
		sd.OutputWindow							= m_hWnd;
		sd.SampleDesc.Count						= 1;
		sd.SampleDesc.Quality					= 0;
		sd.Windowed								= !m_videoMode.fullScreen;
		sd.SwapEffect							= DXGI_SWAP_EFFECT_DISCARD;
	
		//-- 3. prepare PerfHUD (optional).
		ID3D10Device* device		  = NULL;
		IDXGIAdapter* selectedAdapter = NULL; 
		D3D10_DRIVER_TYPE driverType  = D3D10_DRIVER_TYPE_HARDWARE; 

#if USE_PERFHUD
		INFO_MSG("Look for 'NVIDIA PerfHUD' adapter and if it is present, override default settings.");

		IDXGIFactory *pDXGIFactory; 

		hr = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&pDXGIFactory)); 
		if (FAILED(hr))
		{
			ERROR_MSG("Can't create dxgi factory.");
			return false;
		}

		//-- Search for a PerfHUD adapter.   
		UINT nAdapter		  = 0; 
		IDXGIAdapter* adapter = NULL; 

		while (pDXGIFactory->EnumAdapters(nAdapter, &adapter) != DXGI_ERROR_NOT_FOUND) 
		{ 
			if (adapter) 
			{ 
				DXGI_ADAPTER_DESC adaptDesc; 
				if (SUCCEEDED(adapter->GetDesc(&adaptDesc))) 
				{ 
					bool isPerfHUD = wcscmp(adaptDesc.Description, L"NVIDIA PerfHUD") == 0; 

					// Select the first adapter in normal circumstances or the PerfHUD one if it exists. 
					if (nAdapter == 0 || isPerfHUD)
						selectedAdapter = adapter; 

					if (isPerfHUD)
						driverType = D3D10_DRIVER_TYPE_REFERENCE; 
				} 
			} 
			++nAdapter; 
		} 

		INFO_MSG("'NVIDIA PerfHUD' adapter successfully founded.");
#endif //-- USE_PERFHUD

		//-- 4. create swap chain and device.
		hr = D3D10CreateDeviceAndSwapChain(
			selectedAdapter,
			driverType,
			NULL,
#if defined(_DEBUG) || USE_FORCE_DEBUG_MODE
			D3D10_CREATE_DEVICE_DEBUG,
#else
			0,
#endif
			D3D10_SDK_VERSION,
			&sd,
			&m_dxgiSwapChain,
			&device
			);

		if (FAILED(hr))
		{
			ERROR_MSG("Can't create d3d10 device or swap chain.");
			return false;
		}

		//-- 5. capture device to smart object. Now he is responsible for the device.	
		m_dxDevice = device;
		
		//-- 6. extract back buffer texture from swap chain and prepare it.
		{
			ComPtr<ID3D10Texture2D> pBackBuffer;
			hr = m_dxgiSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D),
				reinterpret_cast<void**>(&pBackBuffer));
			if (FAILED(hr) || m_dxDevice.hasError())
			{
				ERROR_MSG("No access to the hardware back buffer.\nDesc: " + m_dxDevice.getErrorDesc());
				return false;
			}

			ITexture::Desc desc;
			desc.width			= m_videoMode.width;
			desc.height			= m_videoMode.height;
			desc.sample.count	= 1;
			desc.sample.quality = 0;
			desc.texType   		= ITexture::TYPE_2D;
			desc.format    		= ITexture::FORMAT_RGBA8_sRGB;
			desc.bindFalgs 		= ITexture::BIND_RENDER_TARGET | ITexture::BIND_SHADER_RESOURCE;
			
			Ptr<DXTexture> texture = new DXTexture(desc);
			if (!texture->init(pBackBuffer))
			{
				return false;
			}

			m_mainColorRT = texture;
		}
		
		//-- 7. create depth stencil buffer if needed.
		{
			ITexture::Desc desc;
			desc.width			= m_videoMode.width;
			desc.height			= m_videoMode.height;
			desc.sample.count	= 1;
			desc.sample.quality = 0;
			desc.texType		= ITexture::TYPE_2D;
			desc.format			= ITexture::FORMAT_D32F;
			desc.bindFalgs		= ITexture::BIND_DEPTH_STENCIL | ITexture::BIND_SHADER_RESOURCE;

			Ptr<DXTexture> texture = new DXTexture(desc);
			if (!texture->init())
			{
				return false;
			}
			
			m_mainDepthRT = texture;
		}
		
		//-- 8. set just now created back and depth buffers as render targets to device.
		{
			ID3D10RenderTargetView* rts = toDxTex(m_mainColorRT)->getRTView();
			m_dxDevice->OMSetRenderTargets(1, &rts,	toDxTex(m_mainDepthRT)->getDSView());
		}
		
		//-- 9. setup view port.
		m_dxCurViewPort.TopLeftX = 0;		
		m_dxCurViewPort.TopLeftY = 0;		
		m_dxCurViewPort.MinDepth = 0;
		m_dxCurViewPort.MaxDepth = 1;
		m_dxCurViewPort.Width    = 0;
		m_dxCurViewPort.Height   = 0;

		//-- 10. everything is ok!
		return true;
	}

	//------------------------------------------
	void DXRenderDevice::doShutDown()
	{
		if (m_dxDevice.isValid())
		{
			m_dxDevice->ClearState();
			m_dxDevice.reset();
		}
	}
	
	//------------------------------------------
	void DXRenderDevice::doSwapBuffers()
	{
		m_dxgiSwapChain->Present(0, 0);

#if defined(_DEBUG) || USE_FORCE_DEBUG_MODE
		if (m_dxDevice.hasError())
			ERROR_MSG("DXGI present failed.\nDesc: " + m_dxDevice.getErrorDesc());
#endif // _DEBUG
	}

	//------------------------------------------
	void DXRenderDevice::doResetToDefaults()
	{
		// ToDo:
		DXShader::resetToDefaults();
	}

	//------------------------------------------
	void DXRenderDevice::doSetViewPort(uint width, uint height)
	{
		if (m_dxCurViewPort.Width != width || m_dxCurViewPort.Height != height)
		{
			m_dxCurViewPort.Width  = width;
			m_dxCurViewPort.Height = height;
			m_dxDevice->RSSetViewports(1, &m_dxCurViewPort);
		}
	}

	//------------------------------------------
	void DXRenderDevice::doSetScissorRect(uint x, uint y, uint width, uint height)
	{
		D3D10_RECT rect;
		rect.left   = x;
		rect.right  = x + width;
		rect.top	= y;
		rect.bottom = y + height;

		m_dxDevice->RSSetScissorRects(1, &rect);
	}

	//------------------------------------------
	void DXRenderDevice::doClear(uint clearFlags, const Color& color, float depth, uint8 stencil)
	{
		if (m_mainColorRT && clearFlags & CLEAR_COLOR)
		{
			m_dxDevice->ClearRenderTargetView(toDxTex(m_mainColorRT)->getRTView(), color.toPtr());
		}

		if (m_mainDepthRT && (clearFlags & CLEAR_DEPTH || clearFlags & CLEAR_STENCIL))
		{
			m_dxDevice->ClearDepthStencilView(
				toDxTex(m_mainDepthRT)->getDSView(),
				dxDepthStencilClearFlags(clearFlags), depth, stencil
				);
		}
	}

	//------------------------------------------
	void DXRenderDevice::doClearColorRT(ITexture* crt, const Color& color)
	{
		m_dxDevice->ClearRenderTargetView(toDxTex(crt)->getRTView(), color.toPtr());
	}

	//------------------------------------------
	void DXRenderDevice::doClearDepthStencilRT(uint clearFlags, ITexture* dsrt, float depth, uint8 stencil)
	{
		m_dxDevice->ClearDepthStencilView(
			toDxTex(dsrt)->getDSView(), dxDepthStencilClearFlags(clearFlags),
			depth, stencil
			);
	}

	//------------------------------------------
	void DXRenderDevice::_drawCommon(EPrimitiveTopology topology, bool indexed/* = true*/)
	{
		//-- 1. set render targets.
		if (m_isRTsChangeStateDirty)
		{
			m_dxCurDepthRT = m_curRTs.depth ? toDxTex(m_curRTs.depth)->getDSView() : NULL;

			for (uint i = 0; i < MAX_MRTS; ++i)
				m_dxCurColorRTs[i] = m_curRTs.colors[i] ? toDxTex(m_curRTs.colors[i])->getRTView() : NULL;

			m_dxDevice->OMSetRenderTargets(MAX_MRTS, m_dxCurColorRTs, m_dxCurDepthRT);
			m_isRTsChangeStateDirty = false;
		}
		
		//-- 2. set render states.
		{
			m_dxDevice->RSSetState(m_dxRasterStates[m_curRasterState]);
			m_dxDevice->OMSetDepthStencilState(m_dxDepthStates[m_curDepthState.id], m_curDepthState.stencilRef);
			m_dxDevice->OMSetBlendState(m_dxBlendStates[m_curBlendState.id], m_curBlendState.factor, m_curBlendState.sampleMask);
		}
		
		//-- 3. set input layout, primitive topology, index- and vertex-buffers.
		{
			m_dxDevice->IASetInputLayout(m_dxVertLayouts[m_curVertLayout]);

			m_dxDevice->IASetPrimitiveTopology(dxPrimTopology[topology]);

			if (indexed)
			{
				m_dxDevice->IASetIndexBuffer(static_cast<DXBuffer*>(m_curIB)->getBuffer(),
					(m_curIB->getElemSize() == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
			}

			for (uint i = 0; i < MAX_VERTEX_STREAMS; ++i)
			{
				const VertexBufferStream& vbs = m_curVBStreams[i];
				if (vbs.buffer)
				{
					m_dxCurVBStreams[i]		   = static_cast<DXBuffer*>(vbs.buffer)->getBuffer();
					m_dxCurVBStreamsOffsets[i] = vbs.offset;
					m_dxCurVBStreamsStrides[i] = vbs.buffer->getElemSize();
				}
			}
			m_dxDevice->IASetVertexBuffers(0, m_curVBStreamsCount, m_dxCurVBStreams,
				m_dxCurVBStreamsStrides, m_dxCurVBStreamsOffsets);
		}
	
		//-- 4. set shader program.
		{
			assert(m_curShader != NULL && "can't perform drawing operation without bounded shader.");
			static_cast<DXShader*>(m_curShader)->bind();
		}
	}
	
	//------------------------------------------
	void DXRenderDevice::doDraw(EPrimitiveTopology topology, uint first, uint count)
	{
		//-- do common draw setup.
		_drawCommon(topology, false);

		//-- finally do actual drawing.
		m_dxDevice->Draw(count, first);

#if defined(_DEBUG) || USE_FORCE_DEBUG_MODE
		if (m_dxDevice.hasError())
			ERROR_MSG("drawing operation failed.\nDesc: " + m_dxDevice.getErrorDesc());
#endif // _DEBUG
	}

	//------------------------------------------
	void DXRenderDevice::doDrawIndexed(EPrimitiveTopology topology, uint first, uint count)
	{
		//-- do common draw setup.
		_drawCommon(topology, true);

		//-- finally do actual drawing.
		m_dxDevice->DrawIndexed(count, first, 0);

#if defined(_DEBUG) || USE_FORCE_DEBUG_MODE
		if (m_dxDevice.hasError())
			ERROR_MSG("drawing operation failed.\nDesc: " + m_dxDevice.getErrorDesc());
#endif // _DEBUG
	}

	//------------------------------------------
	void DXRenderDevice::doDrawInstanced(
		EPrimitiveTopology topology, uint first, uint count, uint instanceCount)
	{
		//-- do common draw setup.
		_drawCommon(topology, false);
		
		//-- finally do actual drawing.
		m_dxDevice->DrawInstanced(count, instanceCount, first, 0);

#if defined(_DEBUG) || USE_FORCE_DEBUG_MODE
		if (m_dxDevice.hasError())
			ERROR_MSG("drawing operation failed.\nDesc: " + m_dxDevice.getErrorDesc());
#endif // _DEBUG
	}

	//------------------------------------------
	void DXRenderDevice::doDrawIndexedInstanced(
		EPrimitiveTopology topology, uint first, uint count, uint instanceCount)
	{
		//-- do common draw setup.
		_drawCommon(topology, true);

		//-- finally do actual drawing.
		m_dxDevice->DrawIndexedInstanced(count, instanceCount, first, 0, 0);

#if defined(_DEBUG) || USE_FORCE_DEBUG_MODE
		if (m_dxDevice.hasError())
			ERROR_MSG("drawing operation failed.\nDesc: " + m_dxDevice.getErrorDesc());
#endif // _DEBUG
	}

	//------------------------------------------
	Ptr<IBuffer> DXRenderDevice::doCreateBuffer(
		IBuffer::EType type, const void* data, uint elemCount, uint elemSize,
		IBuffer::EUsage usage, IBuffer::ECPUAccess cpuAccess)
	{
		Ptr<DXBuffer> dxBuffer = new DXBuffer(type, usage, cpuAccess);
		if (dxBuffer->init(data, elemCount, elemSize))
			return dxBuffer;

		return NULL;
	}

	//------------------------------------------
	Ptr<ITexture> DXRenderDevice::doCreateTexture(
		const ITexture::Desc& desc, const ITexture::Data* data, uint size)
	{
		Ptr<DXTexture> texture = new DXTexture(desc);
		if (texture->init(data, size))
			return texture;

		return NULL;
	}

	//------------------------------------------
	Ptr<IShader> DXRenderDevice::doCreateShader(const char* vs, const char* gs, const char* fs,
		const ShaderMacro* macros, uint mCount)
	{
		Ptr<DXShader> shader = new DXShader(*this);
		if (shader->init(vs, gs, fs, macros, mCount))
			return shader;

		return NULL;
	}

	//------------------------------------------
	void DXRenderDevice::doSetShaderIncludes(IShaderInclude* si)
	{
		if (si != NULL)
		{
			m_shaderIncludes.reset(new DXShaderIncludes(si)); 
		}
		else
		{
			m_shaderIncludes.reset();
		}
	}
	
	//------------------------------------------
	DepthStencilStateID DXRenderDevice::doCreateDepthStencilState(const DepthStencilStateDesc& desc)
	{
		DXDepthStencilState state;
		if (state.create(desc))
		{
			m_dxDepthStates.push_back(state.state);
			return (m_dxDepthStates.size() - 1);
		}
		return INVALID_ID;
	}
	
	//------------------------------------------
	RasterizerStateID DXRenderDevice::doCreateRasterizedState(const RasterizerStateDesc& desc)
	{
		DXRasterizerState state;
		if (state.create(desc))
		{
			m_dxRasterStates.push_back(state.state);
			return (m_dxRasterStates.size() - 1);
		}
		return INVALID_ID;
	}

	//------------------------------------------
	BlendStateID DXRenderDevice::doCreateBlendState(const BlendStateDesc& desc)
	{
		DXBlendState state;
		if (state.create(desc))
		{
			m_dxBlendStates.push_back(state.state);
			return (m_dxBlendStates.size() - 1);
		}
		return INVALID_ID;
	}

	//------------------------------------------
	SamplerStateID DXRenderDevice::doCreateSamplerState(const SamplerStateDesc& desc)
	{
		DXSamplerState state;
		if (state.create(desc))
		{
			m_dxSamplerStates.push_back(state.state);
			return (m_dxSamplerStates.size() - 1);
		}
		return INVALID_ID;
	}

	//------------------------------------------
	VertexLayoutID DXRenderDevice::doCreateVertexLayout(
		const VertexDesc* vd, uint count, const IShader& shader)
	{
		std::vector<D3D10_INPUT_ELEMENT_DESC> dxDescs(count);
		D3D10_INPUT_ELEMENT_DESC oDesc;
		size_t size = 0;

		for (uint i = 0; i < count; ++i)
		{
			const VertexDesc& iDesc = vd[i];

			oDesc.InputSlot			= iDesc.stream;
			oDesc.SemanticName		= dxSemantics[iDesc.type].name;
			oDesc.SemanticIndex		= dxSemantics[iDesc.type].index;
			oDesc.Format			= dxVertFormats[iDesc.format][iDesc.size - 1];
			oDesc.AlignedByteOffset	= size;
			
			// ToDo: reconsider with respect to instancing.
			oDesc.InputSlotClass		= D3D10_INPUT_PER_VERTEX_DATA; 
			oDesc.InstanceDataStepRate	= 0;

			dxDescs[i] = oDesc;
			
			//-- calculate offset.
			size += dxVertFormatSize[iDesc.format] * iDesc.size;
		}
		
		ComPtr<ID3D10InputLayout> dxLayout;
		ID3D10Blob* inputSignature = static_cast<const DXShader*>(&shader)->getInputSignature();

		HRESULT hr = m_dxDevice->CreateInputLayout(&dxDescs[0],	dxDescs.size(),
			inputSignature->GetBufferPointer(),	inputSignature->GetBufferSize(), &dxLayout);

		if (FAILED(hr) || m_dxDevice.hasError())
		{
			ERROR_MSG("Failed to create vertex layout.\nDesc: " + m_dxDevice.getErrorDesc());
			return INVALID_ID;
		}

		m_dxVertLayouts.push_back(dxLayout);
		return (m_dxVertLayouts.size() - 1);
	}

} // render
} // brUGE