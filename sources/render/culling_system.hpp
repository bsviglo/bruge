#pragma once

#include "engine/IComponent.hpp"
#include "engine/ISystem.hpp"

#include <tuple>
#include <vector>
#include <unordered_map>

namespace brUGE
{
namespace render
{

	//--------------------------------------------------------------------------------------------------------------
	struct VisibilitySet
	{
		typedef std::vector<IComponent::Handle> Bucket;
		std::array<Bucket, IComponent::TypeID::C_MAX_COMPONENT_TYPES> m_buckets;
	};

	//------------------------------------------------------------------------------------------------------------------
	class CullingSystem : public ISystem
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

			//-- interface of culling
			void						add(IComponent::Handle handle, const AABB& aabb);
			void						modify(IComponent::Handle handle, const AABB& aabb);
			void						remove(IComponent::Handle handle);

		private:
			//-- ToDo: substitute with more advanced acceleration structure (BVH, Quad-tree, etc.)
			typedef std::vector<std::pair<IComponent::Handle, AABB>> Objects;
			std::array<Objects, IComponent::TypeID::C_MAX_COMPONENT_TYPES> m_objects;

			friend CullingSystem;
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
			RenderCamera*	m_camera;

			//-- output
			VisibilitySet	m_visibilitySet;

			//-- intermediate

			friend CullingSystem;
		};

	public:
		CullingSystem();
		virtual ~CullingSystem() override;

		virtual bool	init(const pugi::xml_node& cfg) override;
		virtual void	update(Handle world,  const DeltaTime& dt) const override;
		virtual void	process(Handle context) const override;

		static TypeID	typeID() { return m_typeID; }

	private:
		static const TypeID m_typeID;
	};

}
}

