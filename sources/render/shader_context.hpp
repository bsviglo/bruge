#pragma once

#include "render_common.h"
#include "render/IShader.h"
#include "render/IRenderDevice.h"
#include "render/state_objects.h"

#include <vector>
#include <map>

namespace brUGE
{
	typedef utils::Ptr<utils::ROData> RODataPtr;

namespace render
{

	//-- forward declaration.
	struct RenderOp;
	typedef std::vector<RenderOp> RenderOps;


	//-- The property of particular material. It represents some shader constants and another
	//-- graphic data like texture, texture buffers and so on. Property may be two types:
	//-- 1. common - means that it is specific for each shader.
	//-- 2. auto   - means that this property is shared between another shader and maybe shader stages.
	//--             e.g. ViewMat, ProjMat, CameraPos and so on.
	//----------------------------------------------------------------------------------------------
	class IProperty : public NonCopyable
	{
	public:
		IProperty() { }
		virtual ~IProperty() { }
		virtual bool apply(IShader& shader) const = 0;

	};
	typedef std::vector<IProperty*> Properties;


	//----------------------------------------------------------------------------------------------
	template<typename T>
	class NumericProperty : public IProperty
	{
	public:
		NumericProperty(const std::string& name, const T& value)
			:	m_name(name), m_value(value) { }

		virtual bool apply(IShader& shader) const
		{
			return shader.setConstantAsRawData(m_name.c_str(), &m_value, sizeof(T));
		}

		void set(const T& value)
		{
			m_value = value;
		}

	private:
		std::string	m_name;
		T			m_value;
	};


	//----------------------------------------------------------------------------------------------
	class TextureProperty : public IProperty
	{
	public:
		TextureProperty(const std::string& name, const Ptr<ITexture>& texture, SamplerStateID state);
		~TextureProperty() { }

		virtual bool apply(IShader& shader) const;

		void set(const Ptr<ITexture>& value)
		{
			m_texture = value;
		}

	private:
		SamplerStateID	m_stateS;
		std::string		m_textureName;
		std::string		m_samplerName;
		Ptr<ITexture>	m_texture;
	};


	//-- Shader include interface implementation.
	//----------------------------------------------------------------------------------------------
	class ShaderIncludeImpl : public render::IShaderInclude
	{
	public:
		ShaderIncludeImpl(const std::string& path);
		virtual ~ShaderIncludeImpl();

		virtual bool open (const char* name, const void*& data, uint& size);
		virtual bool close(const void*& data);

	private:
		typedef std::pair<const void*, RODataPtr>	Include;
		typedef std::vector<Include>				Includes;

		Includes    m_includes;
		std::string m_path;
	};	


	//-- Controls life time cycle of all the shader in the engine and does some additional work
	//-- related to auto-constant applying, sharing some common uniform buffers, managing include
	//-- interface for shaders and so on.
	//----------------------------------------------------------------------------------------------
	class ShaderContext
	{
	public:
		enum EShaderRenderPassType
		{
			SHADER_RENDER_PASS_UNDEFINED	= 0,
			SHADER_RENDER_PASS_Z_ONLY		= 0,
			SHADER_RENDER_PASS_SHADOW_CAST	= 1,
			SHADER_RENDER_PASS_MAIN_COLOR	= 2,
			SHADER_RENDER_PASS_COUNT
		};

		enum EShaderPin
		{
			PIN_NO_ONE	   = 0,
			PIN_BUMP_MAP   = 1 << 0,
			PIN_ALPHA_TEST = 1 << 1,

			PIN_MAX_COUNT  = 2
		};

	public:
		ShaderContext();
		~ShaderContext();

		bool init();
		bool fini();

		//-- load shader.
		Handle			getShader(const char* name, const std::vector<std::string>* pins);
		VertexLayoutID	getVertexLayout(Handle shader, const std::string& desc);
		IShader*		shader(Handle handle);

		const RenderOp& renderOp() const { return *m_renderOp; }
		const Camera&   camera() const   { return *m_camera; }

		void			setCamera(Camera* cam);
		void			applyFor(RenderOp* op, EShaderRenderPassType pass);

	private:
		Handle loadShader(const char* name, const std::vector<std::string>* pins);

	private:
		typedef std::pair<Ptr<IShader>, Properties>	ShaderPair;
		typedef std::vector<ShaderPair>				ShaderAutoProperties;
		typedef std::map<std::string, IProperty*>	AutoProperties;
		typedef std::auto_ptr<ShaderIncludeImpl>	ShaderIncludeImplPtr;
		typedef std::map<std::string, Handle>		ShaderSearchMap;

		ShaderSearchMap		 m_searchMap;
		ShaderAutoProperties m_shaderCache;
		AutoProperties		 m_autoProperties;
		RenderOp*		 	 m_renderOp;
		Camera*				 m_camera;
		ShaderIncludeImplPtr m_shaderIncludes;
	};

} // render
} // brUGE