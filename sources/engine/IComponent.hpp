#pragma once

#include "prerequisites.hpp"
#include <vector>
#include <array>

namespace brUGE
{

	class GameObject;
		
	//-- Base class for all of the possible components in the engine
	//----------------------------------------------------------------------------------------------
	class IComponent
	{
	public:
		enum EType
		{
			TYPE_UNKNOWN				= 0,
			TYPE_TRANSFORM				= 1 << 0,
			TYPE_SKINNED_MESH			= 1 << 1,
			TYPE_STATIC_MESH			= 1 << 2,
			TYPE_RIGID_BODY				= 1 << 3,
			TYPE_COLLISION_BODY			= 1 << 4,
			TYPE_ANIMATION_CONTROLLER	= 1 << 5,
			TYPE_INPUT_CONTROLLER		= 1 << 6,
			TYPE_CAMERA					= 1 << 7,
			TYPE_SCRIPT					= 1 << 8,
			TYPE_LIGHT					= 1 << 9,
		};

	public:
		IComponent(EType type = TYPE_UNKNOWN, Handle owner = CONST_INVALID_HANDLE) : m_type(type), m_owner(owner) { }
		virtual ~IComponent() = 0 { }

		inline EType	type() const  { return m_type; }
		inline Handle	owner() const { return m_owner; } 

	protected:
		EType	m_type;
		Handle	m_owner;
	};
}
