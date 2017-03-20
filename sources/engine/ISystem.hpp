#pragma once

#include "prerequisites.hpp"
#include "IComponent.hpp"
#include "GameObject.hpp"

namespace brUGE
{
	//------------------------------------------------------------------------------------------------------------------
	struct DeltaTime
	{
		float m_renderTime = 0.0f;
		float m_updateTime = 0.0f;
	};

	//-- Base class for all kind of the sub-systems in the engine. Each ISystem derived class corresponds
	//-- to the unique IComponent::EType and processes only those components.
	//-- Each ISystem class may hold one or more IWorld instances to be able to have two spaces open
	//-- at the same time and don't interact with each other.
	//------------------------------------------------------------------------------------------------------------------
	class ISystem : public NonCopyable
	{
	public:
		//-- Structure that holds unique ID of a ISystem derived class.
		//-- Note: it is unique only across single execution of one application.It is NOT suitable for serialization.
		//--------------------------------------------------------------------------------------------------------------
		class TypeID
		{
		public:
			static const uint32 C_MAX_SYSTEM_TYPES = 16;

		public:
			TypeID();

			uint32 id() const		{ return m_id; }
			operator uint32() const { return m_id; }

		private:
			uint32 m_id;
		};

	public:
		//-- Container of all of the components of a specific type on a particular scene. So you may have
		//-- various scenes (IWorld instances) have been loaded at the same time and they won't interfere with each other.
		//--------------------------------------------------------------------------------------------------------------
		class IWorld
		{
		public:
			IWorld() { }
			virtual ~IWorld() = 0 { }

			virtual bool	init() = 0;

			virtual void	activate() = 0;
			virtual void	deactivate() = 0;
		};

		//--------------------------------------------------------------------------------------------------------------
		class IComponentWorld : public IWorld
		{
		public:
		};

		//--------------------------------------------------------------------------------------------------------------
		class ICacheWorld : public IWorld
		{
		public:

			virtual void onGameObjectAdded(Handle gameObj) = 0;
			virtual void onGameObjectRemoved(Handle gameObj) = 0;
			virtual void onComponentAdded(Handle gameObj, IComponent::ID component) = 0;
			virtual void onComponentUpdated(Handle gameObj, IComponent::ID component) = 0;
			virtual void onComponentRemoved(Handle gameObj, IComponent::ID component) = 0;
		};

		//-- Acts as a container for the intermediate data during processing of an IWorld instance.
		//-- One instance of IWorld may have one or more IContext instances associated with it, on the other
		//-- hand each instance of IContext may be associated only with one IWorld instance.
		//--------------------------------------------------------------------------------------------------------------
		class IContext
		{
		public:
			IContext() { }
			virtual ~IContext() = 0 { }

			virtual bool init(ISystem* system, IWorld* world) = 0;
		};

	public:
		ISystem() { }
		virtual ~ISystem() = 0 { }

		virtual bool		init() = 0;

		//-- update the global state of the world
		virtual void		update(IWorld* world, const DeltaTime& dt) const = 0;

		//-- perform work while the world is the constant state. Here we may have multiple Context are working
		//-- separately (even different threads) on the same constant world.
		virtual void		process(IContext* context) const = 0;

		//--
		virtual Handle		createWorld(const pugi::xml_node& cfg = pugi::xml_node()) = 0;
		virtual Handle		createContext(Handle world) = 0;
		virtual void		removeWorld(Handle handle) = 0;
		virtual void		removeContext(Handle handle) = 0;
		virtual IWorld*		world(Handle handle) = 0;
		virtual IContext*	context(Handle handle) = 0;

		//-- Functionality to check a game object on the fact that it has all required components and dependencies for
		//-- this particular system.
		//-- For example AnimationSystem requires you to have these components TYPE_SKINNED_MESH and TYPE_TRANSFORM
		virtual bool		requiredComponents(Handle /*gameObj*/) const = 0;
	};
}
