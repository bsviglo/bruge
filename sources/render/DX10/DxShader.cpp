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
	HRESULT loadAndCompileShader(
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
			D3D10_SHADER_OPTIMIZATION_LEVEL0,
			//D3D10_SHADER_OPTIMIZATION_LEVEL3,
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
}
//--------------------------------------------------------------------------------------------------
// end unnamed namespace.

namespace brUGE
{
namespace render
{	
	
	//----------------------------------------------------------------------------------------------
	DXShader::DXShader(DXRenderDevice& rd) : m_device(rd)
	{
		//-- ToDo: correct document.
		//-- To eliminate some additional checking every resource container has first element as
		//-- an temporal value and the real data located right after this first element.
		for (uint i = 0; i < SHADER_TYPES_COUNT; ++i)
		{
			m_dxResources[i].push_back(nullptr);
			m_dxSamplers[i].push_back(nullptr);
			m_dxUBuffers[i].push_back(nullptr);
		}
	}
	
	//----------------------------------------------------------------------------------------------
	DXShader::~DXShader()
	{

	}
	
	//----------------------------------------------------------------------------------------------
	void DXShader::addSampler(const char* name, EShaderType sType, uint index)
	{
		++index;

		std::string searchName(name);
		std::string::size_type pos = searchName.find_last_of("_sml");
		searchName.erase(pos - 3, 4);

		Handle handle = CONST_INVALID_HANDLE;
		if (m_search_textures.search(searchName.c_str(), handle))
		{
			m_textures[handle].m_smlIndex[sType] = index;
		}
		else
		{
			Texture texture;
			texture.m_smlIndex[sType] = index;

			m_search_textures.insert(searchName.c_str(), m_textures.size());
			m_textures.push_back(texture);
		}
	}

	//----------------------------------------------------------------------------------------------
	void DXShader::addTexture(const char* name, EShaderType sType, uint index)
	{
		++index;

		std::string searchName(name);
		std::string::size_type pos = searchName.find_last_of("_tex");
		searchName.erase(pos - 3, 4);

		Handle handle = CONST_INVALID_HANDLE;
		if (m_search_textures.search(searchName.c_str(), handle))
		{
			m_textures[handle].m_texIndex[sType] = index;
		}
		else
		{
			Texture texture;
			texture.m_texIndex[sType] = index;

			m_search_textures.insert(searchName.c_str(), m_textures.size());
			m_textures.push_back(texture);
		}
	}

	//-- return buffer index.
	//----------------------------------------------------------------------------------------------
	uint DXShader::addUBuffer(const char* name, EShaderType sType, uint size, uint index)
	{
		//-- 1. make offset in the index.
		++index;

		//-- 2. Do search for the buffers with the same name...
		Handle handle = CONST_INVALID_HANDLE;
		if (m_search_ubuffers.search(name, handle))
		{
			m_ubuffers[handle].m_index[sType] = index;
			m_dxUBuffers[sType][index] = m_ubuffers[handle].m_buffer->getBuffer();
			return handle;
		}
		//-- else create a new one and add it to the search map...
		
		//-- 3. fill the common stuff in the description structure.
		UBuffer ubuffer;
		ubuffer.m_isDirty = false;
		ubuffer.m_size	  = size;
		ubuffer.m_data.resize(size);

		//-- 4. set index for the current shader type.
		ubuffer.m_index[sType] = index;

		//-- 5. create a new uniform buffer.
		{
			Ptr<IBuffer> buffer = m_device.createBuffer(IBuffer::TYPE_UNIFORM, NULL, 1, size,
				//IBuffer::USAGE_DEFAULT, IBuffer::CPU_ACCESS_NONE
				IBuffer::USAGE_DYNAMIC, IBuffer::CPU_ACCESS_WRITE
				);

			ubuffer.m_buffer = buffer.downCast<DXBuffer>();
		}

		//-- 6. set currently created buffer as shader resource.
		m_dxUBuffers[sType][index] = ubuffer.m_buffer->getBuffer();

		//-- 7. and finally add it into the list and search map.
		m_search_ubuffers.insert(name, m_ubuffers.size());
		m_ubuffers.push_back(ubuffer);

		return m_ubuffers.size() - 1;
	}

	//------------------------------------------
	void DXShader::addTBuffer(const char* name, EShaderType sType, uint index)
	{
		++index;

		Handle handle = CONST_INVALID_HANDLE;
		if (m_search_tbuffers.search(name, handle))
		{
			m_tbuffers[handle].m_index[sType] = index;
		}
		else
		{
			TBuffer tbuffer;
			tbuffer.m_index.m_shaders[sType] = index;

			m_search_tbuffers.insert(name, m_tbuffers.size());
			m_tbuffers.push_back(tbuffer);
		}
	}

	//------------------------------------------
	void DXShader::addConstant(const char* name, uint ublock, uint offset, uint size)
	{
		Handle handle = CONST_INVALID_HANDLE;
		if (!m_search_constants.search(name, handle))
		{
			Constant c;
			c.m_ublock = ublock;
			c.m_offset = offset;
			c.m_size   = size;

			m_search_constants.insert(name, m_constants.size());
			m_constants.push_back(c);
		}
	}

