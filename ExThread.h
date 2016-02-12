#ifndef __EXTHREAD_H_HAS_INCLUDED__
#define __EXTHREAD_H_HAS_INCLUDED__
#include <thread>
#ifdef _WIN32
#include <Windows.h>

enum PRIORITY
{
		IDLE = THREAD_PRIORITY_IDLE,
		LOWER = THREAD_PRIORITY_LOWEST,
		LOW = THREAD_PRIORITY_BELOW_NORMAL,
		NORMAL = THREAD_PRIORITY_NORMAL,
		HIGH = THREAD_PRIORITY_ABOVE_NORMAL,
		HIGHER = THREAD_PRIORITY_HIGHEST,
		REALTIME = THREAD_PRIORITY_TIME_CRITICAL,
};

#else

#include <unistd.h> 
#include <pthread.h> 
#include <semaphore.h> 
#include <sys/time.h> 


enum PRIORITY
{
		IDLE = 45,
		LOWER = 51,
		LOW = 57,
		NORMAL = 63,
		HIGH = 69,
		HIGHER = 75,
		REALTIME = 81,
};
#endif
#include "ExTypeTraits.h"


inline void SetThreadPrior(std::thread& Thread, PRIORITY priority)
{
#ifdef _WIN32
	SetThreadPriority(Thread.native_handle(), priority);
#else
	sched_param schedparams;
	schedparams.sched_priority = priority;
	pthread_setschedparam(Thread.native_handle(), SCHED_OTHER, &schedparams);
#endif
}

inline void SetThreadPrior(PRIORITY priority)
{
#ifdef _WIN32

	SetThreadPriority(GetCurrentThread(), priority);
#else
	sched_param schedparams;
	schedparams.sched_priority = priority;
	pthread_setschedparam(pthread_self(), SCHED_OTHER, &schedparams);
#endif
}


inline PRIORITY GetThreadPrior(std::thread& Thread)
{
#ifdef _WIN32
	return (PRIORITY)GetThreadPriority(Thread.native_handle());
#else
	sched_param schedparams;
	pthread_getschedparam(Thread.native_handle(), std::make_default_pointer(), &schedparams);
	return (PRIORITY)schedparams.sched_priority;
#endif
}

inline PRIORITY GetThreadPrior()
{
#ifdef _WIN32
	return (PRIORITY)GetThreadPriority(GetCurrentThread());
#else
	sched_param schedparams;
	pthread_getschedparam(pthread_self(), std::make_default_pointer(), &schedparams);
	return (PRIORITY)schedparams.sched_priority;
#endif
}

#endif