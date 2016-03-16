#ifndef __EX_STRING_H_HAS_INCLUDED__
#define __EX_STRING_H_HAS_INCLUDED__

/*
     ExString
	 Paketov
	 2015.

	 Library for optimize work with C-like string in C++.
	 Convert from string to number and number to string 
	 have optimize on account of small number calls.
*/



#include <string>
#include <sstream>
#include <iostream>
#include <string.h>
#if _POSIX_VERSION >= 200112L //If posix standart >= POSIX.1-2001
#include <strings.h>
#endif
#include <stdio.h>
#include <wchar.h>
#include "ExTypeTraits.h"

#ifndef WIN32
#		define CP_ACP                    4       // default to ANSI code page
#       define CP_UTF7                   3       // UTF-7 translation
#       define CP_UTF8                   0       // UTF-8 translation
#       define CP_1251                   1       
#       define CP_KOI8                   2
#else
#include <Windows.h>
#include <Windowsx.h>
#endif

#define OFFSET_P(Pointer1, Pointer2) ((off_t)((size_t)(Pointer2) - (size_t)(Pointer1)))
#define STR_TYPE(_Type, _Str)		((std::is_equal<_Type, wchar_t>::value)?(_Type*)(L ## _Str):(_Type*)(_Str))
#define CHAR_TYPE(_Type, _Char)		((_Type)((std::is_equal<_Type, char>::value)?(_Char):(L ## _Char)))
#define NOT_LESS_Z(Num)				((0 > std::make_signed<decltype(Num)>::type(Num))?0:Num)
#define DIGIT_TO_ALPHA(TypeChar, Dig) ((Dig > 9)? (Dig + (CHAR_TYPE(TypeChar,'a') - 10)): (Dig + CHAR_TYPE(TypeChar,'0')))
#define CMP_I(Val, c)				(((Val) == CHAR_TYPE(decltype(Val), c)) || ((Val) == (CHAR_TYPE(decltype(Val), 'A') + (CHAR_TYPE(decltype(Val), c) - CHAR_TYPE(decltype(Val), 'a')))))



void StringConvertCodePage(unsigned InCp/*CP_.*/, unsigned OutCp, const wchar_t* InStr, std::basic_string<char>& OutStr);
void StringConvertCodePage(unsigned InCp, unsigned OutCp, const wchar_t* InStr, std::basic_string<wchar_t>& OutStr);
void StringConvertCodePage(unsigned InCp, unsigned OutCp, const char* InStr, std::basic_string<char>& OutStr);
void StringConvertCodePage(unsigned InCp, unsigned OutCp, const char* InStr, std::basic_string<wchar_t>& OutStr);



int NumberToString(int Number, char* Str, size_t Len, unsigned char Radix = 10);
int NumberToString(int Number, wchar_t* Str, size_t Len, unsigned char Radix = 10);
int NumberToString(unsigned Number, char* Str, size_t Len, unsigned char Radix = 10);
int NumberToString(unsigned Number, wchar_t* Str, size_t Len, unsigned char Radix = 10);
int NumberToString(long Number, char* Str, size_t Len, unsigned char Radix = 10);
int NumberToString(long Number, wchar_t* Str, size_t Len, unsigned char Radix = 10);
int NumberToString(unsigned long Number, char* Str, size_t Len, unsigned char Radix = 10);
int NumberToString(unsigned long Number, wchar_t* Str, size_t Len, unsigned char Radix = 10);
int NumberToString(long long Number, char* Str, size_t Len, unsigned char Radix = 10);
int NumberToString(long long Number, wchar_t* Str, size_t Len, unsigned char Radix = 10);
int NumberToString(unsigned long long Number, char* Str, size_t Len, unsigned char Radix = 10);
int NumberToString(unsigned long long Number, wchar_t* Str, size_t Len, unsigned char Radix = 10);
int NumberToString(float Val, char* Str, size_t LenBuf, unsigned char RadX, long double Eps);
int NumberToString(float Val, wchar_t* Str, size_t LenBuf, unsigned char RadX, long double Eps);
int NumberToString(float Val, char* Str, size_t LenBuf, unsigned char RadX = 10);
int NumberToString(float Val, wchar_t* Str, size_t LenBuf, unsigned char RadX = 10);
int NumberToString(double Val, char* Str, size_t LenBuf, unsigned char RadX, long double Eps);
int NumberToString(double Val, wchar_t* Str, size_t LenBuf, unsigned char RadX, long double Eps);
int NumberToString(double Val, char* Str, size_t LenBuf, unsigned char RadX = 10);
int NumberToString(double Val, wchar_t* Str, size_t LenBuf, unsigned char RadX = 10);
int NumberToString(long double Val, char* Str, size_t LenBuf, unsigned char RadX, long double Eps);
int NumberToString(long double Val, wchar_t* Str, size_t LenBuf, unsigned char RadX, long double Eps);
int NumberToString(long double Val, char* Str, size_t LenBuf, unsigned char RadX = 10);
int NumberToString(long double Val, wchar_t* Str, size_t LenBuf, unsigned char RadX = 10);

