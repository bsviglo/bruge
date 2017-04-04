#include "render_system.hpp"

//-- rename *_manager -> *_system
#include "mesh_manager.hpp"
#include "light_manager.hpp"
#include "shadow_manager.hpp"

#include "culling_system.hpp"

#include "rttr/registration"

namespace brUGE
{
	using namespace rttr;
	using namespace render;

	RTTR_REGISTRATION
	{
		registration::class_<RenderSystem>("render::RenderSystem")
			.constructor<>()
			.property_readonly("typeID", RenderSystem::typeID)
			.property_readonly("TypeIDPath", []()-> ISystem::TypeIDPath { return { ISystem::TypeID::C_INVALID }; } );

	}

namespace render
{

	//------------------------------------------------------------------------------------------------------------------
	RenderSystem::RenderSystem()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	bool RenderSystem::init(const pugi::xml_node& cfg)
	{
		bool ok = true;

		for (auto typeID : m_systemInitializationOrder)
		{
			ok &= m_ownedSystems[typeID]->init(cfg);
		}

		return ok;
	}

	//------------------------------------------------------------------------------------------------------------------
	void RenderSystem::update(Handle world, const DeltaTime& dt) const
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	void RenderSystem::process(Handle handle) const
	{
		auto& c = *context(handle);

		//-- process shadows
		system<ShadowSystem>().process(c.m_contexts[ShadowSystem::typeID()]);


		CameraSystem::Context cameraContext;
		ShadowSystem::Context shadowContext;
		CullingSystem::Context cullingContext;
		PostProcessingSystem::Context ppContext;
		MeshSystem::Context meshContext;
		LightSystem::Context lightContext;





		auto c = static_cast<Context*>(context);

		//-- passes (main pass)

		//-- cull render objects
		system<CullingSystem>().process(c->context<CullingSystem>());
		system<MeshSystem>().process(c->context<MeshSystem>());



		c->context<CullingSystem>();

		//-- 1. z-only pass.
		{
			SCOPED_TIME_MEASURER_EX("z-pass")

			//-- gather all ROPs.
			RenderOps ops;
			{
				SCOPED_TIME_MEASURER_EX("resolve visibility")
				{
					SCOPED_TIME_MEASURER_EX("meshes")
					m_meshManager->gatherROPs(
						Renderer::PASS_Z_ONLY, false, ops, m_camera->renderCam().m_viewProj
						);
				}
				{
					SCOPED_TIME_MEASURER_EX("terrain")
					m_terrainSystem->gatherROPs(
						Renderer::PASS_Z_ONLY, ops, m_camera->renderCam().m_viewProj,
						m_camera->renderCam().m_invView.applyToOrigin()
						);
				}
			}
			
			rs().beginPass(Renderer::PASS_Z_ONLY);
			rs().setCamera(&m_camera->renderCam());
			rs().shaderContext().updatePerFrameViewConstants();
			rs().addROPs(ops);
			rs().endPass();
		}

		//-- 2. decal manager.
		{
			SCOPED_TIME_MEASURER_EX("decal-pass")

			RenderOps ops;
			m_decalManager->gatherRenderOps(ops);

			rs().beginPass(Renderer::PASS_DECAL);
			rs().setCamera(&m_camera->renderCam());
			rs().shaderContext().updatePerFrameViewConstants();
			rs().addROPs(ops);
			rs().endPass();
		}

		//-- 3. light pass
		{
			SCOPED_TIME_MEASURER_EX("light-pass")

			RenderOps ops;
			m_lightsManager->gatherROPs(ops);

			rs().beginPass(Renderer::PASS_LIGHT);
			rs().setCamera(&m_camera->renderCam());
			rs().shaderContext().updatePerFrameViewConstants();
			rs().addROPs(ops);
			rs().endPass();
		}

		//-- 4. cast shadows
		{
			SCOPED_TIME_MEASURER_EX("cast shadows")

			m_shadowManager->castShadows(m_camera->renderCam(), *m_lightsManager.get(), *m_meshManager.get());
		}

		//-- 5. resolve shadows.
		{
			SCOPED_TIME_MEASURER_EX("resolve shadows")


			m_shadowManager->receiveShadows(&m_camera->renderCam());
		}
		
		//-- 6. main pass.
		{
			SCOPED_TIME_MEASURER_EX("main-pass")

			//-- gather all ROPs.
			RenderOps ops;
			{
				SCOPED_TIME_MEASURER_EX("resolve visibility")
				{
					SCOPED_TIME_MEASURER_EX("meshes")
						m_meshManager->gatherROPs(
						Renderer::PASS_MAIN_COLOR, false, ops, m_camera->renderCam().m_viewProj
						);
				}
				{
					SCOPED_TIME_MEASURER_EX("terrain")
						m_terrainSystem->gatherROPs(
						Renderer::PASS_MAIN_COLOR, ops, m_camera->renderCam().m_viewProj,
						m_camera->renderCam().m_invView.applyToOrigin()
						);
				}
			}

			rs().beginPass(Renderer::PASS_MAIN_COLOR);
			rs().setCamera(&m_camera->renderCam());
			rs().shaderContext().updatePerFrameViewConstants();
			rs().addROPs(ops);
			rs().endPass();
		}

		//-- 8. draw post-processing.
		{
			SCOPED_TIME_MEASURER_EX("post-processing")

			rs().beginPass(Renderer::PASS_POST_PROCESSING);
			rs().addROPs(RenderOps());
			rs().setCamera(&m_camera->renderCam());
			rs().shaderContext().updatePerFrameViewConstants();
			m_postProcessing->draw();
			rs().endPass();
		}

		//-- 9. update debug drawer.
		//-- Note: It implicitly activate passes PASS_DEBUG_WIRE and PASS_DEBUG_SOLID.
		//-- ToDo: reconsider.
		{
			SCOPED_TIME_MEASURER_EX("debug drawer")

			m_debugDrawer->draw();
		}

		//-- 10. draw imgui.
		{
			SCOPED_TIME_MEASURER_EX("imgui")

			TimingPanel::instance().visualize();
			WatchersPanel::instance().visualize();
		}		//-- ToDo:
	}

