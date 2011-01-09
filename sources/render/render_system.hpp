#pragma once

#include "render_common.h"
#include "os/FileSystem.h"
#include "utils/string_utils.h"
#include "utils/Singleton.h"
#include "utils/DynamicLib.h"
#include "render/IShader.h"
#include "render/IRenderDevice.h"
#include "render/state_objects.h"

#include <memory>
#include <vector>
#include <map>

namespace brUGE
{
namespace render
{

	//-- The property of particular material. It represents some shader constants and another
	//-- graphic data like texture, texture buffers and so on. Property may be two types:
	//-- 1. common - means that it is specific for each shader.
	//-- 2. auto   - means that this property is shared between another shader and maybe shader stages.
	//--             e.g. ViewMat, ProjMat, CameraPos and so on.
	//----------------------------------------------------------------------------------------------
	class IProperty : public utils::RefCount
	{
	public:
		IProperty() { }
		virtual ~IProperty() { }
		virtual bool apply(IShader& shader) const = 0;

	private:
		IProperty(const IProperty&);
		IProperty& operator = (const IProperty&);
	};
	typedef std::vector<Ptr<IProperty>> Properties;


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

	private:
		std::string	m_name;
		T			m_value;
	};


	//-- Represents material of the given geometry.
	//----------------------------------------------------------------------------------------------
	struct RenderFx
	{
		RenderFx()
			:	m_shader(CONST_INVALID_HANDLE), m_props(0), m_propsCount(0),
				m_vrtsLayout(CONST_INVALID_HANDLE)
		{ }

		bool			m_isOpaque;
		bool			m_isSkinned;
		bool			m_isBumped;

		VertexLayoutID	m_vrtsLayout;
		Handle			m_shader;
		IProperty**		m_props;
		uint			m_propsCount;
	};


	//-- Represents the minimum quantum of the engine render system work.
	//----------------------------------------------------------------------------------------------
	struct RenderOp
	{
		RenderOp()
			:	m_primTopolpgy(PRIM_TOPOLOGY_TRIANGLE_LIST), m_mainVB(0), m_tangentVB(0), m_IB(0),
				m_weightsTB(0), m_matrixPaletteCount(0), m_matrixPalette(0), m_indicesCount(0),
				m_instanceTB(nullptr), m_instanceCount(0), m_worldMat(nullptr), m_material(nullptr)
		{ }

		//-- primitive topology of geometry.
		uint16				m_indicesCount;
		EPrimitiveTopology	m_primTopolpgy;
		//-- main data of static mesh.
		IBuffer*			m_IB;
		IBuffer*			m_mainVB;
		IBuffer*			m_tangentVB;
		//-- addition data in case if mesh is animated.
		IBuffer*			m_weightsTB;
		const mat4f*		m_matrixPalette;
		uint				m_matrixPaletteCount;
		//-- material of given sub-mesh.
		const RenderFx*		m_material;
		//-- world transformation.
		const mat4f*		m_worldMat;
		//-- instance data.
		IBuffer*			m_instanceTB;
		uint16				m_instanceCount;
	};
	typedef std::vector<RenderOp> RenderOps;


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
		ShaderContext();
		~ShaderContext();

		bool init();
		bool fini();

		//-- load shader.
		Handle			getShader(const char* name);
		VertexLayoutID	getVertexLayout(Handle shader, const std::string& desc);
		
		const RenderOp& renderOp() const { return *m_renderOp; }
		const Camera&   camera() const   { return *m_camera; }

		void			setCamera(Camera* cam);
		void			applyFor(RenderOp* op);

	private:
		Handle loadShader(const char* name);

	private:
		typedef std::pair<Ptr<IShader>, Properties>		ShaderPair;
		typedef std::vector<ShaderPair>					ShaderAutoProperties;
		typedef std::map<std::string, Ptr<IProperty>>	AutoProperties;
		typedef std::auto_ptr<ShaderIncludeImpl>		ShaderIncludeImplPtr;
		typedef std::map<std::string, Handle>			ShaderSearchMap;

		ShaderSearchMap		 m_searchMap;
		ShaderAutoProperties m_shaderCashe;
		AutoProperties		 m_autoProperties;
		RenderOp*		 	 m_renderOp;
		Camera*				 m_camera;
		ShaderIncludeImplPtr m_shaderIncludes;
	};


	//-- The main class of the render system.
	//----------------------------------------------------------------------------------------------
	class RenderSystem : public utils::Singleton<RenderSystem>
	{
	public:
		enum EPassType
		{
			PASS_Z_ONLY,	
			PASS_SHADOW_CAST,
			PASS_SHADOW_RECEIVE,
			PASS_DECAL,
			PASS_LIGHT,
			PASS_MAIN_COLOR,
			PASS_POST_PROCESSING,
			PASS_DEBUG_WIRE,
			PASS_DEBUG_SOLID,
			PASS_COUNT
		};

		struct PassDesc
		{
			Ptr<ITexture>		m_rt;
			DepthStencilStateID m_stateDS;
			RasterizerStateID	m_stateR;
			BlendStateID		m_stateB;
		};

	public:
		RenderSystem();
		~RenderSystem();

		bool init(ERenderAPIType apiType, HWND hWindow, const VideoMode& videoMode);
		void shutDown();

		bool beginFrame();
		bool endFrame();

		bool beginPass(EPassType type);
		void setCamera(Camera* cam);
		void addRenderOps(const RenderOps& ops);
		bool endPass();

		const Camera&		camera()		const { return *m_camera; }
		ERenderAPIType		gapi()			const { return m_renderAPI; }
		ScreenResolution	screenRes()		const { return m_screenRes;  }
		Projection			projection()	const { return m_projection; }
		IRenderDevice*		device()		const { return m_device; }
		ShaderContext*		shaderContext()		  { return &m_shaderContext; }

	private:
		// console functions.
		int _printGAPIType();

		bool initPasses();
		bool finiPasses();

	private:
		VideoMode			m_videoMode;
		Projection			m_projection;
		ScreenResolution	m_screenRes;
		ERenderAPIType		m_renderAPI;
		utils::DynamicLib	m_dynamicLib;
		ShaderContext		m_shaderContext;

		Camera*				m_camera; //-- ToDo: use render camera instead.
		EPassType			m_pass;
		RenderOps			m_renderOps;

		//-- some addition resources for different render passes.
		PassDesc			m_passes[PASS_COUNT];

		IRenderDevice* 		m_device; // deleting performed after closing library *Render.dll
	};

	inline IRenderDevice* rd() { return RenderSystem::instance().device(); }
	inline RenderSystem&  rs() { return RenderSystem::instance(); }


	//----------------------------------------------------------------------------------------------
	class TextureProperty : public IProperty
	{
	public:
		TextureProperty(const std::string& name, const Ptr<ITexture>& texture)
			:	m_texture(texture)
		{
			m_textureName = utils::makeStr("%s_tex", name.c_str());
			m_samplerName = utils::makeStr("%s_sml", name.c_str());
		}

		virtual bool apply(IShader& shader) const
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
				m_samplerID = render::rd()->createSamplerState(sDesc);

				inited = true;
			}

			bool result = true;

			result &= shader.setTexture(m_textureName.c_str(), m_texture.get());
			result &= shader.setSampler(m_samplerName.c_str(), m_samplerID);

			return result;
		}

	private:
		static SamplerStateID	m_samplerID;
		std::string				m_textureName;
		std::string				m_samplerName;
		Ptr<ITexture>			m_texture;
	};

} // render
} // brUGE
