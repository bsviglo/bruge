#pragma once

#include "Renderer.hpp"

#include "engine/IComponent.hpp"
#include "engine/ISystem.hpp"

#include <vector>
#include <unordered_map>

namespace brUGE
{
namespace render
{

	//--------------------------------------------------------------------------------------------------------------
	class VisibilitySet
	{
	private:
		std::unordered_map<ISystem::TypeID, std::vector<IComponent::Handle>> m_buckets;
	};


	//-- All of the render sub-system should be derived from it to be able to correctly response on various rendering
	//-- related events.
	//------------------------------------------------------------------------------------------------------------------
	class IRenderSystem : public ISystem
	{
	public:
		virtual void onVideoModeChanged(const VideoMode& mode) = 0;
	};


	//------------------------------------------------------------------------------------------------------------------
	class RenderSystem : public ISystem
	{
	public:

		//--------------------------------------------------------------------------------------------------------------
		class World : public IWorld
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
		
		private:
		};

		//--------------------------------------------------------------------------------------------------------------
		class Context : public IContext
		{
		public:
			Context();
			virtual ~Context() override;

			virtual bool init() override;

		private:
			RenderOps m_ops;
		};

	public:
		RenderSystem() { }
		virtual ~RenderSystem() override { }

		virtual bool		init(const pugi::xml_node& cfg) override;

		//-- update the global state of the world
		virtual void		update(Handle world, const DeltaTime& dt) const override;

		//-- perform work while the world is the constant state. Here we may have multiple Context are working
		//-- separately (even different threads) on the same constant world.
		virtual void		process(Handle context) const override;

		//--
		virtual Handle		createWorld(const pugi::xml_node& cfg = pugi::xml_node()) override;
		virtual void		removeWorld(Handle world) override;

		virtual Handle		createContext(Handle world) override;
		virtual void		removeContext(Handle context) override;

		static TypeID		typeID() { return m_typeID; }

	private:
		std::unique_ptr<Renderer>	m_renderer;
		static const TypeID			m_typeID;
	};

} // render
} // brUGE