template<typename TypeChar, typename TypeNumber>
int NumberToString(TypeNumber Val, std::basic_string<TypeChar> & Str, size_t LenBuf = 0, unsigned char RadX = 10)
{
   TypeChar Buf[70];
   auto Ret = NumberToString(Val, Buf, 70, RadX);
   Str = Buf;
   return Ret;
}

template<typename TypeChar, typename TypeNumber>
int NumberToString(TypeNumber Val, std::basic_string<TypeChar> & Str, size_t LenBuf, unsigned char RadX, long double Eps)
{
   TypeChar Buf[70];
   auto Ret = NumberToString(Val, Buf, 70, RadX, Eps);
   Str = Buf;
   return Ret;
}

template<typename TypeChar, typename TypeNumber, size_t BufSize>
int NumberToString(TypeNumber Val, TypeChar (&Buf)[BufSize]) { return NumberToString(Val, (TypeChar*)Buf, BufSize, 10); }


//From string to number

int StringToNumber(int* Number, const char* Str, size_t Len, unsigned char Radix = 10);
int StringToNumber(int* Number, const wchar_t* Str, size_t Len, unsigned char Radix = 10);
int StringToNumber(unsigned int* Number, const char* Str, size_t Len, unsigned char Radix = 10);
int StringToNumber(unsigned int* Number, const wchar_t* Str, size_t Len, unsigned char Radix = 10);
int StringToNumber(long long* Number, const char* Str, size_t Len, unsigned char Radix = 10);
int StringToNumber(long long* Number, const wchar_t* Str, size_t Len, unsigned char Radix = 10);
int StringToNumber(unsigned long long* Number, const char* Str, size_t Len, unsigned char Radix = 10);
int StringToNumber(unsigned long long* Number, const wchar_t* Str, size_t Len, unsigned char Radix = 10);
int StringToNumber(float* Number, const char* Str, size_t Len, unsigned char Radix = 10);
int StringToNumber(float* Number, const wchar_t* Str, size_t Len, unsigned char Radix = 10);
int StringToNumber(long double* Number, const char* Str, size_t Len, unsigned char Radix = 10);
int StringToNumber(long double* Number, const wchar_t* Str, size_t Len, unsigned char Radix = 10);

template<typename TypeChar, typename TypeNumber, size_t BufSize>
int StringToNumber(TypeNumber * Number, const TypeChar (&Buf)[BufSize]) { return StringToNumber(Number, Buf, BufSize, 10); }

template<typename TypeChar, typename TypeNumber>
int StringToNumber(TypeNumber * Number, const std::basic_string<TypeChar> & Str, size_t Len = 0, unsigned char Radix = 10) 
{ 
	return StringToNumber(Number, Str.c_str(), Str.length(), Radix); 
}

int StreamToNumber(int* Dest, std::basic_istream<char> & Stream, unsigned char Radix = 10);
int StreamToNumber(int* Dest, std::basic_istream<wchar_t> & Stream, unsigned char Radix = 10);
int StreamToNumber(unsigned int* Dest, std::basic_istream<char> & Stream, unsigned char Radix = 10);
int StreamToNumber(unsigned int* Dest, std::basic_istream<wchar_t> & Stream, unsigned char Radix = 10);
int StreamToNumber(long long* Dest, std::basic_istream<char> & Stream, unsigned char Radix = 10);
int StreamToNumber(long long* Dest, std::basic_istream<wchar_t> & Stream, unsigned char Radix = 10);
int StreamToNumber(unsigned long long* Dest, std::basic_istream<char> & Stream, unsigned char Radix = 10);
int StreamToNumber(unsigned long long* Dest, std::basic_istream<wchar_t> & Stream, unsigned char Radix = 10);
int StreamToNumber(float* Dest, std::basic_istream<char> & Stream, unsigned char Radix = 10);
int StreamToNumber(float* Dest, std::basic_istream<wchar_t> & Stream, unsigned char Radix = 10);
int StreamToNumber(long double* Dest, std::basic_istream<char> & Stream, unsigned char Radix = 10);
int StreamToNumber(long double* Dest, std::basic_istream<wchar_t> & Stream, unsigned char Radix = 10);

