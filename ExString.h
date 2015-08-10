#ifndef __EX_STRING_H__
#define __EX_STRING_H__

#include <string>
#include <sstream>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <wchar.h>
#include <typeinfo>
#include "ExTypeTraits.h"

#ifndef WIN32
#       include <cerrno>
#       ifndef ANDROID
#           include <iconv.h>
#       endif

#       define CP_UTF7                   3       // UTF-7 translation
#       define CP_UTF8                   0       // UTF-8 translation
#       define CP_1251                   1
#       define CP_KOI8                   2
#else
#include <Windows.h>
#include <Windowsx.h>
#endif


#define STR_TYPE(c,Str)			((std::is_equal<c, wchar_t>::value)?(c*)(L ## Str):(c*)(Str))
#define CHAR_TYPE(Type, Char)	((std::is_equal<Type, char>::value)?(Char):(L ## Char))

template<typename OutString>
void ConvertCodePageString(unsigned InCp, unsigned OutCp, const wchar_t * InStr, OutString & OutStr)
{
	std::basic_string<wchar_t> TmpStr(InStr);
	ConvertCodePageString(InCp, OutCp, TmpStr, OutStr);
}

template<typename InString, typename OutString>
void ConvertCodePageString(unsigned InCp, unsigned OutCp, const InString & InStr, OutString & OutStr)
{
	typedef typename InString::value_type _InCharType;
	typedef typename OutString::value_type _OutCharType;
#ifndef ANDROID
#ifndef WIN32
	const char * CodePageStr[] =
	{
		"UTF-8",
		"CP1251",
		"KOI-8",
		"UTF-7"
	};
	iconv_t convert_hnd = (iconv_t)-1;
	size_t SizeInBuf,  SizeOutBuf;
#endif

	if((std::is_equal<_InCharType, wchar_t>::value) && (std::is_equal<_OutCharType,wchar_t>::value))
	{
		OutStr = (_OutCharType*)InStr.c_str();
		return;
	}else if(std::is_equal<_InCharType, wchar_t>::value)
	{
#ifdef WIN32
		unsigned utf8_size = WideCharToMultiByte(OutCp, 0, (LPCWSTR)InStr.c_str(),InStr.length(), NULL, 0,NULL, NULL);
		OutStr.resize(utf8_size);
		WideCharToMultiByte(OutCp, 0, (LPCWSTR)InStr.c_str(),InStr.length(), (LPSTR)&OutStr[0], utf8_size,NULL, NULL);
#else
		convert_hnd = iconv_open(CodePageStr[OutCp], "WCHAR_T");
		if(convert_hnd == (iconv_t)-1)
			return;
		SizeInBuf = (SizeOutBuf = InStr.length() + 1) * sizeof(_InCharType);
		OutStr.resize(SizeOutBuf);
		SizeOutBuf *= sizeof(_OutCharType);
#endif
	}else if(std::is_equal<_OutCharType, wchar_t>::value)
	{

#ifdef WIN32
		unsigned size = MultiByteToWideChar(InCp, 0, (LPCSTR)InStr.c_str(),InStr.length(), NULL, 0);
		OutStr.resize(size);
		MultiByteToWideChar(InCp, 0, (LPCSTR)InStr.c_str(), InStr.length(), (LPWSTR)&OutStr[0], size);
#else
		convert_hnd = iconv_open("WCHAR_T", CodePageStr[InCp]);
		if(convert_hnd == (iconv_t)-1)
			return;
		SizeInBuf = (InStr.length() + 1) * sizeof(_InCharType);
		OutStr.resize(SizeInBuf);
		SizeOutBuf = SizeInBuf * sizeof(_OutCharType);
#endif
	}else
	{
		if(InCp == OutCp)
		{
			OutStr = (_OutCharType*)InStr.c_str();
			return;
		}
#ifdef WIN32
		unsigned size = MultiByteToWideChar(InCp, 0, (LPCSTR)InStr.c_str(),InStr.length(), NULL, 0);
		std::wstring unicode_str(size, '\0');
		MultiByteToWideChar(InCp, 0, (LPCSTR)InStr.c_str(), InStr.length(), &unicode_str[0], size);

		int utf8_size = WideCharToMultiByte(OutCp, 0, unicode_str.c_str(),unicode_str.length(), NULL, 0,NULL, NULL);
		OutStr.resize(utf8_size);
		WideCharToMultiByte(OutCp, 0, unicode_str.c_str(),unicode_str.length(),(LPSTR)&OutStr[0], utf8_size,NULL, NULL);
#else
		convert_hnd = iconv_open(CodePageStr[OutCp], CodePageStr[InCp]);
		if(convert_hnd == (iconv_t)-1)
			return;
		SizeInBuf = (InStr.length() + 1) * sizeof(_InCharType);
		OutStr.resize(SizeInBuf);
		SizeOutBuf = SizeInBuf * sizeof(_OutCharType);
#endif
	}

#ifndef WIN32

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
#endif
#else
	OutStr = (_OutCharType*)InStr.c_str();
#endif

}

