#ifndef __EXTIME_H_HAS_INCLUDED__
#define __EXTIME_H_HAS_INCLUDED__
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <string>
/*
	Convert string to tm structure.
*/

inline int StrToTm(const char* Str, tm* Result)
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


inline int StrToTime(const char* Str, time_t* Result)
{
	tm Tm;
	auto r = StrToTm(Str, &Tm);
	if(r == -1) return -1;
	*Result = mktime(&Tm);
	return r;
}


inline std::basic_string<char> TimeSubToString(time_t t1, time_t t2)
{
	time_t r = t2 - t1;
	int Years = r / (365 * 24 * 60 * 60);
	r %= (365 * 24 * 60 * 60);
	int Days = r / (24 * 60 * 60);
	r %= (24 * 60 * 60);
	int Hours = r / (60 * 60);
	r %= (60 * 60);
	int Minutes = r / 60;
	r %= 60;
	int Seconds = r;

	bool k = false;
	std::basic_string<char> str_r;
	if(Years != 0) str_r = std::to_string(Years) + " years ", k = true;
	if((Days != 0) || k) str_r += std::to_string(Days) + " days ", k = true;
	char Buf[30];
	sprintf_s(Buf, "%02i:%02i:%02i", Hours, Minutes, Seconds);
	return str_r + Buf;
}

inline std::basic_string<char> TimeSubMilisecToString(long long t1, long long t2)
{
	auto& r = TimeSubToString(t1 / 1000, t2 / 1000);
	char Buf[10];
	sprintf_s(Buf, ":%03i", (int)((t2 - t1) % 1000));
	return r + Buf;
}

#endif