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
	class  VertexDeclarations;
	struct RenderOp;
	typedef std::vector<RenderOp> RenderOps;


	//----------------------------------------------------------------------------------------------
	struct RenderStateProperties
	{
		RenderStateProperties() : m_doubleSided(false) { }

		bool m_doubleSided;
	};


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
	
		virtual bool   operator() (Handle handle, IShader& shader) const = 0;
		virtual Handle handle	  (const char* name, const IShader& shader) const = 0;
	};
	typedef std::pair<Handle, IProperty*> PropertyPair;
	typedef std::vector<PropertyPair>	  Properties;


	//----------------------------------------------------------------------------------------------
	class FloatProperty : public IProperty
	{
	public:
		FloatProperty(float value) : m_value(value) { }

		virtual bool operator() (Handle handle, IShader& shader) const
		{
			return shader.setFloat(handle, m_value);
		}

		virtual Handle handle(const char* name, const IShader& shader) const
		{
			return shader.getHandleFloat(name);
		}

		void set(float value)
		{
			m_value = value;
		}

	private:
		float m_value;
	};


	//----------------------------------------------------------------------------------------------
	class Vec4fProperty : public IProperty
	{
	public:
		Vec4fProperty(const vec4f& value) : m_value(value) { }

		virtual bool operator() (Handle handle, IShader& shader) const
		{
			return shader.setVec4f(handle, m_value);
		}

		virtual Handle handle(const char* name, const IShader& shader) const
		{
			return shader.getHandleVec4f(name);
		}

		void set(const vec4f& value)
		{
			m_value = value;
		}

	private:
		vec4f m_value;
	};


	//----------------------------------------------------------------------------------------------
	class TextureProperty : public IProperty
	{
	public:
		TextureProperty(const Ptr<ITexture>& texture, SamplerStateID state)
			:	m_texture(texture), m_stateS(state) { }

		virtual bool operator() (Handle handle, IShader& shader) const
		{
			return shader.setTexture(handle, m_texture.get(), m_stateS);
		}

		virtual Handle handle(const char* name, const IShader& shader) const
		{
			return shader.getHandleTexture(name);
		}

		void set(const Ptr<ITexture>& value = nullptr, SamplerStateID state = CONST_INVALID_HANDLE)
		{
			if (value.isValid())				m_texture = value;
			if (state != CONST_INVALID_HANDLE)	m_stateS  = state;
			
		}

	private:
		SamplerStateID	m_stateS;
		Ptr<ITexture>	m_texture;
	};


	//-- Shader include interface implementation.
	//----------------------------------------------------------------------------------------------
	class ShaderIncludeImpl : public IShaderInclude
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
		enum EPassType
		{
			PASS_UNDEFINED	 = 0,
			PASS_Z_ONLY		 = 0,
			PASS_SHADOW_CAST = 1,
			PASS_MAIN_COLOR	 = 2,
			PASS_COUNT
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

		bool				init();
		bool				updateGlobalConstants();
		bool				updatePerFrameViewConstants();

		//-- load shader.
		Handle				getShader(const char* name, const std::vector<std::string>* pins);
		VertexLayoutID		getVertexLayout(const char* name, Handle shader);
		IShader*			shader(Handle handle);

		const RenderOp&		renderOp() const { return *m_renderOp; }
		const RenderCamera* camera() const   { return m_camera; }

		void				setCamera(const RenderCamera* cam);
		void				applyFor(RenderOp* op);

	private:
		Handle loadShader(const char* name, const std::vector<std::string>* pins);

	private:
		typedef std::pair<Ptr<IShader>, Properties>	ShaderPair;
		typedef std::vector<ShaderPair>				ShaderAutoProperties;
		typedef std::map<std::string, IProperty*>	AutoProperties;
		typedef std::map<std::string, Handle>		ShaderSearchMap;
		typedef std::auto_ptr<ShaderIncludeImpl>	ShaderIncludeImplPtr;
		typedef std::auto_ptr<VertexDeclarations>	VertexDeclarationsPtr;

		ShaderSearchMap		  m_searchMap;
		ShaderAutoProperties  m_shaderCache;
		AutoProperties		  m_autoProperties;
		RenderOp*		 	  m_renderOp;
		const RenderCamera*	  m_camera;
		ShaderIncludeImplPtr  m_shaderIncludes;
		VertexDeclarationsPtr m_vertexDeclarations;

		//-- ToDo:

		//--
		struct GlobalConstants
		{
			vec4f m_time;
		};

		//--
		struct PerFrameConstants
		{
			vec4f m_screenRes;
			vec4f m_farNearPlane;
			vec4f m_cameraPos;
			mat4f m_viewMat;
			mat4f m_invViewMat;
			mat4f m_projMat;
			mat4f m_invProjMat;
			mat4f m_viewProjMat;
			mat4f m_invViewProjMat;
			mat4f m_lastViewProjMat;
			mat4f m_invLastViewProjMat;
		};

		Ptr<IBuffer>		m_globalCB;
		Ptr<IBuffer>		m_perframeCB;
		GlobalConstants		m_globalConstants;
		PerFrameConstants	m_perframeConstants;
	};

} // render
} // brUGE