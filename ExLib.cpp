

#include <typeinfo>
#include "ExString.h"

#ifndef _MSC_VER
#       include <cerrno>
#       ifndef ANDROID
#           include <iconv.h>
#       endif
#else
#include <Windows.h>
#include <Windowsx.h>
#endif



/*
*
*			ExString.h
*
*/
//Converting from number to string


template<bool IsScaleEps, typename TypeChar>
int _d_NumberToString(long double Number, TypeChar * Str, size_t Len, unsigned char Radix, long double Eps1);

template<typename TypeNumber, typename TypeChar>
int _i_NumberToString(TypeNumber Number, TypeChar * Str, size_t Len, unsigned char Radix = 10);

template<bool IsSkipSpace, typename TypeChar, typename TypeNumber>
int _i_StringToNumber(TypeNumber * Dest, const TypeChar * Str, size_t Len, unsigned char Radix = 10);

template<bool IsSkipSpace, bool InfInd, typename TypeNumber, typename TypeChar>
int _d_StringToNumber(TypeNumber * Dest, const TypeChar * Str, size_t Len, unsigned char Radix = 10);

template
<
	bool		IsSkipSpace,
	typename	TypeChar,
	typename	TypeNumber,
	typename	StreamType
>
int _i_StreamToNumber(TypeNumber * Dest, StreamType& InStream, unsigned char Radix);



template
<
	bool		IsSkipSpace,
	bool		InfInd,
	typename	TypeChar,
	typename	TypeNumber,
	typename	StreamType
>
int _d_StreamToNumber(TypeNumber* Dest, StreamType& InStream, unsigned char Radix);


/////////////////////////

struct __stream_io: public __stream_io__base
{
	template<typename TypeChar>
	static inline TypeChar GetChar(std::basic_istream<TypeChar>& s)
	{
		TypeChar c;
		s.get(c);
		return c;
	}

	template<typename TypeChar>
	static inline TypeChar GetChar(FILE* s)
	{
		if(std::is_equal<TypeChar, wchar_t>::value)
			return getwc(s);
		else
			return getc(s);
	}

	template<typename TypeChar>
	static inline void UnGetChar(std::basic_istream<TypeChar>& s, TypeChar v) { s.unget(); }
	static inline void UnGetChar(FILE* s, wchar_t v) { ungetwc(v, s); }
	static inline void UnGetChar(FILE* s, char v) { ungetc(v, s); }
	static inline bool PutChar(FILE* s, char v) { return fputc(v, s) != EOF; }
	static inline bool PutChar(FILE* s, wchar_t v) { return fputwc(v, s) != WEOF; }
	template<typename TypeChar>
	static inline bool PutChar(std::basic_ostream<TypeChar> & s, TypeChar c) { return !s.put(c).fail(); }
};



template<bool IsScaleEps, typename TypeChar>
int _d_NumberToString(long double Number, TypeChar * Str, size_t Len, unsigned char Radix, long double Eps1)
{
	static const long double MinExp = 1.0e-5;
	static const long double MaxExp = 1.0e+15;
	static const long double Inf = 9999e+200 * 9999e+200 * 9999e+200;
	static const long double Ind = Inf * 0;
	static const long double Qnan = -Ind;

	long double uNumber = (Number < 0) ? -Number : Number;
	short Exp = 0;
	int CountWrited = 0;

	if(CountWrited < Len)
	{
		if(Number < 0.0)
			Str[CountWrited++] = CHAR_TYPE(TypeChar, '-');
		if(CountWrited >= Len)
			return -CountWrited;
	} else
		return 0;

	if(Number != Number)   //Is NaN
	{
		const TypeChar * IndVal = STR_TYPE(TypeChar, "1.#IND00");
		for(unsigned char i = 0; (CountWrited < Len) && (i < sizeof("1.#IND00")); CountWrited++, i++)
			Str[CountWrited] = IndVal[i];
		return CountWrited;
	} else if(uNumber == Inf) //Is infinity
	{
		const TypeChar * IndVal = STR_TYPE(TypeChar, "1.#INF00");
		for(unsigned char i = 0; (CountWrited < Len) && (i < sizeof("1.#INF00")); CountWrited++, i++)
			Str[CountWrited] = IndVal[i];
		return CountWrited;
	} else if(uNumber == Qnan) //Is qnan
	{
		const TypeChar * IndVal = STR_TYPE(TypeChar, "1.#QNAN00");
		for(unsigned char i = 0; (CountWrited < Len) && (i < sizeof("1.#QNAN00")); CountWrited++, i++)
			Str[CountWrited] = IndVal[i];
		return CountWrited;
	} else if((uNumber != 0.0) && ((uNumber < MinExp) || (uNumber > MaxExp)))//Is have expanent
	{
		Exp = ((*(short*)((char*)&uNumber + (sizeof(uNumber) - sizeof(Exp))) & 0x7ff0) >> 4) - 1023;
		Exp = (short)(log(pow(2.0, Exp)) / log(Radix));
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
		for(; Integer > 0; Integer /= Radix, Eps1 *= Radix)
		{
			unsigned char digval = Integer % Radix;
			*(--c) = DIGIT_TO_ALPHA(TypeChar, digval);
		}
	} else
	{
		for(; Integer > 0; Integer /= Radix)
		{
			unsigned char digval = Integer % Radix;
			*(--c) = DIGIT_TO_ALPHA(TypeChar, digval);
		}
	}
	if(c == m)
		*(--c) = CHAR_TYPE(TypeChar, '0');

	for(; c < m; c++, CountWrited++)
		if(CountWrited >= Len)
			return -CountWrited;
		else
			Str[CountWrited] = *c;

	Str[CountWrited++] = CHAR_TYPE(TypeChar, '.');
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
	for(; Integer > 1; Integer /= Radix)
	{
		unsigned char digval = Integer % Radix;
		*(--c) = DIGIT_TO_ALPHA(TypeChar, digval);
	}
	if(c == m)
		*(--c) = CHAR_TYPE(TypeChar, '0');

	for(; c < m; c++, CountWrited++)
		if(CountWrited >= Len)
			return -CountWrited;
		else
			Str[CountWrited] = *c;
	if(Exp)
	{
		if(CountWrited >= Len) return -CountWrited;
		Str[CountWrited++] = CHAR_TYPE(TypeChar, 'e');
		if(CountWrited >= Len) return -CountWrited;
		if(Exp > 0)
			Str[CountWrited++] = CHAR_TYPE(TypeChar, '+');
		int y;
		if((y = _i_NumberToString(Exp, Str + CountWrited, Len - CountWrited, Radix)) <= 0)
			return -CountWrited + y;
		CountWrited += y;
	}
	if(CountWrited < Len)
		Str[CountWrited] = CHAR_TYPE(TypeChar, '\0');
	return CountWrited;
}

template<typename TypeNumber, typename TypeChar>
int _i_NumberToString(TypeNumber Number, TypeChar * Str, size_t Len, unsigned char Radix)
{
	TypeChar *s = Str, *b = s + Len;

	typename std::make_unsigned<TypeNumber>::type AbsNum;
	if(std::is_signed<TypeNumber>::value && (Number < 0) && (s < b))
	{
		*(s++) = CHAR_TYPE(TypeChar, '-');
		AbsNum = -Number;
	} else
		AbsNum = Number;

	TypeChar Buf[40];
	TypeChar *m = Buf + 40, *c = m;
	do
	{
		decltype(AbsNum % Radix) digval = AbsNum % Radix;
		*(--c) = DIGIT_TO_ALPHA(TypeChar, digval);
	} while((AbsNum /= Radix) > 0);
	for(; (c < m) && (s < b); c++, s++)
		*s = *c;
	if(s < b)
		*s = CHAR_TYPE(TypeChar, '\0');
	return s - Str;
}

template<bool IsSkipSpace, typename TypeChar, typename TypeNumber>
int _i_StringToNumber(TypeNumber * Dest, const TypeChar * Str, size_t Len, unsigned char Radix)
{
	char Negative = 1;
	int CountReaded = 0;
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
	for(; CountReaded < Len; CountReaded++)
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
	if(t == CountReaded) return -CountReaded;
	*Dest = Ret * Negative;
	return CountReaded;
}

template<bool IsSkipSpace, bool InfInd, typename TypeNumber, typename TypeChar>
int _d_StringToNumber(TypeNumber * Dest, const TypeChar * Str, size_t Len, unsigned char Radix)
{
	static const long double Inf = 9999e+200 * 9999e+200 * 9999e+200;
	static const long double Ind = Inf * 0;
	static const long double Qnan = -Ind;
	int CountReaded;
	long long IntegerPart = 0;
	if((CountReaded = _i_StringToNumber<IsSkipSpace>(&IntegerPart, Str, Len, Radix)) <= 0)
		return -CountReaded;

	if((CountReaded >= Len) || (Str[CountReaded] != CHAR_TYPE(TypeChar, '.')))
	{
		*Dest = TypeNumber(IntegerPart);
		return CountReaded;
	}
	CountReaded++;
	long double Result = IntegerPart;

	if(InfInd && ((CountReaded + 4) < Len) && (Str[CountReaded] == CHAR_TYPE(TypeChar, '#')))
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
		} else if
			(
			((CountReaded + 5) < Len) &&
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
		for(; CountReaded < Len; CountReaded++, CountNum++)
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
		for(; FractPart > 1; FractPart /= Radix)
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
		int r;
		if((r = _i_StringToNumber<false>(&Exp, Str + (CountReaded + 1), Len - (CountReaded + 1), Radix)) > 0)
		{
			Result *= pow((long double)Radix, Exp);
			CountReaded += r;
		}
	}
lblTrueOut:
	*Dest = TypeNumber(Result);
	return CountReaded;
}


template
<
	bool		IsSkipSpace,
	typename	TypeChar,
	typename	TypeNumber,
	typename	StreamType
>
int _i_StreamToNumber(TypeNumber * Dest, StreamType& InStream, unsigned char Radix)
{
	char Negative = 1;
	int CountReaded = 0;
	if(IsSkipSpace)
		CountReaded += _SkipSpace<TypeChar>(InStream);
	TypeChar Cur = __stream_io::GetChar<TypeChar>(InStream);
	CountReaded++;
	if(std::is_signed<TypeNumber>::value)
		switch(Cur)
		{
			case CHAR_TYPE(TypeChar, '-'):
				Negative = -1;
			case CHAR_TYPE(TypeChar, '+'):
				Cur = __stream_io::GetChar<TypeChar>(InStream);
				CountReaded++;
		}
	TypeNumber Ret = (TypeNumber)0;
	size_t t = CountReaded;
	for(;; Cur = __stream_io::GetChar<TypeChar>(InStream), CountReaded++)
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
	__stream_io::UnGetChar(InStream, Cur);
	if(t == CountReaded--) return -CountReaded;
	*Dest = Ret * Negative;
	return CountReaded;
}


template
<
	bool		IsSkipSpace,
	bool		InfInd,
	typename	TypeChar,
	typename	TypeNumber,
	typename	StreamType