template<typename OutString>
void ConvertCodePageString(unsigned InCp, unsigned OutCp, const char * InStr, OutString & OutStr)
{
	std::basic_string<char> TmpStr(InStr);
	ConvertCodePageString(InCp, OutCp, TmpStr, OutStr);
}

template<typename T>
std::basic_string<T> & operator<<(std::basic_string<T> & StrDest, const int Val)
{
	T Buf[20];
	if(std::is_equal<T, wchar_t>::value)
		swprintf((wchar_t*)Buf,20,L"%i", Val);
	else
		sprintf((char*)Buf,"%i", Val);
	StrDest += Buf;
	return StrDest;
}

template<typename T>
std::basic_string<T> & operator<<(std::basic_string<T> & StrDest, std::basic_string<T> & Val)
{
	StrDest += Val;
	return StrDest;
}


template<typename T>
std::basic_string<T> & operator<<(std::basic_string<T> & StrDest,const T * Val)
{
	StrDest += Val;
	return StrDest;
}

template<typename T>
int & operator<<(int & Dest, std::basic_string<T> & InStr)
{
	if(typeid(T) == typeid(wchar_t))
		swscanf((wchar_t*)InStr.c_str(),L"%i", &Dest);
	else
		sscanf((char*)InStr.c_str(),"%i", &Dest);
	return Dest;
}

template<typename T>
std::basic_string<T> & operator<<(std::basic_string<T> & StrDest, T Ch)
{
	StrDest += Ch;
	return StrDest;
}

template<typename _Elem, typename _Traits = std::char_traits<_Elem>, typename _Ax = std::allocator<_Elem> >
class ex_basic_string: public std::basic_string<_Elem,_Traits,_Ax>
{
public:
	template<typename Tp>
	inline ex_basic_string(Tp Val) : std::basic_string<_Elem,_Traits,_Ax>::basic_string(Val) {}

	template<typename Tp, typename Tp2>
	inline ex_basic_string(Tp Val, Tp2 Val2) : std::basic_string<_Elem,_Traits,_Ax>::basic_string(Val, Val2) {}

	inline ex_basic_string() {};


	void ParseInt(int Val, unsigned char RadX = 10)
	{
		resize(11);
		if(std::is_equal<_Elem, wchar_t>::value)
		{
			_itow(Val, (wchar_t*)c_str(), RadX);
		}else
		{
			itoa(Val, (char*)c_str(), RadX);
		}
	}



	int ToInt(unsigned char Radix = 10)
	{
		char Negative = 1;
		_Elem * pCur = (_Elem *)c_str();
		switch(*pCur)
		{
		case CHAR_TYPE(_Elem, '-'):
			Negative = -1;
		case CHAR_TYPE(_Elem, '+'):
			pCur++;
		}
		int Ret = 0;
		for(;;pCur++)
		{
			unsigned char Digit = *pCur - CHAR_TYPE(_Elem, '0');
			if(Digit > 9)
			{
				Digit = *pCur - (CHAR_TYPE(_Elem, 'a') - 10);
				if(Digit >= Radix)
					break;
			}
			Ret = Ret * Radix + Digit;
		}
		return Ret * Negative;
	}

	template<class T>
	static ex_basic_string ToString(T Val)
	{
		ex_basic_string s;
		s << Val;
		return s;
	}

	static ex_basic_string ToString()
	{
		ex_basic_string s;
		return s;
	}
};





//Converting from number to string
template<class TypeChar>
inline size_t NumberToString(int Number, TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
	return _i_NumberToString(Number, Str, Len, Radix) - Str;
}

template<class TypeChar>
inline size_t NumberToString(unsigned Number, TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
	return _i_NumberToString(Number, Str, Len, Radix) - Str;
}

template<class TypeChar>
inline size_t NumberToString(long Number, TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
	return _i_NumberToString(Number, Str, Len, Radix) - Str;
}

template<class TypeChar>
inline size_t NumberToString(unsigned long Number, TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
	return _i_NumberToString(Number, Str, Len, Radix) - Str;
}


