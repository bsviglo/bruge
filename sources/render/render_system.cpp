#include "render_system.hpp"

#include "Camera.h"
#include "state_objects.h"
#include "IRenderDevice.h"
#include "render_dll_Interface.h"
#include "materials.hpp"
#include "SDL/SDL_loadso.h"

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
	ShaderContext::EPassType g_render2shaderPass[] = 
	{
		ShaderContext::PASS_Z_ONLY,
		ShaderContext::PASS_UNDEFINED,
		ShaderContext::PASS_UNDEFINED,
		ShaderContext::PASS_SHADOW_CAST,
		ShaderContext::PASS_UNDEFINED,
		ShaderContext::PASS_MAIN_COLOR,
		ShaderContext::PASS_UNDEFINED,
		ShaderContext::PASS_UNDEFINED,
		ShaderContext::PASS_UNDEFINED,
		ShaderContext::PASS_UNDEFINED
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
		{ "D3D11Render_d.dll",	"D3D11",		"hlsl"}
#else
		{ "GL3Render.dll",		"OpenGL 3.3",	"glsl"},
		{ "D3D11Render.dll",	"D3D11",		"hlsl"}
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

	//------------------------------------------------------------------------------------------------------------------
	bool RenderSystem::init()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	void RenderSystem::update(IWorld* world, const DeltaTime& dt)
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	void RenderSystem::process(IContext* context)
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	bool RenderSystem::checkRequiredComponents(Handle handle) const
	{
		GameObject* gameObj = nullptr;

		gameObj->hasComponentByType(IRE)
	}

	//------------------------------------------------------------------------------------------------------------------


	//----------------------------------------------------------------------------------------------
	RenderSystem::RenderSystem()
		:	m_renderAPI(RENDER_API_D3D11),
			m_shaderContext(new ShaderContext),
			m_materials(new Materials),
			m_renderModuleDLL(nullptr)

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

		m_renderModuleDLL = SDL_LoadObject(g_renderDesc[m_renderAPI].dllString);
		if (!m_renderModuleDLL)
		{
			ERROR_MSG("Can't load render system '%s'.", g_renderDesc[m_renderAPI].dllString);
			return false;
		}

		if (auto create = static_cast<createRender>(SDL_LoadFunction(m_renderModuleDLL, "createRender")))
		{
			RenderDllInterface rcc;
			create(&rcc, &utils::LogManager::instance());
			m_device = rcc.device;
		}
		else
		{
			ERROR_MSG("There is no 'createRender' function in render dll.");
			return false;
		}

		if (!m_device->init(hWindow, videoMode))
		{
			return false;
		}

		if (!m_shaderContext->init())
		{
			return false;
		}

		if (!m_materials->init())
		{
			return false;
		}

		return _initPasses();
	}
	
	//----------------------------------------------------------------------------------------------
	void RenderSystem::shutDown()
	{
		if (m_renderModuleDLL)
		{
			_finiPasses();

			m_materials.reset();
			m_shaderContext.reset();

			m_device->resetToDefaults();
			m_device->shutDown();

			if (auto destroy = static_cast<destroyRender>(SDL_LoadFunction(m_renderModuleDLL, "destroyRender")))
			{
				destroy();
			}
			else
			{
				ERROR_MSG("There is not 'destroyRender' function in render dll.");
				return;
			}

			m_device = nullptr;
			SDL_UnloadObject(m_renderModuleDLL);
			m_renderModuleDLL = nullptr;
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
			rDesc.cullMode			= RasterizerStateDesc::CULL_BACK;
			rDesc.multisampleEnable = (m_videoMode.multiSampling.m_count > 1);
			pass.m_stateR = m_device->createRasterizedState(rDesc);

			rDesc.cullMode = RasterizerStateDesc::CULL_NOTHING;
			pass.m_stateR_doubleSided = m_device->createRasterizedState(rDesc);

			rDesc.cullMode = RasterizerStateDesc::CULL_BACK;
			rDesc.fillMode = RasterizerStateDesc::FILL_WIREFRAME;
			pass.m_stateR_wireframe = m_device->createRasterizedState(rDesc);
	
			BlendStateDesc bDesc;
			pass.m_stateB = m_device->createBlendState(bDesc);

			//-- create depth texture.
			{
				ITexture::Desc desc;
				desc.width			= m_screenRes.width;
				desc.height			= m_screenRes.height;
				desc.sample.count	= m_videoMode.multiSampling.m_count;
				desc.sample.quality	= m_videoMode.multiSampling.m_quality;
				desc.bindFalgs		= ITexture::BIND_RENDER_TARGET | ITexture::BIND_SHADER_RESOURCE;
				desc.format			= ITexture::FORMAT_RGBA32F;
				desc.texType		= ITexture::TYPE_2D;

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
			bDesc.srcBlend		 = BlendStateDesc::BLEND_FACTOR_SRC_ALPHA;
			bDesc.destBlend	 	 = BlendStateDesc::BLEND_FACTOR_INV_SRC_ALPHA;
			bDesc.blendOp		 = BlendStateDesc::BLEND_OP_ADD;
			bDesc.srcBlendAlpha  = BlendStateDesc::BLEND_FACTOR_SRC_ALPHA;
			bDesc.destBlendAlpha = BlendStateDesc::BLEND_FACTOR_INV_SRC_ALPHA;
			bDesc.blendAlphaOp   = BlendStateDesc::BLEND_OP_ADD;
			pass.m_stateB = m_device->createBlendState(bDesc);

			//-- create decals mask texture.
			{
				ITexture::Desc desc;
				desc.width     = m_screenRes.width;
				desc.height    = m_screenRes.height;
				desc.bindFalgs = ITexture::BIND_RENDER_TARGET | ITexture::BIND_SHADER_RESOURCE;
				desc.format	   = ITexture::FORMAT_RGBA8;
				desc.texType   = ITexture::TYPE_2D;

				pass.m_rt = m_device->createTexture(desc, NULL, 0);
				if (!pass.m_rt)
					return false;
			}
		}

		//-- 3. PASS_LIGHT
		{
			PassDesc& pass = m_passes[PASS_LIGHT];

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
			bDesc.srcBlend		 = BlendStateDesc::BLEND_FACTOR_ONE;
			bDesc.destBlend	 	 = BlendStateDesc::BLEND_FACTOR_ONE;
			bDesc.blendOp		 = BlendStateDesc::BLEND_OP_ADD;
			bDesc.srcBlendAlpha  = BlendStateDesc::BLEND_FACTOR_ONE;
			bDesc.destBlendAlpha = BlendStateDesc::BLEND_FACTOR_ONE;
			bDesc.blendAlphaOp   = BlendStateDesc::BLEND_OP_ADD;
			pass.m_stateB = m_device->createBlendState(bDesc);

			//-- create decals mask texture.
			{
				ITexture::Desc desc;
				desc.width			= m_screenRes.width;
				desc.height			= m_screenRes.height;
				desc.sample.count	= m_videoMode.multiSampling.m_count;
				desc.sample.quality	= m_videoMode.multiSampling.m_quality;
				desc.bindFalgs		= ITexture::BIND_RENDER_TARGET | ITexture::BIND_SHADER_RESOURCE;
				desc.format			= ITexture::FORMAT_RGBA8;
				desc.texType		= ITexture::TYPE_2D;

				pass.m_rt = m_device->createTexture(desc, NULL, 0);
				if (!pass.m_rt)
					return false;
			}
		}

		//-- 4. PASS_SHADOW_CAST
		{
			PassDesc& pass = m_passes[PASS_SHADOW_CAST];

			DepthStencilStateDesc dsDesc;
			dsDesc.depthWriteMask = true;
			dsDesc.depthEnable	  = true;
			dsDesc.depthFunc	  = DepthStencilStateDesc::COMPARE_FUNC_LESS_EQUAL;
			pass.m_stateDS = m_device->createDepthStencilState(dsDesc);

			RasterizerStateDesc rDesc;
			rDesc.cullMode			= RasterizerStateDesc::CULL_BACK;
			rDesc.multisampleEnable = 0;
			rDesc.depthBiasFactor	= 1.5f;
			rDesc.depthBiasUnits	= 5.0f;
			pass.m_stateR = m_device->createRasterizedState(rDesc);

			rDesc.cullMode = RasterizerStateDesc::CULL_NOTHING;
			pass.m_stateR_doubleSided = m_device->createRasterizedState(rDesc);

			BlendStateDesc bDesc;
			pass.m_stateB = m_device->createBlendState(bDesc);
		}

		//-- 5. PASS_SHADOW_RECEIVE
		{
			PassDesc& pass = m_passes[PASS_SHADOW_RECEIVE];

			DepthStencilStateDesc dsDesc;
			dsDesc.depthWriteMask = false;
			dsDesc.depthEnable	  = false;
			dsDesc.depthFunc	  = DepthStencilStateDesc::COMPARE_FUNC_ALWAYS;
			pass.m_stateDS = m_device->createDepthStencilState(dsDesc);

			RasterizerStateDesc rDesc;
			rDesc.cullMode			= RasterizerStateDesc::CULL_BACK;
			rDesc.multisampleEnable = 0;

			pass.m_stateR = m_device->createRasterizedState(rDesc);
			pass.m_stateR_doubleSided = pass.m_stateR;
			pass.m_stateR_wireframe = pass.m_stateR; 

			BlendStateDesc bDesc;
			pass.m_stateB = m_device->createBlendState(bDesc);

			//-- create shadows mask texture.
			{
				ITexture::Desc desc;
				desc.width			= m_screenRes.width;
				desc.height			= m_screenRes.height;
				desc.sample.count	= m_videoMode.multiSampling.m_count;
				desc.sample.quality	= m_videoMode.multiSampling.m_quality;
				desc.bindFalgs		= ITexture::BIND_RENDER_TARGET | ITexture::BIND_SHADER_RESOURCE;
				desc.format			= ITexture::FORMAT_RGBA8;
				desc.texType		= ITexture::TYPE_2D;

				pass.m_rt = m_device->createTexture(desc, NULL, 0);
				if (!pass.m_rt)
					return false;
			}
		}

		//-- 6. PASS_MAIN_COLOR
		{
			PassDesc& pass = m_passes[PASS_MAIN_COLOR];

			DepthStencilStateDesc dsDesc;
			dsDesc.depthWriteMask = false;
			dsDesc.depthEnable	  = true;
			dsDesc.depthFunc	  = DepthStencilStateDesc::COMPARE_FUNC_EQUAL;
			pass.m_stateDS = m_device->createDepthStencilState(dsDesc);

			RasterizerStateDesc rDesc;
			rDesc.cullMode			= RasterizerStateDesc::CULL_BACK;
			rDesc.multisampleEnable = (m_videoMode.multiSampling.m_count > 1);
			pass.m_stateR = m_device->createRasterizedState(rDesc);

			rDesc.cullMode = RasterizerStateDesc::CULL_NOTHING;
			pass.m_stateR_doubleSided = m_device->createRasterizedState(rDesc);

			rDesc.cullMode = RasterizerStateDesc::CULL_BACK;
			rDesc.fillMode = RasterizerStateDesc::FILL_WIREFRAME;
			pass.m_stateR_wireframe = m_device->createRasterizedState(rDesc);

			BlendStateDesc bDesc;
			pass.m_stateB = m_device->createBlendState(bDesc);
		}

		//-- PASS_SKY
		{
			PassDesc& pass = m_passes[PASS_SKY];

			DepthStencilStateDesc dsDesc;
			dsDesc.depthWriteMask = false;
			dsDesc.depthEnable	  = true;
			dsDesc.depthFunc	  = DepthStencilStateDesc::COMPARE_FUNC_LESS_EQUAL;
			pass.m_stateDS = m_device->createDepthStencilState(dsDesc);

			RasterizerStateDesc rDesc;
			rDesc.cullMode			= RasterizerStateDesc::CULL_BACK;
			rDesc.multisampleEnable = (m_videoMode.multiSampling.m_count > 1);
			pass.m_stateR = m_device->createRasterizedState(rDesc);

			BlendStateDesc bDesc;
			pass.m_stateB = m_device->createBlendState(bDesc);
		}

		//-- 7. PASS_DEBUG_WIRE
		{
			PassDesc& pass = m_passes[PASS_DEBUG_WIRE];

			DepthStencilStateDesc dsDesc;
			dsDesc.depthWriteMask = false;
			dsDesc.depthEnable	  = true;
			dsDesc.depthFunc	  = DepthStencilStateDesc::COMPARE_FUNC_ALWAYS;
			pass.m_stateDS = m_device->createDepthStencilState(dsDesc);

			RasterizerStateDesc rDesc;
			rDesc.antialiasedLineEnable = false;
			pass.m_stateR = m_device->createRasterizedState(rDesc);

			BlendStateDesc bDesc;
			pass.m_stateB = m_device->createBlendState(bDesc);
		}

		//-- 8. PASS_DEBUG_SOLID
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

		//-- 9. PASS_DEBUG_TRANSPARENT
		{
			PassDesc& pass = m_passes[PASS_DEBUG_TRANSPARENT];

			DepthStencilStateDesc dsDesc;
			dsDesc.depthWriteMask = true;
			dsDesc.depthEnable	  = true;
			dsDesc.depthFunc	  = DepthStencilStateDesc::COMPARE_FUNC_LESS_EQUAL;
			pass.m_stateDS = m_device->createDepthStencilState(dsDesc);

			RasterizerStateDesc rDesc;
			rDesc.cullMode = RasterizerStateDesc::CULL_BACK;

			pass.m_stateR = m_device->createRasterizedState(rDesc);

			BlendStateDesc bDesc;
			bDesc.blendEnable[0] = true;
			bDesc.srcBlend		 = BlendStateDesc::BLEND_FACTOR_ONE;
			bDesc.destBlend	 	 = BlendStateDesc::BLEND_FACTOR_ONE;
			bDesc.blendOp		 = BlendStateDesc::BLEND_OP_ADD;
			bDesc.srcBlendAlpha  = BlendStateDesc::BLEND_FACTOR_SRC_ALPHA;
			bDesc.destBlendAlpha = BlendStateDesc::BLEND_FACTOR_INV_SRC_ALPHA;
			bDesc.blendAlphaOp   = BlendStateDesc::BLEND_OP_ADD;
			pass.m_stateB = m_device->createBlendState(bDesc);
		}

		//-- 9. PASS_DEBUG_OVERRIDE
		{
			PassDesc& pass = m_passes[PASS_DEBUG_OVERRIDE];

			DepthStencilStateDesc dsDesc;
			dsDesc.depthWriteMask = true;
			dsDesc.depthEnable	  = true;
			dsDesc.depthFunc	  = DepthStencilStateDesc::COMPARE_FUNC_ALWAYS;
			pass.m_stateDS = m_device->createDepthStencilState(dsDesc);

			RasterizerStateDesc rDesc;
			rDesc.cullMode = RasterizerStateDesc::CULL_BACK;

			pass.m_stateR = m_device->createRasterizedState(rDesc);

			BlendStateDesc bDesc;
			pass.m_stateB = m_device->createBlendState(bDesc);
		}

		//-- 9. POST_PROCESSING
		{
			PassDesc& pass = m_passes[PASS_POST_PROCESSING];

			DepthStencilStateDesc dsDesc;
			dsDesc.depthWriteMask = false;
			dsDesc.depthEnable	  = false;
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
		PassDesc& pass = m_passes[type];

		switch (type)
		{
		case PASS_Z_ONLY:
			{
				m_device->backToMainFrameBuffer();
				m_device->clear(CLEAR_DEPTH | CLEAR_STENCIL, Color(), 1.0f, 0);
				m_device->clearColorRT(pass.m_rt.get(), Color(1,1,1,1));
				m_device->setRenderTarget(pass.m_rt.get(), rd()->getMainDepthRT());

				m_device->setRasterizerState(pass.m_stateR);
				m_device->setDepthStencilState(pass.m_stateDS, 0);
				m_device->setBlendState(pass.m_stateB, NULL, 0xffffffff);

				m_device->setViewPort(0, 0, m_screenRes.width, m_screenRes.height);
				break;
			}
		case PASS_DECAL:
			{
				m_device->clearColorRT(pass.m_rt.get(), Color(0,0,0,0));
				m_device->setRenderTarget(pass.m_rt.get(), rd()->getMainDepthRT());

				m_device->setRasterizerState(pass.m_stateR);
				m_device->setDepthStencilState(pass.m_stateDS, 0);
				m_device->setBlendState(pass.m_stateB, NULL, 0xffffffff);

				m_device->setViewPort(0, 0, m_screenRes.width, m_screenRes.height);
				break;
			}
		case PASS_LIGHT:
			{
				m_device->clearColorRT(pass.m_rt.get(), Color(0,0,0,0));
				m_device->setRenderTarget(pass.m_rt.get(), rd()->getMainDepthRT());

				m_device->setRasterizerState(pass.m_stateR);
				m_device->setDepthStencilState(pass.m_stateDS, 0);
				m_device->setBlendState(pass.m_stateB, NULL, 0xffffffff);

				m_device->setViewPort(0, 0, m_screenRes.width, m_screenRes.height);
				break;
			}
		case PASS_SHADOW_CAST:
			{
				m_device->setRasterizerState(pass.m_stateR);
				m_device->setDepthStencilState(pass.m_stateDS, 0);
				m_device->setBlendState(pass.m_stateB, NULL, 0xffffffff);
				break;
			}
		case PASS_SHADOW_RECEIVE:
			{
				m_device->clearColorRT(pass.m_rt.get(), Color(0,0,0,0));
				m_device->setRenderTarget(pass.m_rt.get(), nullptr);

				m_device->setRasterizerState(pass.m_stateR);
				m_device->setDepthStencilState(pass.m_stateDS, 0);
				m_device->setBlendState(pass.m_stateB, NULL, 0xffffffff);

				m_device->setViewPort(0, 0, m_screenRes.width, m_screenRes.height);
				break;
			}
		case PASS_MAIN_COLOR:
			{
				m_device->backToMainFrameBuffer();
				m_device->clear(CLEAR_COLOR, Color(0.5f,0.5f,0.5f,1), 0.0f, 0);

				m_device->setRasterizerState(pass.m_stateR);
				m_device->setDepthStencilState(pass.m_stateDS, 0);
				m_device->setBlendState(pass.m_stateB, NULL, 0xffffffff);

				m_device->setViewPort(0, 0, m_screenRes.width, m_screenRes.height);
				break;
			}
		case PASS_SKY:
			{
				m_device->backToMainFrameBuffer();

				m_device->setRasterizerState(pass.m_stateR);
				m_device->setDepthStencilState(pass.m_stateDS, 0);
				m_device->setBlendState(pass.m_stateB, NULL, 0xffffffff);

				m_device->setViewPort(0, 0, m_screenRes.width, m_screenRes.height);
				break;
			}
		case PASS_POST_PROCESSING:
			{
				break;
			}
		case PASS_DEBUG_WIRE:
		case PASS_DEBUG_SOLID:
		case PASS_DEBUG_TRANSPARENT:
		case PASS_DEBUG_OVERRIDE:
			{
				m_device->backToMainFrameBuffer();

				m_device->setRasterizerState(pass.m_stateR);
				m_device->setDepthStencilState(pass.m_stateDS, 0);
				m_device->setBlendState(pass.m_stateB, NULL, 0xffffffff);

				m_device->setViewPort(0, 0, m_screenRes.width, m_screenRes.height);
				break;
			}
		default:
			{
				assert(!"invalid pass type.");
				return false;
			}
		};

		return true;
	}

	//----------------------------------------------------------------------------------------------
	void RenderSystem::setCamera(const RenderCamera* cam)
	{
		assert(cam);

		m_camera = cam;
		m_shaderContext->setCamera(cam);
	}

	//----------------------------------------------------------------------------------------------
	void RenderSystem::addROPs(const RenderOps& ops)
	{
		//-- ToDo: Do some kind of sorting of render operations.

		m_renderOps = ops;
	}

	//----------------------------------------------------------------------------------------------
	bool RenderSystem::endPass()
	{
		_doDraw(m_renderOps);
		m_renderOps.clear();
		return true;
	}

	//----------------------------------------------------------------------------------------------
	void RenderSystem::addImmediateROPs(const RenderOps& ops)
	{
		RenderOps rOps = ops;
		_doDraw(rOps);
	}

	//----------------------------------------------------------------------------------------------
	void RenderSystem::_doDraw(RenderOps& ops)
	{
		for (uint i = 0; i < ops.size(); ++i)
		{
			RenderOp&		ro   = ops[i];
			const RenderFx&	fx   = *ro.m_material;
			PassDesc&		pass = m_passes[m_pass];	

			m_device->setVertexLayout(fx.m_vertexDlcr);

			//-- ToDo: reconsider.
			if (fx.m_rsProps)
			{
				m_device->setRasterizerState(
					fx.m_rsProps->m_doubleSided ? pass.m_stateR_doubleSided : pass.m_stateR
					);

				m_device->setRasterizerState(
					fx.m_rsProps->m_wireframe ? pass.m_stateR_wireframe : pass.m_stateR
					);
			}

			//if (fx.m_bumped)
			//{
			//	IBuffer* buffers[2] = { ro.m_mainVB, ro.m_tangentVB };
			//	m_device->setVertexBuffers(0, buffers, 2);
			//}
			//else
			//{
			//	m_device->setVertexBuffer(0, ro.m_mainVB);
			//}
			m_device->setVertexBuffers(0, ro.m_VBs, ro.m_VBCount);

			if (ro.m_IB)
			{
				m_device->setIndexBuffer(ro.m_IB);
			}

			//-- apply shader for current pass.
			m_shaderContext->applyFor(&ro);

			if (ro.m_instanceCount != 0)
			{
				assert(ro.m_instanceTB);

				if (ro.m_IB)
				{
					m_device->drawIndexedInstanced(
						ro.m_primTopolpgy, ro.m_startIndex, ro.m_baseVertex, ro.m_indicesCount, ro.m_instanceCount
						);
				}
				else
				{
					m_device->drawInstanced(
						ro.m_primTopolpgy, ro.m_startIndex, ro.m_indicesCount, ro.m_instanceCount
						);
				}
			}
			else
			{
				if (ro.m_IB)
				{
					m_device->drawIndexed(ro.m_primTopolpgy, ro.m_startIndex, ro.m_baseVertex, ro.m_indicesCount);
				}
				else
				{
					m_device->draw(ro.m_primTopolpgy, ro.m_startIndex, ro.m_indicesCount);
				}
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	ShaderContext::EPassType RenderSystem::shaderPass(EPassType type) const
	{
		return g_render2shaderPass[type];
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

