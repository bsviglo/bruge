#pragma once

#include "prerequisites.hpp"
#include "render_common.h"
#include "materials.hpp"
#include "vertex_format.hpp"

namespace brUGE
{
	class Camera;

namespace render
{
	class  LightsManager;
	class  MeshManager;
	struct DirectionLight;

	//-- Class is responsible for calculating different kinds of shadows and displaying it on the
	//-- screen. For now it uses only direction light shadows, but in the near future it will be able
	//-- to cast shadows from the point light and spot light.
	//-- ToDo:
	//----------------------------------------------------------------------------------------------
	class ShadowManager : public NonCopyable
	{
	public:
		ShadowManager();
		~ShadowManager();

		bool init();
		
		void castShadows(const RenderCamera& cam, LightsManager& lightManager, MeshManager& meshManager);
		void receiveShadows(const RenderCamera* rCam);

	private:
		uint			m_shadowMapRes;
		Ptr<IBuffer>	m_fsQuadVB;
		Ptr<Material>	m_shadowResolveMaterial;
		Ptr<Material>	m_shadowBlurMaterial;
		SamplerStateID  m_shadowMapSml;
		Ptr<ITexture>	m_noiseMap;
		SamplerStateID  m_noiseMapSml;
		Ptr<ITexture>	m_blurMap;
		SamplerStateID	m_blurMapSml;
		RenderOps		m_receiveROPs;
		RenderOps		m_castROPs;
		RenderOps		m_blurROPs;

		//-- for every split.
		float						m_splitShemeLambda;
		uint						m_splitCount;
		Ptr<ITexture>				m_shadowMaps;
		std::vector<RenderCamera>	m_shadowCameras;
		std::vector<vec4ui>			m_shadowViewPorts;
		std::vector<float>			m_splitPlanes;
	};

} //-- render
} //-- brUGE