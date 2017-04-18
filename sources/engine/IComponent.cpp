#include "IComponent.hpp"
#include <atomic>
#include <cassert>

namespace brUGE
{
	//------------------------------------------------------------------------------------------------------------------
	IComponent::TypeID::TypeID()
	{
		static std::atomic<uint32> next = 0;
		m_id = next.fetch_add(1, std::memory_order_relaxed);
		assert(m_id < C_MAX_COMPONENT_TYPES);
	}

	//------------------------------------------------------------------------------------------------------------------
	const IComponent::TypeID IComponent::TypeID::C_INVALID(0);
}