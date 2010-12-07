#pragma once

#include "render_common.h"
#include "utils/TernaryTree.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
#include "math/Vector4.h"
#include "math/Matrix4x4.h"
#include <vector>

namespace brUGE
{
namespace render
{

	//-- shader macro struct.
	struct ShaderMacro
	{
		char* name;
		char* value;
	};


	//-- shader include interface.
	class IShaderInclude
	{
	public:
		IShaderInclude() { }
		virtual ~IShaderInclude() { }

		virtual bool open (const char* name, const void*& data, uint& size) = 0;
		virtual bool close(const void*& data) = 0;

	private:
		IShaderInclude(const IShaderInclude&);
		IShaderInclude& operator = (const IShaderInclude&);
	};


	//-- Note: By-defaults all the uniforms blocks in the different stages with the same name are shared.
	//--	   If you want to make the different uniform blocks or to update it independently You have
	//--	   to make their with the different names. 		
	//----------------------------------------------------------------------------------------------
	class IShader : public utils::RefCount
	{
	public:
		enum EShaderType
		{
			VS, GS, PS, SHADER_TYPES_COUNT
		};

	public:
		bool setBool (const char* name, bool val)			{ return setConstantAsRawData(name, &val, sizeof(bool) ); }
		bool setFloat(const char* name, float val)			{ return setConstantAsRawData(name, &val, sizeof(float)); }
		bool setInt  (const char* name, int val)			{ return setConstantAsRawData(name, &val, sizeof(int)  ); }

		bool setVec2f(const char* name, const vec2f& val)	{ return setConstantAsRawData(name, &val, sizeof(vec2f)); }
		bool setVec3f(const char* name, const vec3f& val)	{ return setConstantAsRawData(name, &val, sizeof(vec3f)); }
		bool setVec4f(const char* name, const vec4f& val)	{ return setConstantAsRawData(name, &val, sizeof(vec4f)); }
		bool setMat4f(const char* name, const mat4f& val)	{ return setConstantAsRawData(name, &val, sizeof(mat4f)); }

		bool setTexture		 (const char* name, ITexture* texture);
		bool setSampler		 (const char* name, SamplerStateID state);
		bool setUniformBlock (const char* name, const void* data, uint size);
		bool setTextureBuffer(const char* name, IBuffer* buffer);
		
		//-- the main usage of this function is to make this uniform block shared
		//-- between the other shaders.
		bool changeUniformBuffer(const char* name, const Ptr<IBuffer>& newBuffer);

		//-- this method simplifies writing generic code. (i.e. template based code).
		bool setConstantAsRawData(const char* name, const void* data, uint size);

	protected:
		typedef int Index;

		IShader(IRenderDevice& rd) : m_rDevice(rd) {}
		virtual ~IShader()
		{
			for (uint i = 0; i < m_ubuffers.size(); ++i)
				delete [] m_ubuffers[i].data.data;
		}

		virtual bool doChangeUniformBuffer(Index index, IBuffer* newBuffer) = 0;

	private:
		//template<typename RES>
		//bool setResource(
		//	const char* name, RES iRes, const std::vector<Resource<RES> >& resArray,
		//	const FastSearchEx& searchStruct
		//	);
		
		//-- disable copying.
		IShader(IShader&);
		IShader& operator = (IShader&);

	protected:
		void addSampler (const char* name, EShaderType sType, Index auxData);
		void addTexture (const char* name, EShaderType sType, Index auxData);
		void addUBuffer (const char* name, EShaderType sType, uint size, Index auxData);
		void addTBuffer (const char* name, EShaderType sType, Index auxData);
		void addConstant(const char* name, EShaderType sType, Index ublockIndex, uint offset, uint size, void* defaultVal);
		
		//--
		template<typename RES>	
		struct Resource
		{
			Resource():index(-1){}

			RES		data;
			Index	index;
		};
		
		//--
		struct UniformBlockDesc
		{
			UniformBlockDesc():data(NULL),size(0),isDirty(true){}

			Ptr<IBuffer> buffer;
			byte* data;
			uint size;
			bool isDirty;
		};
		
		//--
		struct Constant
		{
			Constant():ublockIndex(-1),offset(0),size(0){}

			Index ublockIndex;
			uint offset;
			uint size;
		};
		
		//--
		typedef Resource<ITexture*>			Texture;
		typedef Resource<SamplerStateID>	Sampler;
		typedef Resource<IBuffer*>			TexBuffer;
		typedef Resource<UniformBlockDesc>	UniformBlock;

		//--
		IRenderDevice& m_rDevice;
		
		//--
		std::vector<Sampler>	  m_samplers;
		std::vector<Texture>	  m_textures;
		std::vector<UniformBlock> m_ubuffers;
		std::vector<Constant>	  m_constants;
		std::vector<TexBuffer>	  m_tbuffers;

		struct IndexEx
		{
			IndexEx()
			{
				for (uint i = 0; i < SHADER_TYPES_COUNT; ++i)
					indices[i] = INVALID_ID;
			}

			Index indices[SHADER_TYPES_COUNT];
		};

		//-- fast access structures.
		typedef utils::TernaryTree<char, Index  > FastSearch;
		typedef utils::TernaryTree<char, IndexEx> FastSearchEx;
		FastSearch   m_search_constants;
		FastSearchEx m_search_samplers;
		FastSearchEx m_search_textures;
		FastSearchEx m_search_ubuffers;
		FastSearchEx m_search_tbuffers;
		
		//-- performs adding into fast search structure.
		void addResSearch(const char* name, EShaderType sType, FastSearchEx& search, Index index);
	};

} // render
} // brUGE
