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
	//------------------------------------------------------------------------------------------------------------------
	class RenderPipelineComponent : IComponent
	{

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

			virtual bool				init(const pugi::xml_node& cfg) override;

			virtual void				activate() override;
			virtual void				deactivate() override;

			virtual IComponent::Handle	createComponent(Handle gameObj, IComponent::TypeID typeID) override;
			virtual IComponent::Handle	createComponent(Handle gameObj, IComponent::TypeID typeID, const pugi::xml_node& cfg) override;
			virtual IComponent::Handle	cloneComponent(Handle srcGameObj, Handle dstGameObj, IComponent::TypeID typeID) override;
			virtual void				removeComponent(IComponent::Handle component) override;
		
		private:
			std::unique_ptr<RenderPipelineComponent> m_renderPipeline;
		};

		//--------------------------------------------------------------------------------------------------------------
		class Context : public IContext
		{
		public:
			Context(const ISystem& system, const IWorld& world);
			virtual ~Context() override;

			virtual bool init() override;

		private:
			std::unordered_map<ISystem::TypeID, Handle> m_contexts;
		};

	public:
		RenderSystem();
		virtual ~RenderSystem() override;

		virtual bool	init(const pugi::xml_node& cfg) override;
		virtual void	update(Handle world,  const DeltaTime& dt) const override;
		virtual void	process(Handle context) const override;

		static TypeID	typeID() { return m_typeID; }

	private:
		std::unique_ptr<Renderer>	m_renderer;
		static const TypeID			m_typeID;
	};

} // render
} // brUGE
