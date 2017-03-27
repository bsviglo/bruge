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
	class GameObject
	{
	public:
		GameObject();
		~GameObject();

		bool									addChild(Handle gameObj);
		bool									delChild(Handle gameObj);
		const std::vector<Handle>&				children() const;
		Handle									parent() const;

		bool									createComponent(IComponent::TypeID typeID);
		bool									removeComponent(Handle component);
		const std::vector<IComponent::Handle>&	components() const;

		template<typename ComponentType>
		bool									hasComponent() const { return true; }

		template<typename ComponentType>
		IComponent::Handle						getComponent() const { return IComponent::Handle(); }

	private:
		Handle														m_parent;
		std::unordered_map<IComponent::TypeID, IComponent::Handle>	m_components;
		std::vector<Handle>											m_childs;
	};
}
