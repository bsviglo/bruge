#pragma once

#include "prerequisites.hpp"
#include "IComponent.hpp"
#include <vector>
#include <array>

namespace brUGE
{
	class IComponent;

	//-- High level item of the game world. Holds various aspects about the game via components.
	//-- This class also defines hierarchy of entities.
	//----------------------------------------------------------------------------------------------
	class GameObject
	{
	public:
		GameObject();
		~GameObject();

		bool		addChild(const std::shared_ptr<GameObject>& entity);
		bool		delChild(const std::shared_ptr<GameObject>& entity);

		bool		addComponent(IComponent::EType type);
		bool		addComponent(const std::unique_ptr<IComponent>& component);
		bool		delComponent(IComponent::EType type);
		IComponent*	findComponentByType(IComponent::EType type);
		bool		hasComponentByType(IComponent::EType type);

	private:
		uint32										m_componentsMask;
		GameObject*									m_parent;
		std::vector<std::unique_ptr<IComponent>>	m_components;
		std::vector<std::shared_ptr<GameObject>>	m_childs;
	};
}