template<class TypeChar>
inline size_t NumberToString(long long Number, TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
	return _i_NumberToString(Number, Str, Len, Radix) - Str;
}

template<class TypeChar>
inline size_t NumberToString(unsigned long long Number, TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
	return _i_NumberToString(Number, Str, Len, Radix) - Str;
}

 
template<class TypeNumber, class TypeChar>
inline TypeChar * _i_NumberToString(TypeNumber Number, TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
	TypeChar * pCur = Str, *MaxIndex = Str + Len;
	if(std::is_signed<TypeNumber>::value && (pCur < MaxIndex) && (Number < 0))
	{
		*(pCur++) = CHAR_TYPE(TypeChar,'-');
		Number = -Number;
	}

	TypeNumber t = 0;
	unsigned char CountDigit = 0;
	do
	{
	      t = t * Radix + Number % Radix;
		  CountDigit++;
	}while((Number /= Radix) > 0);
	do 
	{
		unsigned char  digval = (unsigned char)(t % Radix);
		*pCur = (digval > 9)? (digval + (CHAR_TYPE(TypeChar,'a') - 10)): (digval + CHAR_TYPE(TypeChar,'0'));
		CountDigit--;
	} while ((++pCur < MaxIndex) && (((t /= Radix) > 0) || (CountDigit > 0)));

	if(MaxIndex <= pCur)
		return pCur;
	*pCur = CHAR_TYPE(TypeChar,'\0');
	return pCur;
}

///Float point
template<class TypeChar>
inline size_t NumberToString(float Val, TypeChar * Str, size_t LenBuf, unsigned char RadX, long double Eps)
{
	return _e_NumberToString((long double)Val, Str, LenBuf, RadX, Eps) - Str;
}

template<class TypeChar>
inline size_t NumberToString(float Val, TypeChar * Str, size_t LenBuf, unsigned char RadX = 10)
{
	return _d_NumberToString((long double)Val, Str, LenBuf, RadX, 0.000000001) - Str;
}


template<class TypeChar>
inline size_t NumberToString(double Val, TypeChar * Str, size_t LenBuf, unsigned char RadX, long double Eps)
{
	return _e_NumberToString((long double)Val, Str, LenBuf, RadX, Eps) - Str;
}

template<class TypeChar>
inline size_t NumberToString(double Val, TypeChar * Str, size_t LenBuf, unsigned char RadX = 10)
{
	return _d_NumberToString((long double)Val, Str, LenBuf, RadX, 0.00000000000000001) - Str;
}

template<class TypeChar>
inline size_t NumberToString(long double Val, TypeChar * Str, size_t LenBuf, unsigned char RadX, long double Eps)
{
	return _e_NumberToString(Val, Str, LenBuf, RadX, Eps) - Str;
}

template<class TypeChar>
inline size_t NumberToString(long double Val, TypeChar * Str, size_t LenBuf, unsigned char RadX = 10)
{
	return _d_NumberToString(Val, Str, LenBuf, RadX, 0.00000000000000001) - Str;
}

template<class TypeChar, class TypeNumber>
inline size_t NumberToString(TypeNumber Val, std::basic_string<TypeChar> & Str, size_t LenBuf = 0,unsigned char RadX = 10)
{
   TypeChar Buf[70];
   size_t Ret = NumberToString(Val, Buf, 70, RadX);
   Str += Buf;
   return Ret;
}

template<class TypeChar, class TypeNumber>
inline size_t NumberToString(TypeNumber Val, std::basic_string<TypeChar> & Str, size_t LenBuf, unsigned char RadX, long double Eps)
{
   TypeChar Buf[70];
   size_t Ret = NumberToString(Val, Buf, 70, RadX, Eps);
   Str += Buf;
   return Ret;
}


template<class TypeChar, class TypeNumber, size_t BufSize>
inline size_t NumberToString(TypeNumber Val, TypeChar (&Buf)[BufSize])
{
   return NumberToString(Val, (TypeChar*)Buf, BufSize, 10);
}

