#include "shader_context.hpp"
#include "render_system.hpp"
#include "materials.hpp"
#include "Camera.h"
#include "os/FileSystem.h"
#include "utils/string_utils.h"
#include "vertex_declarations.hpp"

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
	class PerInstanceProperty : public IProperty
	{
	public:
		PerInstanceProperty(ShaderContext& sc) : m_sc(sc) { }
		virtual ~PerInstanceProperty() { }

		virtual bool operator() (Handle handle, IShader& shader) const
		{
			m_value.m_worldMat = *m_sc.renderOp().m_worldMat;
			m_value.m_MVPMat   = mult(m_value.m_worldMat, m_sc.camera()->m_viewProj);
			m_value.m_MVMat	   = mult(m_value.m_worldMat, m_sc.camera()->m_view);
			m_value.m_alphaRef = 0;

			return shader.setUniformBlock(handle, &m_value, sizeof(PerInstanceCB));
		}

		virtual Handle handle(const char* name, const IShader& shader) const
		{
			return shader.getHandleUniformBlock(name);
		}

	private:
		struct PerInstanceCB
		{
			mat4f m_worldMat;
			mat4f m_MVPMat;
			mat4f m_MVMat;
			float m_alphaRef;
			float m_padding[3];
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

		virtual bool operator () (Handle handle, IShader& shader) const
		{
			return shader.setTextureBuffer(handle, m_sc.renderOp().m_weightsTB);
		}

		virtual Handle handle(const char* name, const IShader& shader) const
		{
			return shader.getHandleTextureBuffer(name);
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

		virtual bool operator () (Handle handle, IShader& shader) const
		{
			const RenderOp& ro = m_sc.renderOp();
			if (mat4f* mp = m_matrixPaletteTB->map<mat4f>(IBuffer::ACCESS_WRITE_DISCARD))
			{
				memcpy(mp, ro.m_matrixPalette, sizeof(mat4f) * ro.m_matrixPaletteCount);
				m_matrixPaletteTB->unmap();
			}
			return shader.setTextureBuffer(handle, m_matrixPaletteTB.get());
		}

		virtual Handle handle(const char* name, const IShader& shader) const
		{
			return shader.getHandleTextureBuffer(name);
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

		virtual bool operator () (Handle handle, IShader& shader) const
		{
			const RenderOp& ro = m_sc.renderOp();
			if (void* mp = ro.m_instanceTB->map<void>(IBuffer::ACCESS_WRITE_DISCARD))
			{
				memcpy(mp, ro.m_instanceData, ro.m_instanceSize * ro.m_instanceCount);
				ro.m_instanceTB->unmap();
			}
			return shader.setTextureBuffer(handle, m_sc.renderOp().m_instanceTB);
		}

		virtual Handle handle(const char* name, const IShader& shader) const
		{
			return shader.getHandleTextureBuffer(name);
		}

	private:
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

		virtual bool operator () (Handle handle, IShader& shader) const
		{
			return shader.setTexture(handle, rs().depthTexture(), m_samplerID);
		}

		virtual Handle handle(const char* name, const IShader& shader) const
		{
			return shader.getHandleTexture(name);
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

		virtual bool operator () (Handle handle, IShader& shader) const
		{
			return shader.setTexture(handle, rs().decalsMask(), m_samplerID);
		}

		virtual Handle handle(const char* name, const IShader& shader) const
		{
			return shader.getHandleTexture(name);
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

		virtual bool operator () (Handle handle, IShader& shader) const
		{
			return shader.setTexture(handle, rs().lightsMask(), m_samplerID);
		}

		virtual Handle handle(const char* name, const IShader& shader) const
		{
			return shader.getHandleTexture(name);
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

		virtual bool operator () (Handle handle, IShader& shader) const
		{
			return shader.setTexture(handle, rs().shadowsMask(), m_samplerID);
		}

		virtual Handle handle(const char* name, const IShader& shader) const
		{
			return shader.getHandleTexture(name);
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

		m_vertexDeclarations.reset(new VertexDeclarations);
		if (!m_vertexDeclarations->init())
			return false;

		//-- register auto-constants.
		m_autoProperties["cb_auto_PerInstance"]		= new PerInstanceProperty(*this);
		m_autoProperties["tb_auto_Weights"]			= new WeightsProperty(*this);
		m_autoProperties["tb_auto_MatrixPalette"]	= new MatrixPaletteProperty(*this);
		m_autoProperties["tb_auto_Instancing"]		= new InstancingProperty(*this);
		m_autoProperties["t_auto_depthMap"]			= new DepthMapAutoProperty(*this);
		m_autoProperties["t_auto_decalsMask"]		= new DecalsMaskAutoProperty(*this);
		m_autoProperties["t_auto_lightsMask"]		= new LightsMaskAutoProperty(*this);
		m_autoProperties["t_auto_shadowsMask"]		= new ShadowsMaskAutoProperty(*this);

		//-- init shader uniform buffers.
		m_perframeCB = rd()->createBuffer(
			IBuffer::TYPE_UNIFORM, nullptr, 1, sizeof(PerFrameConstants),
			IBuffer::USAGE_DYNAMIC, IBuffer::CPU_ACCESS_WRITE
			);

		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool ShaderContext::updateGlobalConstants()
	{
		//-- ToDo:
		return false;
	}

	//----------------------------------------------------------------------------------------------
	bool ShaderContext::updatePerFrameViewConstants()
	{
		m_perframeConstants.m_screenRes.x		 = rs().screenRes().width;
		m_perframeConstants.m_screenRes.y		 = rs().screenRes().height;
		m_perframeConstants.m_screenRes.z		 = 1.0f / rs().screenRes().width;
		m_perframeConstants.m_screenRes.w		 = 1.0f / rs().screenRes().height;

		m_perframeConstants.m_farNearPlane.x	 = m_camera->m_projInfo.nearDist;
		m_perframeConstants.m_farNearPlane.y	 = m_camera->m_projInfo.farDist;
		m_perframeConstants.m_farNearPlane.z	 = m_camera->m_projInfo.nearDist + m_camera->m_projInfo.farDist;
		m_perframeConstants.m_farNearPlane.w	 = 1.0f / m_perframeConstants.m_farNearPlane.z;

		m_perframeConstants.m_cameraPos			 = m_camera->m_invView.applyToOrigin().toVec4();
		m_perframeConstants.m_viewMat			 = m_camera->m_view;
		m_perframeConstants.m_invViewMat		 = m_camera->m_invView;

		m_perframeConstants.m_projMat			 = m_camera->m_proj;
		m_perframeConstants.m_invProjMat		 = m_camera->m_proj.getInverted();

		m_perframeConstants.m_viewProjMat		 = m_camera->m_viewProj;
		m_perframeConstants.m_invViewProjMat	 = m_camera->m_viewProj.getInverted();

		m_perframeConstants.m_lastViewProjMat	 = rs().lastViewProjMat();
		m_perframeConstants.m_invLastViewProjMat = rs().invLastViewProjMat();

		//-- update shader per-frame auto-constants.
		PerFrameConstants* p = m_perframeCB->map<PerFrameConstants>(IBuffer::ACCESS_WRITE_DISCARD);
		if (p)
		{
			*p = m_perframeConstants;
			m_perframeCB->unmap();
		}

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
			//-- find auto properties.
			Properties props;
			{
				//-- ToDo: reconsider.
				//-- retrive per-frame properties.
				Handle handle = shader->getHandleUniformBlock("cb_auto_PerFrame");
				if (handle != CONST_INVALID_HANDLE)
				{
					shader->changeUniformBuffer(handle, m_perframeCB);
				}

				//-- find another auto-properties.
				for (auto iter = m_autoProperties.begin(); iter != m_autoProperties.end(); ++iter)
				{
					Handle handle = iter->second->handle(iter->first.c_str(), *shader.get());
					if (handle != CONST_INVALID_HANDLE)
					{
						props.push_back(PropertyPair(handle, iter->second));
					}
				}
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
	VertexLayoutID ShaderContext::getVertexLayout(const char* name, Handle shaderID)
	{
		if (shaderID == CONST_INVALID_HANDLE)
		{
			return CONST_INVALID_HANDLE;
		}
		else
		{
			return m_vertexDeclarations->get(name, *m_shaderCache[shaderID].first.get());
		}
	}

	//----------------------------------------------------------------------------------------------
	IShader* ShaderContext::shader(Handle handle)
	{
		assert(handle != CONST_INVALID_HANDLE);

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
	void ShaderContext::applyFor(RenderOp* op)
	{
		m_renderOp = op;

		assert(op->m_material);

		const RenderFx&	fx = *op->m_material;

		assert(fx.m_shader);
				
		ShaderPair&	sp     = m_shaderCache[fx.m_shader];
		IShader*	shader = sp.first.get();

		assert(shader);

		//-- apply auto-properties predefined for this type of shader.
		for (auto i = sp.second.begin(); i != sp.second.end(); ++i)
		{
			PropertyPair& pp = *i;
			(*pp.second)(pp.first, *shader);
		}

		//-- apply user-configurable properties.
		for (uint i = 0; i != fx.m_propsCount; ++i)
		{
			PropertyPair& pp = fx.m_props[i];
			(*pp.second)(pp.first, *shader);
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

} //-- render
} //-- brUGE