>
int _d_StreamToNumber(TypeNumber* Dest, StreamType& InStream, unsigned char Radix)
{

	static const long double Inf = 9999e+200 * 9999e+200 * 9999e+200;
	static const long double Ind = Inf * 0;
	static const long double Qnan = -Ind;

	long long IntegerPart = 0;
	int CountReaded;
	if((CountReaded = _i_StreamToNumber<IsSkipSpace, TypeChar>(&IntegerPart, InStream, Radix)) < 0)
		return CountReaded;

	TypeChar Cur = __stream_io::GetChar<TypeChar>(InStream);
	if(Cur != CHAR_TYPE(TypeChar, '.'))
	{
		*Dest = TypeNumber(IntegerPart);
		__stream_io::UnGetChar(InStream, Cur);
		return CountReaded;
	}
	CountReaded++;
	long double Result = IntegerPart;
	Cur = __stream_io::GetChar<TypeChar>(InStream);
	CountReaded++;

	if(InfInd && (Cur == CHAR_TYPE(TypeChar, '#')))
	{
		Cur = __stream_io::GetChar<TypeChar>(InStream);
		CountReaded++;
		if(CMP_I(Cur, 'i'))
		{
			Cur = __stream_io::GetChar<TypeChar>(InStream);
			CountReaded++;
			if(CMP_I(Cur, 'n'))
			{
				Cur = __stream_io::GetChar<TypeChar>(InStream);
				CountReaded++;
				switch(Cur)
				{
					case 'F': case 'f':
						Result = IntegerPart * Inf;
						Cur = __stream_io::GetChar<TypeChar>(InStream);
						CountReaded++;
						goto lblSingOut;
					case 'D': case 'd':
						Result = IntegerPart * Ind;
						Cur = __stream_io::GetChar<TypeChar>(InStream);
						CountReaded++;
						goto lblSingOut;
				}
			}
		} else if(CMP_I(Cur, 'q'))
		{
			Cur = __stream_io::GetChar<TypeChar>(InStream);
			CountReaded++;
			if(CMP_I(Cur, 'n'))
			{
				Cur = __stream_io::GetChar<TypeChar>(InStream);
				CountReaded++;
				if(CMP_I(Cur, 'a'))
				{
					Cur = __stream_io::GetChar<TypeChar>(InStream);
					CountReaded++;
					if(CMP_I(Cur, 'n'))
					{
						Result = IntegerPart * Qnan;
						Cur = __stream_io::GetChar<TypeChar>(InStream);
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
		for(;; CountNum++, Cur = __stream_io::GetChar<TypeChar>(InStream), CountReaded++)
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
			FractPart = FractPart * Radix + Digit;
		}
		long double DoubleFract = 0.0;
		for(; FractPart > 1; FractPart /= Radix)
			DoubleFract = (DoubleFract + (long double)(FractPart % Radix)) * (long double)0.1;

		if(Result < 0.0)
			Result -= DoubleFract;
		else
			Result += DoubleFract;
	}

	if(CMP_I(Cur, 'e'))
	{
		int Exp;
		int r;
		if((r = _i_StreamToNumber<false, TypeChar>(&Exp, InStream, Radix)) > 0)
		{
			Result *= pow((long double)Radix, Exp);
			CountReaded += r;
		}
	} else
	{
		__stream_io::UnGetChar(InStream, Cur);
		CountReaded--;
	}
lblTrueOut:
	*Dest = TypeNumber(Result);
	return CountReaded;
}



template<typename _InCharType, typename _OutCharType>
void __StringConvertCodePage(unsigned InCp, unsigned OutCp, const std::basic_string<_InCharType> & InStr, std::basic_string<_OutCharType> & OutStr)
{
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
	size_t SizeInBuf, SizeOutBuf;
#	endif

	if(std::is_equal<_InCharType, wchar_t>::value && std::is_equal<_OutCharType, wchar_t>::value)
	{
		OutStr = (_OutCharType*)InStr.c_str();
		return;
	} else if(std::is_equal<_InCharType, wchar_t>::value)
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
	} else if(std::is_equal<_OutCharType, wchar_t>::value)
	{
#	ifdef WIN32
		unsigned NewSize = MultiByteToWideChar(InCp, 0, (LPCSTR)InStr.c_str(), InStr.length(), nullptr, 0);
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
	} else
	{
		if(InCp == OutCp)
		{
			OutStr = (_OutCharType*)InStr.c_str();
			return;
		}
#	ifdef WIN32
		unsigned size = MultiByteToWideChar(InCp, 0, (LPCSTR)InStr.c_str(), InStr.length(), nullptr, 0);
		std::wstring unicode_str(size, '\0');
		MultiByteToWideChar(InCp, 0, (LPCSTR)InStr.c_str(), InStr.length(), &unicode_str[0], size);

		int NewSize = WideCharToMultiByte(OutCp, 0, unicode_str.c_str(), unicode_str.length(), nullptr, 0, nullptr, nullptr);
		OutStr.resize(NewSize);
		WideCharToMultiByte(OutCp, 0, unicode_str.c_str(), unicode_str.length(), (LPSTR)OutStr.c_str(), NewSize, nullptr, nullptr);
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
			} else if(SizeInBuf == 0)
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

template<typename TypeChar, typename StreamType>
inline size_t _SkipSpace(StreamType&& Stream)
{
	size_t CountSkiped = 0;
	TypeChar c;
	while(true)
	{
		c = __stream_io::GetChar<TypeChar>(Stream);
		if(!IsSpace(c))
			break;
		CountSkiped++;
	}
	__stream_io::UnGetChar(Stream, c);
	return CountSkiped;
}

template<typename InString, typename OutString>
void CodeUrl(const InString & InStr, OutString & OutStr, unsigned InCodePage = CP_UTF8)
{
	typedef typename OutString::value_type _InCharType;
	if(std::is_equal<_InCharType, wchar_t>::value)
	{
		std::string a;
		StringConvertCodePage(0, CP_UTF8, InStr, a);
		unsigned SizeOutStr = a.length() * 3;
		OutStr.resize(SizeOutStr);
		if(std::is_equal<_InCharType, wchar_t>::value)
		{
			for(unsigned i = 0, e = a.length(); i < e; i++)
				swprintf((wchar_t*)&OutStr[i * 3], 4, L"%%%02x", a[i] & 0xff);
		} else
		{
			for(unsigned i = 0, e = a.length(); i < e; i++)
				sprintf((char*)&OutStr[i * 3], "%%%02x", a[i] & 0xff);
		}
	} else
	{
		std::string a;
		if(InCodePage != CP_UTF8)
			StringConvertCodePage(InCodePage, CP_UTF8, InStr, a);
		else
			a = (char*)InStr.c_str();
		unsigned SizeOutStr = a.length() * 3;
		OutStr.resize(SizeOutStr);
		if(std::is_equal<_InCharType, wchar_t>::value)
		{
			for(unsigned i = 0, e = a.length(); i < e; i++)
				swprintf((wchar_t*)&OutStr[i * 3], 4, L"%%%02x", a[i] & 0xff);
		} else
		{
			for(unsigned i = 0, e = a.length(); i < e; i++)
				sprintf((char*)&OutStr[i * 3], "%%%02x", a[i] & 0xff);
		}
	}
}




bool IsSpace(char c)
{
	switch(c)
	{
		case CHAR_TYPE(char, ' '):
		case CHAR_TYPE(char, '\t'):
		case CHAR_TYPE(char, '\n'):
		case CHAR_TYPE(char, '\v'):
		case CHAR_TYPE(char, '\f'):
		case CHAR_TYPE(char, '\r'):
			return true;
	}
	return false;
}

bool IsSpace(wchar_t c)
{
	switch(c)
	{
		case CHAR_TYPE(wchar_t, ' '):
		case CHAR_TYPE(wchar_t, '\t'):
		case CHAR_TYPE(wchar_t, '\n'):
		case CHAR_TYPE(wchar_t, '\v'):
		case CHAR_TYPE(wchar_t, '\f'):
		case CHAR_TYPE(wchar_t, '\r'):
			return true;
	}
	return false;
}


size_t SkipSpace(const char* String, size_t Len)
{
	size_t CountSkiped = 0;
	for(size_t i = 0; i < Len; i++)
	{
		if(!IsSpace(String[i]))
			return CountSkiped;
		CountSkiped++;
	}
	return 0;
}


size_t SkipSpace(const wchar_t* String, size_t Len)
{
	size_t CountSkiped = 0;
	for(size_t i = 0; i < Len; i++)
	{
		if(!IsSpace(String[i]))
			return CountSkiped;
		CountSkiped++;
	}
	return 0;
}


size_t SkipSpace(std::basic_istream<char> & Stream) { return _SkipSpace<char>(Stream); }
size_t SkipSpace(std::basic_istream<wchar_t> & Stream) { return _SkipSpace<wchar_t>(Stream); }
size_t SkipSpace(FILE * Stream) { return _SkipSpace<wchar_t>(Stream); }

void StringConvertCodePage(unsigned InCp, unsigned OutCp, const wchar_t* InStr, std::basic_string<char>& OutStr)
{
	std::basic_string<wchar_t> TmpStr(InStr);
	__StringConvertCodePage(InCp, OutCp, TmpStr, OutStr);
}

void StringConvertCodePage(unsigned InCp, unsigned OutCp, const wchar_t* InStr, std::basic_string<wchar_t>& OutStr)
{
	std::basic_string<wchar_t> TmpStr(InStr);
	__StringConvertCodePage(InCp, OutCp, TmpStr, OutStr);
}

void StringConvertCodePage(unsigned InCp, unsigned OutCp, const char * InStr, std::basic_string<char>& OutStr)
{
	std::basic_string<char> TmpStr(InStr);
	__StringConvertCodePage(InCp, OutCp, TmpStr, OutStr);
}


void StringConvertCodePage(unsigned InCp, unsigned OutCp, const char * InStr, std::basic_string<wchar_t>& OutStr)
{
	std::basic_string<char> TmpStr(InStr);
	__StringConvertCodePage(InCp, OutCp, TmpStr, OutStr);
}


int NumberToString(int Number, char* Str, size_t Len, unsigned char Radix) { return _i_NumberToString(Number, Str, Len, Radix); }
int NumberToString(int Number, wchar_t* Str, size_t Len, unsigned char Radix) { return _i_NumberToString(Number, Str, Len, Radix); }
int NumberToString(unsigned Number, char* Str, size_t Len, unsigned char Radix) { return _i_NumberToString(Number, Str, Len, Radix); }
int NumberToString(long Number, char* Str, size_t Len, unsigned char Radix) { return _i_NumberToString(Number, Str, Len, Radix); }
int NumberToString(long Number, wchar_t* Str, size_t Len, unsigned char Radix) { return _i_NumberToString(Number, Str, Len, Radix); }
int NumberToString(unsigned long Number, char* Str, size_t Len, unsigned char Radix) { return _i_NumberToString(Number, Str, Len, Radix); }
int NumberToString(unsigned long Number, wchar_t* Str, size_t Len, unsigned char Radix) { return _i_NumberToString(Number, Str, Len, Radix); }
int NumberToString(long long Number, char* Str, size_t Len, unsigned char Radix) { return _i_NumberToString(Number, Str, Len, Radix); }
int NumberToString(long long Number, wchar_t* Str, size_t Len, unsigned char Radix) { return _i_NumberToString(Number, Str, Len, Radix); }
int NumberToString(unsigned long long Number, char* Str, size_t Len, unsigned char Radix) { return _i_NumberToString(Number, Str, Len, Radix); }
int NumberToString(unsigned long long Number, wchar_t* Str, size_t Len, unsigned char Radix) { return _i_NumberToString(Number, Str, Len, Radix); }
int NumberToString(float Val, char* Str, size_t LenBuf, unsigned char RadX, long double Eps) { return _d_NumberToString<false>((long double)Val, Str, LenBuf, RadX, Eps); }
int NumberToString(float Val, wchar_t* Str, size_t LenBuf, unsigned char RadX, long double Eps) { return _d_NumberToString<false>((long double)Val, Str, LenBuf, RadX, Eps); }
int NumberToString(float Val, char* Str, size_t LenBuf, unsigned char RadX) { return _d_NumberToString<true>((long double)Val, Str, LenBuf, RadX, 0.000000001); }
int NumberToString(float Val, wchar_t* Str, size_t LenBuf, unsigned char RadX) { return _d_NumberToString<true>((long double)Val, Str, LenBuf, RadX, 0.000000001); }
int NumberToString(double Val, char* Str, size_t LenBuf, unsigned char RadX, long double Eps) { return _d_NumberToString<false>((long double)Val, Str, LenBuf, RadX, Eps); }
int NumberToString(double Val, wchar_t* Str, size_t LenBuf, unsigned char RadX, long double Eps) { return _d_NumberToString<false>((long double)Val, Str, LenBuf, RadX, Eps); }
int NumberToString(double Val, char* Str, size_t LenBuf, unsigned char RadX) { return _d_NumberToString<true>((long double)Val, Str, LenBuf, RadX, 0.00000000000000001); }
int NumberToString(double Val, wchar_t* Str, size_t LenBuf, unsigned char RadX) { return _d_NumberToString<true>((long double)Val, Str, LenBuf, RadX, 0.00000000000000001); }
int NumberToString(long double Val, char* Str, size_t LenBuf, unsigned char RadX, long double Eps) { return _d_NumberToString<false>(Val, Str, LenBuf, RadX, Eps); }
int NumberToString(long double Val, wchar_t* Str, size_t LenBuf, unsigned char RadX, long double Eps) { return _d_NumberToString<false>(Val, Str, LenBuf, RadX, Eps); }
int NumberToString(long double Val, char* Str, size_t LenBuf, unsigned char RadX) { return _d_NumberToString<true>(Val, Str, LenBuf, RadX, 0.00000000000000001); }
int NumberToString(long double Val, wchar_t* Str, size_t LenBuf, unsigned char RadX) { return _d_NumberToString<true>(Val, Str, LenBuf, RadX, 0.00000000000000001); }

int StringToNumber(int* Number, const char* Str, size_t Len, unsigned char Radix) { return _i_StringToNumber<true>(Number, Str, Len, Radix); }
int StringToNumber(int* Number, const wchar_t* Str, size_t Len, unsigned char Radix) { return _i_StringToNumber<true>(Number, Str, Len, Radix); }
int StringToNumber(unsigned int* Number, const char* Str, size_t Len, unsigned char Radix) { return _i_StringToNumber<true>(Number, Str, Len, Radix); }
int StringToNumber(unsigned int* Number, const wchar_t* Str, size_t Len, unsigned char Radix) { return _i_StringToNumber<true>(Number, Str, Len, Radix); }
int StringToNumber(long long* Number, const char* Str, size_t Len, unsigned char Radix) { return _i_StringToNumber<true>(Number, Str, Len, Radix); }
int StringToNumber(long long* Number, const wchar_t* Str, size_t Len, unsigned char Radix) { return _i_StringToNumber<true>(Number, Str, Len, Radix); }
int StringToNumber(unsigned long long* Number, const char* Str, size_t Len, unsigned char Radix) { return _i_StringToNumber<true>(Number, Str, Len, Radix); }
int StringToNumber(unsigned long long* Number, const wchar_t* Str, size_t Len, unsigned char Radix) { return _i_StringToNumber<true>(Number, Str, Len, Radix); }
int StringToNumber(float* Number, const char* Str, size_t Len, unsigned char Radix) { return _d_StringToNumber<true, true>(Number, Str, Len, Radix); }
int StringToNumber(float* Number, const wchar_t* Str, size_t Len, unsigned char Radix) { return _d_StringToNumber<true, true>(Number, Str, Len, Radix); }
int StringToNumber(long double* Number, const char* Str, size_t Len, unsigned char Radix) { return _d_StringToNumber<true, true>(Number, Str, Len, Radix); }
int StringToNumber(long double* Number, const wchar_t* Str, size_t Len, unsigned char Radix) { return _d_StringToNumber<true, true>(Number, Str, Len, Radix); }


int StreamToNumber(int* Dest, std::basic_istream<char> & Stream, unsigned char Radix) { return _i_StreamToNumber<true, char>(Dest, Stream, Radix); }
int StreamToNumber(int* Dest, std::basic_istream<wchar_t> & Stream, unsigned char Radix) { return _i_StreamToNumber<true, wchar_t>(Dest, Stream, Radix); }
int StreamToNumber(unsigned int* Dest, std::basic_istream<char> & Stream, unsigned char Radix) { return _i_StreamToNumber<true, char>(Dest, Stream, Radix); }
int StreamToNumber(unsigned int* Dest, std::basic_istream<wchar_t> & Stream, unsigned char Radix) { return _i_StreamToNumber<true, wchar_t>(Dest, Stream, Radix); }
int StreamToNumber(long long* Dest, std::basic_istream<char> & Stream, unsigned char Radix) { return _i_StreamToNumber<true, char>(Dest, Stream, Radix); }
int StreamToNumber(long long* Dest, std::basic_istream<wchar_t> & Stream, unsigned char Radix) { return _i_StreamToNumber<true, wchar_t>(Dest, Stream, Radix); }
int StreamToNumber(unsigned long long* Dest, std::basic_istream<char> & Stream, unsigned char Radix) { return _i_StreamToNumber<true, char>(Dest, Stream, Radix); }
int StreamToNumber(unsigned long long* Dest, std::basic_istream<wchar_t> & Stream, unsigned char Radix) { return _i_StreamToNumber<true, wchar_t>(Dest, Stream, Radix); }
int StreamToNumber(float* Dest, std::basic_istream<char> & Stream, unsigned char Radix) { return _d_StreamToNumber<true, true, char>(Dest, Stream, Radix); }
int StreamToNumber(float* Dest, std::basic_istream<wchar_t> & Stream, unsigned char Radix) { return _d_StreamToNumber<true, true, wchar_t>(Dest, Stream, Radix); }
int StreamToNumber(long double* Dest, std::basic_istream<char> & Stream, unsigned char Radix) { return _d_StreamToNumber<true, true, char>(Dest, Stream, Radix); }
int StreamToNumber(long double* Dest, std::basic_istream<wchar_t> & Stream, unsigned char Radix) { return _d_StreamToNumber<true, true, wchar_t>(Dest, Stream, Radix); }


int StreamToNumber(int* Dest, FILE* Stream, unsigned char Radix) { return _i_StreamToNumber<true, wchar_t>(Dest, Stream, Radix); }
int StreamToNumber(unsigned int* Dest, FILE* Stream, unsigned char Radix) { return _i_StreamToNumber<true, wchar_t>(Dest, Stream, Radix); }
int StreamToNumber(long long* Dest, FILE* Stream, unsigned char Radix) { return _i_StreamToNumber<true, wchar_t>(Dest, Stream, Radix); }
int StreamToNumber(unsigned long long* Dest, FILE* Stream, unsigned char Radix) { return _i_StreamToNumber<true, wchar_t>(Dest, Stream, Radix); }
int StreamToNumber(float* Dest, FILE* Stream, unsigned char Radix) { return _d_StreamToNumber<true, true, wchar_t>(Dest, Stream, Radix); }
int StreamToNumber(long double* Dest, FILE* Stream, unsigned char Radix) { return _d_StreamToNumber<true, true, wchar_t>(Dest, Stream, Radix); }


int StreamDoubleToNumber(int* Dest, FILE* Stream, unsigned char Radix) { return _d_StreamToNumber<true, true, wchar_t>(Dest, Stream, Radix); }
int StreamDoubleToNumber(unsigned int* Dest, FILE* Stream, unsigned char Radix) { return _d_StreamToNumber<true, true, wchar_t>(Dest, Stream, Radix); }
int StreamDoubleToNumber(long long* Dest, FILE* Stream, unsigned char Radix) { return _d_StreamToNumber<true, true, wchar_t>(Dest, Stream, Radix); }
int StreamDoubleToNumber(unsigned long long* Dest, FILE* Stream, unsigned char Radix) { return _d_StreamToNumber<true, true, wchar_t>(Dest, Stream, Radix); }
int StreamDoubleToNumber(float* Dest, FILE* Stream, unsigned char Radix) { return _d_StreamToNumber<true, true, wchar_t>(Dest, Stream, Radix); }
int StreamDoubleToNumber(long double* Dest, FILE* Stream, unsigned char Radix) { return _d_StreamToNumber<true, true, wchar_t>(Dest, Stream, Radix); }



int StreamDoubleToNumber(int* Dest, std::basic_istream<char>& Stream, unsigned char Radix) { return _d_StreamToNumber<true, true, char>(Dest, Stream, Radix); }
int StreamDoubleToNumber(int* Dest, std::basic_istream<wchar_t>& Stream, unsigned char Radix) { return _d_StreamToNumber<true, true, wchar_t>(Dest, Stream, Radix); }
int StreamDoubleToNumber(unsigned int* Dest, std::basic_istream<char>& Stream, unsigned char Radix) { return _d_StreamToNumber<true, true, char>(Dest, Stream, Radix); }
int StreamDoubleToNumber(unsigned int* Dest, std::basic_istream<wchar_t>& Stream, unsigned char Radix) { return _d_StreamToNumber<true, true, wchar_t>(Dest, Stream, Radix); }
int StreamDoubleToNumber(long long* Dest, std::basic_istream<char>& Stream, unsigned char Radix) { return _d_StreamToNumber<true, true, char>(Dest, Stream, Radix); }
int StreamDoubleToNumber(long long* Dest, std::basic_istream<wchar_t>& Stream, unsigned char Radix) { return _d_StreamToNumber<true, true, wchar_t>(Dest, Stream, Radix); }
int StreamDoubleToNumber(unsigned long long* Dest, std::basic_istream<char>& Stream, unsigned char Radix) { return _d_StreamToNumber<true, true, char>(Dest, Stream, Radix); }
int StreamDoubleToNumber(unsigned long long* Dest, std::basic_istream<wchar_t>& Stream, unsigned char Radix) { return _d_StreamToNumber<true, true, wchar_t>(Dest, Stream, Radix); }
int StreamDoubleToNumber(float* Dest, std::basic_istream<char>& Stream, unsigned char Radix) { return _d_StreamToNumber<true, true, char>(Dest, Stream, Radix); }
int StreamDoubleToNumber(float* Dest, std::basic_istream<wchar_t>& Stream, unsigned char Radix) { return _d_StreamToNumber<true, true, wchar_t>(Dest, Stream, Radix); }
int StreamDoubleToNumber(long double* Dest, std::basic_istream<char>& Stream, unsigned char Radix) { return _d_StreamToNumber<true, true, char>(Dest, Stream, Radix); }
int StreamDoubleToNumber(long double* Dest, std::basic_istream<wchar_t>& Stream, unsigned char Radix) { return _d_StreamToNumber<true, true, wchar_t>(Dest, Stream, Radix); }


int StringDoubleToNumber(int* Number, const char* Str, size_t Len, unsigned char Radix) { return _d_StringToNumber<true, true>(Number, Str, Len, Radix); }
int StringDoubleToNumber(int* Number, const wchar_t* Str, size_t Len, unsigned char Radix) { return _d_StringToNumber<true, true>(Number, Str, Len, Radix); }
int StringDoubleToNumber(unsigned int* Number, const char* Str, size_t Len, unsigned char Radix) { return _d_StringToNumber<true, true>(Number, Str, Len, Radix); }
int StringDoubleToNumber(unsigned int* Number, const wchar_t* Str, size_t Len, unsigned char Radix) { return _d_StringToNumber<true, true>(Number, Str, Len, Radix); }
int StringDoubleToNumber(long long* Number, const char* Str, size_t Len, unsigned char Radix) { return _d_StringToNumber<true, true>(Number, Str, Len, Radix); }
int StringDoubleToNumber(long long* Number, const wchar_t* Str, size_t Len, unsigned char Radix) { return _d_StringToNumber<true, true>(Number, Str, Len, Radix); }
int StringDoubleToNumber(unsigned long long* Number, const char* Str, size_t Len, unsigned char Radix) { return _d_StringToNumber<true, true>(Number, Str, Len, Radix); }
int StringDoubleToNumber(unsigned long long* Number, const wchar_t* Str, size_t Len, unsigned char Radix) { return _d_StringToNumber<true, true>(Number, Str, Len, Radix); }
int StringDoubleToNumber(float* Number, const char* Str, size_t Len, unsigned char Radix) { return _d_StringToNumber<true, true>(Number, Str, Len, Radix); }
int StringDoubleToNumber(float* Number, const wchar_t* Str, size_t Len, unsigned char Radix) { return _d_StringToNumber<true, true>(Number, Str, Len, Radix); }
int StringDoubleToNumber(long double* Number, const char* Str, size_t Len, unsigned char Radix) { return _d_StringToNumber<true, true>(Number, Str, Len, Radix); }
int StringDoubleToNumber(long double* Number, const wchar_t* Str, size_t Len, unsigned char Radix) { return _d_StringToNumber<true, true>(Number, Str, Len, Radix); }



char* UnixPathToSystemPath(const char* Path, char* Dest, size_t LenDest)
{
#ifdef _MSC_VER
	if((Path[0] == '/') && ((Path[1] >= 'a') && (Path[1] <= 'z') || (Path[1] >= 'A') && (Path[1] <= 'Z')) && ((Path[2] == '/') || (Path[2] == '\0')))
	{
		if(Path[2] == '/')
			sprintf_s(Dest, LenDest, "%c:%s", Path[1], Path + 2);
		else
			sprintf_s(Dest, LenDest, "%c:\\", Path[1]);
	} else
		strncpy(Dest, Path, LenDest);
	for(char* r; (r = strchr(Dest, '/')) != nullptr; ) *r = '\\';
#else
	strncpy(Dest, Path, LenDest);
#endif
	return Dest;
}


char* SystemPathToUnixPath(const char* Path, char* Dest, size_t LenDest)
{
#ifdef _MSC_VER
	if(((Path[0] >= 'a') && (Path[0] <= 'z') || (Path[0] >= 'A') && (Path[0] <= 'Z')) && (Path[1] == ':'))
		sprintf_s(Dest, LenDest, "/%c%s", Path[0], Path + 2);
	else
		strncpy(Dest, Path, LenDest);
	for(char* r; (r = strchr(Dest, '\\')) != nullptr; ) *r = '//';
#else
	strncpy(Dest, Path, LenDest);
#endif
	return Dest;
}


/*
*
*											end ExString.h
*
*/


#include "ExQueryUrl.h"

#ifdef WIN_PLATFORM
#	include <io.h>
#	include <fcntl.h>


#	pragma comment(lib, "Ws2_32.lib")

#define poll winsock::WSAPoll
#define inet_pton winsock::inet_pton
using winsock::getaddrinfo;
using winsock::inet_ntop;


namespace winsock
{
	int GetLastErrSocket()
	{
		switch(WSAGetLastError())
		{
			case WSAEADDRINUSE:
#ifdef EADDRINUSE
				return EADDRINUSE;
#else
				break;
#endif
			case WSAEADDRNOTAVAIL:
#ifdef EADDRNOTAVAIL
				return EADDRNOTAVAIL;
#else
				break;
#endif
			case WSAEAFNOSUPPORT:
#ifdef EAFNOSUPPORT
				return EAFNOSUPPORT;
#else
				break;
#endif
			case WSAEALREADY:
#ifdef EALREADY
				return EALREADY;
#else
				break;
#endif
			case WSAEBADF:			return EBADF;
			case WSAECONNABORTED:
#ifdef ECONNABORTED
				return ECONNABORTED;
#else
				break;
#endif
			case WSAECONNREFUSED:
#ifdef ECONNREFUSED
				return ECONNREFUSED;
#else
				break;
#endif
			case WSAECONNRESET:
#ifdef ECONNRESET
				return ECONNRESET;
#else
				break;
#endif
			case WSAEDESTADDRREQ:
#ifdef EDESTADDRREQ
				return EDESTADDRREQ;
#else
				break;
#endif
			case WSAEFAULT:			return EFAULT;
			case WSAEHOSTDOWN:
#ifdef EHOSTDOWN
				return EHOSTDOWN;
#else
				break;
#endif
			case WSAEHOSTUNREACH:
#ifdef EHOSTUNREACH
				return EHOSTUNREACH;
#else
				break;
#endif
			case WSAEINPROGRESS:
#ifdef EINPROGRESS
				return EINPROGRESS;
#else
				break;
#endif
			case WSAEINTR:			return EINTR;
			case WSAEINVAL:			return EINVAL;
			case WSAEISCONN:
#ifdef EISCONN
				return EISCONN;
#else
				break;
#endif
			case WSAELOOP:
#ifdef ELOOP
				return ELOOP;
#else
				break;
#endif
			case WSAEMFILE:			return EMFILE;
			case WSAEMSGSIZE:
#ifdef EMSGSIZE
				return EMSGSIZE;
#else
				break;
#endif
			case WSAENAMETOOLONG:	return ENAMETOOLONG;
			case WSAENETDOWN:
#ifdef ENETDOWN
				return ENETDOWN;
#else
				break;
#endif
			case WSAENETRESET:
#ifdef ENETRESET
				return ENETRESET;
#else
				break;
#endif
			case WSAENETUNREACH:
#ifdef ENETUNREACH
				return ENETUNREACH;
#else
				break;
#endif
			case WSAENOBUFS:
#ifdef ENOBUFS
				return ENOBUFS;
#else
				break;
#endif
			case WSAENOPROTOOPT:
#ifdef ENOPROTOOPT
				return ENOPROTOOPT;
#else
				break;
#endif
			case WSAENOTCONN:
#ifdef ENOTCONN
				return ENOTCONN;
#else
				break;
#endif
			case WSANOTINITIALISED:	return EAGAIN;
			case WSAENOTSOCK:
#ifdef ENOTSOCK
				return ENOTSOCK;
#else
				break;
#endif
			case WSAEOPNOTSUPP:		return EOPNOTSUPP;
			case WSAEPFNOSUPPORT:
#ifdef EPFNOSUPPORT 
				return EPFNOSUPPORT;
#else 
				break;
#endif
			case WSAEPROTONOSUPPORT:
#ifdef EPROTONOSUPPORT
				return EPROTONOSUPPORT;
#else
				break;
#endif
			case WSAEPROTOTYPE:
#ifdef EPROTOTYPE
				return EPROTOTYPE;
#else
				break;
#endif
			case WSAESHUTDOWN:
#ifdef ESHUTDOWN
				return ESHUTDOWN;
#else
				break;
#endif
			case WSAESOCKTNOSUPPORT:
#ifdef ESOCKTNOSUPPORT
				return ESOCKTNOSUPPORT;
#else
				break;
#endif
			case WSAETIMEDOUT:
#ifdef ETIMEDOUT
				return ETIMEDOUT;
#else
				break;
#endif
			case WSAETOOMANYREFS:
#ifdef ETOOMANYREFS
				return ETOOMANYREFS;
#else
				break;
#endif
			case ERROR_IO_PENDING:
			case WSAEWOULDBLOCK:
#ifdef EWOULDBLOCK
				return EWOULDBLOCK;
#else
				return EAGAIN;
#endif
			case WSAHOST_NOT_FOUND:
#ifdef EHOSTUNREACH
				return EHOSTUNREACH;
#else
				break;
#endif
			case WSASYSNOTREADY:
			case WSATRY_AGAIN:		return EAGAIN;
			case WSAVERNOTSUPPORTED:
#ifdef DB_OPNOTSUP
				return DB_OPNOTSUP;
#else
				break;
#endif
			case WSAEACCES:			return EACCES;
			case 0:
				return 0;
		}

		return EFAULT;
	}
};

static netent* readnetnetworklist_()
{
	char Name[BUFSIZ + 1], c, NetworksPath[MAX_PATH];
	::netent * NewNet = (::netent*)malloc(sizeof(::netent));
	unsigned CurNnet = 0;
	char * WinDirPath = getenv("windir");
	if(WinDirPath == NULL)
		goto lblOut;
	StringCopy(NetworksPath, WinDirPath);
	StringAppend(NetworksPath, "\\System32\\drivers\\etc\\networks");
	FILE * FNetworks = fopen(NetworksPath, "rt");
	if(FNetworks != NULL)
	{
		while(!feof(FNetworks))
		{
			SkipSpace(FNetworks);
			if((c = fgetc(FNetworks)) == '#')
			{
				fscanf(FNetworks, "%*[^\n]");
				continue;
			}
			ungetc(c, FNetworks);
			unsigned short IpAddr[4] = {0};
			int CountReaded = fscanf(FNetworks, "%[^ #\n\t\v\f\r] %hu.%hu.%hu.%hu", Name, IpAddr, IpAddr + 1, IpAddr + 2, IpAddr + 3);
			if(CountReaded < 2)
				continue;
			NewNet[CurNnet].n_name = StringDuplicate(Name);
			unsigned NumberAliases = 0;
			NewNet[CurNnet].n_aliases = (char**)malloc(sizeof(char*));
			while(true)
			{
				CountReaded = fscanf(FNetworks, "%*[ \t\v\f\r]%[^ #\n\t\v\f\r]", Name);
				if(CountReaded <= 0)
					break;
				NewNet[CurNnet].n_aliases[NumberAliases] = StringDuplicate(Name);
				NumberAliases++;
				NewNet[CurNnet].n_aliases = (char**)realloc(NewNet[CurNnet].n_aliases, (NumberAliases + 1) * sizeof(char*));
			}
			NewNet[CurNnet].n_aliases[NumberAliases] = NULL;
			NewNet[CurNnet].n_addrtype = AF_INET;
			NewNet[CurNnet].n_net = ((unsigned char)IpAddr[3] << 24) | ((unsigned char)IpAddr[2] << 16) | ((unsigned char)IpAddr[1] << 8) | ((unsigned char)IpAddr[0] << 0);
			CurNnet++;
			NewNet = (decltype(NewNet))realloc(NewNet, (CurNnet + 1) * sizeof(::netent));
		}
		fclose(FNetworks);
	}
lblOut:
	memset(NewNet + CurNnet, 0, sizeof(NewNet[CurNnet]));
	return NewNet;
}
#define readnetnetworklist readnetnetworklist_

static netent* GetNetworksInfo() { static struct ::netent * gn = readnetnetworklist(); return gn; }

static netent* getnetbyname_(const char *name)
{
	for(::netent * gn = GetNetworksInfo(); gn->n_name; gn++)
		if(StringCompare(name, gn->n_name) == 0)
			return gn;
	return NULL;
}
#define getnetbyname getnetbyname_

static netent* getnetbyaddr_(long net, int type)
{
	for(::netent * gn = GetNetworksInfo(); gn->n_name; gn++)
		if((gn->n_net == net) && (gn->n_addrtype == type))
			return gn;
	return NULL;
}
#define getnetbyaddr getnetbyaddr_

static servent* readservicelist_()
{
	char Name[BUFSIZ + 1], ProtocolName[BUFSIZ + 1], c, NetworksPath[MAX_PATH];
	::servent * NewServEnt = (::servent*)malloc(sizeof(::servent));
	unsigned CurNnet = 0;
	char * WinDirPath = getenv("windir");
	if(WinDirPath == NULL)
		goto lblOut;
	StringCopy(NetworksPath, WinDirPath);
	StringAppend(NetworksPath, "\\System32\\drivers\\etc\\services");
	FILE * FServices = fopen(NetworksPath, "rt");
	if(FServices != NULL)
	{
		while(!feof(FServices))
		{
			SkipSpace(FServices);
			if((c = fgetc(FServices)) == '#')
			{
				fscanf(FServices, "%*[^\n]");
				continue;
			}
			ungetc(c, FServices);
			unsigned short Port;
			int CountReaded = fscanf(FServices, "%[^ #\n\t\v\f\r] %hu/%s", Name, &Port, ProtocolName);
			NewServEnt[CurNnet].s_port = htons(Port);
			if(CountReaded < 3)
				continue;
			NewServEnt[CurNnet].s_name = NULL;
			NewServEnt[CurNnet].s_proto = NULL;
			for(unsigned i = 0; (i < CurNnet) && (!NewServEnt[CurNnet].s_proto || !NewServEnt[CurNnet].s_name); i++)
			{
				if(StringICompare(Name, NewServEnt[i].s_name) == 0)
					NewServEnt[CurNnet].s_name = NewServEnt[i].s_name;//For economy memory :)
				if(StringICompare(ProtocolName, NewServEnt[i].s_proto) == 0)
					NewServEnt[CurNnet].s_proto = NewServEnt[i].s_proto;//For economy memory :)
			}
			if(NewServEnt[CurNnet].s_name == NULL)
				NewServEnt[CurNnet].s_name = StringDuplicate(Name);
			if(NewServEnt[CurNnet].s_proto == NULL)
				NewServEnt[CurNnet].s_proto = StringDuplicate(ProtocolName);
			unsigned NumberAliases = 0;
			NewServEnt[CurNnet].s_aliases = (char**)malloc(sizeof(char*));
			while(true)
			{
				CountReaded = fscanf(FServices, "%*[ \t\v\f\r]%[^ #\n\t\v\f\r]", Name);
				if(CountReaded <= 0)
					break;
				NewServEnt[CurNnet].s_aliases[NumberAliases] = StringDuplicate(Name);
				NumberAliases++;
				NewServEnt[CurNnet].s_aliases = (char**)realloc(NewServEnt[CurNnet].s_aliases, (NumberAliases + 1) * sizeof(char*));
			}
			NewServEnt[CurNnet].s_aliases[NumberAliases] = NULL;
			CurNnet++;
			NewServEnt = (decltype(NewServEnt))realloc(NewServEnt, (CurNnet + 1) * sizeof(::servent));
		}
		fclose(FServices);
	}
lblOut:
	memset(NewServEnt + CurNnet, 0, sizeof(NewServEnt[CurNnet]));
	return NewServEnt;
}
#define readservicelist readservicelist_

static servent* GetServiceInfo() { static struct ::servent * gn = readservicelist(); return gn; }

static servent* getservbyport_(int port, const char* proto)
{
	for(::servent * gn = GetServiceInfo(); gn->s_name; gn++)
		if(gn->s_port == port)
		{
			if(proto == NULL)
				return gn;
			if(StringICompare(proto, gn->s_proto) == 0)
				return gn;
		}
	return NULL;
}
#define getservbyport getservbyport_

static servent* getservbyname_(const char* Name, const char* proto)
{
	for(::servent * gn = GetServiceInfo(); gn->s_name; gn++)
		if(StringICompare(Name, gn->s_name) == 0)
		{
			if(proto == NULL)
				return gn;
			if(StringICompare(proto, gn->s_proto) == 0)
				return gn;
		}
	return NULL;
}
#define getservbyname getservbyname_

static void* GetWsa()
{
	static LPWSADATA wd = nullptr;
	if(wd == nullptr)
	{
		wd = (decltype(wd))malloc(sizeof(WSADATA));
		if(WSAStartup(WSA_VERSION, wd) == 0)
			return wd;
		return nullptr;
	}
	return wd;
	return (void*)1;
}

static void EndWsa()
{
	if(GetWsa() != nullptr)
		WSACleanup();
}

int ____f = ([] { GetWsa(); return 0; })();

#else
#	define closesocket(socket)  close(socket)
#endif

QUERY_URL::SOCKET_ADDR::___PORT::_READABLE::operator QUERY_URL::TPORT() const
{
	switch(Addr.sa_family)
	{
		case AF_INET: return htons(AddrInet.sin_port);
		case AF_INET6: return htons(AddrInet6.sin6_port);
	}
	return 0;
}

QUERY_URL::SOCKET_ADDR::___PORT::operator QUERY_URL::TPORT() const
{
	switch(Readable.Addr.sa_family)
	{
		case AF_INET: return Readable.AddrInet.sin_port;
		case AF_INET6: return Readable.AddrInet6.sin6_port;
	}
	return 0;
}

QUERY_URL::TPORT QUERY_URL::SOCKET_ADDR::___PORT::operator=(TPORT Prt)
{
	switch(Readable.Addr.sa_family)
	{
		case AF_INET: return Readable.AddrInet.sin_port = Prt;
		case AF_INET6: return Readable.AddrInet6.sin6_port = Prt;
	}
	return 0;
}
char* QUERY_URL::SOCKET_ADDR::___PORT::ToString(char* DestBuf, size_t LenBuf) const
{
	switch(Readable.Addr.sa_family)
	{
		case AF_INET: sprintf_s(DestBuf, LenBuf, "%i", (int)htons(Readable.AddrInet.sin_port)); return DestBuf;
		case AF_INET6: sprintf_s(DestBuf, LenBuf, "%i", (int)htons(Readable.AddrInet6.sin6_port)); return DestBuf;
	}
	return nullptr;
}
std::basic_string<char> QUERY_URL::SOCKET_ADDR::___PORT::ToString() const { char b[25]; b[0] = '\0'; ToString(b, 25); return b; }

char* QUERY_URL::SOCKET_ADDR::_IP::ToString(char * Dest, size_t Len) const
{
	void* m = *this;
	if(m == nullptr) return nullptr;
	return (char*)inet_ntop(Addr.sa_family, m, Dest, Len);
}
std::basic_string<char> QUERY_URL::SOCKET_ADDR::_IP::ToString() const
{
	char b[INET6_ADDRSTRLEN + 2]; b[0] = '\0';
	ToString(b, INET6_ADDRSTRLEN + 1);
	return b;
}
const char* QUERY_URL::SOCKET_ADDR::_IP::FromString(const char* AddrStr)
{
	void* m = *this;
	if(m == nullptr) return nullptr;
	return (inet_pton(Addr.sa_family, AddrStr, m) == 1) ? AddrStr : nullptr;
}

const std::basic_string<char>& QUERY_URL::SOCKET_ADDR::_IP::FromString(const std::basic_string<char>& AddrStr) { FromString(AddrStr.c_str()); return AddrStr; }


const char* QUERY_URL::ADDRESS_INFO::_PORT_NAME::operator= (const char* New)
{
	PortName->operator=(New);
	return New;
}
std::basic_string<char>& QUERY_URL::ADDRESS_INFO::_PORT_NAME::operator= (std::basic_string<char>& New)
{
	PortName->operator=(New);
	return New;
}

const char* QUERY_URL::ADDRESS_INFO::_HOST_NAME::operator= (const char* New)
{
	HostName->operator=(New);
	return New;
}
std::basic_string<char>& QUERY_URL::ADDRESS_INFO::_HOST_NAME::operator= (std::basic_string<char>& New)
{
	PortName->operator=(New);
	return New;
}

QUERY_URL::ADDRESS_INFO::ADDRESSES::_COUNT::operator int()
{
	int Count = 0;
	for(decltype(ai) i = ai; i; Count++, i = i->ai_next);
	return Count;
}

QUERY_URL::ADDRESS_INFO::ADDRESS_INTERATOR QUERY_URL::ADDRESS_INFO::ADDRESSES::operator[](unsigned Index)
{
	int n = 0;
	for(addrinfo * i = Count.ai; i; n++, i = i->ai_next)
		if(n == Index)
			return ADDRESS_INTERATOR(i);
	return ADDRESS_INTERATOR(nullptr);
}


QUERY_URL::ADDRESS_INFO::ADDRESS_INFO(const char * FullAddress)
{
	const char * _Pos;
	int Pos;
	TPORT Port;
	InitFields();

	if((_Pos = strstr(FullAddress, "://")) != nullptr)
	{
		HostName.PortName->append(FullAddress, (unsigned)_Pos - (unsigned)FullAddress);
		_Pos += 3;
	} else
	{
		HostName.PortName = "http";
		_Pos = FullAddress;
	}
	int CountReaded = sscanf(_Pos, "%*[^/:]%n%*c%hu", &Pos, &Port);
	HostName.HostName->append(_Pos, Pos);
	if(CountReaded > 1)
		HostName.PortName = std::to_string(Port);
	Update();
}

QUERY_URL::ADDRESS_INFO::ADDRESS_INFO(const char * Host, const char * Port)
{
	InitFields();
	PortName = Port;
	HostName = Host;
	Update();
}

QUERY_URL::ADDRESS_INFO::ADDRESS_INFO()
{
	InitFields();
	Update();
}

QUERY_URL::ADDRESS_INFO::~ADDRESS_INFO()
{
	if(HostName.ai != nullptr)
		freeaddrinfo(HostName.ai);
	HostName.PortName->~basic_string();
	HostName.HostName->~basic_string();
}

bool QUERY_URL::ADDRESS_INFO::Update(int iSocktype, int iProtocol, int iFamily, int iFlags)
{
	addrinfo host_info = {0}, *ah = nullptr;
	host_info.ai_socktype = iSocktype;
	host_info.ai_family = iFamily;
	host_info.ai_protocol = iProtocol;
	host_info.ai_flags = iFlags;                   //AI_PASSIVE
	if(getaddrinfo(HostName, PortName, &host_info, &ah) != 0)
		return false;
	if(HostName.ai != nullptr)
		freeaddrinfo(HostName.ai);
	HostName.ai = ah;
	return true;
}

void QUERY_URL::ADDRESS_INFO::InitFields()
{
	HostName.ai = nullptr;
	new(&HostName.PortName)  std::basic_string<char>("");
	new(&HostName.HostName)  std::basic_string<char>("");
}


int QUERY_URL::IPv6ADDR::FromString(const char* BufSource)
{
	char b[200];
	int n = -1;
	sscanf(BufSource, "%199[0-9a-fA-F:]%n", b, &n);
	if(n == -1) return -1;
	if(!QUERY_URL::StringToAddr(BufSource, Addr, AF_INET6)) return -1;
	return n;
}

std::basic_string<char> QUERY_URL::IPv6ADDR::ToString() const
{
	char b[INET6_ADDRSTRLEN + 2]; b[0] = '\0';
	ToString(b, INET6_ADDRSTRLEN + 1);
	return b;
}
char* QUERY_URL::IPv6ADDR::ToString(char* Dest, size_t Len) const { return (AddrToString(Addr, Dest, Len, AF_INET6) ? Dest : nullptr); }


int QUERY_URL::IPv4ADDR::FromString(const char* BufSource)
{
	char b[200]; b[0] = '\0';
	int n = -1;
	sscanf(BufSource, "%199[0-9.]%n", b, &n);
	if(n == -1) return -1;
	if(!QUERY_URL::StringToAddr(BufSource, Addr, AF_INET)) return -1;
	return n;
}

std::basic_string<char> QUERY_URL::IPv4ADDR::ToString() const
{
	char b[INET_ADDRSTRLEN + 2]; b[0] = '\0';
	ToString(b, INET6_ADDRSTRLEN + 1);
	return b;
}
char* QUERY_URL::IPv4ADDR::ToString(char* Dest, size_t Len) const { return (AddrToString(Addr, Dest, Len, AF_INET) ? Dest : nullptr); }

QUERY_URL::PROTOCOL_INTERATOR::_NAME::operator char*()
{
	if(Cur == nullptr) return "";
	return Cur->p_name;
}

QUERY_URL::PROTOCOL_INTERATOR::_INDEX::operator short()
{
	if(Cur == nullptr) return -1;
	return Cur->p_proto;
}


QUERY_URL::PROTOCOL_INTERATOR::P_NAME::_COUNT::operator int()
{
	int Count = 0;
	for(; Cur->p_aliases[Count]; Count++);
	return Count;
}

char* QUERY_URL::PROTOCOL_INTERATOR::P_NAME::operator[](unsigned Index)
{
	if(Index < Count) return Count.Cur->p_aliases[Index];
	return "";
}



QUERY_URL::PORT_SERVICE_INTERATOR::_NAME::operator char*()
{
	if(Cur == nullptr) return "";
	return Cur->s_name;
}

QUERY_URL::PORT_SERVICE_INTERATOR::_PORT::operator QUERY_URL::TPORT()
{
	if(Cur == nullptr) return 0;
	return htons(Cur->s_port);
}

QUERY_URL::PORT_SERVICE_INTERATOR::_PORT_IN_PACKET_FORM::operator QUERY_URL::TPORT()
{
	if(Cur == nullptr) return 0;
	return Cur->s_port;
}


QUERY_URL::PORT_SERVICE_INTERATOR::_USED_PROTOCOL::operator char*()
{
	if(Cur == nullptr) return "";
	return Cur->s_proto;
}

QUERY_URL::PROTOCOL_INTERATOR QUERY_URL::PORT_SERVICE_INTERATOR::_USED_PROTOCOL::GetInfo()
{
	if(Cur == nullptr) return PROTOCOL_INTERATOR(nullptr);
	return GetSystemProtocol(Cur->s_proto);
}



QUERY_URL::PORT_SERVICE_INTERATOR::P_NAME::_COUNT::operator int()
{
	int Count = 0;
	if(Cur == nullptr) return Count;
	for(; Cur->s_aliases[Count]; Count++);
	return Count;
}

char* QUERY_URL::PORT_SERVICE_INTERATOR::P_NAME::operator[](unsigned Index)
{
	if(Index < Count) return Count.Cur->s_aliases[Index];
	return "";
}



char * QUERY_URL::NET_INTERATOR::P_NAME::operator[](unsigned Index)
{
	if(Index < Count) return Count.Cur->n_aliases[Index];
	return "";
}

QUERY_URL::NET_INTERATOR::P_NAME::_COUNT::operator int()
{
	int Count = 0;
	for(; Cur->n_aliases[Count]; Count++);
	return Count;
}

QUERY_URL::NET_INTERATOR::_NUMBER_NET::operator short()
{
	if(Cur == nullptr) return -1;
	return Cur->n_net;
}


QUERY_URL::NET_INTERATOR::_ADDR_TYPE::operator short()
{
	if(Cur == nullptr) return -1;
	return Cur->n_addrtype;
}


QUERY_URL::NET_INTERATOR::_NAME::operator char*()
{
	if(Cur == nullptr) return "";
	return Cur->n_name;
}


QUERY_URL::INFO_HOST_INTERATOR::_NAME::operator char*()
{
	if(Cur == nullptr)
		return "";
	return Cur->h_name;
}

QUERY_URL::INFO_HOST_INTERATOR::_ADDR_TYPE::operator short()
{
	if(Cur == nullptr)
		return -1;
	return Cur->h_addrtype;
}

QUERY_URL::INFO_HOST_INTERATOR::_LENGTH_ADDRESS::operator short() const
{
	if(Cur == nullptr)
		return -1;
	return Cur->h_length;
}

QUERY_URL::INFO_HOST_INTERATOR::P_NAME::_COUNT::operator int()
{
	int Count = 0;
	for(; Cur->h_aliases[Count]; Count++);
	return Count;
}

char* QUERY_URL::INFO_HOST_INTERATOR::P_NAME::operator[](unsigned Index)
{
	if(Index < Count)
		return Count.Cur->h_aliases[Index];
	return "";
}

QUERY_URL::INFO_HOST_INTERATOR::P_ADDRESES::ADDRESS_INTERATOR::ADDRESS_INTERATOR(void * nCur, short nAddrType)
{
	Cur = nCur;
	AddrType = nAddrType;
}


char* QUERY_URL::INFO_HOST_INTERATOR::P_ADDRESES::ADDRESS_INTERATOR::operator()(char * Dest, size_t Len)
{
	if(Cur == nullptr)
		return nullptr;
	inet_ntop(AddrType, Cur, Dest, Len);
	return Dest;
}

QUERY_URL::INFO_HOST_INTERATOR::P_ADDRESES::ADDRESS_INTERATOR::operator std::basic_string<char>()
{
	std::basic_string<char> Buf("", INET6_ADDRSTRLEN + 1);
	operator()((char*)Buf.c_str(), INET6_ADDRSTRLEN + 1);
	return Buf;
}

QUERY_URL::INFO_HOST_INTERATOR::P_ADDRESES::_COUNT::operator int()
{
	int Count = 0;
	for(; Cur->h_addr_list[Count]; Count++);
	return Count;
}

QUERY_URL::INFO_HOST_INTERATOR::P_ADDRESES::ADDRESS_INTERATOR QUERY_URL::INFO_HOST_INTERATOR::P_ADDRESES::operator[](int Index)
{
	if(Index >= Count)
		return ADDRESS_INTERATOR(nullptr, 0);
	return ADDRESS_INTERATOR(Count.Cur->h_addr_list[Index], Count.Cur->h_addrtype);
}

long long QUERY_URL::SendFile(QUERY_URL& InSocket, size_t Count) { return SendFile(InSocket.RemoteIp.hSocket, Count, 0); }

bool QUERY_URL::EvntBind() { return true; }

bool QUERY_URL::EvntConnect() { return true; }

bool QUERY_URL::EvntAcceptClient(TDESCR ClientDescr) { return true; }

int QUERY_URL::EvntGetCountPandingData()
{
#ifdef WIN_PLATFORM
	u_long res = -1;
	if(ioctlsocket(RemoteIp.hSocket, FIONREAD, &res) == SOCKET_ERROR)
	{
		URL_SET_LAST_ERR;
		return -1;
	}
#else
	int res;
	if(ioctl(RemoteIp.hSocket, FIONREAD, &res) < 0)
	{
		URL_SET_LAST_ERR;
		return -1;
	}
#endif
	return res;
}

bool QUERY_URL::EvntIsNotHaveRecvData() { return (CountPandingData <= 0) && IsRecivedFin; }

bool QUERY_URL::EvntBeforeShutdown(int How) { return true; }

bool QUERY_URL::EvntBeforeClose() { return true; }


void QUERY_URL::InitFields()
{
#ifdef WIN_PLATFORM
	RemoteIp.IsNonBlocked = false;
#endif
	RemoteIp.hSocket = INVALID_SOCKET;
	LastError.Clear();
}

int QUERY_URL::SetOption(int hSocket, int Level, int Option, bool& New)
{
	int v = New;
	return setsockopt(hSocket, Level, Option, (char*)&v, sizeof(int));
}

int QUERY_URL::GetOption(int hSocket, int Level, int Option, bool& New)
{
	int v = 0;
	int l = sizeof(v);
	auto r = getsockopt(hSocket, Level, Option, (char*)&v, &l);
	New = v;
	return r;
}


QUERY_URL::__REMOTE_HOST_NAME::operator char*() const
{
	SOCKET_ADDR sa;
	if(!__QUERY_URL_PROPERTY_THIS->RemoteIp.GetRemoteAddress(sa))
		return "";
	return GetInfoAboutHost((void*)sa.Ip, sa.Len, sa.ProtocolFamily).Name;
}

QUERY_URL::__REMOTE_PORT::operator QUERY_URL::TPORT() const
{
	SOCKET_ADDR sa;
	if(!__QUERY_URL_PROPERTY_THIS->RemoteIp.GetRemoteAddress(sa))
		return INVALID_PORT;
	return sa.Port.Readable;
}
//As c string
char* QUERY_URL::__REMOTE_PORT::ToString(char * Dest, size_t Len) const
{
	sprintf_s(Dest, Len, "%i", (int)operator QUERY_URL::TPORT());
	return Dest;
}

//As stl string
std::basic_string<char> QUERY_URL::__REMOTE_PORT::ToString() const
{
	char b[20];
	b[0] = '\0';
	ToString(b, 20);
	return b;
}

//Get more info about remote port
QUERY_URL::PORT_SERVICE_INTERATOR QUERY_URL::__REMOTE_PORT::GetInfo() const { return GetSystemService(operator TPORT()); }



bool QUERY_URL::__REMOTE_IP::GetRemoteAddress(SOCKET_ADDR & Address) const
{
	int Len = sizeof(SOCKET_ADDR);
	if((getpeername(hSocket, Address, &Len) != 0) || (Len != Address.Len))
	{
		URL_SET_LAST_ERR_IN_PROPERTY;
		return false;
	}
	return true;
}

//As c string
char* QUERY_URL::__REMOTE_IP::ToString(char * Dest, size_t Len) const
{
	SOCKET_ADDR sa;
	if(GetRemoteAddress(sa))
	{
		sa.Ip.ToString(Dest, Len);
		return Dest;
	}
	return nullptr;
}

//As stl string
std::basic_string<char> QUERY_URL::__REMOTE_IP::ToString() const
{
	char b[INET6_ADDRSTRLEN + 2]; b[0] = '\0';
	ToString(b, INET6_ADDRSTRLEN + 1);
	return b;
}

//Get info about this IP
QUERY_URL::INFO_HOST_INTERATOR QUERY_URL::__REMOTE_IP::GetInfo() const
{
	SOCKET_ADDR sa;
	if(GetRemoteAddress(sa))
		return GetInfoAboutHost((void*)sa.Ip, sa.Len, sa.ProtocolFamily);
	return INFO_HOST_INTERATOR(nullptr);
}

QUERY_URL::__LOCAL_HOST_NAME::operator char*() const
{
	SOCKET_ADDR sa;
	if(__QUERY_URL_PROPERTY_THIS->LocalIp.GetLocalAddress(sa))
		return GetInfoAboutHost((void*)sa.Ip, sa.Len, sa.ProtocolFamily).Name;
	return "";
}


QUERY_URL::__LOCAL_PORT::operator QUERY_URL::TPORT() const
{
	SOCKET_ADDR sa;
	if(!__QUERY_URL_PROPERTY_THIS->LocalIp.GetLocalAddress(sa))
		return INVALID_PORT;
	return sa.Port.Readable;
}

char* QUERY_URL::__LOCAL_PORT::ToString(char * Dest, size_t Len) const
{
	sprintf_s(Dest, Len, "%i", (int)operator QUERY_URL::TPORT());
	return Dest;
}

std::basic_string<char> QUERY_URL::__LOCAL_PORT::ToString() const
{
	char b[20]; b[0] = '\0';
	ToString(b, 20);
	return b;
}

QUERY_URL::PORT_SERVICE_INTERATOR QUERY_URL::__LOCAL_PORT::GetInfo() const { return GetSystemService(operator TPORT()); }

bool QUERY_URL::__LOCAL_IP::GetLocalAddress(SOCKET_ADDR& Address) const
{
	int Len = sizeof(SOCKET_ADDR);
	if((getsockname(hSocket, Address, &Len) != 0) || (Len != Address.Len))
	{
		URL_SET_LAST_ERR_IN_PROPERTY
			return false;
	}
	return true;
}

char* QUERY_URL::__LOCAL_IP::ToString(char * Dest, size_t Len) const
{
	SOCKET_ADDR sa;
	if(GetLocalAddress(sa))
	{
		sa.Ip.ToString(Dest, Len);
		return Dest;
	}
	return nullptr;
}

std::basic_string<char> QUERY_URL::__LOCAL_IP::ToString() const
{
	char b[INET6_ADDRSTRLEN + 2]; b[0] = '\0';
	ToString(b, INET6_ADDRSTRLEN + 1);
	return b;
}

QUERY_URL::INFO_HOST_INTERATOR QUERY_URL::__LOCAL_IP::GetInfo() const
{
	SOCKET_ADDR sa;
	if(GetLocalAddress(sa))
		return GetInfoAboutHost((void*)sa.Ip, sa.Len, sa.ProtocolFamily);
	return INFO_HOST_INTERATOR(nullptr);
}


QUERY_URL::__PROTOCOL_FAMILY::operator decltype(std::declval<addrinfo>().ai_protocol)()
{
	SOCKET_ADDR SockAddr;
	if(!__QUERY_URL_PROPERTY_THIS->RemoteIp.GetRemoteAddress(SockAddr))
		return -1;
	return SockAddr.ProtocolFamily;
}

const char* QUERY_URL::__PROTOCOL_FAMILY::ToString() const
{
	SOCKET_ADDR SockAddr;
	if(!__QUERY_URL_PROPERTY_THIS->RemoteIp.GetRemoteAddress(SockAddr))
		return "";
	switch(SockAddr.ProtocolFamily)
	{
		case AF_UNSPEC: return "UNSPEC";
		case AF_UNIX: return "UNIX";
		case AF_INET: return "IPv4";
		case AF_INET6: return "IPv6";
		case AF_NETBIOS: return "NETBIOS";
	}
	return "";
}

bool QUERY_URL::__IS_NON_BLOCKED::operator=(bool NewVal)
{
#ifdef WIN_PLATFORM
	u_long nonBlocking = NewVal;
	if(ioctlsocket(hSocket, FIONBIO, &nonBlocking) == SOCKET_ERROR)
		URL_SET_LAST_ERR_IN_PROPERTY
	else
		IsNonBlocked = NewVal;
#else
	int nonBlocking = NewVal;
	if(fcntl(hSocket, F_SETFL, O_NONBLOCK, nonBlocking) == -1)
		URL_SET_LAST_ERR;
#endif
	return NewVal;
}

QUERY_URL::__IS_NON_BLOCKED::operator bool() const
{
#ifdef WIN_PLATFORM
	return IsNonBlocked;
#else
	return fcntl(hSocket, F_GETFL, 0) & O_NONBLOCK;
#endif
}

QUERY_URL::__IS_RECIVED_FIN::operator bool() const
{
	pollfd pfd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	pfd.fd = hSocket;
	if(poll(&pfd, 1, 0) == -1)
	{
		URL_SET_LAST_ERR;
		return false;
	}
	return pfd.revents & POLLHUP;
}

QUERY_URL::__OPTIONS::OPTION_INTERATOR::OPTION_INTERATOR(int nLevel, int nSocket, int nOptIndex)
{
	Level = nLevel;
	hSocket = nSocket;
	OptIndex = nOptIndex;
}

QUERY_URL::__OPTIONS::OPTION_INTERATOR QUERY_URL::__OPTIONS::operator[](int OptIndex) { return OPTION_INTERATOR(SOL_SOCKET, hSocket, OptIndex); }
QUERY_URL::__OPTIONS::OPTION_INTERATOR QUERY_URL::__OPTIONS::operator()(int OptIndex, int nLevel) { return OPTION_INTERATOR(nLevel, hSocket, OptIndex); }

QUERY_URL::PROTOCOL_INTERATOR QUERY_URL::GetSystemProtocol(int Index) { return PROTOCOL_INTERATOR(getprotobynumber(Index)); }
QUERY_URL::PROTOCOL_INTERATOR QUERY_URL::GetSystemProtocol(const char * Name) { return PROTOCOL_INTERATOR(getprotobyname(Name)); }
QUERY_URL::PORT_SERVICE_INTERATOR QUERY_URL::GetSystemService(int PortNumber, const char * Prot) { return PORT_SERVICE_INTERATOR(getservbyport(htons(PortNumber), Prot)); }
QUERY_URL::PORT_SERVICE_INTERATOR QUERY_URL::GetSystemService(const char * Name, const char * Prot) { return PORT_SERVICE_INTERATOR(getservbyname(Name, Prot)); }
QUERY_URL::NET_INTERATOR QUERY_URL::GetSystemNetwork(long net, int type) { return NET_INTERATOR(getnetbyaddr(net, type)); }
QUERY_URL::NET_INTERATOR QUERY_URL::GetSystemNetwork(const char * Name) { return NET_INTERATOR(getnetbyname(Name)); }
QUERY_URL::INFO_HOST_INTERATOR QUERY_URL::GetInfoAboutHost(const void * Addr, int Len, int Type) { return INFO_HOST_INTERATOR(gethostbyaddr((const char*)Addr, Len, Type)); }
QUERY_URL::INFO_HOST_INTERATOR QUERY_URL::GetInfoAboutHost(const char* NameOrTextAddress) { return INFO_HOST_INTERATOR(gethostbyname(NameOrTextAddress)); }

bool QUERY_URL::StringToAddr(const char* Str, void* Dest, int Family) { return inet_pton(Family, Str, Dest) == 1; }
bool QUERY_URL::AddrToString(const void* Src, char* Dest, size_t BufSize, int Family) { return inet_ntop(Family, (void*)Src, Dest, BufSize) != nullptr; }

bool QUERY_URL::Connect(ADDRESS_INFO::ADDRESS_INTERATOR& Address)
{
	if(IsOpen)
	{
		ShutdownSendRecive();
		Close();
	}
	addrinfo *i = Address;
	if((RemoteIp.hSocket = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == INVALID_SOCKET)
	{
		URL_SET_LAST_ERR;
		return false;
	} else if(connect(RemoteIp.hSocket, i->ai_addr, i->ai_addrlen) == SOCKET_ERROR)
	{
		URL_SET_LAST_ERR;
		Close();
		return false;
	}
	RemoteIp.ProtocolType = i->ai_protocol;
	return EvntConnect();
}


bool QUERY_URL::Connect(ADDRESS_INFO& AddrInfo)
{
	if(IsOpen)
	{
		ShutdownSendRecive();
		Close();
	}
	addrinfo *i = AddrInfo;
	for(; i != nullptr; i = i->ai_next)
	{
		if((RemoteIp.hSocket = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == INVALID_SOCKET)
			continue;
		if(connect(RemoteIp.hSocket, i->ai_addr, i->ai_addrlen) != SOCKET_ERROR)
			break;
		Close();
	}
	if(i == nullptr)
	{
		URL_SET_LAST_ERR;
		return false;
	}
	RemoteIp.ProtocolType = i->ai_protocol;
	return EvntConnect();
}


bool QUERY_URL::Connect
(
	const char* Port,
	const char* HostAddr,
	int Socktype,
	int Protocol,
	int Family,
	int Flags
)
{
	if(IsOpen)
	{
		ShutdownSendRecive();
		Close();
	}
	addrinfo hi = {0}, *ah = nullptr, *i;
	hi.ai_socktype = Socktype;
	hi.ai_family = Family;
	hi.ai_protocol = Protocol;
	hi.ai_flags = Flags;                   //AI_PASSIVE

	if(getaddrinfo(HostAddr, Port, &hi, &ah) != 0)
	{
		URL_SET_LAST_ERR;
		return false;
	}

	for(i = ah; i != nullptr; i = i->ai_next)
	{
		if((RemoteIp.hSocket = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == INVALID_SOCKET)
			continue;
		if(connect(RemoteIp.hSocket, i->ai_addr, i->ai_addrlen) != SOCKET_ERROR)
			break;
		Close();
	}
	if(i == nullptr)
	{
		URL_SET_LAST_ERR;
		if(ah != nullptr)
			freeaddrinfo(ah);
		return false;
	}
	RemoteIp.ProtocolType = i->ai_protocol;
	if(ah != nullptr)
		freeaddrinfo(ah);
	return EvntConnect();
}


bool QUERY_URL::Bind(ADDRESS_INFO::ADDRESS_INTERATOR& Address, int MaxConnection)
{
	if(IsOpen)
	{
		ShutdownSendRecive();
		Close();
	}
	addrinfo *i = Address;
	if((RemoteIp.hSocket = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == INVALID_SOCKET)
	{
		URL_SET_LAST_ERR;
		return false;
	} else if(bind(RemoteIp.hSocket, i->ai_addr, i->ai_addrlen) == SOCKET_ERROR)
	{
		Close();
		URL_SET_LAST_ERR;
		return false;
	} else if(listen(RemoteIp.hSocket, MaxConnection) == SOCKET_ERROR)
	{
		Close();
		URL_SET_LAST_ERR;
		return false;
	}
	RemoteIp.ProtocolType = i->ai_protocol;

	return EvntBind();
}

bool QUERY_URL::Bind(ADDRESS_INFO& AddrInfo, int MaxConnection)
{
	if(IsOpen)
	{
		ShutdownSendRecive();
		Close();
	}
	addrinfo *i = AddrInfo;
	for(; i != nullptr; i = i->ai_next)
	{
		if((RemoteIp.hSocket = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == INVALID_SOCKET)
			continue;
		if(bind(RemoteIp.hSocket, i->ai_addr, i->ai_addrlen) == SOCKET_ERROR)
		{
			Close();
			continue;
		}
		if(listen(RemoteIp.hSocket, MaxConnection) == SOCKET_ERROR)
		{
			Close();
			continue;
		}
		RemoteIp.ProtocolType = i->ai_protocol;
		break;
	}
	return EvntBind();
}


bool QUERY_URL::Bind
(
	const char * Port,
	int MaxConnection,
	int Socktype,
	int Protocol,
	int Family,
	int Flags
)
{
	if(IsOpen)
	{
		ShutdownSendRecive();
		Close();
	}
	addrinfo host_info = {0}, *ah = nullptr, *i;
	host_info.ai_socktype = Socktype;
	host_info.ai_family = Family;
	host_info.ai_protocol = Protocol;
	host_info.ai_flags = Flags;                   //AI_PASSIVE
	if(getaddrinfo(nullptr, Port, &host_info, &ah) != 0)
	{
		URL_SET_LAST_ERR;
		return false;
	}

	for(i = ah; i != nullptr; i = i->ai_next)
	{
		if((RemoteIp.hSocket = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == INVALID_SOCKET)
			continue;
		if(bind(RemoteIp.hSocket, i->ai_addr, i->ai_addrlen) == SOCKET_ERROR)
		{
			Close();
			continue;
		}
		if(listen(RemoteIp.hSocket, MaxConnection) == SOCKET_ERROR)
		{
			Close();
			continue;
		}
		RemoteIp.ProtocolType = i->ai_protocol;
		break;
	}

	if(ah != nullptr)
		freeaddrinfo(ah);
	return EvntBind();
}


bool QUERY_URL::AcceptClient(QUERY_URL& DestConnection)
{
	TDESCR ConnectedSocket;
	if((ConnectedSocket = accept(RemoteIp.hSocket, nullptr, nullptr)) == INVALID_SOCKET)
	{
		URL_SET_LAST_ERR;
		return false;
	}
	DestConnection.RemoteIp.hSocket = ConnectedSocket;
	DestConnection.RemoteIp.ProtocolType = RemoteIp.ProtocolType;
#ifdef WIN_PLATFORM
	DestConnection.RemoteIp.IsNonBlocked = false;
#endif
	DestConnection.LastError.Clear();
	return EvntAcceptClient(ConnectedSocket);
}

bool QUERY_URL::AcceptClient(QUERY_URL& DestConnection, SOCKET_ADDR& AddressClient)
{
	TDESCR ConnectedSocket;
	int AddrLen = sizeof(AddressClient);
	if((ConnectedSocket = accept(RemoteIp.hSocket, (sockaddr*)&AddressClient, &AddrLen)) == INVALID_SOCKET)
	{
		URL_SET_LAST_ERR;
		return false;
	}
	DestConnection.RemoteIp.hSocket = ConnectedSocket;
	DestConnection.RemoteIp.ProtocolType = RemoteIp.ProtocolType;
#ifdef WIN_PLATFORM
	DestConnection.RemoteIp.IsNonBlocked = false;
#endif
	DestConnection.LastError.Clear();
	return EvntAcceptClient(ConnectedSocket);
}


bool QUERY_URL::SkipClient()
{
	auto ClientConnection = accept(RemoteIp.hSocket, nullptr, nullptr);
	if(ClientConnection != INVALID_SOCKET)
	{
		closesocket(ClientConnection);
		return true;
	}
	return false;
}

bool QUERY_URL::SkipClient(SOCKET_ADDR& AddressSkipedClient)
{
	int AddrLen = sizeof(AddressSkipedClient);
	auto ClientConnection = accept(RemoteIp.hSocket, (sockaddr*)&AddressSkipedClient, &AddrLen);
	if(ClientConnection != INVALID_SOCKET)
	{
		closesocket(ClientConnection);
		return true;
	}
	return false;
}


bool QUERY_URL::Duplicate(int TargetProcessHandle)
{

#ifdef WIN_PLATFORM
	winsock::WSAPROTOCOL_INFOA ProtInfo;
	if(winsock::WSADuplicateSocketA(RemoteIp.hSocket, TargetProcessHandle, &ProtInfo) == SOCKET_ERROR)
	{
		URL_SET_LAST_ERR;
		return false;
	}
	return true;
#else
	return true;
#endif
}




FILE* QUERY_URL::OpenAsFile(const char * mode)
{
	FILE* File;
#ifdef WIN_PLATFORM
	const char * m = mode;
	int fileflag = 0;
	while(*mode == ' ') ++mode;
	switch(*mode)
	{
		case 'r': fileflag = _O_RDONLY; break;
		case 'w': fileflag = _O_CREAT | _O_WRONLY; break;
		case 'a': fileflag = _O_APPEND; break;
		default:  URL_SET_LAST_ERR_VAL(EFAULT); return nullptr;
	}
	while(*++mode)
		switch(*mode)
		{
			case ' ': break;
			case '+': fileflag |= _O_RDWR; break;
			case 'b': fileflag |= _O_BINARY; break;
			case 't': fileflag |= _O_TEXT; break;
			case 'c': break; case 'n': break;
			default: URL_SET_LAST_ERR_VAL(EFAULT); return nullptr;
		}

	int fd = _open_osfhandle((intptr_t)RemoteIp.hSocket, fileflag);
	if(fd == -1)
	{
		URL_SET_LAST_ERR_VAL(EFAULT);
		return nullptr;
	}
	File = _fdopen(fd, m);
#else
	File = fdopen(RemoteIp.hSocket, mode);
#endif
	if(File == nullptr)
		URL_SET_LAST_ERR;
	return File;
}



QUERY_URL::QUERY_URL()
{
	InitFields();
}

QUERY_URL::~QUERY_URL()
{
	if(IsOpen)
	{
		ShutdownSendRecive();
		Close();
	}
}


bool QUERY_URL::OnlyCreate(int iSocktype, int iProtocol, int iFamily)
{
	if(IsOpen)
	{
		ShutdownSendRecive();
		Close();
	}
	if((RemoteIp.hSocket = socket(iFamily, iSocktype, iProtocol)) == INVALID_SOCKET)
	{
		URL_SET_LAST_ERR;
		return false;
	}
	RemoteIp.ProtocolType = iProtocol;
}


int QUERY_URL::CheckEvents(decltype(std::declval<pollfd>().events) InEventFlags, int Timeout)
{
	pollfd pfd;
	pfd.events = InEventFlags;
	pfd.revents = 0;
	pfd.fd = RemoteIp.hSocket;
	if(poll(&pfd, 1, Timeout) == -1)
	{
		URL_SET_LAST_ERR;
		return -1;
	}
	return pfd.revents;
}



long long QUERY_URL::SendFile(TDESCR InFileDescriptor, size_t Count, off_t Offset)
{
#ifdef WIN_PLATFORM
#pragma comment(lib, "Mswsock.lib")
	WSAOVERLAPPED Overlap = {0};
	DWORD ResFlag, WritedCount = 0;
	if((Overlap.hEvent = winsock::WSACreateEvent()) == NULL)
	{
		URL_SET_LAST_ERR_VAL(EFAULT);
		return -1;
	}
	Overlap.OffsetHigh = ((Overlap.Offset = Offset) >> 32);
	if(!TransmitFile(RemoteIp.hSocket, (HANDLE)InFileDescriptor, Count, 0, &Overlap, nullptr, 0))
	{
		if(winsock::WSAGetLastError() == ERROR_IO_PENDING)
		{
			if(winsock::WSAGetOverlappedResult(RemoteIp.hSocket, &Overlap, &WritedCount, !IsNonBlocked, &ResFlag))
			{
				winsock::WSACloseEvent(Overlap.hEvent);
				return WritedCount;
			}
		}
	}
	winsock::WSACloseEvent(Overlap.hEvent);
	URL_SET_LAST_ERR;
	return -1;
#elif defined(__FreeBSD__)
	off_t sbytes = 0;
	int r = sendfile(RemoteIp.hSocket, InFileDescriptor, Offset, Count, 0, &sbytes, 0);
	if(r == -1)
	{
		if(errno == EAGAIN)
			return (sbytes ? sbytes : -1);
		URL_SET_LAST_ERR;
		return -1;
	}
	return Count;
#elif defined(__linux__)
	off_t o = Offset;
	long long done = 0;
	while(Count)
	{
		off_t todo = (Count > 0x7fffffff) ? 0x7fffffff : Count;
		off_t i = sendfile(RemoteIp.hSocket, InFileDescriptor, &o, todo);
		if(i == todo)
		{
			done += todo;
			Count -= todo;
			if(Count == 0)
				return done;
			continue;
		} else if(i == -1)
		{
			URL_SET_LAST_ERR;
			return -1;
		} else
			return done + i;
	}
	return 0;
#else
	if(Count == 0)
		return 0;
	if(Offset != 0)
		if(lseek(InFileDescriptor, Offset, SEEK_SET))
		{
			URL_SET_LAST_ERR;
			return -1;
		}
	unsigned long long SizeBuf;
	int r, wr, w = 0;
	void* Buf;
#	ifdef SO_SNDBUF
	SizeBuf = SockOptions.SendSizeBuffer;
#	else
	SizeBuf = 0xffff;
#	endif
	SizeBuf = (SizeBuf < Count) ? SizeBuf : Count;
	Buf = malloc(SizeBuf);
	if(Buf == nullptr)
	{
		URL_SET_LAST_ERR;
		return -1;
	}
	while(true)
	{
		if((r = read(InFileDescriptor, Buf, SizeBuf)) == -1)
		{
			if(w > 0)
				return w;
			URL_SET_LAST_ERR;
			free(Buf);
			return -1;
		}
		if((wr = write(RemoteIp.hSocket, Buf, r)) == -1)
		{
			URL_SET_LAST_ERR;
			free(Buf);
			return -1;
		}
		if(r < SizeBuf)
		{
			free(Buf);
			return wr;
		}
		w += wr;
		SizeBuf = ((Count - w) < SizeBuf) ? (Count - w) : SizeBuf;
	}
	free(Buf);
	return 0;
#endif
}


bool QUERY_URL::Close()
{
	if(RemoteIp.hSocket == INVALID_SOCKET)
		return true;
	if(!EvntBeforeClose())
		return false;
	if(closesocket(RemoteIp.hSocket) != SOCKET_ERROR)
	{
		RemoteIp.hSocket = INVALID_SOCKET;
		return true;
	}
	return false;
}

bool QUERY_URL::ShutdownSend()
{
	if(!EvntBeforeShutdown(SHUT_WR))
		return false;
	return shutdown(RemoteIp.hSocket, SHUT_WR) != SOCKET_ERROR;
}

bool QUERY_URL::ShutdownRecive()
{
	if(!EvntBeforeShutdown(SHUT_RD))
		return false;
	return shutdown(RemoteIp.hSocket, SHUT_RD) != SOCKET_ERROR;
}

bool QUERY_URL::ShutdownSendRecive()
{
	if(!EvntBeforeShutdown(SHUT_RDWR))
		return false;
	return shutdown(RemoteIp.hSocket, SHUT_RDWR) != SOCKET_ERROR;
}


int QUERY_URL::Write(const void * Buf, size_t SizeBuf)
{
#ifdef WIN_PLATFORM
	DWORD Written;
	OVERLAPPED Overlap = {0}, *ovlp = nullptr;
lblTryAgain:
	if(!WriteFile((HANDLE)RemoteIp.hSocket, Buf, SizeBuf, &Written, ovlp))
	{
		DWORD LastErr = GetLastError();
		if((LastErr == ERROR_INVALID_PARAMETER) && (ovlp == nullptr))
		{
			if(!IsNonBlocked)
				Overlap.hEvent = CreateEventW(nullptr, true, false, nullptr);
			ovlp = &Overlap;
			goto lblTryAgain;
		} else if(LastErr == ERROR_IO_PENDING)
		{
			if(Overlap.hEvent != NULL)
			{
				WaitForSingleObject(Overlap.hEvent, INFINITE);
				if(GetOverlappedResult((HANDLE)RemoteIp.hSocket, ovlp, &Written, TRUE))
				{
					CloseHandle(Overlap.hEvent);
					return Written;
				}
			} else
			{
				URL_SET_LAST_ERR_VAL(EWOULDBLOCK);
				return -1;
			}
		}
		URL_SET_LAST_ERR_VAL(EFAULT);
		if(Overlap.hEvent != NULL)
			CloseHandle(Overlap.hEvent);
		return -1;
	}
	if(Overlap.hEvent != NULL)
		CloseHandle(Overlap.hEvent);
	return Written;
#else
	size_t Written = write(RemoteIp.hSocket, Buf, SizeBuf);
	if(Written == -1)
		URL_SET_LAST_ERR;
	return Written;
#endif
}

int QUERY_URL::Read(void * Buf, size_t SizeBuf)
{
#ifdef WIN_PLATFORM
	DWORD Readed;
	OVERLAPPED Overlap = {0}, *ovlp = nullptr;
lblTryAgain:
	if(!ReadFile((HANDLE)RemoteIp.hSocket, Buf, SizeBuf, &Readed, ovlp))
	{
		DWORD LastErr = GetLastError();
		if((LastErr == ERROR_INVALID_PARAMETER) && (ovlp == nullptr))
		{
			if(!IsNonBlocked)
				Overlap.hEvent = CreateEventW(nullptr, true, false, nullptr);
			ovlp = &Overlap;
			goto lblTryAgain;
		} else if(LastErr == ERROR_IO_PENDING)
		{
			if(Overlap.hEvent != NULL)
			{
				WaitForSingleObject(Overlap.hEvent, INFINITE);
				if(GetOverlappedResult((HANDLE)RemoteIp.hSocket, ovlp, &Readed, TRUE))
				{
					CloseHandle(Overlap.hEvent);
					return Readed;
				}
			} else
			{
				URL_SET_LAST_ERR_VAL(EWOULDBLOCK);
				return -1;
			}
		}
		URL_SET_LAST_ERR_VAL(EFAULT);
		if(Overlap.hEvent != NULL)
			CloseHandle(Overlap.hEvent);
		return -1;
	}
	if(Overlap.hEvent != NULL)
		CloseHandle(Overlap.hEvent);
#else
	size_t Readed = read(RemoteIp.hSocket, Buf, SizeBuf);
	if(Readed == -1)
		URL_SET_LAST_ERR;
#endif
	return Readed;
}


int QUERY_URL::Send(const void * QueryBuf, size_t SizeBuf, int Flags)
{
	int WritenSize;
	if((WritenSize = send(RemoteIp.hSocket, (const char*)QueryBuf, SizeBuf, Flags)) == SOCKET_ERROR)
		URL_SET_LAST_ERR;
	return WritenSize;
}

int QUERY_URL::Recive(void * Buf, size_t SizeBuf, int Flags)
{
	int ReadedSize;
	if((ReadedSize = recv(RemoteIp.hSocket, (char*)Buf, SizeBuf, Flags)) == SOCKET_ERROR)
		URL_SET_LAST_ERR;
	return ReadedSize;
}

int QUERY_URL::Recive
(
	std::basic_string<char>& StrBuf,
	std::basic_string<char>::size_type MaxLen,
	int Flags
)
{
	char* Buf;
	//Ignore MSG_PEEK
	Flags &= ~MSG_PEEK;
	unsigned CurSize = 0, CountBytesInBuff, ReadedSize = 0;
	CountBytesInBuff = CountPandingData + 2;
	if(CountBytesInBuff < 50)
		CountBytesInBuff = 50;
	StrBuf.resize(CountBytesInBuff);
	Buf = (char*)StrBuf.c_str();
	while(true)
	{
		if(CurSize >= MaxLen)
			break;
		if(CountBytesInBuff > (MaxLen - CurSize))
			CountBytesInBuff = MaxLen - CurSize;
		ReadedSize = Recive(Buf, CountBytesInBuff, Flags);
		if(ReadedSize == SOCKET_ERROR)
		{
			URL_SET_LAST_ERR;
			return -1;
		} else if(ReadedSize == 0)
			break;
		else
		{
			CurSize += ReadedSize;
			CountBytesInBuff = CountPandingData;
			if(CountBytesInBuff == 0)
				CountBytesInBuff = 50;
			StrBuf.resize(CurSize + CountBytesInBuff + 2);
			Buf = (char*)StrBuf.c_str() + CurSize;
		}
	}
	*Buf = '\0';
	return CurSize;
}

int QUERY_URL::ReciveFrom(void * Buffer, size_t LenBuff, SOCKET_ADDR& AddressSender, int Flags)
{
	int Len = sizeof(SOCKET_ADDR);
	int CountRecived;
	if((CountRecived = recvfrom(RemoteIp.hSocket, (char*)Buffer, LenBuff, Flags, AddressSender, &Len)) == SOCKET_ERROR)
		URL_SET_LAST_ERR;
	return CountRecived;
}

int QUERY_URL::ReciveFrom(void * Buffer, size_t LenBuff, ADDRESS_INFO::ADDRESS_INTERATOR& AddressSender, int Flags)
{
	return ReciveFrom(Buffer, LenBuff, AddressSender.GetSocketAddr(), Flags);
}

int QUERY_URL::SendTo(const void * Buffer, size_t LenBuff, SOCKET_ADDR& AddressReciver, int Flags)
{
	int CountSending;
	if((CountSending = sendto(RemoteIp.hSocket, (const char*)Buffer, LenBuff, Flags, AddressReciver, sizeof(SOCKET_ADDR))) == SOCKET_ERROR)
		URL_SET_LAST_ERR;
	return CountSending;
}

int QUERY_URL::SendTo(const void * Buffer, size_t LenBuff, ADDRESS_INFO::ADDRESS_INTERATOR& AddressSender, int Flags)
{
	return SendTo(Buffer, LenBuff, AddressSender.GetSocketAddr(), Flags);
}


int QUERY_URL::SendAndRecive(const void* SendBuf, size_t SizeSendBuf, void* ReciveBuf, unsigned SizeReciveBuf)
{
	if(!Send(SendBuf, SizeSendBuf))
		return -1;
	return Recive(ReciveBuf, SizeReciveBuf);
}

int QUERY_URL::SendAndRecive(std::basic_string<char>& strQuery, void* ReciveBuf, size_t SizeBuf)
{
	if(!Send((void*)strQuery.c_str(), strQuery.length()))
		return -1;
	return Recive(ReciveBuf, SizeBuf);
}

int QUERY_URL::SendAndRecive(const void* SendBuf, unsigned SizeSendBuf, std::basic_string<char>& Result)
{
	if(!Send(SendBuf, SizeSendBuf))
		return -1;
	return Recive(Result);
}

int QUERY_URL::SendAndRecive(const char* SendStr, std::basic_string<char>& Result)
{
	if(!Send(SendStr, strlen(SendStr)))
		return -1;
	return Recive(Result);
}

int QUERY_URL::SendAndRecive(const char* SendStr, void* ReciveBuf, unsigned SizeBuf)
{
	if(!Send(SendStr, strlen(SendStr)))
		return -1;
	return Recive(ReciveBuf, SizeBuf);
}

int QUERY_URL::SendAndRecive(std::basic_string<char>& strQuery, std::basic_string<char>& Result)
{
	if(!Send((void*)strQuery.c_str(), strQuery.length()))
		return -1;
	return Recive(Result);
}



/*
*
*			end ExQueryUrl.h
*
*/

/*
*
*			start ExQueryUrlOpenSSL.h
*
*/


#ifdef HAVE_OPENSSL
#include "ExQueryUrlOpenSSL.h"

bool QUERY_URL_OPEN_SSL::EvntConnect()
{
	EvntBeforeClose();
	if(SSLLastError.ctx == nullptr)
	{
		SSLLastError.ctx = SSL_CTX_new(SSLv23_client_method());
		if(SSLLastError.ctx == nullptr)
			goto SSLErrOut;
	}
	SSLLastError.ssl = SSL_new(SSLLastError.ctx);
	if(SSLLastError.ssl == nullptr)
		goto SSLErrOut;

	if(SSL_set_fd(SSLLastError.ssl, Descriptor) == 0)
		goto SSLErrFree;
	if(SSL_connect(SSLLastError.ssl) < 0)
	{
SSLErrFree:
		SSL_free(SSLLastError.ssl);
SSLErrOut:
		SSLLastError.Set();
		QUERY_URL::SetLastErr(EFAULT);
		QUERY_URL::Close();
		return false;
	}
	return true;
}

bool QUERY_URL_OPEN_SSL::EvntBeforeClose()
{
	if(SSLLastError.ssl != nullptr)
	{
		SSL_shutdown(SSLLastError.ssl);
		SSL_free(SSLLastError.ssl);
		SSLLastError.ssl = nullptr;
	}
	return true;
}

bool QUERY_URL_OPEN_SSL::EvntBeforeShutdown(int)
{
	if(SSLLastError.ssl != nullptr)
		SSL_shutdown(SSLLastError.ssl);

	return true;
}


QUERY_URL_OPEN_SSL::~QUERY_URL_OPEN_SSL()
{
	if(SSLLastError.ssl != nullptr)
	{
		SSL_shutdown(SSLLastError.ssl);
		SSL_free(SSLLastError.ssl);
		SSLLastError.ssl = nullptr;
	}
	if(SSLLastError.ctx != nullptr)
	{
		SSL_CTX_free(SSLLastError.ctx);
		SSLLastError.ctx = nullptr;
	}
}


void QUERY_URL_OPEN_SSL::InitFields()
{
	SSLLastError.ctx = nullptr;
	SSLLastError.ssl = nullptr;
	SSLLastError.Clear();
}


QUERY_URL_OPEN_SSL::QUERY_URL_OPEN_SSL()
{
	SSL_load_error_strings();
	SSL_library_init();
	SSLeay_add_ssl_algorithms();
	InitFields();
}


char* QUERY_URL_OPEN_SSL::REMOTE_CERT::_SUBJECT_NAME::operator()(char * Buf, size_t Len)
{
	if(ssl == nullptr)
		return nullptr;
	X509* Cert = SSL_get_peer_certificate(ssl);
	if(Cert == nullptr)
		return nullptr;
	char * str = X509_NAME_oneline(X509_get_subject_name(Cert), 0, 0);
	if(str == nullptr)
	{
		X509_free(Cert);
		return nullptr;
	}
	strncpy(Buf, str, Len);
	OPENSSL_free(str);
	X509_free(Cert);
	return Buf;
}

QUERY_URL_OPEN_SSL::REMOTE_CERT::_SUBJECT_NAME::operator std::basic_string<char>()
{
	if(ssl == nullptr)
		return "";
	X509* Cert = SSL_get_peer_certificate(ssl);
	if(Cert == nullptr)
		return "";
	char * str = X509_NAME_oneline(X509_get_subject_name(Cert), 0, 0);
	if(str == nullptr)
	{
		X509_free(Cert);
		return "";
	}
	std::basic_string<char> Ret = str;
	OPENSSL_free(str);
	X509_free(Cert);
	return Ret;
}

char* QUERY_URL_OPEN_SSL::REMOTE_CERT::_ISSUER_NAME::operator()(char * Buf, size_t Len)
{
	if(ssl == nullptr)
		return nullptr;
	X509* Cert = SSL_get_peer_certificate(ssl);
	if(Cert == nullptr)
		return nullptr;
	char * str = X509_NAME_oneline(X509_get_issuer_name(Cert), 0, 0);
	if(str == nullptr)
	{
		X509_free(Cert);
		return nullptr;
	}
	strncpy(Buf, str, Len);
	OPENSSL_free(str);
	X509_free(Cert);
	return Buf;
}

QUERY_URL_OPEN_SSL::REMOTE_CERT::_ISSUER_NAME::operator std::basic_string<char>()
{
	if(ssl == nullptr)
		return "";
	X509* Cert = SSL_get_peer_certificate(ssl);
	if(Cert == nullptr)
		return "";
	char * str = X509_NAME_oneline(X509_get_issuer_name(Cert), 0, 0);
	if(str == nullptr)
	{
		X509_free(Cert);
		return "";
	}
	std::basic_string<char> Ret = str;
	OPENSSL_free(str);
	X509_free(Cert);
	return Ret;
}


QUERY_URL_OPEN_SSL::REMOTE_CERT::_IS_HAVE::operator bool()
{
	if(ssl == nullptr)
		return false;
	X509* Cert = SSL_get_peer_certificate(ssl);
	if(Cert != nullptr)
	{
		X509_free(Cert);
		return true;
	}
	return false;
}


char * QUERY_URL_OPEN_SSL::LOCAL_CERT::_SUBJECT_NAME::operator()(char * Buf, size_t Len)
{
	if(ssl == nullptr)
		return nullptr;
	X509* Cert = SSL_get_certificate(ssl);
	if(Cert == nullptr)
		return nullptr;
	char * str = X509_NAME_oneline(X509_get_subject_name(Cert), 0, 0);
	if(str == nullptr)
	{
		X509_free(Cert);
		return nullptr;
	}
	strncpy(Buf, str, Len);
	OPENSSL_free(str);
	X509_free(Cert);
	return Buf;
}

QUERY_URL_OPEN_SSL::LOCAL_CERT::_SUBJECT_NAME::operator std::basic_string<char>()
{
	if(ssl == nullptr)
		return "";
	X509* Cert = SSL_get_certificate(ssl);
	if(Cert == nullptr)
		return "";
	char * str = X509_NAME_oneline(X509_get_subject_name(Cert), 0, 0);
	if(str == nullptr)
	{
		X509_free(Cert);
		return "";
	}
	std::basic_string<char> Ret = str;
	OPENSSL_free(str);
	X509_free(Cert);
	return Ret;
}


char* QUERY_URL_OPEN_SSL::LOCAL_CERT::_ISSUER_NAME::operator()(char * Buf, size_t Len)
{
	if(ssl == nullptr)
		return nullptr;
	X509* Cert = SSL_get_certificate(ssl);
	if(Cert == nullptr)
		return nullptr;
	char * str = X509_NAME_oneline(X509_get_issuer_name(Cert), 0, 0);
	if(str == nullptr)
	{
		X509_free(Cert);
		return nullptr;
	}
	strncpy(Buf, str, Len);
	OPENSSL_free(str);
	X509_free(Cert);
	return Buf;
}

QUERY_URL_OPEN_SSL::LOCAL_CERT::_ISSUER_NAME::operator std::basic_string<char>()
{
	if(ssl == nullptr)
		return "";
	X509* Cert = SSL_get_certificate(ssl);
	if(Cert == nullptr)
		return "";
	char * str = X509_NAME_oneline(X509_get_issuer_name(Cert), 0, 0);
	if(str == nullptr)
	{
		X509_free(Cert);
		return "";
	}
	std::basic_string<char> Ret = str;
	OPENSSL_free(str);
	X509_free(Cert);
	return Ret;
}

QUERY_URL_OPEN_SSL::LOCAL_CERT::_IS_HAVE::operator bool()
{
	if(ssl == nullptr)
		return false;
	X509* Cert = SSL_get_certificate(ssl);
	if(Cert != nullptr)
	{
		X509_free(Cert);
		return true;
	}
	return false;
}


QUERY_URL_OPEN_SSL::_LIFE_TIMEOUT::operator long()
{
	if(ctx == nullptr)
		return -1;
	return SSL_CTX_get_timeout(ctx);
}
long QUERY_URL_OPEN_SSL::_LIFE_TIMEOUT::operator= (long New)
{
	if(ctx == nullptr)
		return -1;
	SSL_CTX_set_timeout(ctx, New);
	return New;
}

QUERY_URL_OPEN_SSL::_IS_VER::operator bool()
{
	if(ctx == nullptr)
		return false;
	return SSL_CTX_get_verify_mode(ctx) != SSL_VERIFY_NONE;
}

QUERY_URL_OPEN_SSL::_VER_MODE::operator int()
{
	if(ctx == nullptr)
		return SSL_VERIFY_NONE;
	return SSL_CTX_get_verify_mode(ctx);
}

int QUERY_URL_OPEN_SSL::_VER_MODE::operator=(int New)
{
	if(ctx == nullptr)
		return SSL_VERIFY_NONE;
	SSL_CTX_set_verify(ctx, New, nullptr);
	return New;
}

bool QUERY_URL_OPEN_SSL::InitCTXVersion(const SSL_METHOD* MethodSSL)
{
	EvntBeforeClose();
	if(SSLLastError.ctx != nullptr)
		SSL_CTX_free(SSLLastError.ctx);
	SSLLastError.ctx = SSL_CTX_new(MethodSSL);
	if(SSLLastError.ctx == nullptr)
	{
		SSLLastError.Set();
		QUERY_URL::SetLastErr(EFAULT);
		return false;
	}
	return true;
}

bool QUERY_URL_OPEN_SSL::SetLocalCertificate
(
	const char * CertFile,
	const char * PrivateKeyFile,
	int TypeCertFile,
	int TypeKeyFile,
	const SSL_METHOD* MethodSSL,
	bool IsVerifyClient,
	const char * CAFile,
	const char * CAPath,
	int ModeVerify,
	int VerifyDepth
)
{
	if(!InitCTXVersion(MethodSSL))
		return false;
	if(SSL_CTX_use_certificate_file(SSLLastError.ctx, CertFile, TypeCertFile) <= 0)
	{
lblErrOut:
		SSLLastError.Set();
		SSL_CTX_free(SSLLastError.ctx);
		SSLLastError.ctx = nullptr;
		QUERY_URL::SetLastErr(EFAULT);
		return false;
	}
	if(PrivateKeyFile == nullptr)
		PrivateKeyFile = CertFile;
	if(SSL_CTX_use_PrivateKey_file(SSLLastError.ctx, PrivateKeyFile, TypeKeyFile) <= 0)
		goto lblErrOut;

	if(!SSL_CTX_check_private_key(SSLLastError.ctx))
		goto lblErrOut;
	if(IsVerifyClient)
	{
		if(!SSL_CTX_load_verify_locations(SSLLastError.ctx, CAFile, CAPath))
			goto lblErrOut;

		SSL_CTX_set_verify(SSLLastError.ctx, ModeVerify, nullptr);
		SSL_CTX_set_verify_depth(SSLLastError.ctx, VerifyDepth);
	}
}

bool QUERY_URL_OPEN_SSL::AcceptClient(QUERY_URL_OPEN_SSL & DestCoonection)
{
	DestCoonection.Close();
	if(!QUERY_URL::AcceptClient(DestCoonection))
		return false;
	DestCoonection.SSLLastError.ssl = SSL_new(SSLLastError.ctx);
	if(DestCoonection.SSLLastError.ssl == nullptr)
	{
		SSLLastError.Set();
		return false;
	}
	if(SSL_set_fd(DestCoonection.SSLLastError.ssl, DestCoonection.Descriptor) == 0)
		goto lblErrOut;
	//https://www.google.ru/?gws_rd=ssl#q=SSL_accept+returned+0
	//http://stackoverflow.com/questions/13855789/ssl-accept-error-on-openssl-examples
	int r = SSL_accept(DestCoonection.SSLLastError.ssl);
	if(r == 0)
	{
		SSLLastError = SSL_get_error(DestCoonection.SSLLastError.ssl, 0);
		goto lblErrOut2;
	} else if(r < 0)
	{
lblErrOut:
		SSLLastError.Set();
lblErrOut2:
		DestCoonection.QUERY_URL::ShutdownSendRecive();
		DestCoonection.QUERY_URL::Close();
		return false;
	}
	return true;
}

int QUERY_URL_OPEN_SSL::Send(const void * QueryBuf, size_t SizeBuf, int Flags)
{
	if(SSLLastError.ssl == nullptr)
		goto lblErr;
	int WritenSize;
	if((WritenSize = SSL_write(SSLLastError.ssl, QueryBuf, SizeBuf)) < 0)
	{
		SSLLastError.Set();
lblErr:
		QUERY_URL::SetLastErr(EFAULT);
		return -1;
	}
	return WritenSize;
}


int QUERY_URL_OPEN_SSL::Recive(void * Buf, size_t SizeBuf, int Flags)
{
	if(SSLLastError.ssl == nullptr)
		goto lblErr;
	int ReadedSize;

	if((ReadedSize = ((Flags & MSG_PEEK) ? SSL_peek : SSL_read)(SSLLastError.ssl, Buf, SizeBuf)) < 0)
	{
		SSLLastError.Set();
lblErr:
		QUERY_URL::SetLastErr(EFAULT);
		return -1;
	}
	return ReadedSize;
}

int QUERY_URL_OPEN_SSL::Recive
(
	std::basic_string<char>& StrBuf,
	std::basic_string<char>::size_type MaxLen,
	int Flags
)
{
	if(SSLLastError.ssl == nullptr)
		goto lblErr;
	char * Buf;
	unsigned CurSize = 0, CountBytesInBuff, ReadedSize = 0;
	CountBytesInBuff = SSL_pending(SSLLastError.ssl);
	if(CountBytesInBuff < 50)
		CountBytesInBuff = 50;
	StrBuf.resize(CountBytesInBuff + 2);
	Buf = (char*)StrBuf.c_str();
	while(true)
	{
		if(CurSize >= MaxLen)
			break;
		if(CountBytesInBuff > (MaxLen - CurSize))
			CountBytesInBuff = MaxLen - CurSize;
		int ReadedSize = SSL_read(SSLLastError.ssl, Buf, CountBytesInBuff);
		if(ReadedSize < 0)
		{
			SSLLastError.Set();
lblErr:
			QUERY_URL::SetLastErr(EFAULT);
			return -1;
		} else if(ReadedSize == 0)
			break;
		else
		{
			CurSize += ReadedSize;
			CountBytesInBuff = SSL_pending(SSLLastError.ssl);
			if(CountBytesInBuff == 0)
				CountBytesInBuff = 50;
			StrBuf.resize(CurSize + CountBytesInBuff + 2);
			Buf = (char*)StrBuf.c_str() + CurSize;
		}
	}
	*Buf = '\0';
	return CurSize;
}

long long QUERY_URL_OPEN_SSL::SendFile(QUERY_URL& InSocket, size_t Count)
{
	if(Count == 0)
		return 0;
	unsigned long long SizeBuf;
	int r, wr, w = 0;
	void* Buf;
#	ifdef SO_SNDBUF
	SizeBuf = SockOptions.SendSizeBuffer;
#	else
	SizeBuf = 0xffff;
#	endif
	SizeBuf = (SizeBuf < Count) ? SizeBuf : Count;
	Buf = malloc(SizeBuf);
	if(Buf == nullptr)
	{
		QUERY_URL::SetLastErr(EFAULT);
		return -1;
	}
	while(true)
	{
		if((r = InSocket.Recive(Buf, SizeBuf)) == -1)
		{
			if(w > 0)
				return w;
			goto lblErr;
		}
		if((wr = Send(Buf, r)) == -1)
		{
lblErr:
			QUERY_URL::SetLastErr(LAST_ERR_SOCKET);
			free(Buf);
			return -1;
		}
		if(r < SizeBuf)
		{
			free(Buf);
			return wr;
		}
		w += wr;
		SizeBuf = ((Count - w) < SizeBuf) ? (Count - w) : SizeBuf;
	}
	free(Buf);
	return 0;
}

long long QUERY_URL_OPEN_SSL::SendFile(TDESCR InFileDescriptor, size_t Count, off_t Offset)
{
	if(Count == 0)
		return 0;
	if(Offset != 0)
	{
#ifdef _WIN32
		if(SetFilePointer((HANDLE)InFileDescriptor, Offset, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
#else
		if(lseek(InFileDescriptor, Offset, SEEK_SET))
#endif			
		{
			QUERY_URL::SetLastErr(LAST_ERR_SOCKET);
			return -1;
		}
	}
	unsigned long long SizeBuf;
	int r, wr, w = 0;
	void* Buf;
#	ifdef SO_SNDBUF
	SizeBuf = SockOptions.SendSizeBuffer;
#	else
	SizeBuf = 0xffff;
#	endif
	SizeBuf = (SizeBuf < Count) ? SizeBuf : Count;
	Buf = malloc(SizeBuf);
	if(Buf == nullptr)
	{
		QUERY_URL::SetLastErr(EFAULT);
		return -1;
	}
	OVERLAPPED Overlap = {0}, *ovlp = nullptr;
	while(true)
	{
#ifdef _WIN32
		lblTryAgain :
		{
			DWORD rt;
			if(!ReadFile((HANDLE)InFileDescriptor, Buf, SizeBuf, &rt, &Overlap))
			{
				DWORD LastErr = GetLastError();
				if(LastErr == ERROR_IO_PENDING)
				{
					if((LastErr == ERROR_INVALID_PARAMETER) && (ovlp == nullptr))
					{
						Overlap.hEvent = CreateEventW(nullptr, true, false, nullptr);
						ovlp = &Overlap;
						goto lblTryAgain;
					} else if(LastErr == ERROR_IO_PENDING)
					{
						if(Overlap.hEvent != NULL)
						{
							WaitForSingleObject(Overlap.hEvent, INFINITE);
							if(!GetOverlappedResult((HANDLE)InFileDescriptor, ovlp, &rt, TRUE))
							{
								CloseHandle(Overlap.hEvent);
								goto lblErr2;
							}
						} else
						{
							QUERY_URL::SetLastErr(EWOULDBLOCK);
							goto lblErr2;
						}
					} else
					{
						if(Overlap.hEvent != NULL)
							CloseHandle(Overlap.hEvent);
						goto lblErr;
					}
				}
			}
			r = rt;
		}
#else
		r = read(InFileDescriptor, Buf, SizeBuf);
#endif
		if(r == -1)
		{
			if(w > 0)
				return w;
			goto lblErr;
		}
		if((wr = Send(Buf, r)) == -1)
		{
lblErr:
			QUERY_URL::SetLastErr(LAST_ERR_SOCKET);
lblErr2:
			free(Buf);
			return -1;
		}
		if(r < SizeBuf)
		{
			free(Buf);
			return wr;
		}
		w += wr;
		SizeBuf = ((Count - w) < SizeBuf) ? (Count - w) : SizeBuf;
	}
	free(Buf);
	return 0;

}




#endif


/*
*
*			end ExQueryUrlOpenSSL.h
*
*/

/*
*
*			start ExTime.h
*
*/

#include "ExTime.h"
#include <stdio.h>

int StrToTm(const char* Str, tm* Result)
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
	static const char  *Week[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	static const char  *Months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
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


int StrToTime(const char* Str, time_t* Result)
{
	tm Tm;
	auto r = StrToTm(Str, &Tm);
	if(r == -1) return -1;
	*Result = mktime(&Tm);
	return r;
}


std::basic_string<char> TimeToString(time_t t)
{
	char b[30]; b[0] = '\0';
	ctime_s(b, sizeof(b), &t);
	auto l = strlen(b);
	if(l > 0) b[l - 1] = '\0';
	return b;
}

std::basic_string<char> TimeSubToString(time_t t1, time_t t2)
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

std::basic_string<char> TimeMilisecSubToString(long long t1, long long t2)
{
	char Buf[10];
	sprintf_s(Buf, ":%03i", (int)((t2 - t1) % 1000));
	return TimeSubToString(t1 / 1000, t2 / 1000) + Buf;
}


/*
*
*			end ExTime.h
*
*/

/*
*
*			start ExThread.h
*
*/
#include "ExThread.h"


#ifdef WIN_PLATFORM
#include <Windows.h>


static int __GetRealPrior(PRIORITY p)
{
	switch(p)
	{
		case PRIORITY::IDLE: return THREAD_PRIORITY_IDLE;
		case PRIORITY::LOWER: return THREAD_PRIORITY_LOWEST;
		case PRIORITY::LOW: return THREAD_PRIORITY_BELOW_NORMAL;
		case PRIORITY::NORMAL: return THREAD_PRIORITY_NORMAL;
		case PRIORITY::HIGH: return THREAD_PRIORITY_ABOVE_NORMAL;
		case PRIORITY::HIGHER: return THREAD_PRIORITY_HIGHEST;
		case PRIORITY::REALTIME: return THREAD_PRIORITY_TIME_CRITICAL;
		default: return THREAD_PRIORITY_NORMAL;
	}
}

static PRIORITY __GetPrior(int Code)
{
	switch(Code)
	{
		case THREAD_PRIORITY_IDLE: return PRIORITY::IDLE;
		case THREAD_PRIORITY_LOWEST: return PRIORITY::LOWER;
		case THREAD_PRIORITY_BELOW_NORMAL: return PRIORITY::LOW;
		case THREAD_PRIORITY_NORMAL: return PRIORITY::NORMAL;
		case THREAD_PRIORITY_ABOVE_NORMAL: return PRIORITY::HIGH;
		case THREAD_PRIORITY_HIGHEST: return PRIORITY::HIGHER;
		case THREAD_PRIORITY_TIME_CRITICAL: return PRIORITY::REALTIME;
		default: return PRIORITY::NONE;
	}
}

#else
#include <unistd.h> 
#include <pthread.h> 
#include <semaphore.h> 
#include <sys/time.h> 

static int __GetRealPrior(PRIORITY p)
{
	switch(p)
	{
		case PRIORITY::IDLE: return 45;
		case PRIORITY::LOWER: return 51;
		case PRIORITY::LOW: return 57;
		case PRIORITY::NORMAL: return 63;
		case PRIORITY::HIGH: return 69;
		case PRIORITY::HIGHER: return 75;
		case PRIORITY::REALTIME: return 81;
		default: return 63;
	}
}

static PRIORITY __GetPrior(int Code)
{
	switch(Code)
	{
		case 45: return PRIORITY::IDLE;
		case 51: return PRIORITY::LOWER;
		case 57: return PRIORITY::LOW;
		case 63: return PRIORITY::NORMAL;
		case 69: return PRIORITY::HIGH;
		case 75: return PRIORITY::HIGHER;
		case 81: return PRIORITY::REALTIME;
		default: return PRIORITY::NONE;
	}
}

#endif

bool SetThreadPrior(PRIORITY priority, std::thread& Thread)
{
#ifdef WIN_PLATFORM
	return SetThreadPriority((std::is_default_ref(Thread)) ? GetCurrentThread() : Thread.native_handle(), __GetRealPrior(priority)) != FALSE;
#else
	sched_param schedparams;
	schedparams.sched_priority = __GetRealPrior(priority);
	return pthread_setschedparam((std::is_default_ref(Thread)) ? pthread_self() : Thread.native_handle(), SCHED_OTHER, &schedparams) == 0;
#endif
}

PRIORITY GetThreadPrior(std::thread& Thread)
{
#ifdef WIN_PLATFORM
	return __GetPrior(GetThreadPriority((std::is_default_ref(Thread)) ? GetCurrentThread() : Thread.native_handle()));
#else
	sched_param schedparams;
	pthread_getschedparam((std::is_default_ref(Thread)) ? pthread_self() : Thread.native_handle(), std::make_default_pointer(), &schedparams);
	return __GetPrior(schedparams.sched_priority);
#endif
}

bool SetThreadAffinity(unsigned long long Mask, std::thread& Thread)
{
#ifdef WIN_PLATFORM
	return SetThreadAffinityMask((std::is_default_ref(Thread)) ? GetCurrentThread() : Thread.native_handle(), Mask) != 0;
#else
	return pthread_setaffinity_np((std::is_default_ref(Thread)) ? pthread_self() : Thread.native_handle(), sizeof(Mask), (const cpu_set_t*)&Mask) == 0;
#endif
}

bool GetThreadAffinity(unsigned long long* Mask, std::thread& Thread)
{
#ifdef WIN_PLATFORM
	GROUP_AFFINITY ga = {0};
	if(GetThreadGroupAffinity((std::is_default_ref(Thread)) ? GetCurrentThread() : Thread.native_handle(), &ga) == FALSE)
		return false;
	*Mask = ga.Mask;
	return true;
#else
	return pthread_getaffinity_np((std::is_default_ref(Thread)) ? pthread_self() : Thread.native_handle(), LenMaskBytes, (const cpu_set_t*)Mask) == 0;
#endif
}

/*
*
*			start ExLoadLibrary.h
*
*/
#include "ExLoadLibrary.h"


#ifdef WIN_PLATFORM
EXTERNAL_LIBRARY::EXTERNAL_LIBRARY(const char* Path) { Handle = (HANDLE_TYPE)LoadLibraryA(Path); }
EXTERNAL_LIBRARY::EXTERNAL_LIBRARY() { Handle = (HANDLE_TYPE)GetModuleHandleA(NULL); }
EXTERNAL_LIBRARY::PROC EXTERNAL_LIBRARY::GetProc(const char* NameProc) const { return GetProcAddress((HMODULE)Handle, NameProc); }
bool EXTERNAL_LIBRARY::Free()
{
	if(FreeLibrary((HMODULE)Handle) == TRUE)
	{
		Handle = NULL;
		return true;
	}
	return false;
}

EXTERNAL_LIBRARY::operator bool() const { return Handle != NULL; }

#else
#include <dlfcn.h>
EXTERNAL_LIBRARY::EXTERNAL_LIBRARY(const char* Path) { Handle = (HANDLE_TYPE)dlopen((char*)Path, RTLD_LAZY); }
EXTERNAL_LIBRARY::EXTERNAL_LIBRARY() { Handle = (HANDLE_TYPE)dlopen(NULL, RTLD_LAZY); }
EXTERNAL_LIBRARY::PROC EXTERNAL_LIBRARY::GetProc(const char* NameProc) const { return dlsym((void*)Handle, NameProc); }
bool EXTERNAL_LIBRARY::Free()
{
	if(dlclose((void*)Handle) == 0)
	{
		Handle = NULL;
		return true;
	}
	return false;
}
EXTERNAL_LIBRARY::operator bool() const { return Handle != NULL; }

#endif



/*


*/
#include "ExHashTableString.h"
#include "ExQueryUrlHTTP.h"


long long  EX_HTTP::GmtAccuracy = 0;

const std::basic_string<char> EX_HTTP::URI_REGEX::Scheme = "(?:[:alpha:][[:alnum:]+-.]*)";
const std::basic_string<char> EX_HTTP::URI_REGEX::PctEncodedChar = "(?:%[0-9a-fA-F]{2})";
const std::basic_string<char> EX_HTTP::URI_REGEX::UnreservedChar = "[[:alnum:]-._~]";
const std::basic_string<char> EX_HTTP::URI_REGEX::SubDelimsChar = "[!&'()*+,;=\\$]";
const std::basic_string<char> EX_HTTP::URI_REGEX::PcharChar = "(?:[[:alnum:]-._~!&'()*+,;=:@\\$]|(?:%[0-9a-fA-F]{2}))";
const std::basic_string<char> EX_HTTP::URI_REGEX::RegNameChar = "(?:[[:alnum:]-._~!&'()*+,;=\\$]|(?:%[0-9a-fA-F]{2}))";
const std::basic_string<char> EX_HTTP::URI_REGEX::QueryChar = "(?:[[:alnum:]-._~!&'()/?*+,;=:@\\$]|(?:%[0-9a-fA-F]{2}))";
const std::basic_string<char> EX_HTTP::URI_REGEX::QueryKeyChar = "(?:[[:alnum:]-._~!'()/?*+,;:@\\$]|(?:%[0-9a-fA-F]{2}))";
const std::basic_string<char> EX_HTTP::URI_REGEX::QueryValChar = "(?:[[:alnum:]-._~!'()/?*+,;:@=\\$]|(?:%[0-9a-fA-F]{2}))";

const std::basic_string<char> EX_HTTP::URI_REGEX::FragmentChar = "(?:[[:alnum:]-._~!&'/?()*+,;=:@\\$]|(?:%[0-9a-fA-F]{2}))";


const std::basic_string<char> EX_HTTP::URI_REGEX::UserInfo = "(?:[[:alnum:]-._~!&'()*+,;:=\\$]|(?:%[0-9a-fA-F]{2}))";//iuserinfo = iunreserved | pct-encoded | sub-delims | ":"


const std::basic_string<char> EX_HTTP::URI_REGEX::IPv4Segment = "(?:25[0-5]|(?:2[0-4]|1{0,1}[0-9]){0,1}[0-9])";
const std::basic_string<char> EX_HTTP::URI_REGEX::IPv4Address = "(?:(?:" + IPv4Segment + "\\.){3,3}" + IPv4Segment + ")";
const std::basic_string<char> EX_HTTP::URI_REGEX::IPv4RegEx = "(" + IPv4Address + ")";

const std::basic_string<char> EX_HTTP::URI_REGEX::IPv6Segment = "[0-9a-fA-F]{1,4}";

const std::basic_string<char> EX_HTTP::URI_REGEX::IPv6Full = "(?:(?:" + IPv6Segment + ":){7,7}" + IPv6Segment + ")";
const std::basic_string<char> EX_HTTP::URI_REGEX::IPv6Short1 = "(?:(?:" + IPv6Segment + ":){1,7}:)";    //1::
const std::basic_string<char> EX_HTTP::URI_REGEX::IPv6Short2 = "(?:(?:" + IPv6Segment + ":){1,6}:" + IPv6Segment + ")";
const std::basic_string<char> EX_HTTP::URI_REGEX::IPv6Short3 = "(?:(?:" + IPv6Segment + ":){1,5}(?::" + IPv6Segment + "){1,2})";
const std::basic_string<char> EX_HTTP::URI_REGEX::IPv6Short4 = "(?:(?:" + IPv6Segment + ":){1,4}(?::" + IPv6Segment + "){1,3})";
const std::basic_string<char> EX_HTTP::URI_REGEX::IPv6Short5 = "(?:(?:" + IPv6Segment + ":){1,3}(?::" + IPv6Segment + "){1,4})";
const std::basic_string<char> EX_HTTP::URI_REGEX::IPv6Short6 = "(?:(?:" + IPv6Segment + ":){1,2}(?::" + IPv6Segment + "){1,5})";
const std::basic_string<char> EX_HTTP::URI_REGEX::IPv6Short7 = "(?:" + IPv6Segment + ":(?:(?::" + IPv6Segment + "){1,6}))";
const std::basic_string<char> EX_HTTP::URI_REGEX::IPv6Short8 = "(?::(?:(?::" + IPv6Segment + "){1,7}|:))"; //::2:3:4:5:6:7:8
const std::basic_string<char> EX_HTTP::URI_REGEX::IPv6_IPv4Mapped = "(?:::(?:ffff(?::0{1,4}){0,1}:){0,1}" + IPv4Address + ")"; //::255.255.255.255 отображённый IPv4
const std::basic_string<char> EX_HTTP::URI_REGEX::IPv6_IPv4Embedded = "(?:(?:" + IPv6Segment + ":){1,4}:" + IPv4Address + ")"; //2001:db8:3:4::192.0.2.33 встроенный IPv4

const std::basic_string<char> EX_HTTP::URI_REGEX::IPv6Address = "(?:"
+ IPv6Full + "|"
+ IPv6Short1 + "|"
+ IPv6Short2 + "|"
+ IPv6Short3 + "|"
+ IPv6Short4 + "|"
+ IPv6Short5 + "|"
+ IPv6Short6 + "|"
+ IPv6Short7 + "|"
+ IPv6Short8 + "|"
+ IPv6_IPv4Mapped + "|"
+ IPv6_IPv4Embedded +
")";

const std::basic_string<char> EX_HTTP::URI_REGEX::IPv6RegEx = "(?:\\[(" + IPv6Address + ")\\])";

const std::basic_string<char> EX_HTTP::URI_REGEX::SchemeRegEx = "(?:(" + Scheme + ")://)";
const std::basic_string<char> EX_HTTP::URI_REGEX::HostRegEx = "(?:" + IPv4RegEx + "|" + IPv6RegEx + "|(" + RegNameChar + "+))";//ihost = IP-literal | IPv4address | ireg-name
const std::basic_string<char> EX_HTTP::URI_REGEX::PortRegEx = "(?::(\\d{1,5}))";
const std::basic_string<char> EX_HTTP::URI_REGEX::PathRegEx = "((?:/" + PcharChar + "*)+)";
const std::basic_string<char> EX_HTTP::URI_REGEX::ArgRegEx = "(?:(" + QueryKeyChar + "+)(?:=(" + QueryValChar + "*))?(?:&|$))";
const std::basic_string<char> EX_HTTP::URI_REGEX::QueryRegEx = "(?:\\?(" + QueryChar + "*)+)";
const std::basic_string<char> EX_HTTP::URI_REGEX::FragmentRegEx = "(?:#(" + FragmentChar + "*))";

const std::basic_string<char> EX_HTTP::URI_REGEX::AuthorityRegEx = "(?:(?:(" + UserInfo + "+)@)?" + HostRegEx + PortRegEx + "?)";//iauthority = [ iuserinfo "@" ] ihost [ ":" port ]

const std::basic_string<char> EX_HTTP::URI_REGEX::HierPart = "(?://" + AuthorityRegEx + PathRegEx + ")";

const std::basic_string<char> EX_HTTP::URI_REGEX::URIRegEx = "(?:" + SchemeRegEx + "?" + AuthorityRegEx + ")?" + PathRegEx + QueryRegEx + "?" + FragmentRegEx + "?";

int EX_HTTP::GetMethodIndex(const char* MethodName)
{
	static HASH_TABLE_STRING_KEY<char, unsigned char, false, unsigned char> HTTPMethods;
	static bool a = ([](decltype(HTTPMethods)& v) -> bool
	{
		for(unsigned char i = 0; i < METHODS::WRONG; i++)
		{
			const char* r = GetMethodName(i);
			if(r[0] != '\0')
				*v.Insert(r) = i;
		}
		return true;
	})(HTTPMethods);

	auto r = HTTPMethods[MethodName];
	if(r == nullptr)
		return METHODS::WRONG;
	return *r;
}


const char* EX_HTTP::GetMsgByStatus(int Status)
{
	switch(Status)
	{
		//Informational
		case 100:	return "Continue";
		case 101:	return "Switching Protocols";
		case 102:	return "Processing";
		case 105:	return "Name Not Resolved";

			//Success
		case 200:	return "OK";
		case 201:	return "Created";
		case 202:	return "Accepted";
		case 203:   return "Non-Authoritative Information";
		case 204:	return "No Content";
		case 205:	return "Reset Content";
		case 206:	return "Partial Content";
		case 207:	return "Multi-Status";
		case 226:	return "IM Used";

			//Redirection 
		case 301:	return "Moved Permanently";
		case 302:	return "Moved Temporarily";
		case 303:	return "See Other";
		case 304:	return "Not Modified";
		case 305:	return "Use Proxy";
		case 307:	return "Temporary Redirect";

			//Client Error 
		case 400:	return "Bad Request";
		case 401:	return "Unauthorized";
		case 402:	return "Payment Required";
		case 403:	return "Forbidden";
		case 404:	return "Not Found";
		case 405:	return "Method Not Allowed";
		case 406:	return "Not Acceptable";
		case 407:	return "Proxy Authentication Required";
		case 408:	return "Request Timeout";
		case 409:	return "Conflict";
		case 410:	return "Gone";
		case 411:	return "Length Required";
		case 412:	return "Precondition Failed";
		case 413:	return "Request Entity Too Large";
		case 414:	return "Request-URI Too Large";
		case 415:	return "Unsupported Media Type";
		case 416:	return "Requested Range Not Satisfiable";
		case 417:	return "Expectation Failed";
		case 418:	return "I'm a teapot"; //:)
		case 422:	return "Unprocessable Entity";
		case 423:	return "Locked";
		case 424:	return "Failed Dependency";
		case 425:	return "Unordered Collection";
		case 426:	return "Upgrade Required";
		case 428:	return "Precondition Required";
		case 429:	return "Too Many Requests";
		case 431:	return "Request Header Fields Too Large";
		case 434:	return "Requested host unavailable";
		case 449:	return "Retry With";
		case 451:	return "Unavailable For Legal Reasons";
		case 456:	return "Unrecoverable Error";

			//Server Error
		case 500:	return "Internal Server Error";
		case 501:	return "Not Implemented";
		case 502:	return "Bad Gateway";
		case 503:	return "Service Unavailable";
		case 504:	return "Gateway Timeout";
		case 505:	return "HTTP Version Not Supported";
		case 506:	return "Variant Also Negotiates";
		case 507:	return "Insufficient Storage";
		case 508:	return "Loop Detected";
		case 509:	return "Bandwidth Limit Exceeded";
		case 510:	return "Not Extended";
		case 511:	return "Network Authentication Required";
	}
	return "";
}


const char* EX_HTTP::GetMethodName(int Number)
{
	switch(Number)
	{
		//Response
		case METHODS::RESPONSE: return "HTTP";

			//Basic
		case METHODS::GET:		return "GET";
		case METHODS::POST:		return "POST";
		case METHODS::OPT:		return "OPTIONS";
		case METHODS::HEAD:		return "HEAD";
		case METHODS::PUT:		return "PUT";
		case METHODS::CONN:		return "CONNECT";
		case METHODS::TRACE:	return "TRACE";
		case METHODS::DEL:		return "DELETE";

			//WebDAV
			//RFC 2518
		case METHODS::PROPFIND:	return "PROPFIND";
		case METHODS::PROPPATCH:return "PROPPATCH";
		case METHODS::MKCOL:	return "MKCOL";
		case METHODS::COPY:		return "COPY";
		case METHODS::MOVE:		return "MOVE";
		case METHODS::LOCK:		return "LOCK";
		case METHODS::UNLOCK:	return "UNLOCK";

			//For version control
			//RFC 3253
		case METHODS::VERCNTRL:	return "VERSION-CONTROL";
		case METHODS::REPORT:	return "REPORT";
		case METHODS::CHKOUT:	return "CHECKOUT";
		case METHODS::CHKIN:	return "CHECKIN";
		case METHODS::UNCHKOUT:	return "UNCHECKOUT";
		case METHODS::MKWRKSPC:	return "MKWORKSPACE";
		case METHODS::UPDATE:	return "UPDATE";
		case METHODS::LABEL:	return "LABEL";
		case METHODS::MERGE:	return "MERGE";
		case METHODS::BSELNCNTRL:return "BASELINE-CONTROL";
		case METHODS::MKACTIV:	return "MKACTIVITY";

			//RFC 3648
		case METHODS::ORDPCH:	return "ORDERPATCH";

			//RFC 3744
		case METHODS::ACL:		return "ACL";

			//Another
		case METHODS::PATCH:	return "PATCH";
		case METHODS::PRI:		return "PRI";
		case METHODS::POLL:		return "POLL";
		case METHODS::SRCH:		return "SEARCH";
		case METHODS::SPACEJMP:	return "SPACEJUMP";

		case METHODS::LINK:		return "LINK";
		case METHODS::UNLINK:	return "UNLINK";

		case METHODS::MKRDCTREF:return "MKREDIRECTREF";
		case METHODS::UPRDCTREF:return "UPDATEREDIRECTREF";
		case METHODS::MKCLNDR:	return "MKCALENDAR";
		case METHODS::REBIND:	return "REBIND";
		case METHODS::UNBIND:	return "UNBIND";
	}
	return "";
}


void EX_HTTP::GetCurGMT(tm& OutTm)
{
	time_t t;
	time(&t);
	TimeToGMT(t, OutTm);
}

void EX_HTTP::TimeToGMT(time_t t, tm& OutTm)
{
	t += GmtAccuracy;
	OutTm = *gmtime(&t);
}

time_t EX_HTTP::GMTToTime(tm& InTm)
{
#ifdef _WIN32
	return _mkgmtime(&InTm) - GmtAccuracy;
#else
	return  timegm(&InTm) - GmtAccuracy;
#endif
}


bool EX_HTTP::WriteGMTTime(char * TimeStr, tm& InTm)
{
	static const char  *Week[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	static const char  *Months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	auto r = sprintf
	(
		TimeStr,
		"%3s, %02i %3s %i %02i:%02i:%02i GMT",
		Week[InTm.tm_wday],
		InTm.tm_mday,
		Months[InTm.tm_mon],
		InTm.tm_year + 1900,
		InTm.tm_hour,
		InTm.tm_min,
		InTm.tm_sec
	);
	return r >= 0;
}

bool EX_HTTP::ReadGMTTime(const char * TimeStr, tm& OutTm)
{
	char DayOfWeekName[6], MonthName[6], MeanTime[6];
	MeanTime[0] = MonthName[0] = DayOfWeekName[0] = '\0';
	unsigned short DayOfMonth, Year, Hour, Minute, Sec;
	auto r = sscanf
	(
		TimeStr,
		"%4[^ \r\n,\t]%*[^1234567890]%2i %4s %i %2i:%2i:%2i %4s",
		DayOfWeekName,
		&OutTm.tm_mday,
		MonthName,
		&OutTm.tm_year,
		&OutTm.tm_hour,
		&OutTm.tm_min,
		&OutTm.tm_sec,
		MeanTime
	);
	if(r < 8)
		return false;

	static const char  *Week[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	static const char  *Months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	OutTm.tm_wday = -1;
	OutTm.tm_year -= 1900;
	for(unsigned i = 0; i < std::countof(Week); i++)
		if(*(unsigned*)DayOfWeekName == *(unsigned*)(Week[i]))
		{
			OutTm.tm_wday = i;
			break;
		}
	if(OutTm.tm_wday == -1)
		return false;
	OutTm.tm_mon = -1;
	for(unsigned i = 0; i < std::countof(Months); i++)
		if(*(unsigned*)MonthName == *(unsigned*)(Months[i]))
		{
			OutTm.tm_mon = i;
			break;
		}
	if(OutTm.tm_mon == -1)
		return false;
	OutTm.tm_isdst = -1;
	OutTm.tm_yday = -1;
	return true;
}


size_t EX_HTTP::SkipInSockBuffer(QUERY_URL* Sock, void* Buf, size_t SizeBuf, size_t CountSkip)
{
	size_t Len = CountSkip;
	do
	{
		size_t l = (Len > SizeBuf) ? SizeBuf : Len;
		int CountReaded = Sock->Recive(Buf, l);
		if(CountReaded <= 0)
			break;
		Len -= CountReaded;
	} while(Len > 0);
	return CountSkip - Len;
}

int EX_HTTP::ReadHeadersRow
(
	char* s,
	void* UsrData,
	bool(*HeadersFunc)(void* UsrData, const char * Key, const char * Val)
)
{
	unsigned i = 0, j, k;
	char *Key;
	while(true)
	{
lblMainLoop:
		for(;; i++)
		{
			switch(s[i])
			{
				case ' ': case '\t': case '\n': case '\v': case '\f':
					continue;
				case ':':
lblSkip:
					for(;; i++)
					{
						if((s[i] == '\r') && (s[i + 1] == '\n'))
						{
							i += 2;
							goto lblMainLoop;
						} else if(s[i] == '\0')
							return i;
					}
				case '\r':
					if(s[i + 1] == '\n')
					{
						i += 2;
						goto lblMainLoop;
					}
					continue;
				case '\0':
					return i;
			}
			if((s[i] >= 'a') && (s[i] <= 'z') || (s[i] >= 'A') && (s[i] <= 'Z'))
				goto lblReadKey;
		}
lblReadKey:
		Key = s + i;
		//Read key
		for(i++; ((s[i] >= 'a') && (s[i] <= 'z')) || ((s[i] >= 'A') && (s[i] <= 'Z')) || (s[i] == '-'); i++);
		j = i;
		for(; (s[i] == ' ') || (s[i] == '\t'); i++);

		if((s[i] == '\r') && (s[i + 1] == '\n'))
		{
			i += 2;
			goto lblMainLoop;
		} else if(s[i] == '\0')
			return i;
		else if(s[i] != ':')
			goto lblSkip;

		//Skip spaces
		for(i++; (s[i] == ' ') || (s[i] == '\t'); i++);

		//Read val
		for(k = i;; i++)
		{
			if((s[i] == '\r') && (s[i + 1] == '\n'))
			{
				if(i > k)
				{
					s[i] = s[j] = '\0';
					if(!HeadersFunc(UsrData, Key, s + k))
						return -1;
				}
				i += 2;
				goto lblMainLoop;
			} else if(s[i] == '\0')
			{
				if(i > k)
				{
					s[j] = '\0';
					if(!HeadersFunc(UsrData, Key, s + k))
						return -1;
				}
				return i;
			}
		}
	}
	return i;
}


int EX_HTTP::ReadStartLineRow
(
	char* s,
	char** Method,
	unsigned* Status,
	char** StatusMsg_Uri,
	char** Ver,
	bool* IsResponse
)
{
	size_t i = 0;
	for(; (s[i] == '\r') && (s[i + 1] == '\n'); i += 2);
	size_t j = i;

	//If method started not a letter
	if((s[i] < 'A') && (s[i] > 'Z'))
		return -1;
	//Read method
	for(i++; ((s[i] >= 'A') && (s[i] <= 'Z')) || (s[i] == '-'); i++);

	if(
		((i - j) == 4) &&
		(s[j] == 'H') &&
		(s[j + 1] == 'T') &&
		(s[j + 2] == 'T') &&
		(s[j + 3] == 'P')
		)
	{
		//If get response
		*IsResponse = true;

		size_t e = i;
		if(s[i] == '/')
		{
			//Read version
			for(i++; ((s[i] >= '0') && (s[i] <= '9')) || (s[i] == '.'); i++);

			if(i > (e + 1))
			{
				if((s[i] == '\r') && (s[i + 1] == '\n'))
					return -1;
				*Ver = s + (e + 1);
				e = i;
			}
		}

		if((s[i] != ' ') && (s[i] != '\t'))
			return -1;
		s[e] = '\0';
		*Method = s + j;
		//Skip spaces
		for(i++; (s[i] == '\t') || (s[i] == ' '); i++);
		{
			unsigned Stat = 0;
			//Read status number
			for(unsigned char Digit; (Digit = s[i] - '0') <= 9; i++)
				Stat = Stat * 10 + Digit;
			*Status = Stat;
		}
		for(; (s[i] == '\t') || (s[i] == ' '); i++);

		for(size_t j = i; ; i++)
			if((s[i] == '\r') && (s[i + 1] == '\n'))
			{
				if(i > j)
					*StatusMsg_Uri = s + j;
				else
					*StatusMsg_Uri = (char*)GetMsgByStatus(*Status);
				goto lblOut2;
			}
	} else
	{
		*IsResponse = false;
		if((s[i] != ' ') && (s[i] != '\t'))
			return -1;
		s[i] = '\0';
		*Method = s + j;
		for(i++; (s[i] == '\t') || (s[i] == ' '); i++);

		//Read URI
		{
			size_t StartQuery = i;
			for(; !IsSpace(s[i]); i++);
			if(i <= StartQuery)
				return -1;
			*StatusMsg_Uri = s + StartQuery;
		}
		if((s[i] == '\r') && (s[i + 1] == '\n'))
			goto lblOut2;
		s[i] = '\0';
		for(i++; (s[i] == '\t') || (s[i] == ' '); i++);

		if((s[i] == 'H') && (s[i + 1] == 'T') && (s[i + 2] == 'T') && (s[i + 3] == 'P'))
		{
			i += 4;
			if(s[i] == '/')
			{
				size_t StartVer = ++i;
				//Read version
				for(; ((s[i] >= '0') && (s[i] <= '9')) || (s[i] == '.'); i++);

				if(i > StartVer)
				{
					*Ver = s + StartVer;
					if((s[i] == '\r') && (s[i + 1] == '\n'))
						goto lblOut2;
					s[i] = '\0';
					i++;
				}
			}
		}
	}

	for(; ; i++)
		if((s[i] == '\r') && (s[i + 1] == '\n'))
		{
lblOut2:
			s[i] = '\0';
			i += 2;
			break;
		}

	return i;
}


bool EX_HTTP::CheckMethodRow(char* s)
{
	size_t i = 0;
	for(; (s[i] == '\r') && (s[i + 1] == '\n'); i += 2);
	size_t j = i;
	if((s[i] < 'A') && (s[i] > 'Z'))
		return false;
	for(i++; ((s[i] >= 'A') && (s[i] <= 'Z')) || (s[i] == '-'); i++);

	if(
		((i - j) == 4) &&
		(s[j] == 'H') &&
		(s[j + 1] == 'T') &&
		(s[j + 2] == 'T') &&
		(s[j + 3] == 'P')
		)
	{
		return (s[i] == ' ') || (s[i] == '\t') || ((s[i] == '/') && IsDigit(s[i + 1]));
	}
	return (i > j) && ((s[i] == ' ') || (s[i] == '\t'));
}



int EX_HTTP::Recive
(
	QUERY_URL* QueryUrl,
	void* UsrData,
	bool(*ResponseFunc)(void*  UsrData, int Status, const char* Msg, const char* ProtoVer),
	bool(*QueryFunc)(void*  UsrData, const char* Method, const char* Path, const char* ProtoVer),
	bool(*HeadersFunc)(void*  UsrData, const char* Key, const char* Val),
	size_t* Readed,
	bool IsPeek,
	size_t MaxLenBuf
)
{
	if(QueryUrl->IsNotHaveRecvData)
		return ERRORS::SOCKET_HAS_DISCONNECTED;

	int CountReaded, Result = ERRORS::SUCCESS, i;
	char* EndHeader = nullptr;
	char TmpBuf[1024];
	void* Buf = TmpBuf;
	size_t CurSizeBuf = 1024, SizeHeader;

	while(true)
	{
		CountReaded = QueryUrl->Recive(Buf, CurSizeBuf - 2, MSG_PEEK);
		if(CountReaded == -1)
		{
			Result = ERRORS::NOT_READED_FROM_SOCKET;
			goto lblOut;
		} else if(CountReaded == 0)
		{
			Result = ERRORS::NOT_HAVE_DATA_IN_SOCKET;
			goto lblOut;
		}
		((char*)Buf)[CountReaded] = '\0';
		for(char* c = (char*)Buf; c[3] != '\0'; c++)
			if((c[0] == '\r') && (c[1] == '\n') && (c[2] == '\r') && (c[3] == '\n'))
			{
				EndHeader = c;
				break;
			}
		if(EndHeader == nullptr)
		{
			if(!CheckMethodRow((char*)Buf))
			{
				Result = ERRORS::NOT_HAVE_METHOD;
				goto lblOut;
			}
			if(Buf == TmpBuf)
			{
				if((Buf = malloc(CurSizeBuf += 300)) == nullptr)
				{
					Result = ERRORS::NOT_ALLOC_MEMORY;
					goto lblOut;
				}
			} else
			{
				if((Buf = realloc(Buf, CurSizeBuf += 300)) == nullptr)
				{
					Result = ERRORS::NOT_ALLOC_MEMORY;
					goto lblOut;
				}
			}
			continue;
		}
		break;
	}

	EndHeader[4] = '\0';
	SizeHeader = (size_t)EndHeader - (size_t)Buf + 4;
	{
		char *Met = "", *StatMsgURI = Met, *Ver = Met;
		unsigned Stat = 0;
		bool IsResponse = false;
		if((i = ReadStartLineRow((char*)Buf, &Met, &Stat, &StatMsgURI, &Ver, &IsResponse)) == -1)
		{
			Result = ERRORS::INVALID_START_LINE;
			goto lblOut;
		}
		if(IsResponse)
		{
			if(!ResponseFunc(UsrData, Stat, StatMsgURI, Ver))
			{
				Result = ERRORS::USER_INTERRUPT;
				goto lblOut;
			}
		} else
		{
			if(!QueryFunc(UsrData, Met, StatMsgURI, Ver))
			{
				Result = ERRORS::USER_INTERRUPT;
				goto lblOut;
			}
		}
	}
	if(ReadHeadersRow((char*)Buf + i, UsrData, HeadersFunc) == -1)
	{
		Result = ERRORS::USER_INTERRUPT;
		goto lblOut;
	}

	if(!IsPeek)
		SkipInSockBuffer(QueryUrl, TmpBuf, sizeof(TmpBuf) - 2, SizeHeader);
lblOut:
	*Readed = SizeHeader;
	if((Buf != nullptr) && (Buf != TmpBuf))
		free(Buf);
	return Result;
}


int EX_HTTP::SendQuery
(
	QUERY_URL* QueryUrl,
	const char* MethodStr,
	const char* Path,
	void* UsrData,
	bool(*HeadersEnumFunc)(void* UsrData, char ** Key, char ** Val),
	const char* ProtoVersion
)
{
	std::basic_string<char> ResponseBuf = MethodStr;
	ResponseBuf.append(" ", 1);
	ResponseBuf.append(Path);
	ResponseBuf.append(" HTTP/", sizeof(" HTTP/") - 1);
	ResponseBuf.append(ProtoVersion);
	ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
	for(char* Key, *Val; HeadersEnumFunc(UsrData, &Key, &Val);)
	{
		ResponseBuf.append(Key);
		ResponseBuf.append(": ", sizeof(": ") - 1);
		ResponseBuf.append(Val);
		ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
	}
	ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
	return QueryUrl->Send(ResponseBuf);
}


int EX_HTTP::SendQuery
(
	QUERY_URL* QueryUrl,
	int TypeMethod,
	const char* Path,
	void* UsrData,
	bool(*HeadersEnumFunc)(void* UsrData, char ** Key, char ** Val),
	const char* ProtoVersion
)
{
	const char * StrMethod = GetMethodName(TypeMethod);
	if(StrMethod[0] == '\0')
		return false;
	return  SendQuery(QueryUrl, StrMethod, Path, UsrData, HeadersEnumFunc, ProtoVersion);
}


int EX_HTTP::SendResponse
(
	QUERY_URL* QueryUrl,
	int Stat,
	void* UsrData,
	bool(*HeadersEnumFunc)(void* UsrData, char ** Key, char ** Val),
	const char* StatMsg,
	const char* ProtoVersion
)
{
	if(StatMsg == nullptr)
		StatMsg = GetMsgByStatus(Stat);
	std::basic_string<char> ResponseBuf("", 50);
	unsigned s = sprintf((char*)ResponseBuf.c_str(), "HTTP/%.20s %i ", ProtoVersion, Stat);
	ResponseBuf.resize(s);
	ResponseBuf.append(StatMsg);
	ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
	for(char* Key, *Val; HeadersEnumFunc(UsrData, &Key, &Val);)
	{
		ResponseBuf.append(Key);
		ResponseBuf.append(": ", sizeof(": ") - 1);
		ResponseBuf.append(Val);
		ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
	}
	ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
	return QueryUrl->Send(ResponseBuf);
}




/*
*	ExBase64.h
*/

#include "ExBase64.h"

template<bool Pad>
static size_t _CodeBase64(unsigned char *Dst, const unsigned char *Src, size_t SrcLen, const unsigned char *basis);
static int _DecodeBase64(unsigned char *Dst, const unsigned char* Src, size_t SrcLen, const unsigned char *DecodeChain);

static const unsigned char CodeChain[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const unsigned char CodeChainURL[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

static const unsigned char DecodeChain[] = {
	77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
	77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
	77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 62, 77, 77, 77, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 77, 77, 77, 77, 77, 77,
	77,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 77, 77, 77, 77, 77,
	77, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
};

static const unsigned char DecodeSeqURL[] = {
	77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
	77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
	77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 62, 77, 77,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 77, 77, 77, 77, 77, 77,
	77,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 77, 77, 77, 77, 63,
	77, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
};

size_t CodeBase64(char *Dest, const void *Src, size_t SrcLen) { return _CodeBase64<true>((unsigned char*)Dest, (const unsigned char*)Src, SrcLen, CodeChain); }
size_t CodeBase64URL(char *Dest, const void *Src, size_t SrcLen) { return _CodeBase64<false>((unsigned char*)Dest, (const unsigned char*)Src, SrcLen, CodeChainURL); }

std::basic_string<char> CodeBase64(const void *Src, size_t SrcLen)
{
	std::basic_string<char> r("", ((float)SrcLen * 1.4f) + 2);
	size_t l = _CodeBase64<true>((unsigned char*)r.data(), (const unsigned char*)Src, SrcLen, CodeChain);
	r.resize(l);
	return r;
}

std::basic_string<char> CodeBase64URL(const void *Src, size_t SrcLen)
{
	std::basic_string<char> r("", ((float)SrcLen * 1.4f) + 2);
	size_t l = _CodeBase64<false>((unsigned char*)r.data(), (const unsigned char*)Src, SrcLen, CodeChainURL);
	r.resize(l);
	return r;
}

template<bool Pad>
static size_t _CodeBase64(unsigned char *Dst, const unsigned char *Src, size_t SrcLen, const unsigned char *CodeChain)
{
	const unsigned char *s = Src;
	unsigned char *d = Dst;
	size_t l = SrcLen;
	while(l > 2)
	{
		*d++ = CodeChain[(s[0] >> 2) & 0x3f];
		*d++ = CodeChain[((s[0] & 3) << 4) | (s[1] >> 4)];
		*d++ = CodeChain[((s[1] & 0x0f) << 2) | (s[2] >> 6)];
		*d++ = CodeChain[s[2] & 0x3f];
		s += 3;
		l -= 3;
	}

	if(l > 0)
	{
		*d++ = CodeChain[(s[0] >> 2) & 0x3f];
		if(l == 1)
		{
			*d++ = CodeChain[(s[0] & 3) << 4];
			if(Pad) *d++ = '=';
		} else
		{
			*d++ = CodeChain[((s[0] & 3) << 4) | (s[1] >> 4)];
			*d++ = CodeChain[(s[1] & 0x0f) << 2];
		}
		if(Pad) *d++ = '=';
	}
	return d - Dst;
}

int DecodeBase64(void *Dst, const char *Src, size_t SrcLen) { return _DecodeBase64((unsigned char*)Dst, (const unsigned char*)Src, SrcLen, DecodeChain); }
int DecodeBase64URL(void *Dst, const char *Src, size_t SrcLen) { return _DecodeBase64((unsigned char*)Dst, (const unsigned char*)Src, SrcLen, DecodeSeqURL); }

std::basic_string<char> DecodeBase64(const char *Src, size_t SrcLen)
{
	std::basic_string<char> r("", ((float)SrcLen * 0.8f) + 2);
	int l = DecodeBase64((void*)r.data(), Src, SrcLen);
	if(l == -1) return std::basic_string<char>();
	r.resize(l);
	return r;
}
std::basic_string<char> DecodeBase64URL(const char *Src, size_t SrcLen)
{
	std::basic_string<char> r("", ((float)SrcLen * 0.8f) + 2);
	int l = DecodeBase64URL((void*)r.data(), Src, SrcLen);
	if(l == -1) return std::basic_string<char>();
	r.resize(l);
	return r;
}

static int _DecodeBase64(unsigned char *Dst, const unsigned char* Src, size_t SrcLen, const unsigned char *DecodeChain)
{
	size_t l = 0;
	/*Checkin base64 sequence*/
	for(; l < SrcLen; l++)
	{
		if(Src[l] == '=') break;
		if((DecodeChain[Src[l]] == 77) || (Src[l] > 122)) return -1;
	}
	if(l % 4 == 1) return -1;
	unsigned char *d = Dst;
	const unsigned char *s = Src;
	/*Decode sequence*/
	while(l > 3)
	{
		*d++ = (unsigned char)(DecodeChain[s[0]] << 2 | DecodeChain[s[1]] >> 4);
		*d++ = (unsigned char)(DecodeChain[s[1]] << 4 | DecodeChain[s[2]] >> 2);
		*d++ = (unsigned char)(DecodeChain[s[2]] << 6 | DecodeChain[s[3]]);
		s += 4;
		l -= 4;
	}

	if(l > 1) *d++ = (unsigned char)(DecodeChain[s[0]] << 2 | DecodeChain[s[1]] >> 4);
	if(l > 2) *d++ = (unsigned char)(DecodeChain[s[1]] << 4 | DecodeChain[s[2]] >> 2);

	return d - Dst;
}

