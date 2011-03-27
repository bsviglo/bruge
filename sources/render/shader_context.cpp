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
	uint calcPinsCode(const std::vector<std::string>& pins)
	{
		uint code = 0;
		for (auto i = pins.begin(); i != pins.end(); ++i)
		{
			if		(*i == "PIN_ALPHA_TEST")	code |= ShaderContext::PIN_ALPHA_TEST;
			else if (*i == "PIN_BUMP_MAP")		code |= ShaderContext::PIN_BUMP_MAP;
			else								assert(!"Invalid pin code.");
		}
		return code;
	}

	//----------------------------------------------------------------------------------------------
	class GlobalProperty : public IProperty
	{
	public:
		GlobalProperty(ShaderContext& sc) : m_sc(sc) { }
		virtual ~GlobalProperty() { }

		virtual bool apply(IShader& shader) const
		{
			m_value.m_screenRes.x = rs().screenRes().width;
			m_value.m_screenRes.y = rs().screenRes().height;
			m_value.m_screenRes.z = 1.0f / m_value.m_screenRes.x;
			m_value.m_screenRes.w = 1.0f / m_value.m_screenRes.y;

			//-- ToDo: reconsider.
			m_value.m_farNearPlane.x = m_sc.camera()->m_projInfo.nearDist;
			m_value.m_farNearPlane.y = m_sc.camera()->m_projInfo.farDist;
			m_value.m_farNearPlane.z = m_value.m_farNearPlane.x + m_value.m_farNearPlane.y;
			m_value.m_farNearPlane.w = 1.0f / m_value.m_farNearPlane.z;

			return shader.setUniformBlock("cb_auto_Global", &m_value, sizeof(GlobalCB));
		}

	private:
		struct GlobalCB
		{
			vec4f m_screenRes;
			vec4f m_farNearPlane;
		};
		mutable GlobalCB m_value;
		ShaderContext&	 m_sc;
	};


	//----------------------------------------------------------------------------------------------
	class PerFrameProperty : public IProperty
	{
	public:
		PerFrameProperty(ShaderContext& sc) : m_sc(sc) { }
		virtual ~PerFrameProperty() { }

		virtual bool apply(IShader& shader) const
		{
			m_value.m_cameraPos			 = m_sc.camera()->m_invView.applyToOrigin().toVec4();
			m_value.m_viewMat			 = m_sc.camera()->m_view;
			m_value.m_invViewProjMat	 = m_sc.camera()->m_invView;
			m_value.m_viewProjMat		 = m_sc.camera()->m_viewProj;
			m_value.m_invViewProjMat	 = m_sc.camera()->m_viewProj.getInverted();
			m_value.m_lastViewProjMat	 = rs().lastViewProjMat();
			m_value.m_invLastViewProjMat = rs().invLastViewProjMat();

			return shader.setUniformBlock("cb_auto_PerFrame", &m_value, sizeof(PerFrameCB));
		}

	private:
		struct PerFrameCB
		{
			vec4f m_cameraPos;
			mat4f m_viewMat;
			mat4f m_invViewMat;
			mat4f m_viewProjMat;
			mat4f m_invViewProjMat;
			mat4f m_lastViewProjMat;
			mat4f m_invLastViewProjMat;
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

			m_value.m_worldMat = *m_sc.renderOp().m_worldMat;
			m_value.m_MVPMat   = mult(m_value.m_worldMat, m_sc.camera()->m_viewProj);
			m_value.m_MVMat	   = mult(m_value.m_worldMat, m_sc.camera()->m_view);
			m_value.m_alphaRef = m_sc.renderOp().m_material->m_sysProps.m_alphaRef;

			return shader.setUniformBlock("cb_auto_PerInstance", &m_value, sizeof(PerInstanceCB));
		}

	private:
		struct PerInstanceCB
		{
			mat4f m_worldMat;
			mat4f m_MVPMat;
			mat4f m_MVMat;
			float m_alphaRef;
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
			//-- ToDo:
			if (!m_sc.renderOp().m_weightsTB) return false;

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


	//----------------------------------------------------------------------------------------------
	class DecalsMaskAutoProperty : public IProperty
	{
	public:
		DecalsMaskAutoProperty(ShaderContext& sc)	: m_sc(sc)
		{
			SamplerStateDesc sDesc;
			sDesc.minMagFilter	= SamplerStateDesc::FILTER_NEAREST;
			sDesc.wrapS		 	= SamplerStateDesc::ADRESS_MODE_CLAMP;
			sDesc.wrapT		 	= SamplerStateDesc::ADRESS_MODE_CLAMP;
			sDesc.wrapR			= SamplerStateDesc::ADRESS_MODE_CLAMP;
			m_samplerID			= render::rd()->createSamplerState(sDesc);
		}

		virtual ~DecalsMaskAutoProperty() { }

		virtual bool apply(IShader& shader) const
		{
			bool result = true;

			result &= shader.setTexture("t_auto_decalsMask_tex", rs().decalsMask());
			result &= shader.setSampler("t_auto_decalsMask_sml", m_samplerID);

			return result;
		}

	private:
		SamplerStateID m_samplerID;
		ShaderContext& m_sc;
	};


	//----------------------------------------------------------------------------------------------
	class LightsMaskAutoProperty : public IProperty
	{
	public:
		LightsMaskAutoProperty(ShaderContext& sc)	: m_sc(sc)
		{
			SamplerStateDesc sDesc;
			sDesc.minMagFilter	= SamplerStateDesc::FILTER_NEAREST;
			sDesc.wrapS		 	= SamplerStateDesc::ADRESS_MODE_CLAMP;
			sDesc.wrapT		 	= SamplerStateDesc::ADRESS_MODE_CLAMP;
			sDesc.wrapR			= SamplerStateDesc::ADRESS_MODE_CLAMP;
			m_samplerID			= render::rd()->createSamplerState(sDesc);
		}

		virtual ~LightsMaskAutoProperty() { }

		virtual bool apply(IShader& shader) const
		{
			bool result = true;

			result &= shader.setTexture("t_auto_lightsMask_tex", rs().lightsMask());
			result &= shader.setSampler("t_auto_lightsMask_sml", m_samplerID);

			return result;
		}

	private:
		SamplerStateID m_samplerID;
		ShaderContext& m_sc;
	};


	//----------------------------------------------------------------------------------------------
	class ShadowsMaskAutoProperty : public IProperty
	{
	public:
		ShadowsMaskAutoProperty(ShaderContext& sc)	: m_sc(sc)
		{
			SamplerStateDesc sDesc;
			sDesc.minMagFilter	= SamplerStateDesc::FILTER_NEAREST;
			sDesc.wrapS		 	= SamplerStateDesc::ADRESS_MODE_CLAMP;
			sDesc.wrapT		 	= SamplerStateDesc::ADRESS_MODE_CLAMP;
			sDesc.wrapR			= SamplerStateDesc::ADRESS_MODE_CLAMP;
			m_samplerID			= render::rd()->createSamplerState(sDesc);
		}

		virtual ~ShadowsMaskAutoProperty() { }

		virtual bool apply(IShader& shader) const
		{
			bool result = true;

			result &= shader.setTexture("t_auto_shadowsMask_tex", rs().shadowsMask());
			result &= shader.setSampler("t_auto_shadowsMask_sml", m_samplerID);

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
	//----------------------------------------------------------------------------------------------
	ShaderContext::ShaderContext()
		:	m_renderOp(NULL)
	{

	}

	//----------------------------------------------------------------------------------------------
	ShaderContext::~ShaderContext()
	{
		for (auto iter = m_autoProperties.begin(); iter != m_autoProperties.end(); ++iter)
			delete iter->second;
	}

	//----------------------------------------------------------------------------------------------
	bool ShaderContext::init()
	{
		//-- create ShaderInclude interface.
		m_shaderIncludes.reset(new ShaderIncludeImpl("resources/shaders/"));
		rd()->setShaderIncludes(m_shaderIncludes.get());

		//-- register auto-constants.
		m_autoProperties["cb_auto_Global"]			= new GlobalProperty(*this);
		m_autoProperties["cb_auto_PerFrame"]		= new PerFrameProperty(*this);
		m_autoProperties["cb_auto_PerInstance"]		= new PerInstanceProperty(*this);
		m_autoProperties["tb_auto_Weights"]			= new WeightsProperty(*this);
		m_autoProperties["tb_auto_MatrixPalette"]	= new MatrixPaletteProperty(*this);
		m_autoProperties["tb_auto_Instancing"]		= new InstancingProperty(*this);
		m_autoProperties["t_auto_diffuseMap"]		= new TextureAutoProperty(*this, "diffuseMap", true);
		m_autoProperties["t_auto_bumpMap"]			= new TextureAutoProperty(*this, "bumpMap", false);
		m_autoProperties["t_auto_depthMap"]			= new DepthMapAutoProperty(*this);
		m_autoProperties["t_auto_decalsMask"]		= new DecalsMaskAutoProperty(*this);
		m_autoProperties["t_auto_lightsMask"]		= new LightsMaskAutoProperty(*this);
		m_autoProperties["t_auto_shadowsMask"]		= new ShadowsMaskAutoProperty(*this);

		return true;
	}

	//----------------------------------------------------------------------------------------------
	Handle ShaderContext::loadShader(const char* name, const std::vector<std::string>* pins)
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

		std::vector<ShaderMacro> macroses;
		if (pins)
		{
			for (auto i = pins->begin(); i != pins->end(); ++i)
			{
				ShaderMacro macro;
				macro.name  = i->c_str();
				macro.value = "1";

				macroses.push_back(macro);
			}
		}

		Ptr<IShader> shader = rd()->createShader(
			src.c_str(), macroses.empty() ? nullptr : &macroses[0], macroses.size()
			);
		if (shader)	
		{
			//-- ToDo: find auto properties. For now we set the all auto properties to the shader
			//--       but only needed will be applied.
			Properties props;

			for (auto iter = m_autoProperties.begin(); iter != m_autoProperties.end(); ++iter)
			{
				props.push_back(iter->second);
			}

			//-- add to shader cache.
			m_shaderCache.push_back(make_pair(shader, props));

			//-- add to shader search map.
			m_searchMap[name] = m_shaderCache.size() - 1;

			return m_shaderCache.size() - 1;
		}

		return CONST_INVALID_HANDLE;
	}

	//----------------------------------------------------------------------------------------------
	Handle ShaderContext::getShader(const char* name, const std::vector<std::string>* pins)
	{
		std::string fullName = name;
		if (pins)
		{
			fullName = makeStr("%s_%d", name, calcPinsCode(*pins));
		}
		
		auto iter = m_searchMap.find(fullName);
		if (iter != m_searchMap.end())
		{
			return iter->second;
		}
		else
		{
			return loadShader(name, pins);
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
		else if (desc == "xyzuv")
		{
			VertexDesc desc[] = 
			{
				{ 0, TYPE_POSITION,	FORMAT_FLOAT, 3},
				{ 0, TYPE_TEXCOORD, FORMAT_FLOAT, 2}
			};
			return rd()->createVertexLayout(desc, 2, *shader);
		}
		else if (desc == "xyzuvn")
		{
			VertexDesc desc[] = 
			{
				{ 0, TYPE_POSITION,	FORMAT_FLOAT, 3},
				{ 0, TYPE_TEXCOORD, FORMAT_FLOAT, 2},
				{ 0, TYPE_NORMAL,   FORMAT_FLOAT, 3}
			};
			return rd()->createVertexLayout(desc, 3, *shader);
		}
		else if (desc == "xyzuvntb")
		{
			VertexDesc desc[] = 
			{
				{ 0, TYPE_POSITION,	 FORMAT_FLOAT, 3},
				{ 0, TYPE_TEXCOORD,  FORMAT_FLOAT, 2},
				{ 0, TYPE_NORMAL,    FORMAT_FLOAT, 3},
				{ 1, TYPE_TANGENT,   FORMAT_FLOAT, 3},
				{ 1, TYPE_BINORMAL,  FORMAT_FLOAT, 3}
			};
			return rd()->createVertexLayout(desc, 5, *shader);
		}
		else if (desc == "nuv2ui")
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
	IShader* ShaderContext::shader(Handle handle)
	{
		assert(handle != CONST_INVALID_HANDLE && handle < static_cast<int>(m_shaderCache.size()));

		ShaderPair& sPair = m_shaderCache[handle];
		return sPair.first.get();
	}

	//----------------------------------------------------------------------------------------------
	void ShaderContext::setCamera(const RenderCamera* cam)
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
			shaderID = fx.m_shader->m_shader[pass].first;
		}
		else
		{
			shaderID = fx.m_shader->m_shader[0].first;
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
	TextureProperty::TextureProperty(
		const std::string& name, const Ptr<ITexture>& texture, SamplerStateID state)
		:	m_texture(texture), m_stateS(state)
	{
		m_textureName = utils::makeStr("%s_tex", name.c_str());
		m_samplerName = utils::makeStr("%s_sml", name.c_str());
	}

	//----------------------------------------------------------------------------------------------
	bool TextureProperty::apply(IShader& shader) const
	{
		bool result = true;

		result &= shader.setTexture(m_textureName.c_str(), m_texture.get());
		result &= shader.setSampler(m_samplerName.c_str(), m_stateS);

		return result;
	}

} //-- render
} //-- brUGE