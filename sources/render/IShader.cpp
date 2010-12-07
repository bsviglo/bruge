#include "IShader.h"
#include "IRenderDevice.h"
#include "IBuffer.h"

namespace brUGE
{
namespace render
{

	//----------------------------------------------------------------------------------------------
	bool IShader::setConstantAsRawData(const char* name, const void* data, uint size)
	{
		Index* constIndex = NULL;
		if (m_search_constants.search(name, constIndex))
		{
			Constant& c = m_constants[*constIndex];
			UniformBlockDesc& ublock = m_ubuffers[c.ublockIndex].data;
			memcpy(ublock.data + c.offset, data, size);
			ublock.isDirty = true;
			return true;
		}
		return false;
	}

	//----------------------------------------------------------------------------------------------
	bool IShader::setUniformBlock(const char* name, const void* data, uint size)
	{
		IndexEx* ublockIndex = NULL;
		if (m_search_ubuffers.search(name, ublockIndex))
		{
			for (uint i = 0; i < SHADER_TYPES_COUNT; ++i)
			{
				Index id = ublockIndex->indices[i]; 
				if (id != INVALID_ID && m_ubuffers[id].data.data != NULL)
				{
					UniformBlockDesc& ublock = m_ubuffers[id].data;
					memcpy(ublock.data, data, size);
					ublock.isDirty = true;
					return true;
				}
			}
		}
		return false;
	}
	
	//----------------------------------------------------------------------------------------------
	bool IShader::changeUniformBuffer(const char* name, const Ptr<IBuffer>& newBuffer)
	{
		IndexEx* ublockIndex = NULL;
		if (m_search_ubuffers.search(name, ublockIndex))
		{
			bool success = true;
			for (uint i = 0; i < SHADER_TYPES_COUNT; ++i)
			{
				Index id = ublockIndex->indices[i];
				if (id != INVALID_ID)
				{
					UniformBlock& ublock = m_ubuffers[id];
					ublock.data.buffer  = newBuffer;
					ublock.data.isDirty = true;
					
					success &= doChangeUniformBuffer(ublock.index, newBuffer.get());
				}
			}
			return success;
		}
		return false;
	}

	//----------------------------------------------------------------------------------------------
	bool IShader::setSampler(const char* name, SamplerStateID state)
	{
		IndexEx* samplerIndex = NULL;
		if (m_search_samplers.search(name, samplerIndex))
		{
			for (uint i = 0; i < SHADER_TYPES_COUNT; ++i)
			{
				Index id = samplerIndex->indices[i];
				if (id != INVALID_ID)
				{
					m_samplers[id].data = state;
				}
			}
			return true;
		}
		return false;
	}

	//----------------------------------------------------------------------------------------------
	bool IShader::setTexture(const char* name, ITexture* texture)
	{
		IndexEx* textureIndex = NULL;
		if (m_search_textures.search(name, textureIndex))
		{
			for (uint i = 0; i < SHADER_TYPES_COUNT; ++i)
			{
				if (textureIndex->indices[i] != INVALID_ID)
				{
					m_textures[textureIndex->indices[i]].data = texture;
				}
			}
			return true;
		}
		return false;
	}

	//----------------------------------------------------------------------------------------------
	bool IShader::setTextureBuffer(const char* name, IBuffer* buffer)
	{
		IndexEx* tbufferIndex = NULL;
		if (m_search_tbuffers.search(name, tbufferIndex))
		{
			for (uint i = 0; i < SHADER_TYPES_COUNT; ++i)
			{
				if (tbufferIndex->indices[i] != INVALID_ID)
				{
					m_tbuffers[tbufferIndex->indices[i]].data = buffer;
				}
			}
			return true;
		}
		return false;
	}

} // render
} // brUGE