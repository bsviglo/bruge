#include "shader_context.hpp"
#include "render_system.hpp"
#include "materials.hpp"
#include "Camera.h"
#include "os/FileSystem.h"
#include "utils/string_utils.h"

using namespace brUGE;
using namespace brUGE::render;
using namespace brUGE::math;
using namespace brUGE::utils;
using namespace brUGE::os;
	 

//-- start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{

	//----------------------------------------------------------------------------------------------
	class PerFrameProperty : public IProperty
	{
	public:
		PerFrameProperty(ShaderContext& sc) : m_sc(sc) { }
		virtual ~PerFrameProperty() { }

		virtual bool apply(IShader& shader) const
		{
			m_value.m_viewMat		 = m_sc.camera().viewMatrix();
			m_value.m_viewProjMat	 = m_sc.camera().viewProjMatrix();
			m_value.m_invViewProjMat = m_sc.camera().invViewProjMatrix();

			m_value.m_screenRes.x	 = rs().screenRes().width;
			m_value.m_screenRes.y	 = rs().screenRes().height;
			m_value.m_screenRes.z	 = 1.0f / rs().screenRes().width;
			m_value.m_screenRes.w	 = 1.0f / rs().screenRes().height;

			return shader.setUniformBlock("cb_auto_PerFrame", &m_value, sizeof(PerFrameCB));
		}

	private:
		struct PerFrameCB
		{
			mat4f m_viewMat;
			mat4f m_viewProjMat;
			mat4f m_invViewProjMat;
			vec4f m_screenRes;
		};
		mutable PerFrameCB  m_value;
		ShaderContext&		m_sc;
	};


	//----------------------------------------------------------------------------------------------
	class PerInstanceProperty : public IProperty
	{
	public:
		PerInstanceProperty(ShaderContext& sc) : m_sc(sc) { }
		virtual ~PerInstanceProperty() { }

		virtual bool apply(IShader& shader) const
		{
			//-- ToDo:
			if (!m_sc.renderOp().m_worldMat) return false;

			m_value.worldMat = *m_sc.renderOp().m_worldMat;
			m_value.MVPMat   = mult(m_value.worldMat, m_sc.camera().viewProjMatrix());

			return shader.setUniformBlock("cb_auto_PerInstance", &m_value, sizeof(PerInstanceCB));
		}

	private:
		struct PerInstanceCB
		{
			mat4f worldMat;
			mat4f MVPMat;
		};
		mutable PerInstanceCB m_value;
		ShaderContext&		  m_sc;
	};


	//----------------------------------------------------------------------------------------------
	class WeightsProperty : public IProperty
	{
	public:
		WeightsProperty(ShaderContext& sc) : m_sc(sc) { }
		virtual ~WeightsProperty() { }

		virtual bool apply(IShader& shader) const
		{
			return shader.setTextureBuffer("tb_auto_Weights", m_sc.renderOp().m_weightsTB);
		}

	private:
		ShaderContext& m_sc;
	};


	//----------------------------------------------------------------------------------------------
	class MatrixPaletteProperty : public IProperty
	{
	public:
		MatrixPaletteProperty(ShaderContext& sc) : m_sc(sc)
		{
			m_matrixPaletteTB = rd()->createBuffer(IBuffer::TYPE_TEXTURE, 0,
				256 * 4, sizeof(vec4f), IBuffer::USAGE_DYNAMIC, IBuffer::CPU_ACCESS_WRITE
				);
		}
		virtual ~MatrixPaletteProperty() { }

		virtual bool apply(IShader& shader) const
		{
			//-- ToDo:
			if (!m_sc.renderOp().m_matrixPaletteCount) return false;

			const RenderOp& ro = m_sc.renderOp();

			if (mat4f* mp = m_matrixPaletteTB->map<mat4f>(IBuffer::ACCESS_WRITE_DISCARD))
			{
				memcpy(mp, ro.m_matrixPalette, sizeof(mat4f) * ro.m_matrixPaletteCount);
				m_matrixPaletteTB->unmap();
			}

			return shader.setTextureBuffer("tb_auto_MatrixPalette", m_matrixPaletteTB.get());
		}

	private:
		Ptr<IBuffer>   m_matrixPaletteTB;
		ShaderContext& m_sc;
	};


	//----------------------------------------------------------------------------------------------
	class InstancingProperty : public IProperty
	{
	public:
		InstancingProperty(ShaderContext& sc) : m_sc(sc) { }
		virtual ~InstancingProperty() { }

		virtual bool apply(IShader& shader) const
		{
			//-- ToDo:
			if (!m_sc.renderOp().m_instanceCount) return false;

			const RenderOp& ro = m_sc.renderOp();

			if (void* mp = ro.m_instanceTB->map<void>(IBuffer::ACCESS_WRITE_DISCARD))
			{
				memcpy(mp, ro.m_instanceData, ro.m_instanceSize * ro.m_instanceCount);
				ro.m_instanceTB->unmap();
			}

			return shader.setTextureBuffer("tb_auto_Instancing", m_sc.renderOp().m_instanceTB);
		}

	private:
		ShaderContext& m_sc;
	};


	//----------------------------------------------------------------------------------------------
	class TextureAutoProperty : public IProperty
	{
	public:
		TextureAutoProperty(ShaderContext& sc, const char* name, bool isDiffuse)
			:	m_sc(sc), m_isDiffuse(isDiffuse)
		{
			m_textureName = utils::makeStr("t_auto_%s_tex", name);
			m_samplerName = utils::makeStr("t_auto_%s_sml", name);

			SamplerStateDesc sDesc;
			sDesc.minMagFilter	= SamplerStateDesc::FILTER_TRILINEAR_ANISO;
			sDesc.wrapS		 	= SamplerStateDesc::ADRESS_MODE_WRAP;
			sDesc.wrapT		 	= SamplerStateDesc::ADRESS_MODE_WRAP;
			sDesc.wrapR			= SamplerStateDesc::ADRESS_MODE_WRAP;
			sDesc.maxAnisotropy = 16;
			m_samplerID			= render::rd()->createSamplerState(sDesc);
		}

		virtual ~TextureAutoProperty() { }

		virtual bool apply(IShader& shader) const
		{
			ITexture* texture = nullptr;

			if (m_isDiffuse)	texture =  m_sc.renderOp().m_material->m_sysProps.m_diffuseMap;
			else				texture =  m_sc.renderOp().m_material->m_sysProps.m_bumpMap;

			//-- ToDo:
			if (!texture) return false;

			bool result = true;

			result &= shader.setTexture(m_textureName.c_str(), texture);
			result &= shader.setSampler(m_samplerName.c_str(), m_samplerID);

			return result;
		}

	private:
		bool		   m_isDiffuse;
		std::string	   m_textureName;
		std::string	   m_samplerName;
		SamplerStateID m_samplerID;
		ShaderContext& m_sc;
	};


	//----------------------------------------------------------------------------------------------
	class DepthMapAutoProperty : public IProperty
	{
	public:
		DepthMapAutoProperty(ShaderContext& sc)	: m_sc(sc)
		{
			SamplerStateDesc sDesc;
			sDesc.minMagFilter	= SamplerStateDesc::FILTER_NEAREST;
			sDesc.wrapS		 	= SamplerStateDesc::ADRESS_MODE_CLAMP;
			sDesc.wrapT		 	= SamplerStateDesc::ADRESS_MODE_CLAMP;
			sDesc.wrapR			= SamplerStateDesc::ADRESS_MODE_CLAMP;
			m_samplerID			= render::rd()->createSamplerState(sDesc);
		}

		virtual ~DepthMapAutoProperty() { }

		virtual bool apply(IShader& shader) const
		{
			bool result = true;

			result &= shader.setTexture("t_auto_depthMap_tex", rs().depthTexture());
			result &= shader.setSampler("t_auto_depthMap_sml", m_samplerID);

			return result;
		}

	private:
		SamplerStateID m_samplerID;
		ShaderContext& m_sc;
	};
	
}
//--------------------------------------------------------------------------------------------------
//-- end unnamed namespace.


