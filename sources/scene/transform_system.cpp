#include "transform_system.hpp"

#include "rttr/registration"

namespace brUGE
{
	//------------------------------------------------------------------------------------------------------------------
	RTTR_REGISTRATION
	{
		rttr::registration::class_<TransformComponent>("TransformComponent")
			.constructor<>()
			.property_readonly("typeID", TransformComponent::typeID)
			.property_readonly("systemTypeID", TransformSystem::typeID);
			
		rttr::registration::class_<TransformSystem>("TransformSystem")
			.constructor<>()
			.property_readonly("typeID", TransformSystem::typeID);
	}

	//------------------------------------------------------------------------------------------------------------------
	const IComponent::TypeID	TransformComponent::m_typeID;
	const ISystem::TypeID		TransformSystem::m_typeID;


	//------------------------------------------------------------------------------------------------------------------
	TransformSystem::TransformSystem()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	TransformSystem::~TransformSystem()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	bool TransformSystem::init(const pugi::xml_node& cfg /* = pugi::xml_node() */)
	{

		return true;
	}

	//------------------------------------------------------------------------------------------------------------------
	TransformSystem::World::World()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	TransformSystem::World::~World()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	bool TransformSystem::World::init(const pugi::xml_node& cfg)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------------------------
	IComponent::Handle TransformSystem::World::createComponent(Handle gameObj, IComponent::TypeID typeID)
	{
		return createComponent(gameObj, typeID, pugi::xml_node());
	}

	//------------------------------------------------------------------------------------------------------------------
	IComponent::Handle TransformSystem::World::createComponent(Handle gameObj, IComponent::TypeID typeID, const pugi::xml_node& cfg)
	{
		auto t = std::make_unique<Transform>();

		if (auto matrixCfg = cfg.find_child("matrix"))
		{
			//-- ToDo: convert cfg -> matrix
			t->m_worldMat = mat4f();
		}
		else
		{
			t->m_worldMat.setIdentity();
		}

		t->m_nodes.emplace_back(std::make_unique<Node>("root", t->m_worldMat));

		auto tComp = std::make_unique<TransformComponent>(gameObj, *t.get());

		//--
		m_components.emplace_back(std::move(tComp));
		m_transforms.emplace_back(std::move(t));

		return IComponent::Handle(TransformComponent::typeID(), TransformSystem::typeID(), m_components.size() - 1);
	}

	//------------------------------------------------------------------------------------------------------------------
	IComponent::Handle TransformSystem::World::cloneComponent(Handle srcGameObj, Handle dstGameObj, IComponent::TypeID typeID)
	{
		//-- ToDo: implement
	}

	//------------------------------------------------------------------------------------------------------------------
	void TransformSystem::World::removeComponent(IComponent::Handle component)
	{
		assert(component.systemTypeID() == TransformSystem::typeID());
		assert(component.handle() != CONST_INVALID_HANDLE);

		m_components[component.handle()].reset();
		m_transforms[component.handle()].reset();
	}

	//------------------------------------------------------------------------------------------------------------------
	TransformSystem::Context::Context()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	TransformSystem::Context::~Context()
	{

	}

	//------------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------------
}