#include "render_system.hpp"

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
		m_systems[MeshSystem::typeID()] = std::make_unique<MeshSystem>();
	}

	//------------------------------------------------------------------------------------------------------------------
	bool RenderSystem::init()
	{
		bool ok = true;

		for (auto& system : m_systems)
		{
			ok &= system.second->init();
		}

		return ok;
	}

	//------------------------------------------------------------------------------------------------------------------
	void RenderSystem::update(IWorld* world, const DeltaTime& dt)
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	void RenderSystem::process(IContext* context)
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	bool RenderSystem::checkRequiredComponents(Handle handle) const
	{
		GameObject* gameObj = nullptr;

		gameObj->hasComponentByType(IRE)
	}

	//------------------------------------------------------------------------------------------------------------------
	Handle RenderSystem::World::createComponent(Handle gameObj, const pugi::xml_node& cfg)
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	Handle RenderSystem::World::createComponent(Handle gameObj)
	{
		auto meshWorld = static_cast<MeshSystem::World>(m_worlds[SYSTEM_MESH]);

		auto meshComponent = meshWorld->createComponent(gameObj);


	}

	//------------------------------------------------------------------------------------------------------------------




} // render
} // brUGE

