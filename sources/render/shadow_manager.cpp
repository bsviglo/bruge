#include "shadow_manager.hpp"
#include "light_manager.hpp"
#include "mesh_manager.hpp"
#include "post_processing.hpp"
#include "loader/ResourcesManager.h"
#include "os/FileSystem.h"
#include "math/math_all.hpp"
#include "Camera.h"
#include "console/WatchersPanel.h"

//-- ToDo:
#include "Engine/Engine.h"
#include "render/render_world.hpp"

using namespace brUGE::os;
using namespace brUGE::math;
using namespace brUGE::render;

//-- start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	//-- watchers.
	bool  g_enableShadows = true;
	float g_width = 0;
	float g_height = 0;
	float g_nearZ = 0;
	float g_farZ  = 0;
	vec3f g_lightPos;
	vec3f g_lightDir;
	bool  g_adjustShadowVolume = false;
	bool  g_useCullingMatrix = true;
	bool  g_fitLightToTexels = true;
	bool  g_blurShadows = false;
	uint  g_ROPs = 0;
	vec4f g_farDistances;
	vec4f g_nearDistances;

	//-- Represents shadow mapping c-buffer constants.
	//----------------------------------------------------------------------------------------------
	struct ShadowConstants
	{
		vec2f m_shadowMapRes;
		vec2f m_invShadowMapRes;
		float m_splitPlanes[4];
		mat4f m_shadowMatrices[4];
	};

	//-- Practical split scheme:
	//--
	//-- CLi = n * (f / n) ^ (i / numSplits)
	//-- CUi = n + (f - n) * (i / numSplits)
	//-- Ci = CLi * (lambda) + CUi * (1 - lambda)
	//--
	//-- lambda scales between logarithmic and uniform
	//----------------------------------------------------------------------------------------------
	void calcSpitPlanes(
		std::vector<float>& splitPlanes,
		float nearPlane, float farPlane, float lambda,	uint numSplits)
	{
		splitPlanes.resize(numSplits + 1);

		lambda = clamp(0.f, lambda, 1.f);

		float dimension, logScheme, uniformScheme;

		//-- make sure border values are right.
		splitPlanes[0] = nearPlane;
		splitPlanes[numSplits] = farPlane;

		// for all splits, except the first and the last.
		for (uint i = 1; i < numSplits; ++i)
		{
			dimension	  = i / (float)numSplits;
			logScheme	  = nearPlane * powf((farPlane / nearPlane), dimension);
			uniformScheme = nearPlane + (farPlane - nearPlane) * dimension;

			splitPlanes[i] = logScheme * lambda + uniformScheme * (1.f - lambda);
		}
	}

	//----------------------------------------------------------------------------------------------
	void calcFrustrumAABB(AABB& aabb, const mat4f& invVPMat)
	{
		vec4f points[8];

		//-- calculate frustum corners.
		points[0] = invVPMat.applyToPoint(vec4f(-1.f, 1.f, 1.f, 1.0f));
		points[1] = invVPMat.applyToPoint(vec4f(-1.f, 1.f, 0.f, 1.0f));
		points[2] = invVPMat.applyToPoint(vec4f( 1.f, 1.f, 0.f, 1.0f));
		points[3] = invVPMat.applyToPoint(vec4f( 1.f, 1.f, 1.f, 1.0f));

		points[4] = invVPMat.applyToPoint(vec4f(-1.f,-1.f, 1.f, 1.0f));
		points[5] = invVPMat.applyToPoint(vec4f(-1.f,-1.f, 0.f, 1.0f));
		points[6] = invVPMat.applyToPoint(vec4f( 1.f,-1.f, 0.f, 1.0f));
		points[7] = invVPMat.applyToPoint(vec4f( 1.f,-1.f, 1.f, 1.0f));

		//-- calculate AABB for current camera frustum.
		for (uint i = 0; i < 8; ++i)
		{
			vec3f point = points[i].toVec3().scale(1.0f / points[i].w);
			aabb.include(point);
		}
	}

	//----------------------------------------------------------------------------------------------
	void calcFrustrumAABB(AABB& aabb, const mat4f& viewMat, const Projection& proj)
	{
		mat4f projMat;
		float aspectRatio = rs().screenRes().width / rs().screenRes().height;
		projMat.setPerspectiveProj(proj.fov, aspectRatio, proj.nearDist, proj.farDist);

		mat4f invVPMat(viewMat);
		invVPMat.postMultiply(projMat);
		invVPMat.invert();

		calcFrustrumAABB(aabb, invVPMat);
	}

	//----------------------------------------------------------------------------------------------
	void calcLightViewMat(mat4f& viewMat, const AABB& aabb, const DirectionLight& dirLight)
	{
		if (g_fitLightToTexels)
		{
			viewMat.setLookAt(dirLight.m_dir.scale(-250.0f), dirLight.m_dir, vec3f(0,1,0));
		}
		else
		{
			float autoDist = aabb.getDimensions().length() * 5.0f;
			vec3f lightPos = aabb.getCenter() - dirLight.m_dir.scale(autoDist);
			viewMat.setLookAt(lightPos, dirLight.m_dir, vec3f(0,1,0));
		}
	}

	//----------------------------------------------------------------------------------------------
	void calcLightProjInfo(Projection& projInfo, const mat4f& lightViewMat, const AABB& aabb)
	{
		//-- transform aabb into light view space.
		AABB lightAABB = aabb.getTranformed(lightViewMat);

		//-- calculate orthogonal matrix parameters.
		projInfo.isOrtho  = true;
		projInfo.isOrthoSpec = false;
		projInfo.nearDist = lightAABB.m_min.z;
		projInfo.farDist  = lightAABB.m_max.z;

		//-- ToDo: remove hard code for current shadow map resolution.
		//-- Removes juggi edges when camera zooming and moving, but can't help in case if user
		//-- rotates camera.
		if (g_fitLightToTexels)
		{
			projInfo.isOrthoSpec = true;

			vec3f minAABB = lightAABB.m_min;
			vec3f maxAABB = lightAABB.m_max;

			vec3f worldUnitsPerTexel = maxAABB - minAABB;
			worldUnitsPerTexel *= 1.0f / 2048.0f;

			minAABB /= worldUnitsPerTexel;
			minAABB  = vec3f(floorf(minAABB.x), floorf(minAABB.y), 0);
			minAABB *= worldUnitsPerTexel;

			maxAABB /= worldUnitsPerTexel;
			maxAABB  = vec3f(floorf(maxAABB.x), floorf(maxAABB.y), 0);
			maxAABB *= worldUnitsPerTexel;

			projInfo.l = minAABB.x;
			projInfo.r = maxAABB.x;
			projInfo.b = minAABB.y;
			projInfo.t = maxAABB.y;
		}
		else
		{
			projInfo.width  = lightAABB.getDimensions().x;
			projInfo.height = lightAABB.getDimensions().y;
		}

		g_width  = projInfo.width;
		g_height = projInfo.height;
	}

	//----------------------------------------------------------------------------------------------
	void calcLightCamera(RenderCamera& rCam, const AABB& aabb, const DirectionLight& dirLight)
	{
		//-- calculate view matrix of the current light.
		mat4f lightViewMat;
		calcLightViewMat(lightViewMat, aabb, dirLight);

		//-- calculate orthogonal projection matrix of the current light.
		Projection projInfo;
		mat4f	   lightProjMat;
		{
			calcLightProjInfo(projInfo, lightViewMat, aabb);

			if (projInfo.isOrthoSpec)
			{
				lightProjMat.setOrthoOffCenterProj(
					projInfo.l, projInfo.r, projInfo.b, projInfo.t, projInfo.nearDist, projInfo.farDist
					);
			}
			else
			{
				lightProjMat.setOrthogonalProj(
					projInfo.width, projInfo.height, projInfo.nearDist, projInfo.farDist
					);
			}
		}

		//-- calculate render camera parameters.
		rCam.m_projInfo = projInfo;
		rCam.m_view	    = lightViewMat;
		rCam.m_proj	    = lightProjMat;
		rCam.m_invView  = lightViewMat.getInverted();
		rCam.m_viewProj = lightViewMat;
		rCam.m_viewProj.postMultiply(lightProjMat);
	}

}
//--------------------------------------------------------------------------------------------------
//-- end unnamed namespace.


