#pragma once

#include "prerequisites.hpp"
#include "IComponent.hpp"
#include "GameObject.hpp"

namespace brUGE
{

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

			virtual bool						init() = 0;

			virtual void						activate() = 0;
			virtual void						deactivate() = 0;

			virtual std::unique_ptr<IComponent>	createComponent() = 0;
			virtual std::unique_ptr<IComponent> cloneComponent(const std::unique_ptr<IComponent>& c) = 0;

			virtual bool						registerGameObject(const std::shared_ptr<GameObject>& entity) = 0;
			virtual bool						unregisterGameObject(const std::shared_ptr<GameObject>& entity) = 0;
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
		virtual void update(IWorld* world) = 0;
		virtual void process(IContext* context) = 0;
	};
}
