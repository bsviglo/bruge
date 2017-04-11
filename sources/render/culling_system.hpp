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

			virtual void				onGameObjectAdded(Handle gameObj) override;
			virtual void				onGameObjectRemoved(Handle gameObj) override;
			virtual void				onComponentAdded(IComponent::Handle component) override;
			virtual void				onComponentRemoved(IComponent::Handle component) override;

		private:
			//-- ToDo: substitute with more advanced acceleration structure (BVH, Octo-tree, Quat-tree, etc.)
			std::vector<std::tuple<IComponent::Handle, AABB>> m_objects;
		};

		//--------------------------------------------------------------------------------------------------------------
		class Context : public IContext
		{
		public:

			//----------------------------------------------------------------------------------------------------------
			struct Config
			{
				bool			m_gatherAABB;
				RenderCamera*	m_camera;
			};

		public:
			Context(const ISystem& system, const IWorld& world);
			virtual ~Context() override;

			virtual bool	init(const Config& cfg) override;

		private:
			Config			m_cfg;
			VisibilitySet	m_visibilitySet;
		};

	public:
		virtual void	update(Handle world, const DeltaTime& dt) const override;
		virtual void	process(Handle context) const override;

	private:
	};

}
}