namespace brUGE
{
namespace render
{

	//----------------------------------------------------------------------------------------------
	ShadowManager::ShadowManager()
		:	m_shadowMapRes(2048), m_splitShemeLambda(0.85f), m_splitCount(4), m_uiEnabled(false),
			m_autoSplitSheme(true), m_bias(1.0f), m_slopeScaleBias(4.0f), m_pVB(nullptr)
	{
		REGISTER_CONSOLE_VALUE("r_shadow_adjust_volume",		bool,  g_adjustShadowVolume);
		REGISTER_CONSOLE_VALUE("r_shadow_use_culling_mat",		bool,  g_useCullingMatrix);
		REGISTER_CONSOLE_VALUE("r_shadow_fit_light_to_texels",	bool,  g_fitLightToTexels);
		REGISTER_CONSOLE_VALUE("r_shadow_enable_blur",			bool,  g_blurShadows);
		REGISTER_CONSOLE_VALUE("r_shadow_enable",				bool,  g_enableShadows);
		REGISTER_CONSOLE_MEMBER_VALUE("r_shadow_enable_ui",		bool, m_uiEnabled, ShadowManager);

		REGISTER_RO_WATCHER("shadow light far distances",  vec4f, g_farDistances);
		REGISTER_RO_WATCHER("shadow light near distances", vec4f, g_nearDistances);

		REGISTER_RO_WATCHER("shadow light pos",		vec3f, g_lightPos);
		REGISTER_RO_WATCHER("shadow light dir",		vec3f, g_lightDir);
		REGISTER_RO_WATCHER("shadow camera width",	float, g_width);
		REGISTER_RO_WATCHER("shadow camera height",	float, g_height);
		REGISTER_RO_WATCHER("shadow camera z-near", float, g_nearZ);
		REGISTER_RO_WATCHER("shadow camera z-far",	float, g_farZ);
		REGISTER_RO_WATCHER("shadow ROPs count",	uint,  g_ROPs);
	}

