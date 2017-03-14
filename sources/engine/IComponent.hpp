#pragma once

#include "prerequisites.hpp"
#include <vector>
#include <array>

namespace brUGE
{

	class GameObject;
		
	//-- Base class for all of the possible components in the engine.
	//-- IComponent should provide default (empty) constructor to be able to create IComponent in the default state.
	//------------------------------------------------------------------------------------------------------------------
	class IComponent
	{
	public:

		//--------------------------------------------------------------------------------------------------------------
		enum EFamilyType : int32
		{
			FAMILY_TYPE_UNKNOWN		= 0,
			FAMILY_TYPE_SYSTEM		= 1 << 0,
			FAMILY_TYPE_RENDER		= 1 << 1,
			FAMILY_TYPE_PHYSICS		= 1 << 2,
			FAMILY_TYPE_AUDIO		= 1 << 3,
			FAMILY_TYPE_SCRIPT		= 1 << 4,
			FAMILY_TYPE_MISC		= 1 << 5,
		};

	public:
		IComponent(EFamilyType familyType = FAMILY_TYPE_UNKNOWN, Handle owner = CONST_INVALID_HANDLE)
			: m_familyType(familyType), m_owner(owner) { }

		virtual ~IComponent() = 0 { }

		inline EFamilyType	family() const	{ return m_familyType; }
		inline Handle		owner() const	{ return m_owner; } 

	protected:
		EFamilyType		m_familyType;
		Handle			m_owner;
	};
}
