#pragma once

#include "prerequisites.hpp"
#include "math/Vector3.hpp"
#include "Color.h"
#include "Mesh.hpp"
#include "render_system.hpp"
#include "Camera.h"

namespace brUGE
{
	struct Transform;

namespace render
{
	//class SkyBox;
	class DebugDrawer;
	class DecalManager;
	class LightsManager;
	class MeshManager;
	class ShadowManager;
	class PostProcessing;
	class TerrainSystem;


	//-- ToDo: document.
	//----------------------------------------------------------------------------------------------
	class RenderWorld : public NonCopyable
	{
	public:
		RenderWorld();
		~RenderWorld();

		bool			init();

		//-- resolve visibility.
		void			update(float dt);
		void			setCamera(const std::shared_ptr<Camera>& cam);
		void			draw();

		DecalManager&	decalManager()   { return *m_decalManager.get(); }
		LightsManager&	lightsManager()  { return *m_lightsManager.get(); }
		MeshManager&	meshManager()	 { return *m_meshManager.get(); }
		TerrainSystem&	terrainSystem()	 { return *m_terrainSystem.get(); }
		PostProcessing& postProcessing() { return *m_postProcessing.get(); }
	
	private:
		std::shared_ptr<Camera>			m_camera;
		std::unique_ptr<DebugDrawer>	m_debugDrawer;
		std::unique_ptr<DecalManager>   m_decalManager;
		std::unique_ptr<LightsManager>  m_lightsManager;
		std::unique_ptr<MeshManager>	m_meshManager;
		std::unique_ptr<ShadowManager>  m_shadowManager;
		std::unique_ptr<PostProcessing> m_postProcessing;
		//std::unique_ptr<SkyBox>		  m_skyBox;
		std::unique_ptr<TerrainSystem>  m_terrainSystem;
	};

} //-- render
} //-- brUGE