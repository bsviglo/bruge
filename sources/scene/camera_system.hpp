#pragma once

#include "prerequisites.hpp"
#include "engine/ISystem.hpp"
#include "utils/Data.hpp"
#include "render/Camera.h"
#include <vector>

namespace brUGE
{
namespace render
{
	//------------------------------------------------------------------------------------------------------------------
	class CameraComponent : public IComponent
	{
	public:
		struct Parameters
		{
			Projection			m_projection;
			ScreenResolution	m_screenResolution;
		};

	public:
		CameraComponent(::Handle owner) : IComponent(owner) { }
		virtual ~CameraComponent() override { }

		static TypeID		typeID() { return m_typeID; }
		const Parameters&	params() const { return m_params; }

	private:
		Parameters			m_params;
		static const TypeID	m_typeID;
	};


	//------------------------------------------------------------------------------------------------------------------
	class CameraSystem : public ISystem
	{
	public:

		//--------------------------------------------------------------------------------------------------------------
		class World : public ISystem::IWorld
		{
		public:
			World();
			virtual ~World() override;

			virtual bool				init() override;

			virtual void				activate() override;
			virtual void				deactivate() override;

			virtual IComponent::Handle	createComponent(Handle gameObj, IComponent::TypeID typeID) override;
			virtual IComponent::Handle	createComponent(Handle gameObj, IComponent::TypeID typeID, const pugi::xml_node& cfg) override;
			virtual IComponent::Handle	cloneComponent(Handle srcGameObj, Handle dstGameObj, IComponent::TypeID typeID) override;
			virtual void				removeComponent(IComponent::Handle component) override;

		private:
			std::vector<std::unique_ptr<CameraComponent>>	m_components;
			std::vector<Camera>								m_cameras;
		};

		//--------------------------------------------------------------------------------------------------------------
		class Context : public ISystem::IContext
		{

		};

	public:
		CameraSystem() { }
		virtual ~CameraSystem() override { }

		virtual bool init() override;
		virtual void update(IWorld* world, const DeltaTime& dt) override;
		virtual void process(IContext* context) override;

	private:
		std::vector<std::unique_ptr<World>>		m_worlds;
		std::vector<std::unique_ptr<Context>>	m_contexts;
	};

}
}