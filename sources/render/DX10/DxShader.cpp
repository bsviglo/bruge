#include "DxShader.hpp"
#include "DxRenderDevice.hpp"
#include "DxBuffer.hpp"
#include "DxDevice.hpp"
#include <D3DX10.h>

using namespace brUGE;
using namespace brUGE::render;

// start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	//--
	const char* dxShaderType[][2] = 
	{
		"vs_4_0", "_VERTEX_SHADER_",
		"gs_4_0", "_GEOMETRY_SHADER_",
		"ps_4_0", "_FRAGMENT_SHADER_"
	};

	// load and compile shader program.
	//------------------------------------------
	HRESULT _loadAndCompileShader(
		DXShader::EShaderType type, const char* src, ID3D10Include* shaderInclude,
		const ShaderMacro* macros, uint mSize, ID3D10Blob** byteCode)
	{
		//-- convert to dx10 shader macro type.
		std::vector<D3D10_SHADER_MACRO> d3d10Macros;
		{
			D3D10_SHADER_MACRO macro;

			if (macros)
			{
				for (uint i = 0; i < mSize; ++i)
				{
					macro.Name		 = macros[i].name;
					macro.Definition = macros[i].value;
					d3d10Macros.push_back(macro);
				}
			}
			
			//-- add shader type macro.
			macro.Name		 = dxShaderType[type][1];
			macro.Definition = "1";
			d3d10Macros.push_back(macro);
				
			macro.Name		 = NULL;
			macro.Definition = NULL;
			d3d10Macros.push_back(macro);
		}
		
		ComPtr<ID3D10Blob> errorLog;
		HRESULT hr = S_FALSE;

		hr = D3DX10CompileFromMemory(
			src, strlen(src), NULL, (d3d10Macros.size()) ? &d3d10Macros[0] : NULL,
			shaderInclude, "main", dxShaderType[type][0],
			D3D10_SHADER_PACK_MATRIX_ROW_MAJOR | D3D10_SHADER_ENABLE_STRICTNESS |
#if _DEBUG
			D3D10_SHADER_OPTIMIZATION_LEVEL0,
#else
			D3D10_SHADER_OPTIMIZATION_LEVEL3,
#endif
			NULL, NULL, byteCode, &errorLog, NULL
			);


		//-- Log debug info.
#if 0
		ComPtr<ID3D10Blob> pIDisassembly;
		LPCSTR commentString = NULL;

		if (*byteCode)
		{
			D3D10DisassembleShader(
				(UINT*)(*byteCode)->GetBufferPointer(), (*byteCode)->GetBufferSize(),
				TRUE, commentString, &pIDisassembly
				);

			INFO_MSG("shader profile '%s'", dxShaderType[type][0]);
			//INFO_MSG(commentString);
			INFO_MSG(static_cast<const char*>(pIDisassembly->GetBufferPointer()));
		}
#endif

		//-- Log debug errors and warnings.
		if (FAILED(hr))
		{
			ERROR_MSG("Compilation failed:");
			ERROR_MSG("shader profile '%s'", dxShaderType[type][0]);
			ERROR_MSG("%s", static_cast<const char*>(errorLog->GetBufferPointer()));

#if 1
			std::string formatedOutput = "Error desc: " +
				std::string(static_cast<const char*>(errorLog->GetBufferPointer())) +
				"\n" + "Source: \n" + std::string(src);

			MessageBoxA(NULL, formatedOutput.c_str(), "Compilation failed.", MB_OK | MB_ICONERROR);
#endif

			assert(0);
		}

		return hr;
	}

	//------------------------------------------
	inline int codeIndex(int shaderType, int resIndex)
	{
		int result = shaderType;
		result |= resIndex << 8;
		return result;
	}

	//------------------------------------------
	inline void decodeIndex(int codedIndex, int& shaderType, int& resIndex)
	{
		shaderType = codedIndex & 0x000000FF;
		resIndex = codedIndex >> 8;
	}
}
//--------------------------------------------------------------------------------------------------
// end unnamed namespace.

namespace brUGE
{
namespace render
{	
	
