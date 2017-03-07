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
			TYPE_RIGED_BODY				= 1 << 3,
			TYPE_COLLISION_BODY			= 1 << 4,
			TYPE_ANIMATION_CONTROLLER	= 1 << 5,
			TYPE_INPUT_CONTROLLER		= 1 << 6,
			TYPE_CAMERA					= 1 << 7,
			TYPE_SCRIPT					= 1 << 8
		};

	public:
		IComponent(EType type = TYPE_UNKNOWN) : m_type(type) { }
		virtual ~IComponent() = 0 { }

		inline EType						type() const  { return m_type; }
		inline std::shared_ptr<GameObject>	owner() const { return m_owner.lock(); } 

	protected:
		EType						m_type;
		std::weak_ptr<GameObject>	m_owner;
	};
}
