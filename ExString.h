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
#include <typeinfo>
#include "ExTypeTraits.h"

#ifndef WIN32
#       include <cerrno>
#       ifndef ANDROID
#           include <iconv.h>
#       endif

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
#define STR_TYPE(_Type,_Str)		((std::is_equal<_Type, wchar_t>::value)?(_Type*)(L ## _Str):(_Type*)(_Str))
#define CHAR_TYPE(_Type, _Char)		((_Type)((std::is_equal<_Type, char>::value)?(_Char):(L ## _Char)))
#define NOT_LESS_Z(Num)				((0 > std::make_signed<decltype(Num)>::type(Num))?0:Num)
#define DIGIT_TO_ALPHA(TypeChar, Dig) ((Dig > 9)? (Dig + (CHAR_TYPE(TypeChar,'a') - 10)): (Dig + CHAR_TYPE(TypeChar,'0')))
#define CMP_I(Val, c)				(((Val) == CHAR_TYPE(decltype(Val), c)) || ((Val) == (CHAR_TYPE(decltype(Val), 'A') + (CHAR_TYPE(decltype(Val), c) - CHAR_TYPE(decltype(Val), 'a')))))


template <typename ResultType>
struct RET_STAT
{
	bool		Success;
	ResultType	Result;

	inline operator bool() const
	{
		return Success;
	}

	inline RET_STAT()
	{
		this->Success = false;
		this->Result = 0;
	}

	inline RET_STAT(ResultType Result)
	{
		this->Success = false;
		this->Result = Result;
	}

	inline RET_STAT(ResultType Result, bool Success)
	{
		this->Success = Success;
		this->Result = Result;
	}

	inline bool operator ()(ResultType & Result) const
	{
		Result = this->Result;
		return Success;
	}

	inline ResultType operator ()(bool & Success) const
	{
		Success = this->Success;
		return this->Result;
	}
};

typedef RET_STAT<size_t> STR_STAT;

template<typename OutString>
void StringConvertCodePage(unsigned InCp, unsigned OutCp, const wchar_t * InStr, OutString & OutStr)
{
	std::basic_string<wchar_t> TmpStr(InStr);
	StringConvertCodePage(InCp, OutCp, TmpStr, OutStr);
}

template<typename OutString>
void StringConvertCodePage(unsigned InCp, unsigned OutCp, const char * InStr, OutString & OutStr)
{
	std::basic_string<char> TmpStr(InStr);
	StringConvertCodePage(InCp, OutCp, TmpStr, OutStr);
}


template<typename InString, typename OutString>
void StringConvertCodePage(unsigned InCp, unsigned OutCp, const InString & InStr, OutString & OutStr)
{
	typedef typename InString::value_type _InCharType;
	typedef typename OutString::value_type _OutCharType;
#ifndef ANDROID
#	ifndef WIN32
	static const char * CodePageStr[] =
	{
		"UTF-8",
		"CP1251",
		"KOI-8",
		"UTF-7",
		"ANSI"
	};
	iconv_t convert_hnd = (iconv_t)-1;
	size_t SizeInBuf,  SizeOutBuf;
#	endif

	if(std::is_equal<_InCharType, wchar_t>::value && std::is_equal<_OutCharType, wchar_t>::value)
	{
		OutStr = (_OutCharType*)InStr.c_str();
		return;
	}else if(std::is_equal<_InCharType, wchar_t>::value)
	{
#	ifdef WIN32
		unsigned NewSize = WideCharToMultiByte(OutCp, 0, (LPCWSTR)InStr.c_str(), InStr.length(), nullptr, 0, nullptr, nullptr);
		OutStr.resize(NewSize);
		WideCharToMultiByte(OutCp, 0, (LPCWSTR)InStr.c_str(), InStr.length(), (LPSTR)OutStr.c_str(), NewSize, nullptr, nullptr);
#	else
		convert_hnd = iconv_open(CodePageStr[OutCp], "WCHAR_T");
		if(convert_hnd == (iconv_t)-1)
			return;
		SizeInBuf = (SizeOutBuf = InStr.length() + 1) * sizeof(_InCharType);
		OutStr.resize(SizeOutBuf);
		SizeOutBuf *= sizeof(_OutCharType);
#	endif
	}else if(std::is_equal<_OutCharType, wchar_t>::value)
	{
#	ifdef WIN32
		unsigned NewSize = MultiByteToWideChar(InCp, 0, (LPCSTR)InStr.c_str(),InStr.length(), nullptr, 0);
		OutStr.resize(NewSize);
		MultiByteToWideChar(InCp, 0, (LPCSTR)InStr.c_str(), InStr.length(), (LPWSTR)OutStr.c_str(), NewSize);
#	else
		convert_hnd = iconv_open("WCHAR_T", CodePageStr[InCp]);
		if(convert_hnd == (iconv_t)-1)
			return;
		SizeInBuf = (InStr.length() + 1) * sizeof(_InCharType);
		OutStr.resize(SizeInBuf);
		SizeOutBuf = SizeInBuf * sizeof(_OutCharType);
#	endif
	}else
	{
		if(InCp == OutCp)
		{
			OutStr = (_OutCharType*)InStr.c_str();
			return;
		}
#	ifdef WIN32
		unsigned size = MultiByteToWideChar(InCp, 0, (LPCSTR)InStr.c_str(),InStr.length(), nullptr, 0);
		std::wstring unicode_str(size, '\0');
		MultiByteToWideChar(InCp, 0, (LPCSTR)InStr.c_str(), InStr.length(), &unicode_str[0], size);

		int NewSize = WideCharToMultiByte(OutCp, 0, unicode_str.c_str(),unicode_str.length(), nullptr, 0, nullptr, nullptr);
		OutStr.resize(NewSize);
		WideCharToMultiByte(OutCp, 0, unicode_str.c_str(),unicode_str.length(),(LPSTR)OutStr.c_str(), NewSize, nullptr, nullptr);
#	else
		convert_hnd = iconv_open(CodePageStr[OutCp], CodePageStr[InCp]);
		if(convert_hnd == (iconv_t)-1)
			return;
		SizeInBuf = (InStr.length() + 1) * sizeof(_InCharType);
		OutStr.resize(SizeInBuf);
		SizeOutBuf = SizeInBuf * sizeof(_OutCharType);
#	endif
	}

#	ifndef WIN32
	for(char * InBuf = (char*)InStr.c_str(), *OutBuf = (char*)OutStr.c_str(); SizeInBuf != 0;)
	{
		if(iconv(convert_hnd, &InBuf, &SizeInBuf, &OutBuf, &SizeOutBuf) == (size_t)-1)
		{
			if(errno == 7)
			{
				unsigned j = (unsigned)OutBuf - (unsigned)OutStr.c_str();
				OutStr.resize(OutStr.length() + 20);
				SizeOutBuf = 20 * sizeof(_OutCharType);
				OutBuf = (char*)OutStr.c_str() + j;
			}
			else if(SizeInBuf == 0)
				break;
			else
			{
				OutStr.clear();
				break;
			}
		}
	}

	iconv_close(convert_hnd);
#	endif
#else
	OutStr = (_OutCharType*)InStr.c_str();
#endif
}