	//------------------------------------------------------------------------------------------------------------------
	Handle RenderSystem::createWorld(const pugi::xml_node& cfg)
	{
		auto world = std::make_unique<World>(*this);

		world->init(cfg);

		m_worlds.emplace_back(std::move(world));

		return m_worlds.size() - 1;
	}

	//------------------------------------------------------------------------------------------------------------------
	Handle RenderSystem::createContext(Handle world)
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	bool RenderSystem::checkRequiredComponents(Handle handle) const
	{
		GameObject* gameObj = nullptr;
	}

	//------------------------------------------------------------------------------------------------------------------
	IComponent::Handle RenderSystem::World::createComponent(Universe::World& world, Handle gameObj, IComponent::TypeID typeID)
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	IComponent::Handle RenderSystem::World::createComponent(Universe::World& world, Handle gameObj, IComponent::TypeID typeID, const pugi::xml_node& cfg)
	{
		auto meshWorld = static_cast<MeshSystem::World>(m_worlds[SYSTEM_MESH]);

		auto meshComponent = meshWorld->createComponent(gameObj);
	}

	//------------------------------------------------------------------------------------------------------------------
	RenderSystem::World::World()
	{
		
	}

	//------------------------------------------------------------------------------------------------------------------
	RenderSystem::World::~World()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	bool RenderSystem::World::init()
	{
		bool ok = true;

		for (auto typeID : m_system.m_systemInitializationOrder)
		{
			ok &= m_ownedworlds[typeID]->init(cfg);
		}

		return ok;
	}

	//------------------------------------------------------------------------------------------------------------------
	RenderSystem::Context::Context()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	RenderSystem::Context::~Context()
	{
	}

	//------------------------------------------------------------------------------------------------------------------
	bool RenderSystem::Context::init()
	{
		bool ok = true;

		for (auto typeID : m_system.m_systemInitializationOrder)
		{
			ok &= m_dependentContexts[typeID]->init();
		}
	}


	//------------------------------------------------------------------------------------------------------------------

} // render
} // brUGE