	//------------------------------------------
	DXShader::DXShader(IRenderDevice& rd) : IShader(rd)
	{

	}
	
	//------------------------------------------
	DXShader::~DXShader()
	{
		for (uint i = 0; i < m_ubuffers.size(); ++i)
			delete [] m_ubuffers[i].data.data;
	}
	
	//------------------------------------------
	void IShader::addResSearch(const char* name, EShaderType sType, FastSearchEx& search, Index index)
	{
		IndexEx* searchIndex = NULL;
		if (!search.search(name, searchIndex))
		{
			IndexEx tmpIndex;
			tmpIndex.indices[sType] = index;
			search.insert(name, tmpIndex);
		}
		else
		{
			searchIndex->indices[sType] = index;
		}
	}

	//------------------------------------------
	void IShader::addSampler(const char* name, EShaderType sType, Index auxData)
	{
		Sampler sampler;
		sampler.data  = INVALID_ID;
		sampler.index = auxData;
		m_samplers.push_back(sampler);
		
		addResSearch(name, sType, m_search_samplers, m_samplers.size() - 1);
	}

	//------------------------------------------
	void IShader::addTexture(const char* name, EShaderType sType, Index auxData)
	{
		Texture texture;
		texture.data  = NULL;
		texture.index = auxData;
		m_textures.push_back(texture);
		
		addResSearch(name, sType, m_search_textures, m_textures.size() - 1);
	}

	//------------------------------------------
	void IShader::addUBuffer(const char* name, EShaderType sType, uint size, Index auxData)
	{
		//-- 1. Fill the common stuff in the description structure.
		UniformBlockDesc ublockDesc;
		ublockDesc.size    = size;	
		
		//-- make dirty because some members of this constant buffer may be setted by default 
		//-- directly in the shader and we retrive they values.
		ublockDesc.isDirty = false;
		
		//-- 2. Do search of the Ubuffer with the same name.
		IndexEx* searchIndex = NULL;
		if (!m_search_ubuffers.search(name, searchIndex))
		{
			//-- 2.1. add new data and buffer.
			ublockDesc.data	  = new byte[size];
			ublockDesc.buffer = m_rDevice.createBuffer(IBuffer::TYPE_UNIFORM, NULL, 1, size,
				IBuffer::USAGE_DEFAULT, IBuffer::CPU_ACCESS_NONE);
		}
		else
		{
			//-- 2.2. Find first valid buffer object.
			Ptr<IBuffer> tmpBuffer;
			for (uint i = 0; i < SHADER_TYPES_COUNT; ++i)
			{
				Index id = searchIndex->indices[i]; 
				if (id != INVALID_ID && m_ubuffers[id].data.data != NULL)
				{
					tmpBuffer = m_ubuffers[id].data.buffer;
					break;
				}
			}
			
			//-- 2.3. add existing buffer and make the data unused.
			ublockDesc.data	  = NULL;
			ublockDesc.buffer = tmpBuffer;
		}
		
		//-- 3. add buffer.
		UniformBlock ublock;
		ublock.index = auxData;
		ublock.data  = ublockDesc;
		m_ubuffers.push_back(ublock);

		//-- 4. add buffer to the fast search structure if it is already not there.
		addResSearch(name, sType, m_search_ubuffers, m_ubuffers.size() - 1);
	}

	//------------------------------------------
	void IShader::addTBuffer(const char* name, EShaderType sType, Index auxData)
	{
		TexBuffer tbuffer;
		tbuffer.data  = NULL;
		tbuffer.index = auxData;
		m_tbuffers.push_back(tbuffer);

		addResSearch(name, sType, m_search_tbuffers, m_tbuffers.size() - 1);
	}