//Converting from number to string

struct __stream_io
{
	template<typename TypeChar>
	static inline TypeChar GetChar(std::basic_istream<TypeChar> & s)
	{
		TypeChar c;
		s.get(c);
		return c;
	}	
	
	template<typename TypeChar>
	static inline TypeChar GetChar(FILE * s)
	{
		if(std::is_equal<TypeChar, wchar_t>::value)
			return getwc(s);
		else
			return getc(s);
	}

	template<typename TypeChar>
	static inline void UnGetChar(std::basic_istream<TypeChar> & s, TypeChar v)
	{
		s.unget();
	}

	static inline void UnGetChar(FILE * s, wchar_t v)
	{
		ungetwc(v, s);
	}

	static inline void UnGetChar(FILE * s, char v)
	{
		ungetc(v, s);
	}

	static inline bool PutChar(FILE * s, char v)
	{
	  return fputc(v, s) != EOF;
	}

	static inline bool PutChar(FILE * s, wchar_t v)
	{
	  return fputwc(v, s) != WEOF;
	}

	template<typename TypeChar>
	static inline bool PutChar(std::basic_ostream<TypeChar> & s, TypeChar c)
	{
		return !s.put(c).fail();
	}	

	static inline bool Write(FILE * s,  char * Str, size_t Len)
	{
		return fputs(Str, s) != EOF;
	}

	static inline bool Write(FILE * s,  wchar_t * Str, size_t Len)
	{
		return fputws(Str, s) != WEOF;
	}	
	
	template<typename TypeChar>
	static inline bool Write(std::basic_ostream<TypeChar> & s, TypeChar * Str, size_t Len)
	{
		return !s.write(Str, Len).fail();
	}	
};


/*
  Decl templates
*/

template<typename TypeNumber, typename TypeChar>
STR_STAT _i_NumberToString(TypeNumber Number, TypeChar * Str, size_t Len, unsigned char Radix);

template
<
	bool		IsSkipSpace, 
	typename	TypeNumber, 
	typename	TypeChar, 
	typename	StreamType,  
	TypeChar	(*GetChar)(StreamType), 
	void		(*UngetChar)(StreamType, TypeChar)
>
STR_STAT _i_StreamToNumber(TypeNumber * Dest, StreamType InStream, unsigned char Radix );

template<bool IsSkipSpace, typename TypeChar, typename TypeNumber>
STR_STAT _i_StringToNumber(TypeNumber * Dest, const TypeChar * Str, size_t Len, unsigned char Radix);

template<bool IsScaleEps, typename TypeChar>
STR_STAT _d_NumberToString(long double Number, TypeChar * Str, size_t Len, unsigned char Radix, long double Eps1);

template
<
	bool		IsSkipSpace, 
	bool		InfInd, 
	typename	TypeNumber, 
	typename	TypeChar, 
	typename	StreamType, 
	TypeChar	(*GetChar)(StreamType), 
	void		(*UngetChar)(StreamType, TypeChar)
>
STR_STAT _d_StreamToNumber(TypeNumber * Dest, StreamType InStream, unsigned char Radix);

template<bool IsSkipSpace, bool InfInd, typename TypeNumber, typename TypeChar>
STR_STAT _d_StringToNumber(TypeNumber * Dest, const TypeChar * Str, size_t Len, unsigned char Radix);


template<typename TypeChar, typename StreamType, TypeChar (*GetChar)(StreamType), void (*UngetChar)(StreamType, TypeChar)>
inline size_t _SkipSpace(StreamType Stream);

/*
  Decl templates
*/


