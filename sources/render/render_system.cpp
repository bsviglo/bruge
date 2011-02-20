#include "render_system.hpp"

#include "Camera.h"
#include "state_objects.h"
#include "IRenderDevice.h"
#include "render_dll_Interface.h"
#include "materials.hpp"

#include "console/Console.h"
#include "console/WatchersPanel.h"
#include "console/TimingPanel.h"

#include "math/math_all.hpp"

using namespace brUGE;
using namespace brUGE::render;
using namespace brUGE::utils;
using namespace brUGE::math;

//-- start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	//-- 
	ShaderContext::EShaderRenderPassType g_render2shaderPass[] = 
	{
		ShaderContext::SHADER_RENDER_PASS_Z_ONLY,
		ShaderContext::SHADER_RENDER_PASS_SHADOW_CAST,
		ShaderContext::SHADER_RENDER_PASS_UNDEFINED,
		ShaderContext::SHADER_RENDER_PASS_UNDEFINED,
		ShaderContext::SHADER_RENDER_PASS_UNDEFINED,
		ShaderContext::SHADER_RENDER_PASS_MAIN_COLOR,
		ShaderContext::SHADER_RENDER_PASS_UNDEFINED,
		ShaderContext::SHADER_RENDER_PASS_UNDEFINED,
		ShaderContext::SHADER_RENDER_PASS_UNDEFINED
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

		if (!m_materials.init())
		{
			return false;
		}

		if (!m_postProcessing.init())
		{
			return false;
		}
		
		//-- register watchers.
		REGISTER_RO_WATCHER("primitives count", uint, m_device->m_primitivesCount);
		REGISTER_RO_WATCHER("draw calls count", uint, m_device->m_drawCallsCount);

		return _initPasses();
	}
	
	//----------------------------------------------------------------------------------------------
	void RenderSystem::shutDown()
	{
		if (m_dynamicLib.isLoaded())
		{
			_finiPasses();
			m_postProcessing.fini();
			m_materials.fini();
			m_shaderContext.fini();
			m_device->resetToDefaults();
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
	bool RenderSystem::_initPasses()
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

		//-- 2. PASS_DECAL
		{
			PassDesc& pass = m_passes[PASS_DECAL];

			DepthStencilStateDesc dsDesc;
			dsDesc.depthWriteMask = false;
			dsDesc.depthEnable	  = true;
			dsDesc.depthFunc	  = DepthStencilStateDesc::COMPARE_FUNC_LESS_EQUAL;
			pass.m_stateDS = m_device->createDepthStencilState(dsDesc);

			RasterizerStateDesc rDesc;
			rDesc.cullMode = RasterizerStateDesc::CULL_BACK;
			pass.m_stateR = m_device->createRasterizedState(rDesc);

			BlendStateDesc bDesc;
			bDesc.blendEnable[0] = true;
			bDesc.srcBlend  = BlendStateDesc::BLEND_FACTOR_SRC_ALPHA;
			bDesc.destBlend = BlendStateDesc::BLEND_FACTOR_INV_SRC_ALPHA;
			bDesc.blendOp   = BlendStateDesc::BLEND_OP_ADD;
			pass.m_stateB = m_device->createBlendState(bDesc);
		}
		
		//-- 3. PASS_MAIN_COLOR
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

		//-- 4. PASS_DEBUG_WIRE
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

		//-- 5. PASS_DEBUG_SOLID
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
	bool RenderSystem::_finiPasses()
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
		m_pass = type;

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
				PassDesc& pass = m_passes[PASS_DECAL];
				m_device->backToMainFrameBuffer();

				m_device->setRasterizerState(pass.m_stateR);
				m_device->setDepthStencilState(pass.m_stateDS, 0);
				m_device->setBlendState(pass.m_stateB, NULL, 0xffffffff);

				m_device->setViewPort(m_screenRes.width, m_screenRes.height);
				break;
			}
		case PASS_MAIN_COLOR:
			{
				PassDesc& pass = m_passes[PASS_MAIN_COLOR];

				m_device->backToMainFrameBuffer();
				m_device->clear(CLEAR_COLOR, Color(0.25f,0.25f,0.25f,1), 0.0f, 0);

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
		_doDraw(m_renderOps);
		return true;
	}

	//----------------------------------------------------------------------------------------------
	void RenderSystem::addImmediateRenderOps(const RenderOps& ops)
	{
		RenderOps rOps = ops;
		_doDraw(rOps);
	}

	//----------------------------------------------------------------------------------------------
	void RenderSystem::_doDraw(RenderOps& ops)
	{
		for (uint i = 0; i < ops.size(); ++i)
		{
			RenderOp&					ro = ops[i];
			const Materials::RenderMat& rm = *ro.m_material->m_shader;

			m_device->setVertexLayout(rm.m_vertDecl);
			m_device->setVertexBuffer(0, ro.m_mainVB);

			if (rm.m_isBumpMaped)
			{
				m_device->setVertexBuffer(1, ro.m_tangentVB);
			}
			if (ro.m_IB)
			{
				m_device->setIndexBuffer(ro.m_IB);
			}

			//-- apply shader for current pass.
			m_shaderContext.applyFor(&ro, g_render2shaderPass[m_pass]);

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
	}

	// console functions.
	//----------------------------------------------------------------------------------------------
	int RenderSystem::_printGAPIType()
	{
		ConWarning(g_renderDesc[gapi()].gapiString);
		return 0;
	}

} // render
} // brUGE

