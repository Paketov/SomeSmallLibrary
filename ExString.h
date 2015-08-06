#ifndef __EX_STRING_H__
#define __EX_STRING_H__

#include <string>
#include <sstream>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <wchar.h>
#include <typeinfo>

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


#define STR_TYPE(c,Str) ((typeid(c) == typeid(wchar_t))?((c*)L ## Str):((c*)Str))
#define CHAR_TYPE(Type, Char)  ((sizeof(Type) == sizeof(char))?(Char):(L ## Char))

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

	if((typeid(_InCharType) == typeid(wchar_t)) && (typeid(_OutCharType) == typeid(wchar_t)))
	{
		OutStr = (_OutCharType*)InStr.c_str();
		return;
	}else if(typeid(_InCharType) == typeid(wchar_t))
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
	}else if(typeid(_OutCharType) == typeid(wchar_t))
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
	if(typeid(T) == typeid(wchar_t))
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
		if(typeid(_Elem) == typeid(wchar_t))
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
	return _u_NumberToString(Number, Str, Len, Radix) - Str;
}

template<class TypeChar>
inline size_t NumberToString(long Number, TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
	return _i_NumberToString(Number, Str, Len, Radix) - Str;
}

template<class TypeChar>
inline size_t NumberToString(unsigned long Number, TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
	return _u_NumberToString(Number, Str, Len, Radix) - Str;
}


template<class TypeChar>
inline size_t NumberToString(long long Number, TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
	return _i_NumberToString(Number, Str, Len, Radix) - Str;
}

template<class TypeChar>
inline size_t NumberToString(unsigned long long Number, TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
	return _u_NumberToString(Number, Str, Len, Radix) - Str;
}

template<class TypeNumber, class TypeChar>
inline TypeChar * _i_NumberToString(TypeNumber Number, TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
	TypeChar * pCur = Str, *MaxIndex = Str + Len;
	if((pCur < MaxIndex) && (Number < 0))
	{
		*(pCur++) = CHAR_TYPE(TypeChar,'-');
		Number = -Number;
	}

	TypeNumber t = 0;
	do
	      t = t * Radix + Number % Radix;
	while((Number /= Radix) > 0);
	do 
	{
		unsigned char  digval = (unsigned char)(t % Radix);
		*pCur = (digval > 9)? (digval + (CHAR_TYPE(TypeChar,'a') - 10)): (digval + CHAR_TYPE(TypeChar,'0'));
	} while ((++pCur < MaxIndex) && (t /= Radix) > 0);
	if(MaxIndex <= pCur)
		return pCur;
	*pCur = CHAR_TYPE(TypeChar,'\0');
	return pCur;
}