template<typename TypeChar>
inline STR_STAT NumberToString(int Number, TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
	return _i_NumberToString(Number, Str, Len, Radix);
}

template<typename TypeChar>
inline STR_STAT NumberToString(unsigned Number, TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
	return _i_NumberToString(Number, Str, Len, Radix);
}

template<typename TypeChar>
inline STR_STAT NumberToString(long Number, TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
	return _i_NumberToString(Number, Str, Len, Radix);
}

template<typename TypeChar>
inline STR_STAT NumberToString(unsigned long Number, TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
	return _i_NumberToString(Number, Str, Len, Radix);
}


template<typename TypeChar>
inline STR_STAT NumberToString(long long Number, TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
	return _i_NumberToString(Number, Str, Len, Radix);
}

template<typename TypeChar>
inline STR_STAT NumberToString(unsigned long long Number, TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
	return _i_NumberToString(Number, Str, Len, Radix);
}

///Float point
template<typename TypeChar>
inline STR_STAT NumberToString(float Val, TypeChar * Str, size_t LenBuf, unsigned char RadX, long double Eps)
{
	return _d_NumberToString<false>((long double)Val, Str, LenBuf, RadX, Eps);
}

template<typename TypeChar>
inline STR_STAT NumberToString(float Val, TypeChar * Str, size_t LenBuf, unsigned char RadX = 10)
{
	return _d_NumberToString<true>((long double)Val, Str, LenBuf, RadX, 0.000000001);
}

template<typename TypeChar>
inline STR_STAT NumberToString(double Val, TypeChar * Str, size_t LenBuf, unsigned char RadX, long double Eps)
{
	return _d_NumberToString<false>((long double)Val, Str, LenBuf, RadX, Eps);
}

template<typename TypeChar>
inline STR_STAT NumberToString(double Val, TypeChar * Str, size_t LenBuf, unsigned char RadX = 10)
{
	return _d_NumberToString<true>((long double)Val, Str, LenBuf, RadX, 0.00000000000000001);
}

template<typename TypeChar>
inline STR_STAT NumberToString(long double Val, TypeChar * Str, size_t LenBuf, unsigned char RadX, long double Eps)
{
	return _d_NumberToString<false>(Val, Str, LenBuf, RadX, Eps);
}

template<typename TypeChar>
inline STR_STAT NumberToString(long double Val, TypeChar * Str, size_t LenBuf, unsigned char RadX = 10)
{
	return _d_NumberToString<true>(Val, Str, LenBuf, RadX, 0.00000000000000001);
}

template<typename TypeChar, typename TypeNumber>
inline STR_STAT NumberToString(TypeNumber Val, std::basic_string<TypeChar> & Str, size_t LenBuf = 0,unsigned char RadX = 10)
{
   TypeChar Buf[70];
   STR_STAT Ret = NumberToString(Val, Buf, 70, RadX);
   Str += Buf;
   return Ret;
}

template<typename TypeChar, typename TypeNumber>
inline STR_STAT NumberToString(TypeNumber Val, std::basic_string<TypeChar> & Str, size_t LenBuf, unsigned char RadX, long double Eps)
{
   TypeChar Buf[70];
   STR_STAT Ret = NumberToString(Val, Buf, 70, RadX, Eps);
   Str += Buf;
   return Ret;
}

template<typename TypeChar, typename TypeNumber, size_t BufSize>
inline STR_STAT NumberToString(TypeNumber Val, TypeChar (&Buf)[BufSize])
{
   return NumberToString(Val, (TypeChar*)Buf, BufSize, 10);
}

template<typename TypeNumber, typename TypeChar>
STR_STAT _i_NumberToString(TypeNumber Number, TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
	TypeChar *s = Str, *b = s + Len;

	typename std::make_unsigned<TypeNumber>::type AbsNum;
    if(std::is_signed<TypeNumber>::value && (Number < 0) && (s < b))
	{
		*(s++) = CHAR_TYPE(TypeChar,'-');
		AbsNum = -Number;
	}else
		AbsNum = Number;

	TypeChar Buf[40];
	TypeChar *m = Buf + 40, *c = m;
	do
	{
		  decltype(AbsNum % Radix) digval = AbsNum % Radix;
		  *(--c) = DIGIT_TO_ALPHA(TypeChar, digval);
	}while((AbsNum /= Radix) > 0);
	for(;(c < m) && (s < b);c++, s++)
		*s = *c;
	if(s < b)
		*s = CHAR_TYPE(TypeChar,'\0');
	return STR_STAT(s - Str, true);
}

