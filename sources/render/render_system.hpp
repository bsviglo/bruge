#pragma once

#include "engine/IComponent.hpp"
#include "engine/ISystem.hpp"

#include <vector>
#include <map>

namespace brUGE
{
namespace render
{

	//------------------------------------------------------------------------------------------------------------------
	class RenderSystem : public ISystem
	{
	public:

		//--------------------------------------------------------------------------------------------------------------
		class World : public IWorld
		{
		public:
			World() { }
			virtual ~World() override { }

			virtual bool				init() override;

			virtual void				activate() override;
			virtual void				deactivate() override;

			virtual IComponent::Handle	createComponent(Universe::World& world, Handle gameObj, IComponent::TypeID typeID) override;
			virtual IComponent::Handle	createComponent(Universe::World& world, Handle gameObj, IComponent::TypeID typeID, const pugi::xml_node& cfg) override;
			virtual IComponent::Handle	cloneComponent (Universe::World& world, Handle srcGameObj, Handle dstGameObj, IComponent::TypeID typeID) override;
			virtual bool				removeComponent(Universe::World& world, Handle gameObj, IComponent::Handle component) override;

		private:
			std::unordered_map<ISystem::TypeID, std::unique_ptr<IWorld>> m_worlds;
		};

		//--------------------------------------------------------------------------------------------------------------
		class Context : public IContext
		{
		public:
			Context() { }
			virtual ~Context() override { }

			virtual bool init(ISystem* system, IWorld* world) override;

		private:
			std::unordered_map<ISystem::TypeID, std::unique_ptr<IContext>> m_contexts;
		};

	public:
		RenderSystem() { }
		virtual ~RenderSystem() override { }

		virtual bool		init() override;

		//-- update the global state of the world
		virtual void		update(IWorld* world, const DeltaTime& dt) const override;

		//-- perform work while the world is the constant state. Here we may have multiple Context are working
		//-- separately (even different threads) on the same constant world.
		virtual void		process(IContext* context) const override;

		//--
		virtual Handle		createWorld(const pugi::xml_node& cfg = pugi::xml_node()) override;
		virtual void		removeWorld(Handle handle) override;

		virtual Handle		createContext(Handle world) override;
		virtual void		removeContext(Handle handle) override;

		static TypeID		typeID() { return m_typeID; }

		//-- Functionality to check a game object on the fact that it has all required components and dependencies for
		//-- this particular system.
		//-- For example AnimationSystem requires you to have these components TYPE_SKINNED_MESH and TYPE_TRANSFORM
		virtual bool		checkRequiredComponents(Handle /*gameObj*/) const override;

	private:
		std::unordered_map<ISystem::TypeID, std::unique_ptr<ISystem>>	m_systems;
		static const TypeID												m_typeID;
	};

} // render
} // brUGE