template<class TypeChar>
inline TypeChar * _d_NumberToString(long double Number, TypeChar * Str, size_t Len, unsigned char Radix, long double Eps1)
{
	static const long double MinExp = 1.0e-5;
	static const long double MaxExp = 1.0e+15;
	static const long double Inf = 9999e+200 * 9999e+200 * 9999e+200;
	long double uNumber = (Number < 0)? -Number: Number;
	short Exp = 0;
	TypeChar * pCur = Str, *MaxIndex = Str + Len;

	if((pCur < MaxIndex) && (Number < 0.0))
		*(pCur++) = CHAR_TYPE(TypeChar, '-');

    if(Number != Number)   //Is NaN
	{	
		const TypeChar * IndVal = STR_TYPE(TypeChar, "1.#IND00");
		for(unsigned char i = 0, m = min((MaxIndex - pCur), sizeof("1.#IND00") - 1);i < m;i++)
			(*pCur++) = IndVal[i];
		*pCur = CHAR_TYPE(TypeChar,'\0');
		return pCur;
	}else if(uNumber == Inf) //Is infinity
	{
		const TypeChar * IndVal = STR_TYPE(TypeChar, "1.#INF00");
		for(unsigned char i = 0, m = min((MaxIndex - pCur), sizeof("1.#INF00") - 1);i < m;i++)
			(*pCur++) = IndVal[i];
		*pCur = CHAR_TYPE(TypeChar,'\0');
		return pCur;
	}else if((uNumber != 0.0) && ((uNumber < MinExp) || (uNumber > MaxExp)))//Is have expanent
	{
		Exp = ((*(short*)((char*)&uNumber + (sizeof(uNumber) - sizeof(Exp))) & 0x7ff0) >> 4) - 1023;
		Exp = (short)(log(pow(2.0,Exp)) / log(Radix));
		uNumber *= pow(Radix, -Exp);
	}

	unsigned long long Integer = (unsigned long long)uNumber;
	uNumber = uNumber - Integer + 1.0;

	{
		unsigned long long t = 0;
		unsigned char CountDigit = 0;
		for(;Integer > 0;Integer /= Radix, CountDigit++)
		{
			t = t * Radix + Integer % Radix;
			Eps1 *= Radix;
		}
		do 
		{
			unsigned char  digval = (unsigned char) (t % Radix);
			*pCur = (digval > 9)? (digval + (CHAR_TYPE(TypeChar,'a') - 10)): (digval + CHAR_TYPE(TypeChar,'0'));
			CountDigit--;
		} while ((++pCur < MaxIndex) && (((t /= Radix) > 0) || (CountDigit > 0)));
	}

	if(MaxIndex <= pCur)
		return pCur;
	*(pCur++) = CHAR_TYPE(TypeChar,'.');
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
	{
		unsigned long long t = 0;
		for(;Integer > 1;Integer /= Radix)
			t = t * Radix + Integer % Radix;
		do 
		{
			unsigned char  digval = (unsigned char) (t % Radix);
			*pCur = (digval > 9)? (digval + (CHAR_TYPE(TypeChar,'a') - 10)): (digval + CHAR_TYPE(TypeChar,'0'));
		} while ((++pCur < MaxIndex) && (t /= Radix) > 0);
	}

	if(MaxIndex <= pCur)
		return pCur;
	if(Exp)
	{
		*(pCur++) = CHAR_TYPE(TypeChar, 'e');
		if(MaxIndex <= pCur)
			return pCur;
		if(Exp > 0)
			*(pCur++) = CHAR_TYPE(TypeChar, '+');
		pCur = _i_NumberToString(Exp, pCur, MaxIndex - pCur, Radix);
		if(MaxIndex <= pCur)
			return pCur;
	}
	*pCur = CHAR_TYPE(TypeChar,'\0');
	return pCur;
}


