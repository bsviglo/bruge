#include "render_system.hpp"

#include "Camera.h"
#include "state_objects.h"
#include "IRenderDevice.h"
#include "render_dll_Interface.h"

#include "console/Console.h"
#include "console/WatchersPanel.h"
#include "console/TimingPanel.h"

#include "math/Matrix4x4.h"
#include "math/Vector3.h"
#include "math/Vector4.h"

using namespace brUGE;
using namespace brUGE::render;
using namespace brUGE::utils;
using namespace brUGE::os;
using namespace brUGE::math;

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
			m_value.viewMat     = m_sc.camera().viewMatrix();
			m_value.viewProjMat = m_sc.camera().viewProjMatrix();

			return shader.setUniformBlock("cb_auto_PerFrame", &m_value, sizeof(PerFrameCB));
		}

	private:
		struct PerFrameCB
		{
			mat4f viewMat;
			mat4f viewProjMat;
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


	//-- 
	struct RenderDesc
	{
		const char* dllString;
		const char* gapiString;
		const char* shaderExt;
	};

	//-- 
	RenderDesc const g_renderDesc[] = 
	{
#ifdef _DEBUG
		{ "GL3Render_d.dll",	"OpenGL 3.3",	"glsl"},
		{ "DX10Render_d.dll",	"DirectX 10",	"hlsl"}
#else
		{ "GL3Render.dll",		"OpenGL 3.3",	"glsl"},
		{ "DX10Render.dll",		"DirectX 10",	"hlsl"}
#endif //-- _DEBUG
	};
}
//--------------------------------------------------------------------------------------------------
//-- end unnamed namespace.


namespace brUGE
{
	DEFINE_SINGLETON(render::RenderSystem)

namespace render
{

	//-- ToDo: reconsider.
	/*static*/ SamplerStateID TextureProperty::m_samplerID = CONST_INVALID_HANDLE;

	//----------------------------------------------------------------------------------------------
	RenderSystem::RenderSystem() : m_renderAPI(RENDER_API_DX10)
	{
		//-- register console functions.
		REGISTER_CONSOLE_METHOD("r_gapiType", _printGAPIType, RenderSystem);		
	}

	//----------------------------------------------------------------------------------------------
	RenderSystem::~RenderSystem()
	{

	}

	//----------------------------------------------------------------------------------------------
	bool RenderSystem::init(ERenderAPIType apiType, HWND hWindow, const VideoMode& videoMode)
	{
		m_renderAPI = apiType;
		m_videoMode = videoMode;

		m_screenRes.width  = m_videoMode.width;
		m_screenRes.height = m_videoMode.height;

		if (!m_dynamicLib.load(g_renderDesc[m_renderAPI].dllString))
		{
			ERROR_MSG("Can't load render system '%s'.", g_renderDesc[m_renderAPI].dllString);
			return false;
		}

		createRender create = static_cast<createRender>(m_dynamicLib.getSymbol("createRender"));
		if (create == NULL)
		{
			ERROR_MSG("function 'getSymbol('createRender')' return NULL pointer.");
			shutDown(); //-- unload library.
			return false;
		}

		RenderDllInterface rcc;
		create(&rcc, &utils::LogManager::instance());
		m_device = rcc.device;

		if (!m_device->init(hWindow, videoMode))
		{
			return false;
		}

		if (!m_shaderContext.init())
		{
			return false;
		}
		
		//-- register watchers.
		REGISTER_RO_WATCHER("primitives count", uint, m_device->m_primitivesCount);
		REGISTER_RO_WATCHER("draw calls count", uint, m_device->m_drawCallsCount);

		return initPasses();
	}
	
	//----------------------------------------------------------------------------------------------
	void RenderSystem::shutDown()
	{
		if (m_dynamicLib.isLoaded())
		{
			finiPasses();
			m_shaderContext.fini();
			m_device->shutDown();

			destroyRender destroy = static_cast<destroyRender>(m_dynamicLib.getSymbol("destroyRender"));
			if (destroy == NULL)
			{
				ERROR_MSG("function 'getSymbol('destroyRender')' return NULL pointer.");
				return;
			}

			destroy();
			m_device = NULL;
			m_dynamicLib.free();
		}
	}

