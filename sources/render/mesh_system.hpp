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

	class MeshCollector;

	//------------------------------------------------------------------------------------------------------------------
	struct MeshInstance
	{
		std::shared_ptr<Mesh>			m_mesh;
		std::shared_ptr<SkinnedMesh>	m_skinnedMesh;
		MatrixPalette					m_worldPalette;
		Transform*						m_transform;

		AABB							m_localBounds;
		AABB							m_worldBounds;
	};

	//------------------------------------------------------------------------------------------------------------------
	class StaticMeshComponent : public IComponent
	{
	public:
		struct Parameters
		{
			std::string	m_resource;
		};

	public:
		StaticMeshComponent(::Handle owner, MeshInstance& instance) : IComponent(owner), m_instance(instance) { }
		virtual ~StaticMeshComponent() override { }

		static TypeID		typeID() { return m_typeID; }
		const Parameters&	params() const { return m_params; }

	private:
		Parameters			m_params;
		MeshInstance&		m_instance;
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
		SkinnedMeshComponent(::Handle owner, MeshInstance& instance) : IComponent(owner), m_instance(instance) { }
		virtual ~SkinnedMeshComponent() override { }

		static TypeID		typeID() { return m_typeID; }
		const Parameters&	params() const { return m_params; }

	private:
		Parameters			m_params;
		MeshInstance&		m_instance;
		static const TypeID	m_typeID;
	};

	//------------------------------------------------------------------------------------------------------------------
	class MeshSystem : public IRenderSystem
	{
	public:

		//--------------------------------------------------------------------------------------------------------------
		class World : public IWorld
		{
		public:
			World(const ISystem& system, Handle universeWorld);
			virtual ~World() override;

			virtual bool				init(const pugi::xml_node& cfg) override;

			virtual void				activate() override;
			virtual void				deactivate() override;

			virtual IComponent::Handle	createComponent(Handle gameObj, IComponent::TypeID typeID) override;
			virtual IComponent::Handle	createComponent(Handle gameObj, IComponent::TypeID typeID, const pugi::xml_node& cfg) override;
			virtual IComponent::Handle	cloneComponent(Handle srcGameObj, Handle dstGameObj, IComponent::TypeID typeID) override;
			virtual void				removeComponent(IComponent::Handle component) override;

		private:
			std::vector<std::unique_ptr<StaticMeshComponent>>	m_staticMeshComponets;
			std::vector<std::unique_ptr<SkinnedMeshComponent>>	m_skinnedMeshComponents;
			std::vector<std::unique_ptr<MeshInstance>>			m_staticMeshInstances;
			std::vector<std::unique_ptr<MeshInstance>>			m_skinnedMeshInstances;

			friend MeshSystem;
		};

		//--------------------------------------------------------------------------------------------------------------
		class Context : public IContext
		{
		public:
			Context(const ISystem& system, const IWorld& world);
			virtual ~Context() override;

			virtual bool init() override;

		public:
			//-- input
			VisibilitySet*					m_visibilitySet;
			RenderCamera*					m_renderCamera;
			Renderer::EPassType				m_passType;
			bool							m_useInstancing;

			//-- input/output
			RenderOps*						m_rops;

			//-- intermediate
			std::unique_ptr<MeshCollector>	m_meshCollector;

			friend MeshSystem;
		};

	public:
		MeshSystem();
		virtual ~MeshSystem() override { }

		virtual bool	init(const pugi::xml_node& cfg) override;

		void			gatherROPs(std::shared_ptr<Context>& c);

		//-- ToDo:
		virtual void	update(Handle world,  const DeltaTime& dt) const override;
		virtual void	process(Handle context) const override;

		static TypeID	typeID() { return m_typeID; }
				
	public:
		static const TypeID m_typeID;
	};

} //-- render
} //-- brUGE