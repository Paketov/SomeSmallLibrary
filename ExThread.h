#ifndef __EXTHREAD_H_HAS_INCLUDED__
#define __EXTHREAD_H_HAS_INCLUDED__

#include <thread>


#ifdef _MSC_VER
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
		NONE
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
		NONE
};
#endif
#include "ExTypeTraits.h"


inline bool SetThreadPrior(PRIORITY priority, std::thread& Thread = std::make_default_reference())
{
#ifdef _MSC_VER
	return SetThreadPriority((std::is_default_ref(Thread))? GetCurrentThread(): Thread.native_handle(), priority) != FALSE;
#else
	sched_param schedparams;
	schedparams.sched_priority = priority;
	return pthread_setschedparam((std::is_default_ref(Thread))? pthread_self(): Thread.native_handle(), SCHED_OTHER, &schedparams) == 0;
#endif
}

inline PRIORITY GetThreadPrior(std::thread& Thread = std::make_default_reference())
{
#ifdef _MSC_VER
	return (PRIORITY)GetThreadPriority((std::is_default_ref(Thread))? GetCurrentThread(): Thread.native_handle());
#else
	sched_param schedparams;
	pthread_getschedparam((std::is_default_ref(Thread))? pthread_self(): Thread.native_handle(), std::make_default_pointer(), &schedparams);
	return (PRIORITY)schedparams.sched_priority;
#endif
}

inline bool SetThreadAffinity(unsigned long long Mask, std::thread& Thread = std::make_default_reference())
{
#ifdef _MSC_VER
	return SetThreadAffinityMask((std::is_default_ref(Thread))? GetCurrentThread(): Thread.native_handle(), Mask) != 0;
#else
	return pthread_setaffinity_np((std::is_default_ref(Thread))? pthread_self(): Thread.native_handle(), sizeof(Mask), (const cpu_set_t*)&Mask) == 0;
#endif
}

inline bool GetThreadAffinity(unsigned long long* Mask, std::thread& Thread = std::make_default_reference())
{
#ifdef _MSC_VER
	GROUP_AFFINITY ga = {0};
	if(GetThreadGroupAffinity((std::is_default_ref(Thread))? GetCurrentThread(): Thread.native_handle(), &ga) == FALSE)
		return false;
	*Mask = ga.Mask;
	return true;
#else
	return pthread_getaffinity_np((std::is_default_ref(Thread))? pthread_self(): Thread.native_handle(), LenMaskBytes, (const cpu_set_t*)Mask) == 0;
#endif
}

#endif