int StreamToNumber(int* Dest, FILE* Stream = stdin, unsigned char Radix = 10);
int StreamToNumber(unsigned int* Dest, FILE* Stream = stdin, unsigned char Radix = 10);
int StreamToNumber(long long* Dest, FILE* Stream = stdin, unsigned char Radix = 10);
int StreamToNumber(unsigned long long* Dest, FILE* Stream = stdin, unsigned char Radix = 10);
int StreamToNumber(float* Dest, FILE* Stream = stdin, unsigned char Radix = 10);
int StreamToNumber(long double* Dest, FILE* Stream = stdin, unsigned char Radix = 10);


int StreamDoubleToNumber(int* Dest, FILE* Stream = stdin, unsigned char Radix = 10);
int StreamDoubleToNumber(unsigned int* Dest, FILE* Stream = stdin, unsigned char Radix = 10);
int StreamDoubleToNumber(long long* Dest, FILE* Stream = stdin, unsigned char Radix = 10);
int StreamDoubleToNumber(unsigned long long* Dest, FILE* Stream = stdin, unsigned char Radix = 10);
int StreamDoubleToNumber(float* Dest, FILE* Stream = stdin, unsigned char Radix = 10);
int StreamDoubleToNumber(long double* Dest, FILE* Stream = stdin, unsigned char Radix = 10);



int StreamDoubleToNumber(int* Dest, std::basic_istream<char>& Stream, unsigned char Radix = 10);
int StreamDoubleToNumber(int* Dest, std::basic_istream<wchar_t>& Stream, unsigned char Radix = 10);
int StreamDoubleToNumber(unsigned int* Dest, std::basic_istream<char>& Stream, unsigned char Radix = 10);
int StreamDoubleToNumber(unsigned int* Dest, std::basic_istream<wchar_t>& Stream, unsigned char Radix = 10);
int StreamDoubleToNumber(long long* Dest, std::basic_istream<char>& Stream, unsigned char Radix = 10);
int StreamDoubleToNumber(long long* Dest, std::basic_istream<wchar_t>& Stream, unsigned char Radix = 10);
int StreamDoubleToNumber(unsigned long long* Dest, std::basic_istream<char>& Stream, unsigned char Radix = 10);
int StreamDoubleToNumber(unsigned long long* Dest, std::basic_istream<wchar_t>& Stream, unsigned char Radix = 10);
int StreamDoubleToNumber(float* Dest, std::basic_istream<char>& Stream, unsigned char Radix = 10);
int StreamDoubleToNumber(float* Dest, std::basic_istream<wchar_t>& Stream, unsigned char Radix = 10);
int StreamDoubleToNumber(long double* Dest, std::basic_istream<char>& Stream, unsigned char Radix = 10);
int StreamDoubleToNumber(long double* Dest, std::basic_istream<wchar_t>& Stream, unsigned char Radix = 10);


int StringDoubleToNumber(int* Number, const char* Str, size_t Len, unsigned char Radix = 10);
int StringDoubleToNumber(int* Number, const wchar_t* Str, size_t Len, unsigned char Radix = 10);
int StringDoubleToNumber(unsigned int* Number, const char* Str, size_t Len, unsigned char Radix = 10);
int StringDoubleToNumber(unsigned int* Number, const wchar_t* Str, size_t Len, unsigned char Radix = 10);
int StringDoubleToNumber(long long* Number, const char* Str, size_t Len, unsigned char Radix = 10);
int StringDoubleToNumber(long long* Number, const wchar_t* Str, size_t Len, unsigned char Radix = 10);
int StringDoubleToNumber(unsigned long long* Number, const char* Str, size_t Len, unsigned char Radix = 10);
int StringDoubleToNumber(unsigned long long* Number, const wchar_t* Str, size_t Len, unsigned char Radix = 10);
int StringDoubleToNumber(float* Number, const char* Str, size_t Len, unsigned char Radix = 10);
int StringDoubleToNumber(float* Number, const wchar_t* Str, size_t Len, unsigned char Radix = 10);
int StringDoubleToNumber(long double* Number, const char* Str, size_t Len, unsigned char Radix = 10);
int StringDoubleToNumber(long double* Number, const wchar_t* Str, size_t Len, unsigned char Radix = 10);

