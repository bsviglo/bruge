#pragma once

#include "prerequisites.hpp"
#include "math/Vector3.hpp"
#include "Color.h"
#include "Mesh.hpp"
#include "render_system.hpp"
#include "Camera.h"
#include <memory>

namespace brUGE
{
	struct Transform;

namespace render
{
	class DebugDrawer;
	class imguiRender;
	class DecalManager;
	class LightsManager;
	class MeshManager;
	class ShadowManager;


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
		void			setCamera(const Ptr<Camera>& cam);
		void			draw();

		DecalManager&	decalManager()  { return *m_decalManager.get(); }
		LightsManager&	lightsManager() { return *m_lightsManager.get(); }
		MeshManager&	meshManager()	{ return *m_meshManager.get(); }
	
	private:
		typedef std::unique_ptr<DebugDrawer>   DebugDrawerPtr;
		typedef std::unique_ptr<DecalManager>  DecalManagerPtr;
		typedef std::unique_ptr<imguiRender>   ImguiRenderPtr;
		typedef std::unique_ptr<LightsManager> LightsManagerPtr;
		typedef std::unique_ptr<MeshManager>   MeshManagerPtr;
		typedef std::unique_ptr<ShadowManager> ShadowManagerPtr;
		
		Ptr<Camera>		 m_camera;
		DebugDrawerPtr	 m_debugDrawer;
		DecalManagerPtr  m_decalManager;
		ImguiRenderPtr	 m_imguiRender;
		LightsManagerPtr m_lightsManager;
		MeshManagerPtr	 m_meshManager;
		ShadowManagerPtr m_shadowManager;
		RenderOps		 m_renderOps;
	};

} //-- render
} //-- brUGE