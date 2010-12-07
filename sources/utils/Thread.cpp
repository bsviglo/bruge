#include "Thread.h"
#include <process.h>

typedef unsigned (__stdcall *PTHREAD_START) (void *);

#define chBEGINTHREADEX(psa, cbStack, pfnStartAddr, \
	pvParam, fdwCreate, pdwThreadID)                 \
	((HANDLE) _beginthreadex(                     \
	(void *) (psa),                            \
	(unsigned) (cbStack),                      \
	(PTHREAD_START) (pfnStartAddr),            \
	(void *) (pvParam),                        \
	(unsigned) (fdwCreate),                    \
	(unsigned *) (pdwThreadID))) 


namespace brUGE
{
namespace utils
{

	//------------------------------------------
	Thread::Thread()
	{

	}

	//------------------------------------------
	Thread::~Thread()
	{
		CloseHandle(m_hThread);
	}

	//------------------------------------------
	bool Thread::createThread(LPTHREAD_START_ROUTINE func)
	{

		// TODO: Разобраться почему так важно не использовать функцию CreateThread
		// а вместо нее необходимо исползовать _beginthreadex если использовать C++

		//hThread = CreateThread(NULL, 0, _func, 0, 0, &threadId);
		m_hThread = chBEGINTHREADEX(NULL, 0, func, 0, 0, &m_threadId);
		if (m_hThread)
		{
			return true;
		}
		return false;
	}

	//------------------------------------------
	bool Thread::setPriority(int priority/* = THREAD_PRIORITY_NORMAL*/)
	{
		return SetThreadPriority(m_hThread, priority) != 0;
	}

	//------------------------------------------
	bool Thread::terminateThread()
	{
		return TerminateThread(m_hThread, 0) != 0; 
	}

} // utils
} // brUGE