template<class TypeChar>
inline TypeChar * _e_NumberToString(long double Number, TypeChar * Str, size_t Len, unsigned char Radix, long double Eps1)
{
	static const long double MinExp = 1.0e-5;
	static const long double MaxExp = 1.0e+15;
	static const long double Inf = 9999e+200 * 9999e+200 * 9999e+200;

	long double uNumber = (Number < 0)? -Number: Number;
	short Exp = 0;
	TypeChar * pCur = Str, *MaxIndex = Str + Len;

	if((pCur < MaxIndex) && (Number < 0.0))
		*(pCur++) = CHAR_TYPE(TypeChar, '-');

    if(Number != Number)   //Is NaN
	{	
	    const TypeChar * IndVal = STR_TYPE(TypeChar, "1.#IND00");
		for(unsigned char i = 0, m = min((MaxIndex - pCur), sizeof("1.#IND00") - 1);i < m;i++)
			(*pCur++) = IndVal[i];
		*pCur = CHAR_TYPE(TypeChar,'\0');
		return pCur;
	}else if(uNumber == Inf) //Is infinity
	{
		const TypeChar * IndVal = STR_TYPE(TypeChar, "1.#INF00");
		for(unsigned char i = 0, m = min((MaxIndex - pCur), sizeof("1.#INF00") - 1);i < m;i++)
			(*pCur++) = IndVal[i];
		*pCur = CHAR_TYPE(TypeChar,'\0');
		return pCur;
	}else if((uNumber != 0.0) && ((uNumber < MinExp) || (uNumber > MaxExp)))  //Is have expanent
	{
		Exp = ((*(short*)((char*)&uNumber + (sizeof(uNumber) - sizeof(Exp))) & 0x7ff0) >> 4) - 1023;
		Exp = (short)(log(pow(2.0,Exp)) / log(Radix));
		uNumber *= pow(Radix, -Exp);
	}

	unsigned long long Integer = (unsigned long long)uNumber;
	uNumber = uNumber - Integer + 1.0;
	{
		unsigned long long t = 0;
		unsigned char CountDigit = 0;
		for(;Integer > 0;Integer /= Radix, CountDigit++)
			t = t * Radix + Integer % Radix;
		do 
		{
			unsigned char  digval = (unsigned char) (t % Radix);
			*pCur = (digval > 9)? (digval + (CHAR_TYPE(TypeChar,'a') - 10)): (digval + CHAR_TYPE(TypeChar,'0'));
			CountDigit--;
		} while ((++pCur < MaxIndex) && (((t /= Radix) > 0) || (CountDigit > 0)));
	}

	if(MaxIndex <= pCur)
		return pCur;
	*(pCur++) = CHAR_TYPE(TypeChar,'.');
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
	{
		unsigned long long t = 0;
		for(;Integer > 1;Integer /= Radix)
			t = t * Radix + Integer % Radix;
		do 
		{
			unsigned char  digval = (unsigned char) (t % Radix);
			*pCur = (digval > 9)? (digval + (CHAR_TYPE(TypeChar,'a') - 10)): (digval + CHAR_TYPE(TypeChar,'0'));
		} while ((++pCur < MaxIndex) && (t /= Radix) > 0);
	}

	if(MaxIndex <= pCur)
		return pCur;
	if(Exp)
	{
		*(pCur++) = CHAR_TYPE(TypeChar, 'e');
		if(MaxIndex <= pCur)
			return pCur;
		if(Exp > 0)
			*(pCur++) = CHAR_TYPE(TypeChar, '+');
		pCur = _i_NumberToString(Exp, pCur, MaxIndex - pCur, Radix);
		if(MaxIndex <= pCur)
			return pCur;
	}
	*pCur = CHAR_TYPE(TypeChar,'\0');
	return pCur;
}


template<class TypeChar, class TypeNumber>
inline size_t StringToNumber(TypeNumber * Number, const TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
   if(std::is_floating_point<TypeNumber>::value)
	   return _d_StringToNumber<true>(Number, Str, Len, Radix) - Str;
   else
	   return _i_StringToNumber(Number, Str, Len, Radix) - Str;
}


template<class TypeChar, class TypeNumber, size_t BufSize>
inline size_t StringToNumber(TypeNumber * Number, const TypeChar (&Buf)[BufSize])
{
   if(std::is_floating_point<TypeNumber>::value)
	   return _d_StringToNumber<true>(Number, Buf, BufSize, 10) - Buf;
   else
	   return _i_StringToNumber(Number, Buf, BufSize, 10) - Buf;
}

template<class TypeChar, class TypeNumber>
inline size_t StringToNumber(TypeNumber * Number, const std::basic_string<TypeChar> & Str, size_t Len = 0, unsigned char Radix = 10)
{
     return StringToNumber(Number, Str.c_str(), Str.length(), Radix);
}

//From string to number

template<class TypeChar, class TypeNumber>
const TypeChar * _i_StringToNumber(TypeNumber * Dest, const TypeChar * Str, size_t Len, unsigned char Radix = 10)
{		
	char Negative = 1;
	const TypeChar * pCur = Str, *MaxChar = Str + Len;
	if(std::is_signed<TypeNumber>::value && (Len != 0))
		switch(*pCur)
		{
			case CHAR_TYPE(TypeChar, '-'):
				Negative = -1;
			case CHAR_TYPE(TypeChar, '+'):
				pCur++;
		}
	TypeNumber Ret = (TypeNumber)0;
	const TypeChar * Ind = pCur;
	for(;Ind < MaxChar;Ind++)
	{
		unsigned char Digit = *Ind - CHAR_TYPE(TypeChar, '0');
		if(Digit > 9)
		{
			Digit = *pCur - (CHAR_TYPE(TypeChar, 'a') - 10);
			if(Digit >= Radix)
				Digit = *Ind - (CHAR_TYPE(TypeChar, 'A') - 10);
			if(Digit >= Radix)
				break;
		}
		Ret = Ret * Radix + Digit;
	}
	if(Ind == pCur)
		return Str;
	*Dest =  Ret * Negative;
	return Ind;
}


