#pragma once

#include "prerequisites.hpp"
#include "IComponent.hpp"
#include "GameObject.hpp"

namespace brUGE
{
	//----------------------------------------------------------------------------------------------
	struct DeltaTime
	{
		float m_renderTime = 0.0f;
		float m_updateTime = 0.0f;
	};

	//-- Base class for all kind of the sub-systems in the engine. Each ISystem derived class corresponds
	//-- to the unique IComponent::EType and processes only those components.
	//-- Each ISystem class may hold one or more IWorld instances to be able to have two spaces open
	//-- at the same time and don't interact with each other.
	//----------------------------------------------------------------------------------------------
	class ISystem : public NonCopyable
	{
	public:
		//-- Container of all of the components of a specific type on a particular scene. So you may have
		//-- various scenes (IWorld instances) have been loaded at the same time and they won't interfere with each other.
		//----------------------------------------------------------------------------------------------
		class IWorld
		{
		public:
			IWorld() { }
			virtual ~IWorld() = 0 { }

			virtual bool	init() = 0;

			virtual void	activate() = 0;
			virtual void	deactivate() = 0;

			virtual Handle	createComponent() = 0;
			virtual Handle	createComponent(const pugi::xml_node& data) = 0;
			virtual Handle	cloneComponent(Handle id) = 0;
			virtual bool	removeComponent(Handle id) = 0;

			virtual bool	registerGameObject(Handle gameObj) = 0;
			virtual bool	unregisterGameObject(Handle gameObj) = 0;
		};

		//-- Acts as a container for the intermediate data during processing of an IWorld instance.
		//-- One instance of IWorld may have one or more IContext instances associated with it, on the other
		//-- hand each instance of IContext may be associated only with one IWorld instance.
		//----------------------------------------------------------------------------------------------
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

		virtual bool init() = 0;

		//-- update the global state of the world
		virtual void update(IWorld* world, const DeltaTime& dt) = 0;

		//-- perform work while the world is the constant state. Here we may have multiple Context are working
		//-- separately (even different threads) on the same constant world.
		virtual void process(IContext* context) = 0;

		//-- Functionality to check a game object on the fact that it has all required components and dependencies for
		//-- this particular system.
		inline bool checkRequiredComponents(Handle /*gameObj*/) { return true; }
	};
}
