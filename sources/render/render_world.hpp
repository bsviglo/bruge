#pragma once

#include "prerequisites.h"
#include "math/Vector3.hpp"
#include "Color.h"
#include "Mesh.hpp"
#include "render_system.hpp"
#include <memory>

namespace brUGE
{
	struct Transform;

namespace render
{
	class Camera;
	class DebugDrawer;
	class imguiRender;
	class DecalManager;
	class LightsManager;

	//----------------------------------------------------------------------------------------------
	struct MeshInstance
	{
		struct Desc
		{
			Desc() : fileName(nullptr) { }

			const char* fileName;
		};

		Ptr<Mesh>		    m_mesh;
		Ptr<SkinnedMesh>    m_skinnedMesh;
		Joint::WorldPalette m_worldPalette;
		Transform*			m_transform;
	};


	//----------------------------------------------------------------------------------------------
	class RenderWorld
	{
	private:
		//-- make non-copyable.
		RenderWorld(const RenderWorld&);
		RenderWorld& operator = (const RenderWorld&);

	public:
		RenderWorld();
		~RenderWorld();

		bool init();
		bool fini();

		//-- resolve visibility.
		void			update(float dt);
		void			setCamera(const Ptr<Camera>& cam);
		void			resolveVisibility();

		//-- models.
		Handle			addMeshDef(const MeshInstance::Desc& desc, Transform* transform);
		bool			delMeshDef(Handle handle);
		MeshInstance*	getMeshDef(Handle handle);

		DecalManager&	decalManager()  { return *m_decalManager.get(); }
		LightsManager&	lightsManager() { return *m_lightsManager.get(); }
		
	private:
		MeshInstance* setupMeshInst(const MeshInstance::Desc& desc, Transform* transform);

	private:
		typedef std::vector<MeshInstance*>	   MeshInstances;
		typedef std::unique_ptr<DebugDrawer>   DebugDrawerPtr;
		typedef std::unique_ptr<DecalManager>  DecalManagerPtr;
		typedef std::unique_ptr<imguiRender>   ImguiRenderPtr;
		typedef std::unique_ptr<LightsManager> LightsManagerPtr;
		
		MeshInstances    m_meshInstances;
		Ptr<Camera>		 m_camera;
		DebugDrawerPtr	 m_debugDrawer;
		DecalManagerPtr  m_decalManager;
		ImguiRenderPtr	 m_imguiRender;
		LightsManagerPtr m_lightsManager;
		RenderOps		 m_renderOps;
	};

} //-- render
} //-- brUGE