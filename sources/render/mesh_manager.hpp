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

	//------------------------------------------------------------------------------------------------------------------
	class StaticMeshComponent : public IComponent
	{
	public:
		struct Parameters
		{
			std::string	m_resource;
		};

	public:
		StaticMeshComponent(::Handle owner) : IComponent(owner) { }
		virtual ~StaticMeshComponent() override { }

		static TypeID		typeID() { return m_typeID; }
		const Parameters&	params() const { return m_params; }

	private:
		Parameters			m_params;
		::Handle			m_instance;
		static const TypeID	m_typeID;
	};

	//------------------------------------------------------------------------------------------------------------------
	class SkinnedMeshComponent : public IComponent
	{
	public:
		struct Parameters
		{
			std::string m_resource;
		};

	public:
		SkinnedMeshComponent(::Handle owner) : IComponent(owner) { }
		virtual ~SkinnedMeshComponent() override { }

		static TypeID		typeID() { return m_typeID; }
		const Parameters&	params() const { return m_params; }


	private:
		Parameters			m_params;
		::Handle			m_instance;

		static const TypeID	m_typeID;
	};

	//------------------------------------------------------------------------------------------------------------------
	class MeshSystem : public IRenderSystem
	{
	public:

		//--------------------------------------------------------------------------------------------------------------
		class World : public ISystem::IWorld
		{
		public:
			World();
			virtual ~World() override;

			virtual bool				init() override;

			virtual void				activate() override;
			virtual void				deactivate() override;

			virtual IComponent::Handle	createComponent(Handle gameObj, IComponent::TypeID typeID) override;
			virtual IComponent::Handle	createComponent(Handle gameObj, IComponent::TypeID typeID, const pugi::xml_node& cfg) override;
			virtual IComponent::Handle	cloneComponent (Handle srcGameObj, Handle dstGameObj, IComponent::TypeID typeID) override;
			virtual bool				removeComponent(Handle gameObj, IComponent::Handle component) override;

			virtual void				onGameObjectAdded(Handle gameObj) override;
			virtual void				onGameObjectRemoved(Handle gameObj) override;
			virtual void				onComponentAdded(Handle gameObj, IComponent::Handle component) override;
			virtual void				onComponentRemoved(Handle gameObj, IComponent::Handle component) override;

		private:
			std::vector<std::unique_ptr<StaticMeshComponent>>	m_staticMehComponets;
			std::vector<std::unique_ptr<SkinnedMeshComponent>>	m_skinnedMeshComponents;
			std::vector<std::unique_ptr<MeshInstance>>			m_meshInstances;

			friend MeshSystem;
		};

		//----------------------------------------------------------------------------------------------
		class Context : public ISystem::IContext
		{
		public:
			virtual ~Context() override;

			virtual bool init() override;

		private:
			std::unique_ptr<MeshCollector>	m_meshCollector;
			RenderOps						m_rops;

			friend MeshSystem;
		};

	public:
		MeshSystem();
		virtual ~MeshSystem() override { }

		virtual bool	init(const pugi::xml_node& cfg) override;
		virtual void	update(Handle world,  const DeltaTime& dt) const override;
		virtual void	process(Handle context) const override;

		static TypeID	typeID() { return m_typeID; }
				
	public:
		static const TypeID m_typeID;
	};



	//-- ToDo: legacy




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
		uint				gatherROPs(Renderer::EPassType pass, bool instanced, RenderOps& rops, const mat4f& viewPort, AABB* aabb = nullptr);

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