	//----------------------------------------------------------------------------------------------
	ShadowManager::~ShadowManager()
	{

	}

	//----------------------------------------------------------------------------------------------
	bool ShadowManager::init()
	{
		//-- setup full-screen quad.
		{
			VertexXYZUV vertices[4];

			//-- left-top
			vertices[0].m_pos.set(-1.f, 1.f, 0.0f);
			vertices[0].m_tc.set (0.f, 0.f);

			//-- left-bottom		
			vertices[1].m_pos.set(-1.f, -1.f, 0.0f);
			vertices[1].m_tc.set (0.f, 1.f);

			//-- right-top
			vertices[2].m_pos.set(1.f, 1.f, 0.0f);
			vertices[2].m_tc.set (1.f, 0.f);

			//-- right-bottom
			vertices[3].m_pos.set(1.f, -1.f, 0.0f);
			vertices[3].m_tc.set (1.f, 1.f);

			if (!(m_fsQuadVB = rd()->createBuffer(IBuffer::TYPE_VERTEX, vertices, 4, sizeof(VertexXYZUV))))
				return false;

			m_pVB = m_fsQuadVB.get();
		}

		//-- load material.
		{
			std::vector<Ptr<Material>> mtllib;
			RODataPtr file = FileSystem::instance().readFile("resources/materials/shadows.mtl");
			if (!file || !rs().materials().createMaterials(mtllib, *file))
			{
				return false;
			}

			if (mtllib.size() != 2)
				return false;

			m_shadowResolveMaterial	= mtllib[0];
			m_shadowBlurMaterial	= mtllib[1];
		}

		//-- load noise texture.
		{
			m_noiseMap = ResourcesManager::instance().loadTexture("textures/noise.dds");
			if (!m_noiseMap)
				return false;

			SamplerStateDesc sDesc;
			sDesc.minMagFilter	= SamplerStateDesc::FILTER_NEAREST;
			sDesc.wrapR			= SamplerStateDesc::ADRESS_MODE_WRAP;
			sDesc.wrapS			= SamplerStateDesc::ADRESS_MODE_WRAP;
			sDesc.wrapT			= SamplerStateDesc::ADRESS_MODE_WRAP;

			m_noiseMapSml = rd()->createSamplerState(sDesc);
			if (m_noiseMapSml == CONST_INVALID_HANDLE)
				return false;
		}

		//-- create rops for drawing.
		{
			RenderOp op;
			op.m_indicesCount = 4;
			op.m_primTopolpgy = PRIM_TOPOLOGY_TRIANGLE_STRIP;
			op.m_VBs		  = &m_pVB;
			op.m_VBCount	  = 1;
			op.m_material	  = m_shadowResolveMaterial->renderFx();

			m_receiveROPs.push_back(op);

			op.m_material = m_shadowBlurMaterial->renderFx();
			m_blurROPs.push_back(op);
		}

		//-- create blur map.
		{
			//-- ToDo: reconsider. see comment in the materials.cpp
			m_blurMap = Engine::instance().renderWorld().postProcessing().find("BBCopy");
			if (!m_blurMap)
				return false;

			SamplerStateDesc sDesc;
			sDesc.minMagFilter	= SamplerStateDesc::FILTER_BILINEAR;
			sDesc.wrapR			= SamplerStateDesc::ADRESS_MODE_CLAMP;
			sDesc.wrapS			= SamplerStateDesc::ADRESS_MODE_CLAMP;
			sDesc.wrapT			= SamplerStateDesc::ADRESS_MODE_CLAMP;

			m_blurMapSml = rd()->createSamplerState(sDesc);
			if (m_blurMapSml == CONST_INVALID_HANDLE)
				return false;
		}

		//-- create shadow map.
		{
			ITexture::Desc desc;
			desc.width		= m_shadowMapRes * m_splitCount;
			desc.height		= m_shadowMapRes;
			desc.bindFalgs	= ITexture::BIND_DEPTH_STENCIL | ITexture::BIND_SHADER_RESOURCE;
			desc.format		= ITexture::FORMAT_D32F;
			desc.texType	= ITexture::TYPE_2D;

			m_shadowMaps = rd()->createTexture(desc, NULL, 0);
			if (!m_shadowMaps)
				return false;

			//-- create view port for the each individual shadow map.
			m_shadowViewPorts.resize(m_splitCount);
			for (uint i = 0; i < m_splitCount; ++i)
			{
				m_shadowViewPorts[i] = vec4ui(m_shadowMapRes * i, 0, m_shadowMapRes, m_shadowMapRes);
			}

			SamplerStateDesc sDesc;
			sDesc.minMagFilter	= SamplerStateDesc::FILTER_NEAREST;
			sDesc.wrapR			= SamplerStateDesc::ADRESS_MODE_CLAMP;
			sDesc.wrapS			= SamplerStateDesc::ADRESS_MODE_CLAMP;
			sDesc.wrapT			= SamplerStateDesc::ADRESS_MODE_CLAMP;

			m_shadowMapSml = rd()->createSamplerState(sDesc);
			if (m_shadowMapSml == CONST_INVALID_HANDLE)
				return false;
		}

		//-- ToDo:
		m_shadowCameras.resize(m_splitCount);
		m_splitPlanes.resize(m_splitCount, 0);

		//-- create UI.
		m_ui.reset(new UI(*this));

		return true;
	}