template<bool IsScaleEps, typename TypeChar>
STR_STAT _d_NumberToString(long double Number, TypeChar * Str, size_t Len, unsigned char Radix, long double Eps1)
{
	static const long double MinExp = 1.0e-5;
	static const long double MaxExp = 1.0e+15;
	static const long double Inf = 9999e+200 * 9999e+200 * 9999e+200;
	static const long double Ind = Inf * 0;
	static const long double Qnan = -Ind;

	long double uNumber = (Number < 0)? -Number: Number;
	short Exp = 0;
	size_t CountWrited = 0;

	if(CountWrited < Len)
	{
		if(Number < 0.0)
			Str[CountWrited++] = CHAR_TYPE(TypeChar, '-');
		if(CountWrited >= Len)
			return CountWrited;
	}else
	     return 0;

    if(Number != Number)   //Is NaN
	{	
		const TypeChar * IndVal = STR_TYPE(TypeChar, "1.#IND00");
		for(unsigned char i = 0;(CountWrited < Len) && (i < sizeof("1.#IND00"));CountWrited++, i++)
			Str[CountWrited] = IndVal[i];
		return STR_STAT(CountWrited, true);
	}else if(uNumber == Inf) //Is infinity
	{
		const TypeChar * IndVal = STR_TYPE(TypeChar, "1.#INF00");
		for(unsigned char i = 0;(CountWrited < Len) && (i < sizeof("1.#INF00"));CountWrited++, i++)
			Str[CountWrited] = IndVal[i];
		return STR_STAT(CountWrited, true);
	}else if(uNumber == Qnan) //Is qnan
	{
		const TypeChar * IndVal = STR_TYPE(TypeChar, "1.#QNAN00");
		for(unsigned char i = 0;(CountWrited < Len) && (i < sizeof("1.#QNAN00"));CountWrited++, i++)
			Str[CountWrited] = IndVal[i];
		return STR_STAT(CountWrited, true);
	}else if((uNumber != 0.0) && ((uNumber < MinExp) || (uNumber > MaxExp)))//Is have expanent
	{
		Exp = ((*(short*)((char*)&uNumber + (sizeof(uNumber) - sizeof(Exp))) & 0x7ff0) >> 4) - 1023;
		Exp = (short)(log(pow(2.0,Exp)) / log(Radix));
		uNumber *= pow(Radix, -Exp);
	}

	unsigned long long Integer = (unsigned long long)uNumber;
	uNumber = uNumber - Integer + 1.0;
	if(uNumber >= 2.0)
	{
		uNumber--;
		Integer++;
	}

	TypeChar Buf[40];
	TypeChar *m = Buf + 40, *c = m;

	if(IsScaleEps)
	{
		for(;Integer > 0;Integer /= Radix, Eps1 *= Radix)
		{
			unsigned char digval = Integer % Radix;				
			*(--c) = DIGIT_TO_ALPHA(TypeChar, digval);
		}
	}else
	{
		for(;Integer > 0;Integer /= Radix)
		{
			unsigned char digval = Integer % Radix;
			*(--c) = DIGIT_TO_ALPHA(TypeChar, digval);
		}
	}			
	if(c == m)
		*(--c) = CHAR_TYPE(TypeChar, '0');

	for(;c < m;c++, CountWrited++)
		if(CountWrited >= Len)
			return CountWrited;
		else
			Str[CountWrited] = *c;

	Str[CountWrited++] = CHAR_TYPE(TypeChar,'.');
	{
		long double Eps2 = 1.0 - Eps1;	
	    while(((uNumber - (unsigned long long)uNumber) > Eps1) &&
			  ((uNumber - (unsigned long long)uNumber) < Eps2))
		{
			Eps2 = 1.0 - (Eps1 *= Radix);
			uNumber *= Radix;
		}
	}
	
	Integer = (unsigned long long)uNumber;
	if((uNumber - Integer) >= 0.5)
		Integer++;
	
	c = m;
	for(;Integer > 1; Integer /= Radix)
	{
		unsigned char digval = Integer % Radix;
		*(--c) = DIGIT_TO_ALPHA(TypeChar, digval);
	}
	if(c == m)
		*(--c) = CHAR_TYPE(TypeChar, '0');

	for(;c < m;c++, CountWrited++)
		if(CountWrited >= Len)
			return CountWrited;
		else
			Str[CountWrited] = *c;
	if(Exp)
	{
		if(CountWrited >= Len)
				return CountWrited;
		Str[CountWrited++] = CHAR_TYPE(TypeChar, 'e');
		if(CountWrited >= Len)
				return CountWrited;
		if(Exp > 0)
			Str[CountWrited++] = CHAR_TYPE(TypeChar, '+');
		size_t y;
		if(!_i_NumberToString(Exp, Str + CountWrited, Len - CountWrited, Radix)(y))
			return CountWrited + y;
		CountWrited += y;
	}
	if(CountWrited < Len)
		Str[CountWrited] = CHAR_TYPE(TypeChar, '\0');
	return STR_STAT(CountWrited, true);
}

template<typename TypeChar, typename TypeNumber>
inline STR_STAT StringToNumber(TypeNumber * Number, const TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
   if(std::is_floating_point<TypeNumber>::value)
	   return _d_StringToNumber<true, true>(Number, Str, Len, Radix);
   else
	   return _i_StringToNumber<true>(Number, Str, Len, Radix);
}

template<typename TypeChar, typename TypeNumber, size_t BufSize>
inline STR_STAT StringToNumber(TypeNumber * Number, const TypeChar (&Buf)[BufSize])
{
   if(std::is_floating_point<TypeNumber>::value)
	   return _d_StringToNumber<true, true>(Number, Buf, BufSize, 10);
   else
	   return _i_StringToNumber<true>(Number, Buf, BufSize, 10);
}

template<typename TypeChar, typename TypeNumber>
inline STR_STAT StringToNumber(TypeNumber * Number, const std::basic_string<TypeChar> & Str, size_t Len = 0, unsigned char Radix = 10)
{
     return StringToNumber(Number, Str.c_str(), Str.length(), Radix);
}

//From string to number