namespace brUGE
{
namespace render
{

	//-- ToDo: reconsider.
	/*static*/ SamplerStateID TextureProperty::m_samplerID = CONST_INVALID_HANDLE;

	//----------------------------------------------------------------------------------------------
	ShaderContext::ShaderContext()
		:	m_renderOp(NULL)
	{

	}

	//----------------------------------------------------------------------------------------------
	ShaderContext::~ShaderContext()
	{

	}

	//----------------------------------------------------------------------------------------------
	bool ShaderContext::init()
	{
		//-- create ShaderInclude interface.
		m_shaderIncludes.reset(new ShaderIncludeImpl("resources/shaders/"));
		rd()->setShaderIncludes(m_shaderIncludes.get());

		//-- register auto-constants.
		m_autoProperties["cb_auto_PerFrame"]		= new PerFrameProperty(*this);
		m_autoProperties["cb_auto_PerInstance"]		= new PerInstanceProperty(*this);
		m_autoProperties["tb_auto_Weights"]			= new WeightsProperty(*this);
		m_autoProperties["tb_auto_MatrixPalette"]	= new MatrixPaletteProperty(*this);
		m_autoProperties["tb_auto_Instancing"]		= new InstancingProperty(*this);
		m_autoProperties["t_auto_diffuseMap"]		= new TextureAutoProperty(*this, "diffuseMap", true);
		m_autoProperties["t_auto_bumpMap"]			= new TextureAutoProperty(*this, "bumpMap", false);
		m_autoProperties["t_auto_depthMap"]			= new DepthMapAutoProperty(*this);

		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool ShaderContext::fini()
	{
		for (auto iter = m_autoProperties.begin(); iter != m_autoProperties.end(); ++iter)
			delete iter->second;

		m_shaderIncludes.reset();
		m_shaderCache.clear();
		m_searchMap.clear();
		m_autoProperties.clear();

		return true;
	}

	//----------------------------------------------------------------------------------------------
	Handle ShaderContext::loadShader(const char* name)
	{
		FileSystem& fs = FileSystem::instance();

		RODataPtr data = fs.readFile(makeStr("resources/shaders/%s.%s",
			name, (rs().gapi() == RENDER_API_GL3 ? "glsl" : "hlsl"))
			);
		if (!data.get())
		{
			return CONST_INVALID_HANDLE;
		}

		// ToDo: reconsider.
		std::string src;
		data->getAsString(src);

		Ptr<IShader> shader = rd()->createShader(src.c_str(), NULL, 0);
		if (shader)	
		{
			//-- ToDo: find auto properties. For now we set the all auto properties to the shader
			//--       but only needed will be applied.
			Properties props;

			for (auto iter = m_autoProperties.begin(); iter != m_autoProperties.end(); ++iter)
			{
				props.push_back(iter->second);
			}

			//-- add to shader cash.
			m_shaderCache.push_back(make_pair(shader, props));

			//-- add to shader search map.
			m_searchMap[name] = m_shaderCache.size() - 1;

			return m_shaderCache.size() - 1;
		}

		return CONST_INVALID_HANDLE;
	}

	//----------------------------------------------------------------------------------------------
	Handle ShaderContext::getShader(const char* name)
	{
		auto iter = m_searchMap.find(name);
		if (iter != m_searchMap.end())
		{
			return iter->second;
		}
		else
		{
			return loadShader(name);
		}
	}

	//----------------------------------------------------------------------------------------------
	VertexLayoutID ShaderContext::getVertexLayout(Handle shaderID, const std::string& desc)
	{
		IShader* shader = NULL;
		if (shaderID == CONST_INVALID_HANDLE)
		{
			return CONST_INVALID_HANDLE;
		}
		else
		{
			shader = m_shaderCache[shaderID].first.get();
		}

		if		(desc == "xyzc")
		{
			VertexDesc desc[] = 
			{
				{ 0, TYPE_POSITION,	FORMAT_FLOAT, 3},
				{ 0, TYPE_TEXCOORD, FORMAT_FLOAT, 4}
			};
			return rd()->createVertexLayout(desc, 2, *shader);
		}
		else if (desc == "xyzn")
		{
			VertexDesc desc[] = 
			{
				{ 0, TYPE_POSITION,	FORMAT_FLOAT, 3},
				{ 0, TYPE_NORMAL,	FORMAT_FLOAT, 3}
			};
			return rd()->createVertexLayout(desc, 2, *shader);
		}
		else if (desc == "xyztc")
		{
			VertexDesc desc[] = 
			{
				{ 0, TYPE_POSITION,	FORMAT_FLOAT, 3},
				{ 0, TYPE_TEXCOORD, FORMAT_FLOAT, 2}
			};
			return rd()->createVertexLayout(desc, 2, *shader);
		}
		else if (desc == "xyztcn")
		{
			VertexDesc desc[] = 
			{
				{ 0, TYPE_POSITION,	FORMAT_FLOAT, 3},
				{ 0, TYPE_TEXCOORD, FORMAT_FLOAT, 2},
				{ 0, TYPE_NORMAL,   FORMAT_FLOAT, 3}
			};
			return rd()->createVertexLayout(desc, 3, *shader);
		}
		else if (desc == "xyztcntb")
		{
			VertexDesc desc[] = 
			{
				{ 0, TYPE_POSITION,	 FORMAT_FLOAT, 3},
				{ 0, TYPE_TEXCOORD,  FORMAT_FLOAT, 2},
				{ 0, TYPE_NORMAL,    FORMAT_FLOAT, 3},
				{ 1, TYPE_TEXCOORD1, FORMAT_FLOAT, 3},
				{ 1, TYPE_TEXCOORD1, FORMAT_FLOAT, 3}
			};
			return rd()->createVertexLayout(desc, 5, *shader);
		}
		else if (desc == "ntc2ui")
		{
			VertexDesc desc[] = 
			{
				{ 0, TYPE_NORMAL,	 FORMAT_FLOAT, 3},
				{ 0, TYPE_TEXCOORD0, FORMAT_FLOAT, 2},
				{ 0, TYPE_TEXCOORD1, FORMAT_UINT,  1},
				{ 0, TYPE_TEXCOORD2, FORMAT_UINT,  1}
			};
			return rd()->createVertexLayout(desc, 4, *shader);
		}
		else
		{
			assert(0 && "not implemented yet.");
		}

		return CONST_INVALID_HANDLE;
	}

	//----------------------------------------------------------------------------------------------
	void ShaderContext::setCamera(Camera* cam)
	{
		assert(cam);

		m_camera = cam;
	}

	//----------------------------------------------------------------------------------------------
	void ShaderContext::applyFor(RenderOp* op, EShaderRenderPassType pass)
	{
		m_renderOp = op;

		assert(op->m_material);

		const RenderFx&	fx = *op->m_material;

		assert(fx.m_shader);

		Handle shaderID = CONST_INVALID_HANDLE;

		if (fx.m_shader->m_isMultipass)
		{
			shaderID = fx.m_shader->m_shaders[pass];
		}
		else
		{
			shaderID = fx.m_shader->m_shaders[0];
		}

		assert(shaderID != CONST_INVALID_HANDLE);
				
		ShaderPair&		sp     = m_shaderCache[shaderID];
		IShader*		shader = sp.first.get();

		//-- apply auto-properties predefined for this type of shader.
		for (auto i = sp.second.begin(); i != sp.second.end(); ++i)
		{
			(*i)->apply(*shader);
		}

		//-- apply user-configurable properties.
		for (uint i = 0; i != fx.m_propsCount; ++i)
		{
			fx.m_props[i]->apply(*shader);
		}

		rd()->setShader(shader);
	}


	//----------------------------------------------------------------------------------------------
	ShaderIncludeImpl::ShaderIncludeImpl(const std::string& path) : m_path(path)
	{

	}

	//----------------------------------------------------------------------------------------------
	ShaderIncludeImpl::~ShaderIncludeImpl()
	{
		assert(m_includes.size() == 0);
	}

	//----------------------------------------------------------------------------------------------
	bool ShaderIncludeImpl::open(const char* name, const void*& data, uint& size)
	{
		RODataPtr file = FileSystem::instance().readFile(m_path + std::string(name));
		if (file.get())
		{
			data = file->ptr();
			size = file->length();

			//-- add to cache.
			m_includes.push_back(std::make_pair(file->ptr(), file));

			return true;
		}
		else
		{
			data = NULL;
			size = 0;

			return false;
		}
	}

	//----------------------------------------------------------------------------------------------
	bool ShaderIncludeImpl::close(const void*& data)
	{
		for (uint i = 0; i < m_includes.size(); ++i)
		{
			if (m_includes[i].first == data)
			{
				m_includes[i] = m_includes.back();
				m_includes.pop_back();
				return true;
			}
		}

		return false;
	}

	//----------------------------------------------------------------------------------------------
	TextureProperty::TextureProperty(const std::string& name, const Ptr<ITexture>& texture)
		:	m_texture(texture)
	{
		m_textureName = utils::makeStr("%s_tex", name.c_str());
		m_samplerName = utils::makeStr("%s_sml", name.c_str());
	}

	//----------------------------------------------------------------------------------------------
	bool TextureProperty::apply(IShader& shader) const
	{
		//-- ToDo: reconsider.
		static bool inited = false;
		if (!inited)
		{
			SamplerStateDesc sDesc;
			sDesc.minMagFilter	= SamplerStateDesc::FILTER_BILINEAR;
			sDesc.wrapS		 	= SamplerStateDesc::ADRESS_MODE_BORDER;
			sDesc.wrapT		 	= SamplerStateDesc::ADRESS_MODE_BORDER;
			sDesc.wrapR			= SamplerStateDesc::ADRESS_MODE_BORDER;
			sDesc.borderColour  = Color(0,0,0,0);
			m_samplerID			= render::rd()->createSamplerState(sDesc);

			inited = true;
		}

		bool result = true;

		result &= shader.setTexture(m_textureName.c_str(), m_texture.get());
		result &= shader.setSampler(m_samplerName.c_str(), m_samplerID);

		return result;
	}

} //-- render
} //-- brUGE