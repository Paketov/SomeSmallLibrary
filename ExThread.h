#ifndef __EXTHREAD_H_HAS_INCLUDED__
#define __EXTHREAD_H_HAS_INCLUDED__

#include <thread>
#include "ExTypeTraits.h"

enum PRIORITY
{
		IDLE,
		LOWER,
		LOW,
		NORMAL,
		HIGH,
		HIGHER,
		REALTIME,
		NONE
};

bool SetThreadPrior(PRIORITY priority, std::thread& Thread = std::make_default_reference());
PRIORITY GetThreadPrior(std::thread& Thread = std::make_default_reference());
bool SetThreadAffinity(unsigned long long Mask, std::thread& Thread = std::make_default_reference());
bool GetThreadAffinity(unsigned long long* Mask, std::thread& Thread = std::make_default_reference());

#endif