template<class TypeNumber, class TypeChar>
inline TypeChar * _u_NumberToString(TypeNumber Number, TypeChar * Str, size_t Len, unsigned char Radix = 10)
{
	TypeChar * pCur = Str, *MaxIndex = Str + Len;
	TypeNumber t = 0;
	do
	      t = t * Radix + Number % Radix;
	while((Number /= Radix) > 0);
	do 
	{
		unsigned char  digval = (unsigned char) (t % Radix);
		*pCur = (digval > 9)? (digval + (CHAR_TYPE(TypeChar,'a') - 10)): (digval + CHAR_TYPE(TypeChar,'0'));
	} while ((++pCur < MaxIndex) && (t /= Radix) > 0);
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



template<class TypeChar>
inline TypeChar * _d_NumberToString(long double Number, TypeChar * Str, size_t Len, unsigned char Radix, long double Eps1)
{
	static const long double MinExp = 1.0e-5;
	static const long double MaxExp = 1.0e+15;
	long double uNumber = (Number < 0)? -Number: Number;
	short Exp = 0;
	TypeChar * pCur = Str, *MaxIndex = Str + Len;
	if((Number != 0.0) && ((uNumber < MinExp) || (uNumber > MaxExp)))
	{
		Exp = ((*(short*)((char*)&uNumber + (sizeof(uNumber) - sizeof(Exp))) & 0x7ff0) >> 4) - 1023;
		Exp = (short)(log(pow(2.0,Exp)) / log(Radix));
		uNumber *= pow(Radix, -Exp);
	}

	if((pCur < MaxIndex) && (Number < 0.0))
		*(pCur++) = CHAR_TYPE(TypeChar, '-');
	unsigned long long Integer = (unsigned long long)uNumber;
	uNumber = uNumber - Integer + 1.0;

	{
		unsigned long long t = 0;
		for(;Integer > 0;Integer /= Radix)
		{
			t = t * Radix + Integer % Radix;
			Eps1 *= Radix;
		}
		do 
		{
			unsigned char  digval = (unsigned char) (t % Radix);
			*pCur = (digval > 9)? (digval + (CHAR_TYPE(TypeChar,'a') - 10)): (digval + CHAR_TYPE(TypeChar,'0'));
		} while ((++pCur < MaxIndex) && (t /= Radix) > 0);
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
	long double uNumber = (Number < 0)? -Number: Number;
	short Exp = 0;
	TypeChar * pCur = Str, *MaxIndex = Str + Len;
	if((Number != 0.0) && ((uNumber < MinExp) || (uNumber > MaxExp)))
	{
		Exp = ((*(short*)((char*)&uNumber + (sizeof(uNumber) - sizeof(Exp))) & 0x7ff0) >> 4) - 1023;
		Exp = (short)(log(pow(2.0,Exp)) / log(Radix));
		uNumber *= pow(Radix, -Exp);
	}

	if((pCur < MaxIndex) && (Number < 0.0))
		*(pCur++) = CHAR_TYPE(TypeChar, '-');
	unsigned long long Integer = (unsigned long long)uNumber;
	uNumber = uNumber - Integer + 1.0;
	{
		unsigned long long t = 0;
		for(;Integer > 0;Integer /= Radix)
			t = t * Radix + Integer % Radix;
		do 
		{
			unsigned char  digval = (unsigned char) (t % Radix);
			*pCur = (digval > 9)? (digval + (CHAR_TYPE(TypeChar,'a') - 10)): (digval + CHAR_TYPE(TypeChar,'0'));
		} while ((++pCur < MaxIndex) && (t /= Radix) > 0);
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

template<typename InString, typename OutString>
void CodeUrl(const InString & InStr, OutString & OutStr, unsigned InCodePage = CP_UTF8)
{
	typedef typename OutString::value_type _InCharType;
	if(typeid(_InCharType) == typeid(wchar_t))
	{
		std::string a;
		ConvertCodePageString(0,CP_UTF8,InStr, a);
		unsigned SizeOutStr = a.length() * 3;
		OutStr.resize(SizeOutStr);
		if(typeid(_InCharType) == typeid(wchar_t))
		{
			for(unsigned i = 0, e = a.length(); i < e; i++)
				swprintf((wchar_t*)&OutStr[i * 3],4,L"%%%02x",a[i] & 0xff);
		}
		else
		{
			for(unsigned i = 0, e = a.length(); i < e; i++)
				sprintf((char*)&OutStr[i * 3],"%%%02x",a[i] & 0xff);
		}
	}
	else
	{
		std::string a;
		if(InCodePage != CP_UTF8)
			ConvertCodePageString(InCodePage,CP_UTF8,InStr, a);
		else
			a = (char*)InStr.c_str();
		unsigned SizeOutStr = a.length() * 3;
		OutStr.resize(SizeOutStr);
		if(typeid(_InCharType) == typeid(wchar_t))
		{
			for(unsigned i = 0, e = a.length(); i < e; i++)
				swprintf((wchar_t*)&OutStr[i * 3],4,L"%%%02x",a[i] & 0xff);
		}
		else
		{
			for(unsigned i = 0, e = a.length(); i < e; i++)
				sprintf((char*)&OutStr[i * 3],"%%%02x",a[i] & 0xff);
		}
	}
}



namespace std
{		
	
	template <class Type, unsigned Offset = 0>
	class decl_spec_val
	{
		char __a[Offset];
	public:
		Type Val;

		inline Type operator()() const
		{
		    return Val;
		}

		inline Type operator()(const Type NewVal)
		{
			return Val = NewVal;
		}
	};

	template <class Type>
	class decl_spec_val<Type, 0>
	{ 	
	public:
		Type Val;

		inline Type operator()() const
		{
		    return Val;
		}

		inline Type operator()(const Type NewVal)
		{
			return Val = NewVal;
		}
	};
	
	template <class Type, unsigned Offset, size_t Len>
	class decl_spec_val<Type[Len], Offset>
	{
		char __a[Offset];
	public:

		Type Val[Len];

	    inline Type * operator()()
		{
		    return Val;
		}

	  	inline Type * operator()(Type * NewVal)
		{
			memcpy(Val, NewVal, sizeof(Val));
			return NewVal;
		}
	};

	template <class Type, size_t Len>
	class decl_spec_val<Type[Len], 0>
	{ 	
	public:
		Type Val[Len];

	    inline Type * operator()()
		{
		    return Val;
		}

	  	inline Type * operator()(Type * NewVal)
		{
			memcpy(Val, NewVal, sizeof(Val));
			return NewVal;
		}
	};

	template <class Type, Type InitVal, bool OnWriteErr = false>
	class decl_const_val
	{	
	public:

		static const Type Val = InitVal;

		inline Type operator()() const
		{
		    return Val;
		}

		inline Type operator()(const Type NewVal) const
		{
			typedef std::enable_if<!OnWriteErr>::type;
			return NewVal;
		}

	};



};

#endif
