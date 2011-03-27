#include "shadow_manager.hpp"
#include "light_manager.hpp"
#include "mesh_manager.hpp"
#include "loader/ResourcesManager.h"
#include "os/FileSystem.h"
#include "math/math_all.hpp"
#include "Camera.h"
#include "console/WatchersPanel.h"
#include "console/Console.h"

using namespace brUGE::os;
using namespace brUGE::math;
using namespace brUGE::render;

//-- start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	//-- watchers.
	float g_width = 0;
	float g_height = 0;
	float g_nearZ = 0;
	float g_farZ  = 0;
	vec3f g_lightPos;
	vec3f g_lightDir;
	bool  g_adjustShadowVolume = false;
	bool  g_adjustFrustum = false;
	uint  g_ROPs = 0;

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
			if (g_adjustFrustum)
			{
				if (point.y < 0) point.y = 0;
			}
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
		float autoDist = aabb.getDimensions().length() * 0.5f;
		vec3f lightPos = aabb.getCenter() - dirLight.m_dir.scale(autoDist);
		viewMat.setLookAt(lightPos, dirLight.m_dir, vec3f(0,1,0));
	}

	//----------------------------------------------------------------------------------------------
	void calcLightProjInfo(Projection& projInfo, const mat4f& lightViewMat, const AABB& aabb)
	{
		//-- transform aabb into light view space.
		AABB lightAABB = aabb.getTranformed(lightViewMat);

		//-- calculate orthogonal matrix parameters.
		projInfo.isOrtho  = true;
		projInfo.width    = lightAABB.getDimensions().x;
		projInfo.height   = lightAABB.getDimensions().y;
		projInfo.nearDist = lightAABB.m_min.z;
		projInfo.farDist  = lightAABB.m_max.z;
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
			lightProjMat.setOrthogonalProj(
				projInfo.width, projInfo.height, projInfo.nearDist, projInfo.farDist
				);
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
		: m_shadowMapRes(2048), m_splitShemeLambda(0.9f), m_splitCount(4)
	{
		REGISTER_CONSOLE_VALUE("shadow_adjust_volume",  bool,  g_adjustShadowVolume);
		REGISTER_CONSOLE_VALUE("shadow_adjust_frustum", bool,  g_adjustFrustum);

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
		}

		//-- load material.
		{
			std::vector<Ptr<Material>> mtllib;
			RODataPtr file = FileSystem::instance().readFile("resources/materials/shadows.mtl");
			if (!file || !rs().materials()->createMaterials(mtllib, *file))
			{
				return false;
			}

			m_shadowResolveMaterial	= mtllib[0];
		}

		//-- create rops for drawing.
		{
			RenderOp op;
			op.m_primTopolpgy = PRIM_TOPOLOGY_TRIANGLE_STRIP;
			op.m_mainVB		  = &*m_fsQuadVB;
			op.m_indicesCount = 4;
			op.m_material	  = m_shadowResolveMaterial->renderFx();

			m_receiveROPs.push_back(op);
		}

		//-- create shadow map
		{
			ITexture::Desc desc;
			desc.width		= m_shadowMapRes;
			desc.height		= m_shadowMapRes;
			desc.bindFalgs	= ITexture::BIND_DEPTH_STENCIL | ITexture::BIND_SHADER_RESOURCE;
			desc.format		= ITexture::FORMAT_D32F;
			desc.texType	= ITexture::TYPE_2D;

			m_shadowMaps.resize(m_splitCount);
			for (uint i = 0; i < m_splitCount; ++i)
			{
				m_shadowMaps[i] = rd()->createTexture(desc, NULL, 0);
				if (!m_shadowMaps[i])
					return false;
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

		return true;
	}

	//----------------------------------------------------------------------------------------------
	void ShadowManager::castShadows(const RenderCamera& cam, LightsManager& lightManager, MeshManager& meshManager)
	{
		//-- ToDo: gather all light casted shadows.
		const DirectionLight& dirLight = lightManager.getDirLight(0);

		//-- Now for each split generate shadow map.

		//-- 1. calculate split planes.
		calcSpitPlanes(
			m_splitPlanes, cam.m_projInfo.nearDist, cam.m_projInfo.farDist,
			m_splitShemeLambda, m_splitCount
			);

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
		g_ROPs = 0;
		for (uint i = 0; i < m_splitCount; ++i)
		{
			RenderCamera& curCam = m_shadowCameras[i];

			m_castROPs.clear();

			//-- Adjust shadow volume so, than it represents in the light view space the smaller area.
			//-- For doing this we calculate AABB for all visible for current split meshes and try
			//-- to compute how much area relative to the original split AABB it fits. If it does
			//-- smaller then we can use it's AABB instead of the split AABB.
			if (g_adjustShadowVolume)
			{
				AABB aabb;
				meshManager.gatherROPs(m_castROPs, curCam.m_viewProj, &aabb);

				Projection projInfo;
				calcLightProjInfo(projInfo, curCam.m_view, aabb);

				if (projInfo.width < curCam.m_projInfo.width)
				{
					calcLightCamera(curCam, aabb, dirLight);
				}
			}
			else
			{
				meshManager.gatherROPs(m_castROPs, curCam.m_viewProj);
			}
			g_ROPs += m_castROPs.size();

			//-- draw ROPs into the particular shadow map.
			{
				rs().beginPass(RenderSystem::PASS_SHADOW_CAST);
				rd()->clearDepthStencilRT(CLEAR_DEPTH, m_shadowMaps[i].get(), 1.0f, 0);
				rd()->setRenderTarget(nullptr, m_shadowMaps[i].get());
				rd()->setViewPort(m_shadowMapRes, m_shadowMapRes);
				rs().setCamera(&curCam);
				rs().addROPs(m_castROPs);
				rs().endPass();
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void ShadowManager::receiveShadows(const RenderCamera* rCam)
	{
		//-- ToDo: make texture array instead of the set of individual textures.
		Handle shaderID = m_receiveROPs[0].m_material->m_shader->m_shader[0].first;
		IShader* shader = rs().shaderContext()->shader(shaderID);
		{
			shader->setVec4f("g_splitPlanes", *reinterpret_cast<vec4f*>(&m_splitPlanes[1]));
			shader->setVec4f("g_splitCount", vec4f(m_splitCount, 0, 0, 0));
			shader->setMat4f("g_shadowMat0", m_shadowCameras[0].m_viewProj);
			shader->setMat4f("g_shadowMat1", m_shadowCameras[1].m_viewProj);
			shader->setMat4f("g_shadowMat2", m_shadowCameras[2].m_viewProj);
			shader->setMat4f("g_shadowMat3", m_shadowCameras[3].m_viewProj);

			shader->setTexture("g_shadowMap0_tex", m_shadowMaps[0].get());
			shader->setSampler("g_shadowMap0_sml", m_shadowMapSml);
			shader->setTexture("g_shadowMap1_tex", m_shadowMaps[1].get());
			shader->setSampler("g_shadowMap1_sml", m_shadowMapSml);
			shader->setTexture("g_shadowMap2_tex", m_shadowMaps[2].get());
			shader->setSampler("g_shadowMap2_sml", m_shadowMapSml);
			shader->setTexture("g_shadowMap3_tex", m_shadowMaps[3].get());
			shader->setSampler("g_shadowMap3_sml", m_shadowMapSml);
		}

		rs().beginPass(RenderSystem::PASS_SHADOW_RECEIVE);
		rs().setCamera(rCam);
		rs().addImmediateROPs(m_receiveROPs);
		rs().endPass();
	}

} //-- render
} //-- brUGE