#include "Timer.h"
#include <time.h>

namespace brUGE
{
	
DEFINE_SINGLETON(utils::Timer)	

namespace utils
{
	
	// 
	//------------------------------------------
	Timer::Timer() : m_isPaused(false)
	{
		QueryPerformanceFrequency( &m_freq );	 
		m_startTime.QuadPart = 0;
	}

	// 
	//------------------------------------------
	Timer::~Timer()
	{

	}

	// 
	//------------------------------------------
	void Timer::start()
	{
		if (m_isPaused)
		{
			m_startTime.QuadPart += tickCount() - m_pauseTime.QuadPart;
			m_isPaused = false;
		}
		else
		{
			m_startTime.QuadPart = tickCount();
		}
	}

	// 
	//------------------------------------------
	void Timer::restart()
	{
		m_isPaused = false;
		m_startTime.QuadPart = _tickCount();
	}
	
	// 
	//------------------------------------------
	uint64 Timer::tickCount()
	{
		if (m_isPaused)
		{
			return m_pauseTime.QuadPart;
		}
		else
		{
			return _tickCount();
		}
	}

	// 
	//------------------------------------------
	uint64 Timer::time()
	{
		return static_cast<uint64>((tickCount() - m_startTime.QuadPart) * 1000 / m_freq.QuadPart);
	}

	//
	//------------------------------------------
	uint64 Timer::timeInUS()
	{
		return static_cast<uint64>((tickCount() - m_startTime.QuadPart) * 1000000 / m_freq.QuadPart);
	}

	// 
	//------------------------------------------
	double Timer::ticksToSeconds(uint64 ticks)
	{
		return static_cast<double>(ticks / m_freq.QuadPart);
	}

	// 
	//------------------------------------------
	void Timer::pause()
	{
		m_isPaused = true;
		m_pauseTime.QuadPart = tickCount();
	}

	// 
	//------------------------------------------
	uint64 Timer::_tickCount()
	{
		LARGE_INTEGER currentTime;
		QueryPerformanceCounter(&currentTime);
		return currentTime.QuadPart;
	}

} // utils
} // brUGE
