#ifndef __EXTIME_H_HAS_INCLUDED__
#define __EXTIME_H_HAS_INCLUDED__
#include <time.h>
#include <stdio.h>
#include <stdint.h>

/*
	Convert string to tm structure.
*/

inline int strtotm(const char* Str, tm* Result)
{
	tm OutTm;
	char DayOfWeekName[6] = {0}, MonthName[6] = {0};
	int n = -1;
	if(sscanf
			(
				Str, 
				"%4s %4s %2i %2i:%2i:%2i %i%n", 
				DayOfWeekName, 
				MonthName,
				&OutTm.tm_mday,	 
				&OutTm.tm_hour, 
				&OutTm.tm_min, 
				&OutTm.tm_sec, 
				&OutTm.tm_year,
				&n
			) 
	< 7) return -1;
	OutTm.tm_mon = OutTm.tm_wday = -1;
	OutTm.tm_year -= 1900;
	static const char  *Week[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	static const char  *Months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	for(unsigned i = 0; i < (sizeof(Week) / sizeof(Week[0])); i++)
		if(*(uint32_t*)DayOfWeekName == *(uint32_t*)(Week[i]))
		{
			OutTm.tm_wday = i;
			break;
		}
	if(OutTm.tm_wday == -1) return -1;
	for(unsigned i = 0; i < (sizeof(Months) / sizeof(Months[0])); i++)
		if(*(uint32_t*)MonthName == *(uint32_t*)(Months[i]))
		{
			OutTm.tm_mon = i;
			break;
		}
	if(OutTm.tm_mon == -1) return -1;
	OutTm.tm_isdst = -1;
	OutTm.tm_yday = -1;
	*Result = OutTm;
	return n;
}


inline int strtotime(const char* Str, time_t* Result)
{
	tm Tm;
	auto r = strtotm(Str, &Tm);
	if(r == -1) return -1;
	*Result = mktime(&Tm);
	return r;
}


inline unsigned long long time_millisec() { return time(nullptr) * 1000 + clock() % 1000; }

#endif