	//------------------------------------------
	void IShader::addConstant(
		const char* name, EShaderType /*sType*/, Index ublockIndex,
		uint offset, uint size, void* defaultVal)
	{
		//-- ToDo: reconsider. 

		Index* searchIndex = NULL;
		if (!m_search_constants.search(name, searchIndex))
		{
			Constant c;
			c.ublockIndex = ublockIndex;
			c.offset = offset;
			c.size = size;
			m_constants.push_back(c);
			
			//-- ToDo: Why it doesn't work!!!!?????????
			//-- set default value.
			if (defaultVal)
			{
				UniformBlockDesc& ublock = m_ubuffers[ublockIndex].data;
				memcpy(ublock.data + offset, defaultVal, size);
			}

			m_search_constants.insert(name, m_constants.size() - 1);
		}
	}

	//------------------------------------------
	bool DXShader::init(const char* vs, const char* gs,	const char* fs,
		const ShaderMacro *macros, uint mCount)
	{
		ID3D10Include* si = static_cast<DXRenderDevice*>(&m_rDevice)->shaderInclude();
		ComPtr<ID3D10Blob> byteCode;
		HRESULT hr = FALSE;

		//-- vs
		if (vs)
		{
			hr = _loadAndCompileShader(VS, vs, si, macros, mCount, &m_inputSignature);
			if (FAILED(hr))	return false;

			hr = dxDevice()->CreateVertexShader(m_inputSignature->GetBufferPointer(), m_inputSignature->GetBufferSize(), &m_vs);
			if (FAILED(hr))
			{
				ERROR_MSG("Failed to create vertex shader program.\nDesc: " + dxDevice().getErrorDesc());
				return false;
			}

			hr = _reflectShader(m_inputSignature.get(), VS);
			if (FAILED(hr))
			{
				ERROR_MSG("Failed to reflect vertex shader program.\nDesc: " + dxDevice().getErrorDesc());
				return false;
			}
		}

		//-- gs
		if (gs)
		{
			hr = _loadAndCompileShader(GS, gs, si, macros, mCount, &byteCode);
			if (FAILED(hr)) return false;

			hr = dxDevice()->CreateGeometryShader(byteCode->GetBufferPointer(), byteCode->GetBufferSize(), &m_gs);
			if (FAILED(hr))
			{
				ERROR_MSG("Failed to create geometry shader program.\nDesc: " + dxDevice().getErrorDesc());
				return false;
			}

			hr = _reflectShader(byteCode.get(), GS);
			if (FAILED(hr))
			{
				ERROR_MSG("Failed to reflect geometry shader program.\nDesc: " + dxDevice().getErrorDesc());
				return false;
			}
		}

		//-- ps
		if (fs)
		{
			hr = _loadAndCompileShader(PS, fs, si, macros, mCount, &byteCode);
			if (FAILED(hr))	return false;

			hr = dxDevice()->CreatePixelShader(byteCode->GetBufferPointer(), byteCode->GetBufferSize(), &m_ps);
			if (FAILED(hr))
			{
				ERROR_MSG("Failed to create fragment shader program.\nDesc: " + dxDevice().getErrorDesc());
				return false;
			}

			hr = _reflectShader(byteCode.get(), PS) ;
			if (FAILED(hr))
			{
				ERROR_MSG("Failed to reflect fragment shader program.\nDesc: " + dxDevice().getErrorDesc());
				return false;
			}
		}
		return true;
	}
	
	//------------------------------------------
	bool DXShader::doChangeUniformBuffer(Index index, IBuffer* newBuffer)
	{
		int st, ri;
		decodeIndex(index, st, ri);
		m_dxUBuffers[st][ri] = static_cast<DXBuffer*>(newBuffer)->getBuffer();
		return true;
	}
	
