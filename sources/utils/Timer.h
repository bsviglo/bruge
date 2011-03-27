#pragma once

#include "prerequisites.hpp"
#include "utils/Singleton.h"

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <windows.h>

//
// Timer
//
namespace brUGE
{
namespace utils
{

	//----------------------------------------------------------------------------------------------
	class Timer : public Singleton<Timer>
	{		
	public:
		Timer();				
		~Timer();

		// time elapsed after start of the timer in miliseconds.
		uint64 time();	
		
		// time elapsed after start of the timer in microseconds.
		uint64 timeInUS();

		// begin timer.
		void start();	
		
		// Temporarily stop timer.
		// To again wake up timer, call start function.
		// Note: If function restart will be call, regardless that the timer was paused, it restarts.
		//	     e.g. it's state will be reseted.		
		void pause();
		
		// Make full reload of the timer.
		// It's like start function once at the start of the application.
		void restart();

		// return number of ticks, after start.
		uint64 tickCount();	

		// convert ticks to milliseconds.
		double ticksToSeconds(uint64 ticks);

	private:
		uint64 _tickCount();

	private:
		LARGE_INTEGER m_startTime;
		LARGE_INTEGER m_freq;
		LARGE_INTEGER m_pauseTime;	
		bool		  m_isPaused;
	};

} // utils
} // brUGE
