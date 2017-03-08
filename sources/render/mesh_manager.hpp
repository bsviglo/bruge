#pragma once

#include "prerequisites.hpp"
#include "engine/ISystem.hpp"
#include "render_common.h"
#include "render_system.hpp"
#include "Mesh.hpp"

namespace brUGE
{
	struct Transform;

namespace render
{

	//----------------------------------------------------------------------------------------------
	class StaticMeshComponent : public IComponent
	{
	public:
		StaticMeshComponent() : IComponent(TYPE_STATIC_MESH) { }
		virtual ~StaticMeshComponent() override { }

	private:
		Handle m_instance;
	};

	//----------------------------------------------------------------------------------------------
	class SkinnedMeshComponent : public IComponent
	{
	public:
		SkinnedMeshComponent() : IComponent(TYPE_SKINNED_MESH) { }
		virtual ~SkinnedMeshComponent() override { }

	private:
		Handle m_instance;
	};

	//----------------------------------------------------------------------------------------------
	class MeshSystem : public ISystem
	{
	public:

		//----------------------------------------------------------------------------------------------
		class World : public ISystem::IWorld
		{
		public:
			World();
			virtual ~World() override;

			virtual bool						init() override;

			virtual void						activate() override;
			virtual void						deactivate() override;

			virtual std::unique_ptr<IComponent>	createComponent() override;
			virtual std::unique_ptr<IComponent> cloneComponent(const std::unique_ptr<IComponent>& c) override;

			virtual bool						registerGameObject(const std::shared_ptr<GameObject>& entity) override;
			virtual bool						unregisterGameObject(const std::shared_ptr<GameObject>& entity) override;

		private:
			std::vector<std::unique_ptr<MeshInstance>> m_meshInstances;
		};

		//----------------------------------------------------------------------------------------------
		class Context : public ISystem::IContext
		{
		public:

		private:
			std::unique_ptr<MeshCollector>	m_meshCollector;
			RenderOps						m_rops;
		};

	public:
		MeshSystem();
		virtual ~MeshSystem() override;

		virtual bool init() override;
		virtual void update(IWorld* world) override;
		virtual void process(IContext* context) override;

	public:
		std::vector<std::unique_ptr<World>>		m_worlds;
		std::vector<std::unique_ptr<Context>>	m_contexts;
	};


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
		Handle				createMeshInstance(const MeshInstance::Desc& desc, Transform* transform);
		void				removeMeshInstance(Handle handle);
		MeshInstance&		getMeshInstance(Handle handle);

	private:
		std::vector<std::unique_ptr<MeshInstance>>	m_meshInstances;
		std::unique_ptr<MeshCollector>				m_meshCollector;
	};

} //-- render
} //-- brUGE