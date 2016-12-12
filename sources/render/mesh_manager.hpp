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

	class MeshCollector;

	//----------------------------------------------------------------------------------------------
	struct MeshInstance
	{
		struct Desc
		{
			Desc() : fileName(nullptr) { }

			const char* fileName;
		};

		std::shared_ptr<Mesh>			m_mesh;
		std::shared_ptr<SkinnedMesh>	m_skinnedMesh;
		MatrixPalette					m_worldPalette;
		Transform*						m_transform;
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
		std::vector<std::unique_ptr<MeshInstance>>	m_meshInstances;
		std::unique_ptr<MeshCollector>				m_meshCollector;
	};

} //-- render
} //-- brUGE