	//------------------------------------------
	void DXShader::_updateRes()
	{
		DXRenderDevice* rd = static_cast<DXRenderDevice*>(&m_rDevice);
		int st, ri;

		for (uint i = 0; i < m_samplers.size(); ++i)
		{
			const Sampler& s = m_samplers[i];
			decodeIndex(s.index, st, ri);
			m_dxSamplers[st][ri] = rd->getSamplerState(s.data);
		}

		for (uint i = 0; i < m_textures.size(); ++i)
		{
			const Texture& t = m_textures[i];
			decodeIndex(t.index, st, ri);
			m_dxResources[st][ri] = static_cast<DXTexture*>(t.data)->getSRView();
		}

		for (uint i = 0; i < m_tbuffers.size(); ++i)
		{
			const TexBuffer& tb = m_tbuffers[i];
			decodeIndex(tb.index, st, ri);
			m_dxResources[st][ri] = static_cast<DXBuffer*>(tb.data)->getSRView();
		}

		for (uint i = 0; i < m_ubuffers.size(); ++i)
		{
			UniformBlock&	  ub	 = m_ubuffers[i];
			UniformBlockDesc& ubDesc = ub.data;
			decodeIndex(ub.index, st, ri);
			m_dxUBuffers[st][ri] = static_cast<DXBuffer*>(ubDesc.buffer.get())->getBuffer();

			//-- we check ubDesc.data because if it is NULL it's means that used the shared cbuffer.
			if (ubDesc.isDirty && ubDesc.data != NULL)
			{
				dxDevice()->UpdateSubresource(m_dxUBuffers[st][ri], 0, NULL, ubDesc.data, 0, 0);
				ubDesc.isDirty = false;
			}
		}
	}

	//------------------------------------------
	void DXShader::bind()
	{
		ID3D10Device* d = dxDevice().get();
		
		//-- make resources up-to-date.
		_updateRes();
		
		d->VSSetShader(m_vs.get());
		d->GSSetShader(m_gs.get());
		d->PSSetShader(m_ps.get());

		if (m_vs)
		{
			if (m_dxResources[VS].size() > 0) d->VSSetShaderResources(0, m_dxResources[VS].size(), &m_dxResources[VS][0]);
			if (m_dxSamplers [VS].size() > 0) d->VSSetSamplers       (0, m_dxSamplers [VS].size(), &m_dxSamplers [VS][0]);
			if (m_dxUBuffers [VS].size() > 0) d->VSSetConstantBuffers(0, m_dxUBuffers [VS].size(), &m_dxUBuffers [VS][0]);
		}

		if (m_gs)
		{
			if (m_dxResources[GS].size() > 0) d->GSSetShaderResources(0, m_dxResources[GS].size(), &m_dxResources[GS][0]);
			if (m_dxSamplers [GS].size() > 0) d->GSSetSamplers		 (0, m_dxSamplers [GS].size(), &m_dxSamplers [GS][0]);
			if (m_dxUBuffers [GS].size() > 0) d->GSSetConstantBuffers(0, m_dxUBuffers [GS].size(), &m_dxUBuffers [GS][0]);
		}

		if (m_ps)
		{
			if (m_dxResources[PS].size() > 0) d->PSSetShaderResources(0, m_dxResources[PS].size(), &m_dxResources[PS][0]);
			if (m_dxSamplers [PS].size() > 0) d->PSSetSamplers		 (0, m_dxSamplers [PS].size(), &m_dxSamplers [PS][0]);
			if (m_dxUBuffers [PS].size() > 0) d->PSSetConstantBuffers(0, m_dxUBuffers [PS].size(), &m_dxUBuffers [PS][0]);
		}
	}

