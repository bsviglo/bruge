#pragma once

#include "Dx_common.hpp"
#include "DxTexture.hpp"
#include "DxBuffer.hpp"
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
		DXShader(DXRenderDevice& rd);
		virtual ~DXShader();
		
		bool			init(const char* vs, const char* gs, const char* fs, const ShaderMacro *macros, uint mCount);
		void			bind();
		static void		resetToDefaults();
		ID3D10Blob*		getInputSignature() const { return m_inputSignature.get(); }

	protected:
		virtual Handle	doGetHandleBool			(const char* name) const		{ return getHandle(m_search_constants, name); }
		virtual Handle	doGetHandleFloat		(const char* name) const		{ return getHandle(m_search_constants, name); }
		virtual Handle	doGetHandleInt			(const char* name) const		{ return getHandle(m_search_constants, name); }

		virtual Handle	doGetHandleVec2f		(const char* name) const		{ return getHandle(m_search_constants, name); }
		virtual Handle	doGetHandleVec3f		(const char* name) const		{ return getHandle(m_search_constants, name); }
		virtual Handle	doGetHandleVec4f		(const char* name) const		{ return getHandle(m_search_constants, name); }
		virtual Handle	doGetHandleMat4f		(const char* name) const		{ return getHandle(m_search_constants, name); }

		virtual Handle	doGetHandleTexture		(const char* name) const		{ return getHandle(m_search_textures,  name); }
		virtual Handle	doGetHandleUniformBlock	(const char* name) const		{ return getHandle(m_search_ubuffers,  name); }
		virtual Handle	doGetHandleTextureBuffer(const char* name) const		{ return getHandle(m_search_tbuffers,  name); }

		virtual bool	doSetBool				(Handle id, bool val)			{ return setConstantAsRawData(id, &val, sizeof(bool));  }
		virtual bool	doSetFloat				(Handle id, float val)			{ return setConstantAsRawData(id, &val, sizeof(float)); }
		virtual bool	doSetInt				(Handle id, int val)			{ return setConstantAsRawData(id, &val, sizeof(int));	}

		virtual bool	doSetVec2f				(Handle id, const vec2f& val)	{ return setConstantAsRawData(id, &val, sizeof(vec2f)); }
		virtual bool	doSetVec3f				(Handle id, const vec3f& val)	{ return setConstantAsRawData(id, &val, sizeof(vec3f)); }
		virtual bool	doSetVec4f				(Handle id, const vec4f& val)	{ return setConstantAsRawData(id, &val, sizeof(vec4f)); }
		virtual bool	doSetMat4f				(Handle id, const mat4f& val)	{ return setConstantAsRawData(id, &val, sizeof(mat4f)); }

		virtual bool	doSetTexture			(Handle id, ITexture* texture, SamplerStateID state);
		virtual bool	doSetUniformBlock		(Handle id, const void* data, uint size);
		virtual bool	doSetTextureBuffer		(Handle id, IBuffer* buffer);

		virtual bool	doChangeUniformBuffer	(Handle id, const Ptr<IBuffer>& newBuffer);

	private:
		typedef utils::TernaryTree<char, Handle> FastSearch;

		HRESULT reflectShader(ID3D10Blob* byteCode, EShaderType shaderType);
		void	updateRes();
		Handle  getHandle(const FastSearch& database, const char* name) const;
		bool	setConstantAsRawData(Handle id, const void* data, uint size);
		void	addSampler(const char* name, EShaderType sType, uint index);
		void	addTexture(const char* name, EShaderType sType, uint index);
		uint	addUBuffer(const char* name, EShaderType sType, uint size, uint index);
		void	addTBuffer(const char* name, EShaderType sType, uint index);
		void	addConstant(const char* name, uint ublock, uint offset, uint size);
		
		DXRenderDevice&				 m_device;
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

		//------------------------------------------------------------------------------------------
		struct Index
		{
			Index()
			{
				for (uint i = 0; i < SHADER_TYPES_COUNT; ++i)
					m_shaders[i] = 0;
			}

			const uint& operator[] (uint i) const { return m_shaders[i]; }
			uint&		operator[] (uint i) { return m_shaders[i]; }

			uint m_shaders[SHADER_TYPES_COUNT];
		};

		//------------------------------------------------------------------------------------------
		struct UBuffer
		{
			UBuffer() : m_size(0), m_isDirty(false) { }

			bool			  m_isDirty;
			uint			  m_size;
			std::vector<byte> m_data;
			Ptr<DXBuffer>	  m_buffer;
			Index			  m_index;
		};

		//------------------------------------------------------------------------------------------
		struct TBuffer
		{
			Index m_index;
		};

		//------------------------------------------------------------------------------------------
		struct Constant
		{
			Constant() : m_ublock(0), m_offset(0), m_size(0) { }

			uint  m_ublock;
			uint  m_offset;
			uint  m_size;
		};

		//------------------------------------------------------------------------------------------
		struct Texture
		{
			Index m_texIndex;
			Index m_smlIndex;
		};

		std::vector<UBuffer>  m_ubuffers;
		std::vector<Constant> m_constants;
		std::vector<TBuffer>  m_tbuffers;
		std::vector<Texture>  m_textures;

		FastSearch m_search_constants;
		FastSearch m_search_textures;
		FastSearch m_search_ubuffers;
		FastSearch m_search_tbuffers;
	};

} // render
} // brUGE
