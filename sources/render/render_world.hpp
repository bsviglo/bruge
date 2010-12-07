#pragma once

#include "prerequisites.h"
#include "utils/Ptr.h"
#include "math/Vector3.h"
#include "Color.h"
#include "Mesh.hpp"
#include "render_system.hpp"
#include "DebugDrawer.h"
#include <memory>

namespace brUGE
{
	struct Transform;

namespace render
{
	class  Camera;
	class  SkinnedMesh;
	class  Mesh;

	//----------------------------------------------------------------------------------------------
	struct LightInstance
	{
		struct Desc
		{
			Desc() : m_radius(0.0f) { }

			vec3f m_pos;
			vec3f m_dir;
			float m_radius;
			Color m_color;
		};

		Desc m_data;
	};


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
	public:
		bool init();
		bool fini();

		//-- resolve visibility.
		void				 setCamera(const Ptr<Camera>& cam);
		void				 resolveVisibility();

		//-- lights.
		Handle				 addLightDef	(const LightInstance::Desc& lInst);
		bool				 delLightDef	(Handle handle);
		const LightInstance* getLightDef	(Handle handle);	

		//-- models.
		Handle				 addMeshDef		(const MeshInstance::Desc& desc, Transform* transform);
		bool				 delMeshDef		(Handle handle);
		const MeshInstance*  getMeshDef		(Handle handle);	
		
	private:
		MeshInstance* setupMeshInst(const MeshInstance::Desc& desc, Transform* transform);

	private:
		typedef std::vector<MeshInstance*>	  MeshInstances;
		typedef std::vector<LightInstance*>	  LightInstances;
		typedef std::unique_ptr<DebugDrawer>  DebugDrawerPtr;
		
		LightInstances	m_lightInstances;
		MeshInstances   m_meshInstances;
		Ptr<Camera>		m_camera;
		DebugDrawerPtr	m_debugDrawer;
		RenderOps		m_renderOps;
	};

} //-- render
} //-- brUGE