	//------------------------------------------
	/*static*/ void DXShader::resetToDefaults()
	{
		ID3D10Device* d = dxDevice().get();
		
		//-- ToDo: reconsider.
		static bool isInited = false;
		static ID3D10ShaderResourceView* resources[D3D10_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
		static ID3D10SamplerState*		 samplers [D3D10_COMMONSHADER_SAMPLER_SLOT_COUNT]; 		
		static ID3D10Buffer*			 buffers  [D3D10_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
		if (!isInited)
		{
			memset(resources, NULL, D3D10_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
			memset(samplers,  NULL, D3D10_COMMONSHADER_SAMPLER_SLOT_COUNT);
			memset(buffers,   NULL, D3D10_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);
			isInited = true;
		}

		d->VSSetShader(NULL);
		d->GSSetShader(NULL);
		d->PSSetShader(NULL);

		d->VSSetShaderResources(0, D3D10_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, resources);
		d->VSSetSamplers	   (0, D3D10_COMMONSHADER_SAMPLER_SLOT_COUNT, samplers);
		d->VSSetConstantBuffers(0, D3D10_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, buffers);

		d->GSSetShaderResources(0, D3D10_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, resources);
		d->GSSetSamplers	   (0, D3D10_COMMONSHADER_SAMPLER_SLOT_COUNT, samplers);
		d->GSSetConstantBuffers(0, D3D10_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, buffers);

		d->PSSetShaderResources(0, D3D10_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, resources);
		d->PSSetSamplers	   (0, D3D10_COMMONSHADER_SAMPLER_SLOT_COUNT, samplers);
		d->PSSetConstantBuffers(0, D3D10_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, buffers);
	}
	
	//------------------------------------------
	HRESULT DXShader::_reflectShader(ID3D10Blob* byteCode, EShaderType shaderType)
	{
		//-- 1. try to reflect shader.
		ComPtr<ID3D10ShaderReflection> reflect;
		D3D10_SHADER_DESC shaderDesc;
	
		HRESULT hr = D3D10ReflectShader(byteCode->GetBufferPointer(), byteCode->GetBufferSize(), &reflect);
		if (FAILED(hr))	return hr;

		//-- 2. try to retrieve reflection data descriptor.
		hr = reflect->GetDesc(&shaderDesc);
		if (FAILED(hr))	return hr;
		
		D3D10_SHADER_INPUT_BIND_DESC resDesc;
		D3D10_SHADER_BUFFER_DESC	 cbDesc;
		D3D10_SHADER_VARIABLE_DESC	 vDesc;
		
		//-- 3. collect only all the c-buffers, t-buffers will be collected later.
		m_dxUBuffers[shaderType].resize(shaderDesc.ConstantBuffers);
		for (uint index = 0, i = 0; i < shaderDesc.ConstantBuffers; ++i)
		{
			//-- 3.1. check buffer type, if it's not c-buffer then skip it, we prepare it below.
			reflect->GetConstantBufferByIndex(i)->GetDesc(&cbDesc);
			if (cbDesc.Type == D3D10_CT_CBUFFER)
			{
				//-- 3.2. increase the buffer size at 1.
				m_dxUBuffers[shaderType].push_back(NULL);

				//-- 3.3. create the new uniform buffer descriptor at the interface side.
				addUBuffer(cbDesc.Name, shaderType, cbDesc.Size, codeIndex(shaderType, index));
				
				//-- 3.4. convert it to the DX format.
				uint ubufferIndex = m_ubuffers.size() - 1;
				m_dxUBuffers[shaderType][index] =
					static_cast<DXBuffer*>(m_ubuffers[ubufferIndex].data.buffer.get())->getBuffer();
				
				//-- 3.5. reflect all the variables in the current constant buffer.
				for (uint j = 0; j < cbDesc.Variables; ++j)
				{
					reflect->GetConstantBufferByIndex(i)->GetVariableByIndex(j)->GetDesc(&vDesc);
					addConstant(vDesc.Name, shaderType, ubufferIndex,
						vDesc.StartOffset, vDesc.Size, vDesc.DefaultValue
						);
				}
				
				//-- go to next.
				++index;
			}
		}
		
		//-- 4. collect all shader's resources.
		for (uint i = 0; i < shaderDesc.BoundResources; ++i)
		{
			reflect->GetResourceBindingDesc(i, &resDesc);
			switch (resDesc.Type)
			{
			case D3D10_SIT_TBUFFER:
				{
					addTBuffer(resDesc.Name, shaderType, codeIndex(shaderType, resDesc.BindPoint));
					m_dxResources[shaderType].push_back(NULL);
				}
				break;
			case D3D10_SIT_TEXTURE:
				{
					addTexture(resDesc.Name, shaderType, codeIndex(shaderType, resDesc.BindPoint));
					m_dxResources[shaderType].push_back(NULL);
				}
				break;
			case D3D10_SIT_SAMPLER:
				{
					addSampler(resDesc.Name, shaderType, codeIndex(shaderType, resDesc.BindPoint));
					m_dxSamplers[shaderType].push_back(NULL);
				}
				break;
			}
		}
		
		//-- 5. successful exit.
		return S_OK;
	}

} // render
} // brUGE