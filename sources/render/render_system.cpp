#include "render_system.hpp"

//-- rename *_manager -> *_system
#include "mesh_system.hpp"
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
			.property_readonly("typeID", RenderSystem::typeID);
	}

namespace render
{

	//------------------------------------------------------------------------------------------------------------------
	RenderSystem::RenderSystem()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	RenderSystem::~RenderSystem()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	bool RenderSystem::init(const pugi::xml_node& cfg)
	{
		bool ok = true;

		for (auto typeID : m_childSystemInitOrder)
		{
			ok &= m_childSystems[typeID]->init(cfg);
		}

		return ok;
	}

	//------------------------------------------------------------------------------------------------------------------
	void RenderSystem::update(Handle world, const DeltaTime& dt) const
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	void RenderSystem::process(Handle cHandle) const
	{
		//-- ToDo: reconsider access to the worlds\contexts\systems
		auto&		c = static_cast<Context&>(context(cHandle));
		const auto& w = static_cast<const World&>(c.world());

		auto& camContext    = engine().system<CameraSystem>.context(c.m_contexts[CameraSystem::typeID()]);
		auto& meshContext   = engine().system<MeshSystem>.context(c.m_contexts[MeshSystem::typeID()]);
		auto& lightContext  = engine().system<LightSystem>.context(c.m_contexts[LightSystem::typeID()]);
		auto& cullContext   = engine().system<CullingSystem>.context(c.m_contexts[CullingSystem::typeID()]);
		auto& shadowContext = engine().system<ShadowSystem>.context(c.m_contexts[ShadowSystem::typeID()]);

		//-- gather visible objects for main pass
		{
			cullContext.m_camera = &camContext.m_camera;
			engine().system<CullingSystem>().process(c.m_contexts[CullingSystem::typeID()]);
		}

		//-- gather z-only rops and execute them
		{
			RenderOps ops;

			meshContext.m_visibilitySet = &cullContext.m_visibilitySet;
			meshContext.m_camera		= &camContext.m_camera;
			meshContext.m_pass			= Renderer::PASS_Z_ONLY;
			meshContext.m_useInstancing = true;

			engine().system<MeshSystem>().process(c.m_contexts[MeshSystem::typeID()]);

			//-- ToDo:
			rs().beginPass(Renderer::PASS_Z_ONLY);
			rs().setCamera(&m_camera->renderCam());
			rs().shaderContext().updatePerFrameViewConstants();
			rs().addROPs(ops);
			rs().endPass();
		}

		//-- process shadows
		{
			//-- seems like changin process to cast and receive maybe more robust
			engine().system<ShadowSystem>().process(c.m_contexts[ShadowSystem::typeID()]);

			//-- ToDo: execute draw commands
		}

		//-- 


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
	IComponent::Handle RenderSystem::World::createComponent(Handle gameObj, IComponent::TypeID typeID)
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	IComponent::Handle RenderSystem::World::createComponent(Handle gameObj, IComponent::TypeID typeID, const pugi::xml_node& cfg)
	{
		
	}

	//------------------------------------------------------------------------------------------------------------------
	IComponent::Handle RenderSystem::World::cloneComponent(Handle srcGameObj, Handle dstGameObj, IComponent::TypeID typeID)
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	void RenderSystem::World::removeComponent(IComponent::Handle component)
	{

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

		for (auto typeID : m_system.m_childSystemInitOrder)
		{
			ok &= m_childWorlds[typeID]->init(cfg);
		}

		return ok;
	}

	//------------------------------------------------------------------------------------------------------------------
	RenderSystem::Context::Context(const ISystem& system, const IWorld& world)
		:	IContext(system, world)
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	RenderSystem::Context::~Context()
	{
	}

	//------------------------------------------------------------------------------------------------------------------
	bool RenderSystem::Context::init()
	{
		auto& uWorld = engine().universe().world(m_world.m_universeWorld);

		m_contexts[CullingSystem::typeID()]	= engine().system<CullingSystem>().createContext(uWorld.world(CullingSystem::typeID()));
		m_contexts[MeshSystem::typeID()]	= engine().system<MeshSystem>().createContext(uWorld.world(MeshSystem::typeID()));
		m_contexts[LightSystem::typeID()]	= engine().system<LightSystem>().createContext(uWorld.world(LightSystem::typeID()));
		m_contexts[CameraSystem::typeID()]	= engine().system<CameraSystem>().createContext(uWorld.world(CameraSystem::typeID()));
		m_contexts[ShadowSystem::typeID()]	= engine().system<ShadowSystem>().createContext(uWorld.world(ShadowSystem::typeID()));

		bool ok = true;

		//-- init
		for (auto& c : m_contexts)
		{
			
		}

		return ok;
	}


	//------------------------------------------------------------------------------------------------------------------

} // render
} // brUGE

