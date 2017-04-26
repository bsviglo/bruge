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
		const std::vector<Handle>&				children() const	{ return m_childs; }

		bool									changeParent(Handle newParent);
		Handle									parent() const		{ return m_parent; }

		bool									addComponent(IComponent::Handle handle);
		bool									createComponent(IComponent::TypeID typeID);
		void									removeComponent(IComponent::Handle component);
		const std::vector<IComponent::Handle>&	components() const;

		template<typename ComponentType>
		bool									hasComponent() const;

		template<typename ComponentType>
		IComponent::Handle						getComponent() const;

	private:
		Handle														m_parent;
		std::unordered_map<IComponent::TypeID, IComponent::Handle>	m_components;
		std::vector<Handle>											m_childs;
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