template<bool IsSkipSpace, typename TypeChar, typename TypeNumber>
STR_STAT _i_StringToNumber(TypeNumber * Dest, const TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
	char Negative = 1;
	size_t CountReaded = 0;
	if(IsSkipSpace)
		CountReaded += SkipSpace(Str, Len);
	if(std::is_signed<TypeNumber>::value && (CountReaded < Len))
		switch(Str[CountReaded])
		{
			case CHAR_TYPE(TypeChar, '-'):
				Negative = -1;
			case CHAR_TYPE(TypeChar, '+'):
				CountReaded++;
		}
	TypeNumber Ret = (TypeNumber)0;
	size_t t = CountReaded;
	for(;CountReaded < Len;CountReaded++)
	{
		unsigned char Digit = Str[CountReaded] - CHAR_TYPE(TypeChar, '0');
		if(Digit > 9)
		{
			Digit = Str[CountReaded] - (CHAR_TYPE(TypeChar, 'a') - 10);
			if(Digit >= Radix)
				Digit = Str[CountReaded] - (CHAR_TYPE(TypeChar, 'A') - 10);
			if(Digit >= Radix)
				break;
		}
		Ret = Ret * Radix + Digit;
	}
	if(t == CountReaded)
		return CountReaded;
	*Dest =  Ret * Negative;
	return STR_STAT(CountReaded, true);
}

template<bool IsSkipSpace, bool InfInd, typename TypeNumber, typename TypeChar>
STR_STAT _d_StringToNumber(TypeNumber * Dest, const TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
	static const long double Inf = 9999e+200 * 9999e+200 * 9999e+200;
	static const long double Ind = Inf * 0;
	static const long double Qnan = -Ind;
	size_t CountReaded;
	long long IntegerPart = 0;
	if(!_i_StringToNumber<IsSkipSpace>(&IntegerPart, Str, Len, Radix)(CountReaded))
		return CountReaded;

	if((CountReaded >= Len) || (Str[CountReaded] != CHAR_TYPE(TypeChar, '.')))
	{
		*Dest = TypeNumber(IntegerPart);
		return STR_STAT(CountReaded, true);
	}
	CountReaded++;
	long double Result = IntegerPart;

	if(InfInd && ((CountReaded + 4) <  Len) && (Str[CountReaded] == CHAR_TYPE(TypeChar, '#')))
	{
		if
		(
			CMP_I(Str[CountReaded + 1], 'i') && 
			CMP_I(Str[CountReaded + 2], 'n')
		)
		{
			switch(Str[CountReaded + 3])
			{
			case 'F':
			case 'f'://#INF
				Result *= Inf;
				CountReaded += 4;
				goto lblSingOut;
			case 'D':
			case 'd'://#IND
				Result *= Ind;
				CountReaded += 4;
				goto lblSingOut;
			}
		}else if
		(
			((CountReaded + 5) < Len)		 && 
			CMP_I(Str[CountReaded + 1], 'q') && 
			CMP_I(Str[CountReaded + 2], 'n') && 
			CMP_I(Str[CountReaded + 3], 'a') && 
			CMP_I(Str[CountReaded + 4], 'n')
		)
		{//#QNAN
			Result *= Qnan;
			CountReaded += 5;
			goto lblSingOut;
		}
		goto lblTrueOut;
	}

lblSingOut:

	//Get fraction part
	{
		unsigned long long	FractPart = 1;
		unsigned			CountNum = 0;
		for(;CountReaded < Len; CountReaded++, CountNum++)
		{
			unsigned char Digit = Str[CountReaded] - CHAR_TYPE(TypeChar, '0');
			if(Digit > 9)
			{
				Digit = Str[CountReaded] - (CHAR_TYPE(TypeChar, 'a') - 10);
				if(Digit >= Radix)
					Digit = Str[CountReaded] - (CHAR_TYPE(TypeChar, 'A') - 10);
				if(Digit >= Radix)
					break;
			}
			FractPart = FractPart * Radix + Digit;
		}
		long double DoubleFract = 0.0;
		for(;FractPart > 1; FractPart /= Radix)
			DoubleFract = (DoubleFract + (long double)(FractPart % Radix)) * (long double)0.1;
		
		if(Result < 0.0)
			Result -= DoubleFract;
		else
			Result += DoubleFract;
	}
	if(CountReaded >= Len)
		goto lblTrueOut;
	if(CMP_I(Str[CountReaded], 'e'))
	{
	   int Exp;
	   size_t r;
	   if(_i_StringToNumber<false>(&Exp, Str + (CountReaded + 1), Len - (CountReaded + 1), Radix)(r))
	   {
		   Result *= pow((long double)Radix, Exp);
		   CountReaded += r;
	   }
	}
lblTrueOut:
	*Dest =  TypeNumber(Result);
	return STR_STAT(CountReaded, true);
}

template<typename TypeNumber, typename TypeChar>
inline STR_STAT StreamToNumber(TypeNumber * Dest, std::basic_istream<TypeChar> & Stream, unsigned char Radix = 10)
{
	if(std::is_floating_point<TypeNumber>::value)
		return _d_StreamToNumber
				<
					true, 
					true, 
					TypeNumber, 
					TypeChar, 
					std::basic_istream<TypeChar> &, 
					__stream_io::GetChar, 
					__stream_io::UnGetChar
				>
				(Dest, Stream, Radix);
	else
		return _i_StreamToNumber
				<
					true, 
					TypeNumber, 
					TypeChar, 
					std::basic_istream<TypeChar> &, 
					__stream_io::GetChar, 
					__stream_io::UnGetChar
				>
				(Dest, Stream, Radix);
}

