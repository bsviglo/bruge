#pragma once

#include "prerequisites.hpp"
#include "render_common.h"
#include "render_system.hpp"
#include "Mesh.hpp"

namespace brUGE
{

	struct Transform;

namespace render
{

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
	class MeshManager : public NonCopyable
	{
	public:
		MeshManager();
		~MeshManager();

		bool				init();
		void				update(float dt);
		uint				gatherROPs(RenderSystem::EPassType pass, bool instanced, RenderOps& rops, const mat4f& viewPort, AABB* aabb = nullptr);

		//-- models.
		Handle				addMesh(const MeshInstance::Desc& desc, Transform* transform);
		void				delMesh(Handle handle);
		MeshInstance&		getMesh(Handle handle);

	private:
		typedef std::vector<MeshInstance*> MeshInstances;

		MeshInstances m_meshInstances;
	};

} //-- render
} //-- brUGE