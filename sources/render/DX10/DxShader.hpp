#pragma once

#include "Dx_common.hpp"
#include "DxTexture.hpp"
#include "render/IShader.h"

#include <vector>

namespace brUGE
{
namespace render
{

	//----------------------------------------------------------------------------------------------
	class DXShaderIncludes : public ID3D10Include
	{
	public:
		DXShaderIncludes(IShaderInclude* si) : m_pimpl(si) { }
		~DXShaderIncludes() { }

		STDMETHOD(Open)(
			THIS_ D3D10_INCLUDE_TYPE /*IncludeType*/, LPCSTR pFileName,
			LPCVOID /*pParentData*/, LPCVOID *ppData, UINT *pBytes)
		{
			return m_pimpl->open(pFileName, *ppData, *pBytes);
		}
		STDMETHOD(Close)(THIS_ LPCVOID pData)
		{
			return m_pimpl->close(pData);
		}

	private:
		IShaderInclude* m_pimpl; //-- memory deallocation performed externally.
	};


	//----------------------------------------------------------------------------------------------
	class DXShader : public IShader
	{
	public:
		DXShader(IRenderDevice& rd);
		virtual ~DXShader();
		
		bool		init(const char* vs, const char* gs, const char* fs, const ShaderMacro *macros, uint mCount);
		void		bind();
		static void	resetToDefaults();
		ID3D10Blob* getInputSignature() const { return m_inputSignature.get(); }

	protected:
		virtual bool doChangeUniformBuffer(Index index, IBuffer* newBuffer);

	private:
		HRESULT _reflectShader(ID3D10Blob* byteCode, EShaderType shaderType);
		void	_updateRes();

		ComPtr<ID3D10Blob>			 m_inputSignature;
		ComPtr<ID3D10VertexShader>	 m_vs;
		ComPtr<ID3D10PixelShader>	 m_ps;
		ComPtr<ID3D10GeometryShader> m_gs;
		
		typedef std::vector<ID3D10ShaderResourceView*>	DXResources;
		typedef std::vector<ID3D10SamplerState*>		DXSamplers;
		typedef std::vector<ID3D10Buffer*>				DXBuffers;

		// resources per shader type.
		DXResources	m_dxResources[SHADER_TYPES_COUNT];
		DXSamplers 	m_dxSamplers [SHADER_TYPES_COUNT];
		DXBuffers	m_dxUBuffers [SHADER_TYPES_COUNT];
	};

} // render
} // brUGE