template<typename TypeNumber>
inline STR_STAT StreamToNumber(TypeNumber * Dest, FILE * Stream = stdin, unsigned char Radix = 10)
{

	if(std::is_floating_point<TypeNumber>::value)
		return _d_StreamToNumber
				<
					true, 
					true, 
					TypeNumber, 
					wchar_t, 
					FILE *, 
					__stream_io::GetChar, 
					__stream_io::UnGetChar
				>
				(Dest, Stream, Radix);
	else
		return _i_StreamToNumber
				<
					true, 
					TypeNumber, 
					wchar_t, 
					FILE *, 
					__stream_io::GetChar, 
					__stream_io::UnGetChar
				>
				(Dest, Stream, Radix);
}

template<typename TypeNumber>
inline STR_STAT StreamDoubleToNumber(TypeNumber * Dest, FILE * Stream = stdin, unsigned char Radix = 10)
{
	return _d_StreamToNumber
			<
				true, 
				true, 
				TypeNumber, 
				wchar_t, 
				FILE *, 
				__stream_io::GetChar, 
				__stream_io::UnGetChar
			>
			(Dest, Stream, Radix);
}

template<typename TypeNumber, typename TypeChar>
inline STR_STAT StreamDoubleToNumber(TypeNumber * Dest, std::basic_istream<TypeChar> & Stream, unsigned char Radix = 10)
{
	return _d_StreamToNumber
			<
				true, 
				true, 
				TypeNumber, 
				TypeChar, 
				std::basic_istream<TypeChar> &, 
				__stream_io::GetChar, 
				__stream_io::UnGetChar
			>
			(Dest, Stream, Radix);
}

template<typename TypeChar, typename TypeNumber>
inline STR_STAT StringDoubleToNumber(TypeNumber * Number, const TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
	   return _d_StringToNumber<true, true>(Number, Str, Len, Radix);
}

template<typename TypeChar, typename TypeNumber, size_t BufSize>
inline STR_STAT StringDoubleToNumber(TypeNumber * Number, const TypeChar (&Buf)[BufSize])
{
	return StringDoubleToNumber(Number, Buf, BufSize, 10);
}

template<typename TypeChar, typename TypeNumber>
inline STR_STAT StringDoubleToNumber(TypeNumber * Number, const std::basic_string<TypeChar> & Str, size_t Len = 0, unsigned char Radix = 10)
{
     return StringDoubleToNumber(Number, Str.c_str(), Str.length(), Radix);
}

template
<
	bool		IsSkipSpace, 
	typename	TypeNumber, 
	typename	TypeChar, 
	typename	StreamType,  
	TypeChar	(*GetChar)(StreamType), 
	void		(*UngetChar)(StreamType, TypeChar)
>
STR_STAT _i_StreamToNumber(TypeNumber * Dest, StreamType InStream, unsigned char Radix = 10)
{		
	char Negative = 1;
	size_t CountReaded = 0;
	if(IsSkipSpace)
		CountReaded += _SkipSpace<TypeChar, StreamType, GetChar, UngetChar>(InStream);
	TypeChar Cur = GetChar(InStream);
	CountReaded++;
	if(std::is_signed<TypeNumber>::value)
		switch(Cur)
		{
			case CHAR_TYPE(TypeChar, '-'):
				Negative = -1;
			case CHAR_TYPE(TypeChar, '+'):
				Cur = GetChar(InStream);
				CountReaded++;
		}
	TypeNumber Ret = (TypeNumber)0;
	size_t t = CountReaded;
	for(;;Cur = GetChar(InStream), CountReaded++)
	{
		unsigned char Digit = Cur - CHAR_TYPE(TypeChar, '0');
		if(Digit > 9)
		{
			Digit = Cur - (CHAR_TYPE(TypeChar, 'a') - 10);
			if(Digit >= Radix)
				Digit = Cur - (CHAR_TYPE(TypeChar, 'A') - 10);
			if(Digit >= Radix)
				break;
		}
		Ret = Ret * Radix + Digit;
	}
	UngetChar(InStream, Cur);
	if(t == CountReaded--)
		return STR_STAT(CountReaded);
	*Dest = Ret * Negative;
	return STR_STAT(CountReaded, true);
}

template
<
	bool		IsSkipSpace, 
	bool		InfInd, 
	typename	TypeNumber, 
	typename	TypeChar, 
	typename	StreamType, 
	TypeChar	(*GetChar)(StreamType), 
	void		(*UngetChar)(StreamType, TypeChar)
