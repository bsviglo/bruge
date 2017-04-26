#pragma once

#include "prerequisites.hpp"
#include "IComponent.hpp"
#include "pugixml/pugixml.hpp"

#include <array>

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
		//-- Note: it is unique only across single execution of one application. It is NOT suitable for serialization.
		//--------------------------------------------------------------------------------------------------------------
		class TypeID
		{
		public:
			static const uint32 C_MAX_SYSTEM_TYPES = 16;
			static const TypeID C_INVALID;

		public:
			TypeID();

			uint32 id() const		{ return m_id; }
			operator uint32() const { return m_id; }

		private:
			TypeID(uint32 ivalidID) : m_id(0) { }

			uint32 m_id;
		};

	public:
		//-- Container of all of the components of a specific type on a particular scene. So you may have
		//-- various scenes (IWorld instances) have been loaded at the same time and they won't interfere with each other.
		//--------------------------------------------------------------------------------------------------------------
		class IWorld
		{
		public:
			IWorld(const ISystem& system, Handle universeWorld) : m_system(system), m_universeWorld(universeWorld) { }
			virtual ~IWorld() = 0 { }

			virtual bool				init(const pugi::xml_node& cfg) = 0;

			virtual void				activate() = 0;
			virtual void				deactivate() = 0;

			virtual IComponent::Handle	createComponent(Handle gameObj, IComponent::TypeID typeID) = 0;
			virtual IComponent::Handle	createComponent(Handle gameObj, IComponent::TypeID typeID, const pugi::xml_node& cfg) = 0;
			virtual IComponent::Handle	cloneComponent(Handle srcGameObj, Handle dstGameObj, IComponent::TypeID typeID) = 0;
			virtual void				removeComponent(IComponent::Handle component) = 0;

			virtual void				onGameObjectAdded(Handle gameObj)					{ }
			virtual void				onGameObjectRemoved(Handle gameObj)					{ }
			virtual void				onComponentAdded(IComponent::Handle component)		{ }
			virtual void				onComponentRemoved(IComponent::Handle component)	{ }

		protected:
			const ISystem&	m_system;
			const Handle	m_universeWorld;
		};

		//-- Acts as a container for the intermediate data during processing of an IWorld instance.
		//-- One instance of IWorld may have one or more IContext instances associated with it, on the other
		//-- hand each instance of IContext may be associated only with one IWorld instance.
		//--
		//-- Note: In terms of Render System into IContext should be moved all the resources associated with each particular
		//-- camera view. For example Shadow System may hold in a IContext instance all of the data associated with
		//-- this camera.
		//--------------------------------------------------------------------------------------------------------------
		class IContext
		{
		public:
			IContext(const ISystem& system, const IWorld& world) : m_system(system), m_world(m_world) { }
			virtual ~IContext() = 0 { }

			virtual bool init() = 0;

			inline  const ISystem&	system() const	{ return m_system; }
			inline  const IWorld&	world() const	{ return m_world; }

		protected:
			const ISystem&	m_system;
			const IWorld&	m_world;
		};

	public:
		ISystem() { }
		virtual ~ISystem() = 0 { }

		virtual bool		init(const pugi::xml_node& cfg = pugi::xml_node()) = 0;

		//-- update the global state of the world
		virtual void		update(Handle world, const DeltaTime& dt) const = 0;

		//-- perform work while the world is the constant state. Here we may have multiple Context are working
		//-- separately (even different threads) on the same constant world.
		virtual void		process(Handle context) const = 0;

		//--
		virtual Handle		createWorld(Handle universeWorld, const pugi::xml_node& cfg = pugi::xml_node()) = 0;
		virtual void		removeWorld(Handle handle) = 0;

		virtual Handle		createContext(Handle world) = 0;
		virtual void		removeContext(Handle handle) = 0;

		inline IWorld&		world(Handle handle) const;
		inline IContext&	context(Handle handle) const;

		//-- Functionality to check a game object on the fact that it has all required components and dependencies for
		//-- this particular system.
		//-- For example AnimationSystem requires you to have these components TYPE_SKINNED_MESH and TYPE_TRANSFORM
		virtual bool		requiredComponents(Handle /*gameObj*/) const = 0;

	protected:
		std::vector<std::unique_ptr<IWorld>>	m_worlds;
		std::vector<std::unique_ptr<IContext>>	m_contexts;
	};
}
