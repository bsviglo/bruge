#pragma once

#include "prerequisites.hpp"
#include "IComponent.hpp"

#include <vector>
#include <unordered_map>

namespace brUGE
{
	class IComponent;

	//-- High level item of the game world. Holds various aspects about the game via components.
	//-- This class also defines hierarchy of entities.
	//------------------------------------------------------------------------------------------------------------------
	class GameObject final
	{
	public:
		GameObject() { }
		~GameObject() { }

		bool									addChild(Handle gameObj);
		bool									delChild(Handle gameObj);
		const std::vector<Handle>&				children() const;
		Handle									parent() const;

		bool									addComponent(IComponent::Handle handle);
		bool									createComponent(IComponent::TypeID typeID);
		bool									removeComponent(Handle component);
		const std::vector<IComponent::Handle>&	components() const;

		template<typename ComponentType>
		bool									hasComponent() const { return false; }

		template<typename ComponentType>
		IComponent::Handle						getComponent() const { return m_components[ComponentType::typeID()]; }

	private:
		Handle														m_parent;
		std::unordered_map<IComponent::TypeID, IComponent::Handle>	m_components;
		std::vector<Handle>											m_childs;
	};

	//------------------------------------------------------------------------------------------------------------------
	bool GameObject::addComponent(IComponent::Handle handle)
	{
		m_components[handle.typeID()] = handle;
		return true;
	}
}
