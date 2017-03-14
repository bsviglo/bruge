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
	//------------------------------------------------------------------------------------------------------------------
	class GameObject
	{
	public:
		GameObject();
		~GameObject();

		bool						addChild(Handle gameObj);
		bool						delChild(Handle gameObj);
		const std::vector<Handle>&	children() const;
		Handle						parent() const;

		bool						createComponent(int32 mask);
		bool						removeComponent(Handle component);
		const std::vector<Handle>&	components() const;

		Handle						findComponentByFamilyType(IComponent::EFamilyType type) const;
		bool						hasComponentByFamilyType(IComponent::EFamilyType type) const;

	private:
		uint32				m_componentsFamilyMask;
		Handle				m_parent;
		std::vector<Handle>	m_components;
		std::vector<Handle>	m_childs;
	};
}
