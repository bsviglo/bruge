#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace brUGE
{
namespace utils
{
	//
	// ToDo: Yet this wrapper is very simple and ugly class.
	//------------------------------------------------------------
	class Thread
	{
	public:
		Thread();
		~Thread();

		// create thread.
		// func - main function of this thread.
		bool createThread(LPTHREAD_START_ROUTINE func);

		// assign priority to thread.
		bool setPriority(int priority = THREAD_PRIORITY_NORMAL);

		// terminate thread.
		// Note: by defaults it terminates in normal state.
		bool terminateThread();

	private:
		HANDLE m_hThread;
		DWORD m_threadId;
	};
}
}
