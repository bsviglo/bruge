#pragma once

#include "prerequisites.hpp"
#include <vector>
#include <array>

namespace brUGE
{

	class GameObject;
		
	//-- Base class for all of the possible components in the engine
	//------------------------------------------------------------------------------------------------------------------
	class IComponent
	{
	public:

		//--------------------------------------------------------------------------------------------------------------
		enum EFamilyType : int32
		{
			FAMILY_TYPE_UNKNOWN,
			FAMILY_TYPE_RENDER,
			FAMILY_TYPE_PHYSICS,
			FAMILY_TYPE_AUDIO,
			FAMILY_TYPE_SCRIPT
		};

		//--------------------------------------------------------------------------------------------------------------
		enum EType : int32
		{
			TYPE_UNKNOWN				= 0,
			TYPE_TRANSFORM				= 1 << 0,
			TYPE_RIGID_BODY				= 1 << 1,
			TYPE_COLLISION_BODY			= 1 << 2,
			TYPE_ANIMATION_CONTROLLER	= 1 << 3,
			TYPE_INPUT_CONTROLLER		= 1 << 4,
			TYPE_CAMERA					= 1 << 5,
			TYPE_SCRIPT					= 1 << 6,
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
