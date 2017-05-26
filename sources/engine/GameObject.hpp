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
		GameObject() = default;
		~GameObject() = default;

		bool									addComponent(IComponent::Handle handle);
		bool									createComponent(IComponent::TypeID typeID);
		void									removeComponent(IComponent::Handle component);
		const std::vector<IComponent::Handle>&	components() const;

		template<typename ComponentType>
		bool									hasComponent() const;

		template<typename ComponentType>
		IComponent::Handle						getComponent() const;

	private:
		std::unordered_map<IComponent::TypeID, IComponent::Handle>	m_components;
	};

	//------------------------------------------------------------------------------------------------------------------
	template<typename ComponentType>
	bool GameObject::hasComponent() const
	{
		m_components
	}

	//------------------------------------------------------------------------------------------------------------------
	template<typename ComponentType>
	IComponent::Handle GameObject::getComponent() const
	{
	}

}