>
STR_STAT _d_StreamToNumber(TypeNumber * Dest, StreamType InStream, unsigned char Radix = 10)
{
	static const long double Inf = 9999e+200 * 9999e+200 * 9999e+200;
	static const long double Ind = Inf * 0;
	static const long double Qnan = -Ind;

	long long IntegerPart = 0;
	size_t CountReaded;
	if(!_i_StreamToNumber<IsSkipSpace, long long, TypeChar, StreamType, GetChar, UngetChar>(&IntegerPart, InStream, Radix)(CountReaded))
		return CountReaded;

	TypeChar Cur = GetChar(InStream);
	if(Cur != CHAR_TYPE(TypeChar, '.'))
	{		
		*Dest = TypeNumber(IntegerPart);
		UngetChar(InStream, Cur);
		return STR_STAT(CountReaded, true);
	}
	CountReaded++;
	long double Result = IntegerPart;
	Cur = GetChar(InStream);
	CountReaded++;

	if(InfInd && (Cur == CHAR_TYPE(TypeChar, '#')))
	{
		Cur = GetChar(InStream);
		CountReaded++;
		if(CMP_I(Cur, 'i'))
		{
			Cur = GetChar(InStream);
			CountReaded++;
			if(CMP_I(Cur, 'n'))
			{
				Cur = GetChar(InStream);
				CountReaded++;
				switch(Cur)
				{
				case 'F':
				case 'f':
					Result = IntegerPart * Inf;
					Cur = GetChar(InStream);
					CountReaded++;
					goto lblSingOut;
				case 'D':
				case 'd':
					Result = IntegerPart * Ind;
					Cur = GetChar(InStream);
					CountReaded++;
					goto lblSingOut;
				}
			}
		}else if(CMP_I(Cur, 'q'))
		{
			Cur = GetChar(InStream);
			CountReaded++;
			if(CMP_I(Cur, 'n'))
			{
				Cur = GetChar(InStream);
				CountReaded++;
				if(CMP_I(Cur, 'a'))
				{
					Cur = GetChar(InStream);
					CountReaded++;
					if(CMP_I(Cur, 'n'))
					{
						Result = IntegerPart * Qnan;
						Cur = GetChar(InStream);
						CountReaded++;
						goto lblSingOut;
					}
				}
			}
		}
		goto lblTrueOut;
	}

lblSingOut:

	//Get fraction part
	{
		unsigned long long FractPart = 1;
		unsigned CountNum = 0;
		for(;;CountNum++, Cur = GetChar(InStream), CountReaded++)
		{
			unsigned char Digit = Cur - CHAR_TYPE(TypeChar, '0');
			if(Digit > 9)
			{
				Digit = Cur - (CHAR_TYPE(TypeChar, 'a') - 10);
				if(Digit >= Radix)
					Digit = Cur- (CHAR_TYPE(TypeChar, 'A') - 10);
				if(Digit >= Radix)
					break;
			}
			FractPart = FractPart * Radix + Digit;
		}
		long double DoubleFract = 0.0;
		for(;FractPart > 1; FractPart /= Radix)
			DoubleFract = (DoubleFract + (long double)(FractPart % Radix)) * (long double)0.1;

		if(Result < 0.0)
			Result -= DoubleFract;
		else
			Result += DoubleFract;
	}

	if(CMP_I(Cur, 'e'))
	{
		int Exp;
		size_t r;
		if(_i_StreamToNumber<false, int, TypeChar, StreamType, GetChar, UngetChar>(&Exp, InStream, Radix)(r))
			Result *= pow((long double)Radix, Exp);
		CountReaded += r;
	}else
	{
		UngetChar(InStream, Cur);
		CountReaded--;
	}
lblTrueOut:
	*Dest = TypeNumber(Result);
	return STR_STAT(CountReaded, true);
}

template<typename TypeNumber, typename TypeChar>
inline STR_STAT NumberToStream(TypeNumber Number, std::basic_ostream<TypeChar> & Stream, unsigned char Radix = 10)
{
	TypeChar Buf[70];
	size_t CountWrited = NumberToString(Number, Buf, 70, Radix).Result;
	return STR_STAT(CountWrited, __stream_io::Write(Stream, Buf, CountWrited));
}

template<typename TypeNumber>
inline STR_STAT NumberToStream(TypeNumber Number, FILE * Stream = stdout, unsigned char Radix = 10)
{
	char Buf[70];
	size_t CountWrited = NumberToString(Number, Buf, 70, Radix).Result;
	return STR_STAT(CountWrited, __stream_io::Write(Stream, Buf, CountWrited));
}

template<typename TypeNumber, typename TypeChar>
inline STR_STAT NumberToStream(TypeNumber Number, std::basic_ostream<TypeChar> & Stream, unsigned char Radix, long double Eps)
{
	TypeChar Buf[70];
	size_t CountWrited = NumberToString((long double)Number, Buf, 70, Radix, Eps).Result;
	return STR_STAT(CountWrited, __stream_io::Write(Stream, Buf, CountWrited));
}


template<typename TypeNumber>
inline STR_STAT NumberToStream(TypeNumber Number, FILE * Stream, unsigned char Radix, long double Eps)
{
	char Buf[70];
	size_t CountWrited = NumberToString((long double)Number, Buf, 70, Radix, Eps).Result;
	return STR_STAT(CountWrited, __stream_io::Write(Stream, Buf, CountWrited));
}


inline size_t StringLength(const char * Str)
{
  return strlen(Str);
}

inline size_t StringLength(const wchar_t * Str)
{
  return wcslen(Str);
}

inline const char * StringSearch(const char * BaseStr, const char * SubStr)
{
  return strstr(BaseStr, SubStr);
}

inline const wchar_t * StringSearch(const wchar_t * BaseStr, const wchar_t * SubStr)
{
  return wcsstr(BaseStr, SubStr);
}

inline int StringCompare(const char * Str1, const char * Str2, size_t MaxCount)
{
  return strncmp(Str1, Str2, MaxCount);
}

inline int StringCompare(const wchar_t * Str1, const wchar_t * Str2, size_t MaxCount)
{
  return wcsncmp(Str1, Str2, MaxCount);
}