	//----------------------------------------------------------------------------------------------
	bool RenderSystem::initPasses()
	{
		//-- 1. PASS_Z_ONLY
		{
			PassDesc& pass = m_passes[PASS_Z_ONLY];

			DepthStencilStateDesc dsDesc;
			dsDesc.depthWriteMask = true;
			dsDesc.depthEnable	  = true;
			dsDesc.depthFunc	  = DepthStencilStateDesc::COMPARE_FUNC_LESS_EQUAL;
			pass.m_stateDS = m_device->createDepthStencilState(dsDesc);

			RasterizerStateDesc rDesc;
			rDesc.cullMode = RasterizerStateDesc::CULL_BACK;
			pass.m_stateR = m_device->createRasterizedState(rDesc);
	
			BlendStateDesc bDesc;
			pass.m_stateB = m_device->createBlendState(bDesc);

			//-- create depth texture.
			{
				ITexture::Desc desc;
				desc.width     = m_screenRes.width;
				desc.height    = m_screenRes.height;
				desc.bindFalgs = ITexture::BIND_RENDER_TARGET | ITexture::BIND_SHADER_RESOURCE;
				desc.format	   = ITexture::FORMAT_R32F;
				desc.texType   = ITexture::TYPE_2D;

				pass.m_rt = m_device->createTexture(desc, NULL, 0);
				if (!pass.m_rt)
					return false;
			}
		}
		
		//-- 2. PASS_MAIN_COLOR
		{
			PassDesc& pass = m_passes[PASS_MAIN_COLOR];

			DepthStencilStateDesc dsDesc;
			dsDesc.depthWriteMask = false;
			dsDesc.depthEnable	  = true;
			dsDesc.depthFunc	  = DepthStencilStateDesc::COMPARE_FUNC_EQUAL;
			pass.m_stateDS = m_device->createDepthStencilState(dsDesc);

			RasterizerStateDesc rDesc;
			rDesc.cullMode = RasterizerStateDesc::CULL_BACK;
			pass.m_stateR = m_device->createRasterizedState(rDesc);

			BlendStateDesc bDesc;
			pass.m_stateB = m_device->createBlendState(bDesc);
		}

		//-- 3. PASS_DEBUG_WIRE
		{
			PassDesc& pass = m_passes[PASS_DEBUG_WIRE];

			DepthStencilStateDesc dsDesc;
			dsDesc.depthWriteMask = false;
			dsDesc.depthEnable	  = true;
			dsDesc.depthFunc	  = DepthStencilStateDesc::COMPARE_FUNC_ALWAYS;
			pass.m_stateDS = m_device->createDepthStencilState(dsDesc);

			RasterizerStateDesc rDesc;
			pass.m_stateR = m_device->createRasterizedState(rDesc);

			BlendStateDesc bDesc;
			pass.m_stateB = m_device->createBlendState(bDesc);
		}

		//-- 3. PASS_DEBUG_SOLID
		{
			PassDesc& pass = m_passes[PASS_DEBUG_SOLID];

			DepthStencilStateDesc dsDesc;
			dsDesc.depthWriteMask = true;
			dsDesc.depthEnable	  = true;
			dsDesc.depthFunc	  = DepthStencilStateDesc::COMPARE_FUNC_LESS_EQUAL;
			pass.m_stateDS = m_device->createDepthStencilState(dsDesc);

			RasterizerStateDesc rDesc;
			rDesc.cullMode = RasterizerStateDesc::CULL_BACK;
			pass.m_stateR = m_device->createRasterizedState(rDesc);

			BlendStateDesc bDesc;
			pass.m_stateB = m_device->createBlendState(bDesc);
		}
		
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool RenderSystem::finiPasses()
	{
		for (uint i = 0; i < PASS_COUNT; ++i)
		{
			m_passes[i].m_rt = NULL;
		}

		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool RenderSystem::beginFrame()
	{
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool RenderSystem::endFrame()
	{
		SCOPED_TIME_MEASURER_EX("endFrame");

		m_device->resetToDefaults();
		m_device->swapBuffers();

		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool RenderSystem::beginPass(EPassType type)
	{
		switch (type)
		{
		case PASS_Z_ONLY:
			{
				PassDesc& pass = m_passes[PASS_Z_ONLY];

				m_device->clear(CLEAR_DEPTH, Color(), 1.0f, 0);
				m_device->clearColorRT(pass.m_rt.get(), Color(1,1,1,1));
				m_device->setRenderTarget(pass.m_rt.get(), rd()->getMainDepthRT());

				m_device->setRasterizerState(pass.m_stateR);
				m_device->setDepthStencilState(pass.m_stateDS, 0);
				m_device->setBlendState(pass.m_stateB, NULL, 0xffffffff);

				m_device->setViewPort(m_screenRes.width, m_screenRes.height);
				break;
			}
		case PASS_SHADOW_CAST:
			{
				break;
			}
		case PASS_SHADOW_RECEIVE:
			{
				break;
			}
		case PASS_DECAL:
			{
				break;
			}
		case PASS_MAIN_COLOR:
			{
				PassDesc& pass = m_passes[PASS_MAIN_COLOR];

				m_device->backToMainFrameBuffer();
				m_device->clear(CLEAR_COLOR, Color(0,0,0,0), 0.0f, 0);

				m_device->setRasterizerState(pass.m_stateR);
				m_device->setDepthStencilState(pass.m_stateDS, 0);
				m_device->setBlendState(pass.m_stateB, NULL, 0xffffffff);

				m_device->setViewPort(m_screenRes.width, m_screenRes.height);
				break;
			}
		case PASS_POST_PROCESSING:
			{
				break;
			}
		case PASS_DEBUG_WIRE:
			{
				PassDesc& pass = m_passes[PASS_DEBUG_WIRE];

				m_device->backToMainFrameBuffer();

				m_device->setRasterizerState(pass.m_stateR);
				m_device->setDepthStencilState(pass.m_stateDS, 0);
				m_device->setBlendState(pass.m_stateB, NULL, 0xffffffff);

				m_device->setViewPort(m_screenRes.width, m_screenRes.height);
				break;
			}
		case PASS_DEBUG_SOLID:
			{
				PassDesc& pass = m_passes[PASS_DEBUG_SOLID];

				m_device->backToMainFrameBuffer();

				m_device->setRasterizerState(pass.m_stateR);
				m_device->setDepthStencilState(pass.m_stateDS, 0);
				m_device->setBlendState(pass.m_stateB, NULL, 0xffffffff);

				m_device->setViewPort(m_screenRes.width, m_screenRes.height);
				break;
			}
		default:
			{
				assert(0 && "invalid pass type.");
				return false;
			}
		};

		return true;
	}

	//----------------------------------------------------------------------------------------------
	void RenderSystem::setCamera(Camera* cam)
	{
		assert(cam);

		m_camera = cam;
		m_shaderContext.setCamera(cam);
	}

	//----------------------------------------------------------------------------------------------
	void RenderSystem::addRenderOps(const RenderOps& ops)
	{
		//-- ToDo: Do some kind of sorting of render operations.

		m_renderOps = ops;
	}

	//----------------------------------------------------------------------------------------------
	bool RenderSystem::endPass()
	{
		for (uint i = 0; i < m_renderOps.size(); ++i)
		{
			RenderOp& ro = m_renderOps[i];

			m_device->setVertexLayout(ro.m_material->m_vrtsLayout);
			m_device->setVertexBuffer(0, ro.m_mainVB);

			if (ro.m_material->m_isBumped)
			{
				m_device->setVertexBuffer(1, ro.m_tangentVB);
			}
			if (ro.m_IB)
			{
				m_device->setIndexBuffer(ro.m_IB);
			}

			m_shaderContext.applyFor(&ro);

			if (ro.m_instanceCount != 0)
			{
				assert(ro.m_instanceTB);
				m_device->drawIndexedInstanced(
					ro.m_primTopolpgy, 0, ro.m_indicesCount, ro.m_instanceCount
					);	
			}
			else
			{
				if (ro.m_IB)
				{
					m_device->drawIndexed(ro.m_primTopolpgy, 0, ro.m_indicesCount);
				}
				else
				{
					m_device->draw(ro.m_primTopolpgy, 0, ro.m_indicesCount);
				}
			}
		}

		return true;
	}

	// console functions.
	//----------------------------------------------------------------------------------------------
	int RenderSystem::_printGAPIType()
	{
		ConWarning(g_renderDesc[gapi()].gapiString);
		return 0;
	}


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

		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool ShaderContext::fini()
	{
		m_autoProperties.clear();
		m_shaderCashe.clear();
		m_searchMap.clear();

		m_shaderIncludes.reset();

		return true;
	}

	//----------------------------------------------------------------------------------------------
	Handle ShaderContext::loadShader(const char* name)
	{
		FileSystem& fs = FileSystem::instance();

		RODataPtr data = fs.readFile(makeStr("resources/shaders/%s.%s",
			name, g_renderDesc[rs().gapi()].shaderExt).c_str()
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
			m_shaderCashe.push_back(make_pair(shader, props));

			//-- add to shader search map.
			m_searchMap[name] = m_shaderCashe.size() - 1;

			return m_shaderCashe.size() - 1;
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
			shader = m_shaderCashe[shaderID].first.get();
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
	void ShaderContext::applyFor(RenderOp* op)
	{
		m_renderOp = op;

		const RenderFx&	fx	   = *op->m_material;
		ShaderPair&		sp     = m_shaderCashe[fx.m_shader];
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

} // render
} // brUGE

