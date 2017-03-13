#pragma once

#include "prerequisites.hpp"
#include "engine/ISystem.hpp"
#include "utils/Data.hpp"
#include "render/Camera.h"
#include <vector>

namespace brUGE
{
	//------------------------------------------------------------------------------------------------------------------
	class CameraComponent : public IComponent
	{
	public:
		CameraComponent() : IComponent(IComponent::TYPE_CAMERA) { }
		virtual ~CameraComponent() override { }

	private:
		Handle m_camera;
	};


	//------------------------------------------------------------------------------------------------------------------
	class CameraSystem : public ISystem
	{
	public:

		//--------------------------------------------------------------------------------------------------------------
		class World : public ISystem::IWorld
		{
		public:
			World() { }
			virtual ~World() override { }

			virtual bool	init() override;

			virtual void	activate() override;
			virtual void	deactivate() override;

			virtual Handle	createComponent() override;
			virtual Handle	cloneComponent(Handle id) override;
			virtual bool	removeComponent(Handle id) override;

			virtual bool	registerGameObject(Handle entity) override;
			virtual bool	unregisterGameObject(Handle entity) override;

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