template<typename TypeChar, typename TypeNumber, size_t BufSize>
int StringDoubleToNumber(TypeNumber * Number, const TypeChar (&Buf)[BufSize]) { return StringDoubleToNumber(Number, Buf, BufSize, 10); }

template<typename TypeChar, typename TypeNumber>
int StringDoubleToNumber(TypeNumber * Number, const std::basic_string<TypeChar> & Str, size_t Len = 0, unsigned char Radix = 10)
{
     return StringDoubleToNumber(Number, Str.c_str(), Str.length(), Radix);
}
struct __stream_io__base
{
	static inline bool Write(FILE* s,  char* Str, size_t Len) { return fputs(Str, s) != EOF;}
	static inline bool Write(FILE* s,  wchar_t* Str, size_t Len) { return fputws(Str, s) != WEOF; }		
	template<typename TypeChar>
	static inline bool Write(std::basic_ostream<TypeChar>& s, TypeChar* Str, size_t Len) { return !s.write(Str, Len).fail(); }	
};


template<typename TypeNumber, typename TypeChar>
int NumberToStream(TypeNumber Number, std::basic_ostream<TypeChar> & Stream, unsigned char Radix = 10)
{
	TypeChar Buf[70];
	size_t CountWrited = NumberToString(Number, Buf, 70, Radix);
	if(CountWrited < 0) return 0;
	return __stream_io__base::Write(Stream, Buf, CountWrited) ? CountWrited: -CountWrited;
}

template<typename TypeNumber>
int NumberToStream(TypeNumber Number, FILE * Stream = stdout, unsigned char Radix = 10)
{
	char Buf[70];
	size_t CountWrited = NumberToString(Number, Buf, 70, Radix);
	if(CountWrited < 0) return 0;
	return __stream_io__base::Write(Stream, Buf, CountWrited) ? CountWrited: -CountWrited;
}

template<typename TypeNumber, typename TypeChar>
int NumberToStream(TypeNumber Number, std::basic_ostream<TypeChar> & Stream, unsigned char Radix, long double Eps)
{
	TypeChar Buf[70];
	size_t CountWrited = NumberToString((long double)Number, Buf, 70, Radix, Eps);
	if(CountWrited < 0) return 0;
	return __stream_io__base::Write(Stream, Buf, CountWrited) ? CountWrited: -CountWrited;
}

template<typename TypeNumber>
int NumberToStream(TypeNumber Number, FILE * Stream, unsigned char Radix, long double Eps)
{
	char Buf[70];
	size_t CountWrited = NumberToString((long double)Number, Buf, 70, Radix, Eps);
	if(CountWrited < 0) return 0;
	return __stream_io__base::Write(Stream, Buf, CountWrited) ? CountWrited: -CountWrited;
}

inline size_t StringLength(const char * Str) { return strlen(Str); }
inline size_t StringLength(const wchar_t * Str) { return wcslen(Str); }
inline const char * StringSearch(const char * BaseStr, const char * SubStr) { return strstr(BaseStr, SubStr); }
inline const wchar_t * StringSearch(const wchar_t * BaseStr, const wchar_t * SubStr) { return wcsstr(BaseStr, SubStr); }
inline int StringCompare(const char * Str1, const char * Str2, size_t MaxCount) { return strncmp(Str1, Str2, MaxCount); }
inline int StringCompare(const wchar_t * Str1, const wchar_t * Str2, size_t MaxCount) { return wcsncmp(Str1, Str2, MaxCount); }
inline int StringCompare(const char * Str1, const char * Str2) { return strcmp(Str1, Str2); }
inline int StringCompare(const wchar_t * Str1, const wchar_t * Str2) { return wcscmp(Str1, Str2); }

inline int StringICompare(const char * Str1, const char * Str2, size_t MaxCount)
{
#if _POSIX_VERSION >= 200112L //If posix standart >= POSIX.1-2001
	return strncasecmp(Str1, Str2, MaxCount);
#else
	return strnicmp(Str1, Str2, MaxCount);
#endif
}

