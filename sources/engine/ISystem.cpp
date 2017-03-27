#include "ISystem.hpp"
#include <atomic>
#include <cassert>

namespace brUGE
{
	//------------------------------------------------------------------------------------------------------------------
	ISystem::TypeID::TypeID()
	{
		static std::atomic<uint32> next = 1;
		m_id = next.fetch_add(1, std::memory_order_relaxed);
		assert(m_id < C_MAX_SYSTEM_TYPES);
	}

	//------------------------------------------------------------------------------------------------------------------
	const ISystem::TypeID ISystem::TypeID::C_INVALID(0);
}