	//------------------------------------------
	bool DXShader::init(const char* vs, const char* gs,	const char* fs,
		const ShaderMacro *macros, uint mCount)
	{
		ID3D10Include* si = m_device.shaderInclude();
		ComPtr<ID3D10Blob> byteCode;
		HRESULT hr = FALSE;

		//-- vs
		if (vs)
		{
			hr = loadAndCompileShader(VS, vs, si, macros, mCount, &m_inputSignature);
			if (FAILED(hr))	return false;

			hr = dxDevice()->CreateVertexShader(m_inputSignature->GetBufferPointer(), m_inputSignature->GetBufferSize(), &m_vs);
			if (FAILED(hr))
			{
				ERROR_MSG("Failed to create vertex shader program.\nDesc: " + dxDevice().getErrorDesc());
				return false;
			}

			hr = reflectShader(m_inputSignature.get(), VS);
			if (FAILED(hr))
			{
				ERROR_MSG("Failed to reflect vertex shader program.\nDesc: " + dxDevice().getErrorDesc());
				return false;
			}
		}

		//-- gs
		if (gs)
		{
			hr = loadAndCompileShader(GS, gs, si, macros, mCount, &byteCode);
			if (FAILED(hr)) return false;

			hr = dxDevice()->CreateGeometryShader(byteCode->GetBufferPointer(), byteCode->GetBufferSize(), &m_gs);
			if (FAILED(hr))
			{
				ERROR_MSG("Failed to create geometry shader program.\nDesc: " + dxDevice().getErrorDesc());
				return false;
			}

			hr = reflectShader(byteCode.get(), GS);
			if (FAILED(hr))
			{
				ERROR_MSG("Failed to reflect geometry shader program.\nDesc: " + dxDevice().getErrorDesc());
				return false;
			}
		}

		//-- ps
		if (fs)
		{
			hr = loadAndCompileShader(PS, fs, si, macros, mCount, &byteCode);
			if (FAILED(hr))	return false;

			hr = dxDevice()->CreatePixelShader(byteCode->GetBufferPointer(), byteCode->GetBufferSize(), &m_ps);
			if (FAILED(hr))
			{
				ERROR_MSG("Failed to create fragment shader program.\nDesc: " + dxDevice().getErrorDesc());
				return false;
			}

			hr = reflectShader(byteCode.get(), PS) ;
			if (FAILED(hr))
			{
				ERROR_MSG("Failed to reflect fragment shader program.\nDesc: " + dxDevice().getErrorDesc());
				return false;
			}
		}
		return true;
	}

	//----------------------------------------------------------------------------------------------
	Handle DXShader::getHandle(const FastSearch& database, const char* name) const
	{
		Handle handle = CONST_INVALID_HANDLE;
		database.search(name, handle);
		return handle;
	}