inline int StringICompare(const wchar_t * Str1, const wchar_t * Str2, size_t MaxCount)
{
#if _POSIX_VERSION >= 200112L  //If posix standart >= POSIX.1-2001
	return wcsncasecmp(Str1, Str2, MaxCount);
#else
	return wcsnicmp(Str1, Str2, MaxCount);
#endif
}

inline int StringICompare(const char * Str1, const char * Str2)
{
#if _POSIX_VERSION >= 200112L //If posix standart >= POSIX.1-2001
	return strcasecmp(Str1, Str2);
#else
	return stricmp(Str1, Str2);
#endif
}

inline int StringICompare(const wchar_t * Str1, const wchar_t * Str2)
{
#if _POSIX_VERSION >= 200112L  //If posix standart >= POSIX.1-2001
	return wcscasecmp(Str1, Str2);
#else
	return wcsicmp(Str1, Str2);
#endif
}

inline char * StringUpper(char * Dest) { return strupr(Dest); }
inline wchar_t * StringUpper(wchar_t * Dest) { return wcsupr(Dest); }
inline char * StringLower(char * Dest) { return strlwr(Dest); }
inline wchar_t * StringLower(wchar_t * Dest) { return wcslwr(Dest); }
inline char * StringAppend(char * Dest, const char * Source) { return strcat(Dest, Source); }
inline wchar_t * StringAppend(wchar_t * Dest, const wchar_t * Source) { return wcscat(Dest, Source); }
inline char * StringAppend(char * Dest, const char * Source, size_t Count) { return strncat(Dest, Source, Count); }
inline wchar_t * StringAppend(wchar_t * Dest, const wchar_t * Source, size_t Count) { return wcsncat(Dest, Source, Count); }
inline char * StringCopy(char * Dest, const char * Source, size_t MaxCount) { return strncpy(Dest, Source, MaxCount); }
inline wchar_t * StringCopy(wchar_t * Dest, const wchar_t * Source, size_t MaxCount) { return wcsncpy(Dest, Source, MaxCount); }
inline char * StringCopy(char * Dest, const char * Source) { return strcpy(Dest, Source); }
inline wchar_t * StringCopy(wchar_t * Dest, const wchar_t * Source) { return wcscpy(Dest, Source); }
inline char * StringDuplicate(const char * Source) { return strdup(Source); }
inline wchar_t * StringDuplicate(const wchar_t * Source) { return wcsdup(Source); }
template<typename TypeChar>
std::basic_string<TypeChar> StringDuplicate(std::basic_string<TypeChar>& SourceStr) { return SourceStr; }

template<typename TypeChar>
std::basic_string<TypeChar>& StringReplace(std::basic_string<TypeChar>& Source, const TypeChar* SrchSubStr,  const TypeChar* ReplSubStr)
{
	size_t LenSearchStr = StringLength(SrchSubStr);
	if(LenSearchStr == 0)
		return Source;
	size_t FoundedIndex = Source.find(SrchSubStr, LenSearchStr);
	if(FoundedIndex != std::basic_string<TypeChar>::npos)
		Source.replace(FoundedIndex, LenSearchStr, ReplSubStr);
	return Source;
}

template<typename TypeChar>
std::basic_string<TypeChar>& StringReplace(
	std::basic_string<TypeChar>& Source, 
	const std::basic_string<TypeChar>& SrchSubStr,  
	const std::basic_string<TypeChar>& ReplSubStr)
{
	return StringReplace(Source, SrchSubStr.c_str(), ReplSubStr.c_str());
}

size_t SkipSpace(std::basic_istream<char> & Stream);
size_t SkipSpace(std::basic_istream<wchar_t> & Stream);
size_t SkipSpace(FILE* Stream);

inline size_t SkipSpace(const char* String, size_t Len);
inline size_t SkipSpace(const wchar_t* String, size_t Len);

template<typename TypeChar>
inline bool IsDigit(TypeChar c) { return (CHAR_TYPE(TypeChar, '0') <= c) && (CHAR_TYPE(TypeChar, '9') >= c); }
inline bool IsLatter(wchar_t c) { return iswalpha(c) != 0; }
inline bool IsLatter(char c) { return isalpha(c) != 0; }

bool IsSpace(char c);
bool IsSpace(wchar_t c);

#endif