template<bool InfInd, class TypeNumber, class TypeChar>
const TypeChar * _d_StringToNumber(TypeNumber * Dest, const TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
	const TypeChar * pCur = Str, *MaxIndex = Str + Len;
	long long IntegerPart = 0;
	static const long double Inf = 9999e+200 * 9999e+200 * 9999e+200;
	static const long double Ind = Inf * 0;
	static const long double Qnan = -Ind;

	pCur = _i_StringToNumber(&IntegerPart, pCur, MaxIndex - pCur, Radix);

	if(*pCur != CHAR_TYPE(TypeChar, '.'))
	{
		*Dest = (TypeNumber)IntegerPart;
		return pCur;
	}
	pCur++;

	long double Result = IntegerPart;

	if(InfInd && ((pCur + 4) <  MaxIndex) && (*pCur == CHAR_TYPE(TypeChar, '#')))
	{
		if((pCur[1] == CHAR_TYPE(TypeChar, 'I')) || (pCur[1] == CHAR_TYPE(TypeChar, 'i')))
		{
			if((pCur[2] == CHAR_TYPE(TypeChar, 'N')) || (pCur[2] == CHAR_TYPE(TypeChar, 'n')))
			{
				switch(pCur[3])
				{
				case 'F':
				case 'f':
					Result = IntegerPart * Inf;
					pCur += 4;
					goto lblSingOut;
				case 'D':
				case 'd':
					Result = IntegerPart * Ind;
					pCur += 4;
					goto lblSingOut;
				}
			}
		}else if(((pCur + 5) <  MaxIndex) && ((pCur[1] == CHAR_TYPE(TypeChar, 'Q')) || (pCur[1] == CHAR_TYPE(TypeChar, 'q'))))
		{
			if((pCur[2] == CHAR_TYPE(TypeChar, 'N')) || (pCur[2] == CHAR_TYPE(TypeChar, 'n')))
				if((pCur[3] == CHAR_TYPE(TypeChar, 'A')) || (pCur[3] == CHAR_TYPE(TypeChar, 'a')))
					if((pCur[4] == CHAR_TYPE(TypeChar, 'N')) || (pCur[4] == CHAR_TYPE(TypeChar, 'n')))
					{
						Result = IntegerPart * Qnan;
						pCur += 5;
						goto lblSingOut;
					}
		}
		*Dest = Result;
		return pCur;
	}

lblSingOut:

	//Get fraction part
	{
		unsigned long long FractPart = 1;
		unsigned CountNum = 0;
		for(;pCur < MaxIndex;pCur++, CountNum++)
		{
			unsigned char Digit = *pCur - CHAR_TYPE(TypeChar, '0');
			if(Digit > 9)
			{
				Digit = *pCur - (CHAR_TYPE(TypeChar, 'a') - 10);
				if(Digit >= Radix)
					Digit = *pCur - (CHAR_TYPE(TypeChar, 'A') - 10);
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

	if(pCur >= MaxIndex)
	{
		*Dest = Result;
		return pCur;
	}

	if((*pCur == CHAR_TYPE(TypeChar, 'e')) || (*pCur == CHAR_TYPE(TypeChar, 'E')))
	{
	   int Exp = 0;
	   const TypeChar * pCur_ = _i_StringToNumber(&Exp, pCur + 1, MaxIndex - (pCur + 1), Radix);
	   if(pCur_ != (pCur + 1))
	   {
		   Result *= pow((long double)Radix, Exp);
		   pCur = pCur_;
	   }
	}

	*Dest = Result;
	return pCur;
}


template<typename InString, typename OutString>
void CodeUrl(const InString & InStr, OutString & OutStr, unsigned InCodePage = CP_UTF8)
{
	typedef typename OutString::value_type _InCharType;
	if(std::is_equal<_InCharType, wchar_t>::value)
	{
		std::string a;
		ConvertCodePageString(0,CP_UTF8,InStr, a);
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
			ConvertCodePageString(InCodePage,CP_UTF8,InStr, a);
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