	//----------------------------------------------------------------------------------------------
	bool DXShader::setConstantAsRawData(Handle handle, const void* data, uint size)
	{
		if (handle == CONST_INVALID_HANDLE || data == nullptr || size == 0)
			return false;

		Constant& c  = m_constants[handle];
		UBuffer&  ub = m_ubuffers[c.m_ublock];

		memcpy(&ub.m_data[0] + c.m_offset, data, size);
		ub.m_isDirty = true;

		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool DXShader::doSetTexture(Handle handle, ITexture* texture, SamplerStateID state)
	{
		if (handle == CONST_INVALID_HANDLE || texture == nullptr || state == CONST_INVALID_HANDLE)
			return false;

		Texture& t = m_textures[handle];
		
		for (uint i = 0; i < SHADER_TYPES_COUNT; ++i)
		{
			m_dxResources[i][t.m_texIndex[i]] = static_cast<DXTexture*>(texture)->getSRView();
			m_dxSamplers [i][t.m_smlIndex[i]] = m_device.getSamplerState(state);
		}
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool DXShader::doSetTextureBuffer(Handle handle, IBuffer* buffer)
	{
		if (handle == CONST_INVALID_HANDLE || buffer == nullptr)
			return false;

		TBuffer& t = m_tbuffers[handle];
		
		for (uint i = 0; i < SHADER_TYPES_COUNT; ++i)
		{
			m_dxResources[i][t.m_index[i]] = static_cast<DXBuffer*>(buffer)->getSRView();
		}
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool DXShader::doSetUniformBlock(Handle handle, const void* data, uint size)
	{
		if (handle == CONST_INVALID_HANDLE || data == nullptr || size == 0)
			return false;

		UBuffer& ub = m_ubuffers[handle];

		memcpy(&ub.m_data[0], data, size);
		ub.m_isDirty = true;

		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool DXShader::doChangeUniformBuffer(Handle handle, const Ptr<IBuffer>& newBuffer)
	{
		if (handle == CONST_INVALID_HANDLE || !newBuffer.isValid())
			return false;

		UBuffer& ub  = m_ubuffers[handle];
		ub.m_buffer  = newBuffer.downCast<DXBuffer>();
		ub.m_isDirty = false;

		for (uint i = 0; i < SHADER_TYPES_COUNT; ++i)
		{
			m_dxUBuffers[i][ub.m_index[i]] = ub.m_buffer->getBuffer();
		}
		return true;
	}

	//----------------------------------------------------------------------------------------------
	void DXShader::updateRes()
	{
		for (auto i = m_ubuffers.begin(); i != m_ubuffers.end(); ++i)
		{
			if (i->m_isDirty)
			{
				void* data = i->m_buffer->map<void>(IBuffer::ACCESS_WRITE_DISCARD);
				if (data)
				{
					memcpy(data, &i->m_data[0], i->m_size);
					i->m_buffer->unmap();
				}
				//dxDevice()->UpdateSubresource(i->m_buffer->getBuffer(), 0, NULL, &i->m_data[0], 0, 0);
				i->m_isDirty = false;
			}
		}
	}

	//------------------------------------------
	void DXShader::bind()
	{
		ID3D10Device* d = dxDevice().get();
		
		//-- make resources up-to-date.
		updateRes();
		
		d->VSSetShader(m_vs.get());
		d->GSSetShader(m_gs.get());
		d->PSSetShader(m_ps.get());

		if (m_vs)
		{
			if (m_dxResources[VS].size() > 1) d->VSSetShaderResources(0, m_dxResources[VS].size() - 1, &m_dxResources[VS][1]);
			if (m_dxSamplers [VS].size() > 1) d->VSSetSamplers       (0, m_dxSamplers [VS].size() - 1, &m_dxSamplers [VS][1]);
			if (m_dxUBuffers [VS].size() > 1) d->VSSetConstantBuffers(0, m_dxUBuffers [VS].size() - 1, &m_dxUBuffers [VS][1]);
		}

		if (m_gs)
		{
			if (m_dxResources[GS].size() > 1) d->GSSetShaderResources(0, m_dxResources[GS].size() - 1, &m_dxResources[GS][1]);
			if (m_dxSamplers [GS].size() > 1) d->GSSetSamplers		 (0, m_dxSamplers [GS].size() - 1, &m_dxSamplers [GS][1]);
			if (m_dxUBuffers [GS].size() > 1) d->GSSetConstantBuffers(0, m_dxUBuffers [GS].size() - 1, &m_dxUBuffers [GS][1]);
		}

		if (m_ps)
		{
			if (m_dxResources[PS].size() > 1) d->PSSetShaderResources(0, m_dxResources[PS].size() - 1, &m_dxResources[PS][1]);
			if (m_dxSamplers [PS].size() > 1) d->PSSetSamplers		 (0, m_dxSamplers [PS].size() - 1, &m_dxSamplers [PS][1]);
			if (m_dxUBuffers [PS].size() > 1) d->PSSetConstantBuffers(0, m_dxUBuffers [PS].size() - 1, &m_dxUBuffers [PS][1]);
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
	HRESULT DXShader::reflectShader(ID3D10Blob* byteCode, EShaderType shaderType)
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
		m_dxUBuffers[shaderType].resize(shaderDesc.ConstantBuffers + 1);
		for (uint index = 0, i = 0; i < shaderDesc.ConstantBuffers; ++i)
		{
			//-- 3.1. check buffer type, if it's not c-buffer then skip it, we prepare it later.
			reflect->GetConstantBufferByIndex(i)->GetDesc(&cbDesc);
			if (cbDesc.Type == D3D10_CT_CBUFFER)
			{
				//-- 3.2. create the new uniform buffer descriptor at the interface side.
				uint ublock = addUBuffer(cbDesc.Name, shaderType, cbDesc.Size, index);
				
				//-- 3.3. reflect all the variables in the current constant buffer.
				for (uint j = 0; j < cbDesc.Variables; ++j)
				{
					reflect->GetConstantBufferByIndex(i)->GetVariableByIndex(j)->GetDesc(&vDesc);
					addConstant(vDesc.Name, ublock,	vDesc.StartOffset, vDesc.Size);
				}
				
				//-- 3.4. go to next.
				++index;
			}
		}

		//-- make initial offset.
		m_dxResources[shaderType].resize(1);
		m_dxSamplers [shaderType].resize(1);
		
		//-- 4. collect all shader's resources.
		for (uint i = 0; i < shaderDesc.BoundResources; ++i)
		{
			reflect->GetResourceBindingDesc(i, &resDesc);
			switch (resDesc.Type)
			{
			case D3D10_SIT_TBUFFER:
				{
					addTBuffer(resDesc.Name, shaderType, resDesc.BindPoint);
					m_dxResources[shaderType].push_back(NULL);
				}
				break;
			case D3D10_SIT_TEXTURE:
				{
					addTexture(resDesc.Name, shaderType, resDesc.BindPoint);
					m_dxResources[shaderType].push_back(NULL);
				}
				break;
			case D3D10_SIT_SAMPLER:
				{
					addSampler(resDesc.Name, shaderType, resDesc.BindPoint);
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