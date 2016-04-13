#pragma once

#include <time.h>
#include <string>
/*
	Convert string to tm structure.
*/

int StrToTm(const char* Str, tm* Result);
int StrToTime(const char* Str, time_t* Result);
std::basic_string<char> TimeToString(time_t t);
std::basic_string<char> TimeSubToString(time_t t1, time_t t2);
std::basic_string<char> TimeMilisecSubToString(long long t1, long long t2);