inline int StringCompare(const char * Str1, const char * Str2)
{
  return strcmp(Str1, Str2);
}

inline int StringCompare(const wchar_t * Str1, const wchar_t * Str2)
{
  return wcscmp(Str1, Str2);
}


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

inline char * StringAppend(char * Dest, const char * Source)
{
    return strcat(Dest, Source);
}

inline wchar_t * StringAppend(wchar_t * Dest, const wchar_t * Source)
{
    return wcscat(Dest, Source);
}

inline char * StringAppend(char * Dest, const char * Source, size_t Count)
{
    return strncat(Dest, Source, Count);
}

inline wchar_t * StringAppend(wchar_t * Dest, const wchar_t * Source, size_t Count)
{
    return wcsncat(Dest, Source, Count);
}


inline char * StringCopy(char * Dest, const char * Source, size_t MaxCount)
{
  return strncpy(Dest, Source, MaxCount);
}

inline wchar_t * StringCopy(wchar_t * Dest, const wchar_t * Source, size_t MaxCount)
{
  return wcsncpy(Dest, Source, MaxCount);
}

inline char * StringCopy(char * Dest, const char * Source)
{
  return strcpy(Dest, Source);
}

inline wchar_t * StringCopy(wchar_t * Dest, const wchar_t * Source)
{
  return wcscpy(Dest, Source);
}


inline char * StringDuplicate(const char * Source)
{
   return strdup(Source);
}

inline wchar_t * StringDuplicate(const wchar_t * Source)
{
   return wcsdup(Source);
}

template<typename TypeChar, typename StreamType, TypeChar (*GetChar)(StreamType), void (*UngetChar)(StreamType, TypeChar)>
inline size_t _SkipSpace(StreamType Stream)
{
	size_t CountSkiped = 0;
	TypeChar c;
	while(true)
	{
		c = GetChar(Stream);
		if(!IsSpace(c))
			break;
		CountSkiped++;
	}
	UngetChar(Stream, c);
	return CountSkiped;
}

template<typename TypeChar>
inline size_t SkipSpace(std::basic_istream<TypeChar> & Stream)
{
	return _SkipSpace
			<
				TypeChar, 
				std::basic_istream<TypeChar> &, 
				__stream_io::GetChar, 
				__stream_io::UnGetChar
			>
			(Stream);
}

inline size_t SkipSpace(FILE * Stream)
{
	return _SkipSpace
			<
				wchar_t, 
				FILE *, 
				__stream_io::GetChar, 
				__stream_io::UnGetChar
			>
			(Stream);
}

template<typename TypeChar>
inline size_t SkipSpace(const TypeChar * String, size_t Len)
{
	size_t CountSkiped = 0;
	for(size_t i = 0;i < Len; i++)
	{
	    if(!IsSpace(String[i]))
			return CountSkiped;
		CountSkiped++;
	}
	return 0;
}

template<typename TypeChar>
inline bool IsDigit(TypeChar c)
{
	return (CHAR_TYPE(TypeChar, '0') <= c) && (CHAR_TYPE(TypeChar, '9') >= c);
}

inline bool IsLatter(wchar_t c)
{
	return iswalpha(c) != 0;
}

inline bool IsLatter(char c)
{
	return isalpha(c) != 0;
}

template<typename TypeChar>
inline bool IsSpace(TypeChar c)
{
	switch(c)
	{
		case CHAR_TYPE(TypeChar, ' '):
		case CHAR_TYPE(TypeChar, '\t'):
		case CHAR_TYPE(TypeChar, '\n'):
		case CHAR_TYPE(TypeChar, '\v'):
		case CHAR_TYPE(TypeChar, '\f'):
		case CHAR_TYPE(TypeChar, '\r'):
			return true;
	}
	return false;
}

template<typename InString, typename OutString>
void CodeUrl(const InString & InStr, OutString & OutStr, unsigned InCodePage = CP_UTF8)
{
	typedef typename OutString::value_type _InCharType;
	if(std::is_equal<_InCharType, wchar_t>::value)
	{
		std::string a;
		StringConvertCodePage(0,CP_UTF8,InStr, a);
		unsigned SizeOutStr = a.length() * 3;
		OutStr.resize(SizeOutStr);
		if(std::is_equal<_InCharType, wchar_t>::value)
		{
			for(unsigned i = 0, e = a.length(); i < e; i++)
				swprintf((wchar_t*)&OutStr[i * 3],4,L"%%%02x",a[i] & 0xff);
		}else
		{
			for(unsigned i = 0, e = a.length(); i < e; i++)
				sprintf((char*)&OutStr[i * 3],"%%%02x",a[i] & 0xff);
		}
	}else
	{
		std::string a;
		if(InCodePage != CP_UTF8)
			StringConvertCodePage(InCodePage,CP_UTF8,InStr, a);
		else
			a = (char*)InStr.c_str();
		unsigned SizeOutStr = a.length() * 3;
		OutStr.resize(SizeOutStr);
		if(std::is_equal<_InCharType, wchar_t>::value)
		{
			for(unsigned i = 0, e = a.length(); i < e; i++)
				swprintf((wchar_t*)&OutStr[i * 3],4,L"%%%02x",a[i] & 0xff);
		}else
		{
			for(unsigned i = 0, e = a.length(); i < e; i++)
				sprintf((char*)&OutStr[i * 3],"%%%02x",a[i] & 0xff);
		}
	}
}


#endif