	//----------------------------------------------------------------------------------------------
	void ShadowManager::update(float /*dt*/)
	{
		if (m_uiEnabled) m_ui->update();
	}

	//----------------------------------------------------------------------------------------------
	void ShadowManager::castShadows(const RenderCamera& cam, LightsManager& lightManager, MeshManager& meshManager)
	{
		if (!g_enableShadows) return;

		//-- ToDo: gather all light casted shadows.
		const DirectionLight& dirLight = lightManager.getDirLight(0);

		//-- Now for each split generate shadow map.

		//-- 1. calculate split planes.
		m_cameraFarNearDist.set(cam.m_projInfo.nearDist, cam.m_projInfo.farDist);
		if (m_autoSplitSheme)
		{
			calcSpitPlanes(
				m_splitPlanes, m_cameraFarNearDist.x, m_cameraFarNearDist.y,
				m_splitShemeLambda, m_splitCount
				);
		}

		//-- 2. calculate shadow light camera for each split.
		for (uint i = 0; i < m_splitCount; ++i)
		{
			AABB aabb;
			Projection proj;
			proj.nearDist = m_splitPlanes[i];
			proj.farDist  = m_splitPlanes[i + 1];
			proj.fov      = cam.m_projInfo.fov;
			
			calcFrustrumAABB(aabb, cam.m_view, proj);
			calcLightCamera(m_shadowCameras[i], aabb, dirLight);
		}

		//-- 3. gather ROPs for each particular split.

		//-- 4. clear shadow map.
		rd()->clearDepthStencilRT(CLEAR_DEPTH, m_shadowMaps.get(), 1.0f, 0);

		g_ROPs = 0;
		for (uint i = 0; i < m_splitCount; ++i)
		{
			RenderCamera& curCam	  = m_shadowCameras[i];
			Projection&   curProjInfo = curCam.m_projInfo;

			m_castROPs.clear();

			//-- calculate view proj matrix for object culling.
			mat4f cullingVPMat;
			if (curProjInfo.isOrthoSpec)
			{
				cullingVPMat.setOrthoOffCenterProj(
					curProjInfo.l, curProjInfo.r, curProjInfo.b, curProjInfo.t, 0, curProjInfo.farDist
					);
			}
			else
			{
				cullingVPMat.setOrthogonalProj(
					curProjInfo.width, curProjInfo.height, 0, curProjInfo.farDist
					);
			}
			cullingVPMat.preMultiply(curCam.m_view);

			//-- Try to make far near distance difference as small as possible.
			if (g_adjustShadowVolume)
			{
				AABB aabb;
				meshManager.gatherROPs(
					RenderSystem::PASS_SHADOW_CAST, false,
					m_castROPs, g_useCullingMatrix ? cullingVPMat : curCam.m_viewProj, &aabb
					);

				Projection projInfo;
				calcLightProjInfo(projInfo, curCam.m_view, aabb);
				
				//-- adjust shadow light near plane.
				if (projInfo.nearDist > curProjInfo.nearDist)
					curProjInfo.nearDist = projInfo.nearDist;
				
				//-- adjust shadow light far plane.
				if (projInfo.farDist < curProjInfo.farDist)
					curProjInfo.farDist = projInfo.farDist;

				//-- calculate new projection matrix for current camera split.
				if (curProjInfo.isOrthoSpec)
				{
					curCam.m_viewProj.setOrthoOffCenterProj(
						curProjInfo.l, curProjInfo.r, curProjInfo.b, curProjInfo.t, 0, curProjInfo.farDist
						);
				}
				else
				{
					curCam.m_viewProj.setOrthogonalProj(
						curProjInfo.width, curProjInfo.height, curProjInfo.nearDist, curProjInfo.farDist
						);
				}
				curCam.m_viewProj.preMultiply(curCam.m_view);
			}
			else
			{
				meshManager.gatherROPs(
					RenderSystem::PASS_SHADOW_CAST, false,
					m_castROPs, g_useCullingMatrix ? cullingVPMat : curCam.m_viewProj
					);
			}

			//-- update watchers.
			g_ROPs			   += m_castROPs.size();
			g_nearDistances[i]  = curProjInfo.nearDist;
			g_farDistances[i]   = curProjInfo.farDist;

			//-- draw ROPs into the particular shadow map.
			{
				const vec4ui& curViewPort = m_shadowViewPorts[i];
				rs().beginPass(RenderSystem::PASS_SHADOW_CAST);
				rd()->setRenderTarget(nullptr, m_shadowMaps.get());
				rd()->setViewPort(curViewPort.x, curViewPort.y, curViewPort.z, curViewPort.w);
				rs().setCamera(&curCam);
				rs().shaderContext().updatePerFrameViewConstants();
				rs().addROPs(m_castROPs);
				rs().endPass();
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void ShadowManager::receiveShadows(const RenderCamera* rCam)
	{
		if (!g_enableShadows)
		{
			rd()->clearColorRT(rs().shadowsMask(), Color(0,0,0,0));
			return;
		}

		Handle shaderID = m_receiveROPs[0].m_material->m_shader;
		IShader* shader = rs().shaderContext().shader(shaderID);
		{
			ShadowConstants constants;
			constants.m_shadowMapRes	  = vec2f(m_shadowMapRes, m_shadowMapRes);
			constants.m_invShadowMapRes   = vec2f(1.0f / m_shadowMapRes, 1.0f / m_shadowMapRes);
			for (uint i = 0; i < m_splitCount; ++i)
			{
				constants.m_shadowMatrices[i] = m_shadowCameras[i].m_viewProj;
				constants.m_splitPlanes[i]	  = m_splitPlanes[i + 1];
			}

			shader->setUniformBlock("g_shadowConstants", &constants, sizeof(ShadowConstants));
			shader->setTexture("g_shadowMap", m_shadowMaps.get(), m_shadowMapSml);
			shader->setTexture("g_noiseMap", m_noiseMap.get(), m_noiseMapSml);
		}

		rs().beginPass(RenderSystem::PASS_SHADOW_RECEIVE);
		rs().setCamera(rCam);
		rs().shaderContext().updatePerFrameViewConstants();
		rs().addImmediateROPs(m_receiveROPs);
		rs().endPass();

		//-- do optional blur pass.
		if (g_blurShadows)
		{
			Handle shaderID = m_blurROPs[0].m_material->m_shader;
			IShader* shader = rs().shaderContext().shader(shaderID);
			{
				shader->setTexture("g_srcMap", m_blurMap.get(), m_blurMapSml);
			}

			rd()->copyTexture(rs().shadowsMask(), m_blurMap.get());
			rd()->setRenderTarget(rs().shadowsMask(), nullptr);
			rs().addImmediateROPs(m_blurROPs);
		}
	}
	
	//----------------------------------------------------------------------------------------------
	ShadowManager::UI::UI(ShadowManager& sm) : m_self(sm), m_scroll(0)
	{
	}

	//----------------------------------------------------------------------------------------------
	ShadowManager::UI::~UI()
	{
	}

	//----------------------------------------------------------------------------------------------
	void ShadowManager::UI::update()
	{
		//uint width	= rs().screenRes().width;
		//uint height = rs().screenRes().height;
		//
		//imguiBeginScrollArea("Shadows", width-300-10, 10, 300, height-20, &m_scroll);
		//{
		//	imguiCheck("enable shadows",	   &g_enableShadows);
		//	imguiCheck("adjust shadow volume", &g_adjustShadowVolume);
		//	imguiCheck("use culling matrix",   &g_useCullingMatrix);
		//	imguiCheck("fit light to texels",  &g_fitLightToTexels);
		//	imguiCheck("blur shadows",		   &g_blurShadows);
		//
		//	imguiSeparatorLine();
		//	imguiLabel("split sheme:");
		//	{
		//		imguiIndent();
		//		imguiCheck("auto split sheme", &m_self.m_autoSplitSheme);
		//		imguiSlider("split sheme lambda", &m_self.m_splitShemeLambda, 0.0f, 1.0f, 0.01f);
		//
		//		imguiLabel("split planes:");
		//		for (uint i = 1; i <= m_self.m_splitCount; ++i)
		//		{
		//			imguiSlider(
		//				"split", &m_self.m_splitPlanes[i],
		//				m_self.m_cameraFarNearDist.x, m_self.m_cameraFarNearDist.y, 0.1f,
		//				!m_self.m_autoSplitSheme
		//				);
		//		}
		//
		//		imguiUnindent();
		//	}
		//
		//	imguiSeparatorLine();
		//	imguiLabel("biasing:");
		//	{
		//		imguiIndent();
		//		imguiSlider("depth bias factor", &m_self.m_bias, 0.0f, 5.0f, 0.01f);
		//		imguiSlider("depth slope scale bias units", &m_self.m_slopeScaleBias, 0.0f, 20.0f, 0.01f);
		//		imguiUnindent();
		//	}
		//}
		//imguiEndScrollArea();
	}


} //-- render
} //-- brUGE