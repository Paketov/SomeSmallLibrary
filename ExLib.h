#ifndef __EXSTRING_H__
#define __EXSTRING_H__

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


#define STR(c,Str) ((typeid(c) == typeid(wchar_t))?((c*)L ## Str):((c*)Str))


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
	}
	else if(typeid(_InCharType) == typeid(wchar_t))
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
	}
	else if(typeid(_OutCharType) == typeid(wchar_t))
	{

#ifdef WIN32
		unsigned size = MultiByteToWideChar(InCp, 0, (LPCSTR)InStr.c_str(),InStr.length(), NULL, 0);
		OutStr.resize(size);
		MultiByteToWideChar(InCp, 0, (LPCSTR)InStr.c_str(), InStr.length(), (LPWSTR)&OutStr[0], size);
#else
		convert_hnd = iconv_open("WCHAR_T",CodePageStr[InCp] );
		if(convert_hnd == (iconv_t)-1)
			return;
		SizeInBuf = (InStr.length() + 1) * sizeof(_InCharType);
		OutStr.resize(SizeInBuf);
		SizeOutBuf = SizeInBuf * sizeof(_OutCharType);
#endif
	}
	else
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

#define CHAR_TYPE(Type, Char)  ((sizeof(Type) == sizeof(char))?(Char):(L ## Char))

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

};






class DYN_MEM
{
public:

	void * Last;
	void * Max;

public:

	void * operator new(size_t, unsigned NewSize)
	{
		DYN_MEM * s = (DYN_MEM *)malloc(NewSize + sizeof(DYN_MEM));
		if(s == NULL)
			return NULL;
		s->Last = s + 1;
		s->Max = (char*)s + (sizeof(DYN_MEM) + NewSize);
		return s;
	}


	unsigned GetSize()
	{
		return (unsigned)Max - (unsigned)this - sizeof(DYN_MEM);
	}

	unsigned SizeFree()
	{
		return (unsigned)Max - (unsigned)Last; 
	}

};



template<DYN_MEM ** StartDynMem, class T = void, const bool DontDel = false>
class PBUF
{
protected:
	const unsigned SezeT(void*)
	{
		return 1;
	}

	template<class F>
	const unsigned SezeT(F*)
	{
		return sizeof(F);
	}

	T * StartBlock;
public:

	PBUF()
	{
		void * EndBlock = (void*)((unsigned)(*StartDynMem)->Last + SezeT((T*)0));
		if(EndBlock >= (*StartDynMem)->Max)
		{
			StartBlock = NULL;
			return;
		}
		StartBlock = (T*)(*StartDynMem)->Last;
		(*StartDynMem)->Last = EndBlock;
	}


	PBUF(unsigned NewCount)
	{
		void * EndBlock = (void*)((unsigned)(*StartDynMem)->Last + SezeT((T*)0) * NewCount);
		if(EndBlock >= (*StartDynMem)->Max)
		{
			StartBlock = NULL;
			return;
		}
		StartBlock = (T*)(*StartDynMem)->Last;
		(*StartDynMem)->Last = EndBlock;
	}

	operator T*()
	{
		return StartBlock;
	}

	bool Resize(unsigned NewSize)
	{
		void * NewEndBlock = (void*)((unsigned)StartBlock + NewSize);
		if(NewEndBlock >= (*StartDynMem)->Max)
			return false;
		(*StartDynMem)->Last = NewEndBlock;
		return true;
	}

	~PBUF()
	{
		if(!DontDel)
		{
			if(StartBlock == NULL)
				return;
			if(StartBlock >= (*StartDynMem)->Last)
				throw 0;
			(*StartDynMem)->Last = StartBlock;
		}
	}

};

template<DYN_MEM ** StartDynMem, class T = void, const bool DontDel = false>
class PBUFEX : public PBUF<StartDynMem,T, DontDel>
{
	unsigned Size;
public:

	PBUFEX() : PBUF()
	{
		Size = SezeT((T*)0);
	}

	PBUFEX(unsigned NewCount) : PBUF(NewCount)
	{
		Size = SezeT((T*)0) * NewCount;
	}

	unsigned GetSize()
	{
		return Size;
	}

	unsigned GetCount()
	{
		return GetSize() / SezeT((T*)0);
	}

	bool isLastBlock()
	{
		return ((char*)StartBlock + Size) == (*StartDynMem)->Last;
	}

	void * GetNextBlock()
	{
		return ((char*)StartBlock + Size);
	}

	bool Resize(unsigned NewSize)
	{
		if(PBUF::Resize(NewSize))
		{
			Size = NewSize;
			return true;
		}
		return false;
	}
};




#ifdef WIN32

#ifdef USE_WONDER_CONTROLS

#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")

#endif

class WND_COMBO;



class EX_WND
{
public:
	union
	{	
		HWND hWnd;

		class{
		public:
			union{
				HWND hWnd;
				class{
					HWND hWnd;
				public:
					operator int()
					{
						return GetWindowTextLength(hWnd);
					}
				} length;
			};

			template <class T>
			operator std::basic_string<T>()
			{
				std::basic_string<T> Str;
				if(typeid(T) == typeid(WCHAR))
				{
					int Length = GetWindowTextLengthW(hWnd);
					Str.resize(Length);
					GetWindowTextW(hWnd,(LPWSTR)Str.c_str(),Length + 1);
				}else
				{
					int Length = GetWindowTextLengthA(hWnd);
					Str.resize(Length);
					GetWindowTextA(hWnd,(LPSTR)Str.c_str(),Length + 1);
				}
				return Str;
			}

			std::basic_string<WCHAR> & operator=(std::basic_string<WCHAR> & Str)
			{
				SetWindowTextW(hWnd, (LPCWSTR)Str.c_str());
				return Str;
			}

			template <class T>
			std::basic_string<T> & operator=(std::basic_string<T> & Str)
			{

				SetWindowTextA(hWnd, (LPCSTR)Str.c_str());
				return Str;
			}

			LPCSTR operator=(LPCSTR Str)
			{
				SetWindowTextA(hWnd, Str);
				return Str;
			}

			LPCWSTR operator=(LPCWSTR Str)
			{
				SetWindowTextW(hWnd, Str);
				return Str;
			}
			//For int

			operator int()
			{
				return GetDlgItemInt(::GetParent(hWnd),GetDlgCtrlID(hWnd), NULL, TRUE);
			}

			operator unsigned()
			{
				return GetDlgItemInt(::GetParent(hWnd),GetDlgCtrlID(hWnd), NULL, FALSE);
			}

			int operator=(int Val)
			{
				SetDlgItemInt(::GetParent(hWnd),GetDlgCtrlID(hWnd), Val, TRUE);
				return Val;
			}

			unsigned operator=(unsigned Val)
			{
				SetDlgItemInt(::GetParent(hWnd),GetDlgCtrlID(hWnd), Val, FALSE);
				return Val;
			}


			inline int operator()(LPWSTR Buffer, int Len = 0x0fffffff)
			{
				return GetWindowTextW(hWnd, Buffer, Len);	
			}

			inline int operator()(LPSTR Buffer, int Len = 0x0fffffff)
			{
				return GetWindowTextA(hWnd, Buffer, Len);
			}
		} Text;

		class {
			HWND hWnd;
		public:
			inline operator int()
			{
				return GetDlgCtrlID(hWnd);
			}

			int operator=(int NewId)
			{
				return (int)SetWindowLongPtr(hWnd, GWLP_ID, (LONG_PTR)NewId);
			}
		} Id;

		class
		{
			HWND hWnd;
		public:
			inline operator WNDPROC()
			{
				return (WNDPROC)GetWindowLongPtrW(hWnd, GWLP_WNDPROC);
			}

			inline WNDPROC operator=(WNDPROC NewProc)
			{
				return (WNDPROC)SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)NewProc);
			}

			inline LRESULT CALLBACK operator()(UINT Msg, WPARAM wParam, LPARAM lParam)
			{
				return ::SendMessageW(hWnd, Msg,wParam, lParam);
			}

		} WndProcW;

		class
		{
			HWND hWnd;
		public:
			inline operator WNDPROC()
			{
				return (WNDPROC)GetWindowLongPtrA(hWnd, GWLP_WNDPROC);
			}

			inline WNDPROC operator=(WNDPROC NewProc)
			{
				return (WNDPROC)SetWindowLongPtrA(hWnd, GWLP_WNDPROC, (LONG_PTR)NewProc);
			}

			inline LRESULT CALLBACK operator()(UINT Msg, WPARAM wParam, LPARAM lParam)
			{
				return ::SendMessageA(hWnd, Msg,wParam, lParam);
			}
		} WndProcA;

		class
		{
			HWND hWnd;
		public:
			template<class T>
			inline operator T()
			{
				return (T)GetWindowLongPtr(hWnd, GWL_USERDATA);
			}

			template<class T>
			inline T operator=(T NewData)
			{
				return (T)SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)NewData);
			}
		} UserData;

		class{
			HWND hWnd;
		public:
			inline operator LONG_PTR()
			{
				return GetWindowLongPtr(hWnd, GWL_EXSTYLE);
			}

			inline LONG_PTR operator=(LONG_PTR NewStyle)
			{
				return SetWindowLongPtr(hWnd, GWL_EXSTYLE, NewStyle);
			}

		} ExStyle;	

		class{
			HWND hWnd;
		public:
			inline operator LONG_PTR()
			{
				return GetWindowLongPtr(hWnd, GWL_STYLE);
			}

			inline LONG_PTR operator=(LONG_PTR NewStyle)
			{
				return SetWindowLongPtr(hWnd, GWL_STYLE, NewStyle);
			}
		} Style;

		class{
			HWND hWnd;
		public:

			inline operator EX_WND()
			{
				return GetParent(hWnd);
			}

			inline operator HWND()
			{
				return GetParent(hWnd);
			}

			inline EX_WND operator=(EX_WND NewStyle)
			{
				return ::SetParent(hWnd, NewStyle);
			}

			inline HWND operator=(HWND NewStyle)
			{
				return ::SetParent(hWnd, NewStyle);
			}

			EX_WND * operator->()
			{
				return (EX_WND *)&hWnd;
			}
		} Parent;

		class
		{
			HWND hWnd;
		public:
			inline operator HINSTANCE()
			{
				return (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
			}

			inline HINSTANCE operator=(HINSTANCE NewInst)
			{
				return (HINSTANCE)SetWindowLongPtr(hWnd, GWLP_HINSTANCE, (LONG_PTR)NewInst);
			}
		} Instance;

		class{
			HWND hWnd;
		public:

			inline operator std::basic_string<WCHAR>()
			{
				std::basic_string<WCHAR> Str;
				Str.resize(255);
				GetClassNameW(hWnd,(LPWSTR)Str.c_str(),254);
				return Str;
			}

			inline operator std::basic_string<char>()
			{
				std::basic_string<char> Str;
				Str.resize(255);
				GetClassNameA(hWnd,(LPSTR)Str.c_str(),254);
				return Str;
			}
		} NameClass;

		class{
			HWND hWnd;
		public:
			inline operator HDC()
			{
				return ::GetDC(hWnd);
			}
		} Dc;

		class
		{
			HWND hWnd;
		public:
			inline operator bool()
			{
				return ::IsWindow(hWnd);
			}
		} IsWindow;

		class
		{
			HWND hWnd;
		public:
			inline operator bool()
			{
				return ::IsWindowVisible(hWnd);
			}
		} IsVisible;

		class
		{
			HWND hWnd;
		public:
			inline operator bool()
			{
				return ::IsWindowUnicode(hWnd);
			}
		} IsUnicode;

		class{
		   HWND hWnd;
		public:
			operator bool()
			{
				return ::IsIconic(hWnd);
			}
		} IsIconic;

		class
		{
			HWND hWnd;
		public:
			inline WND_COMBO * operator->()
			{
				return (WND_COMBO *)&hWnd;
			}

			inline operator HWND()
			{
				return hWnd;
			}

			inline operator EX_WND()
			{
				return hWnd;
			}
		} AsCombo;


	};



	inline operator HWND()
	{
		return hWnd;
	}

	inline EX_WND()
	{	
		hWnd = NULL;
	}

	inline EX_WND(HWND nhWnd)
	{
		hWnd = nhWnd;
	}


	inline operator bool()
	{
		return hWnd != NULL;
	}

	inline operator EX_WND*()
	{
		return this;
	}

	inline bool operator !()
	{
		return hWnd == NULL;
	}

	inline HWND * operator &()
	{
		return &hWnd;
	}

	inline bool operator==(HWND Wnd)
	{
		return Wnd == hWnd;
	}

	inline bool operator!=(HWND Wnd)
	{
		return Wnd != hWnd;
	}

	inline bool operator==(EX_WND Wnd)
	{
		return Wnd.hWnd == hWnd;
	}

	inline bool operator!=(EX_WND Wnd)
	{
		return Wnd.hWnd != hWnd;
	}

	///

	inline EX_WND operator[](int nIDDlgItem)
	{
		return GetDlgItem(hWnd, nIDDlgItem);
	}

	inline BOOL BringToTop()
	{
	   return BringWindowToTop(hWnd);
	}

	//
	inline BOOL Show(int nCmdShow = SW_SHOW)
	{
		return ShowWindow(hWnd,nCmdShow);
	}
	//

	inline BOOL Update()
	{
		return UpdateWindow(hWnd);
	}
	//

	inline BOOL Invalidate()
	{
		return InvalidateRect(hWnd, NULL, FALSE);
	}

	inline BOOL Invalidate(const RECT *lpRect)
	{
		return InvalidateRect(hWnd, lpRect, FALSE);
	}

	inline BOOL Validate(RECT * lpRect)
	{
		return ::ValidateRect(hWnd, lpRect);
	}

	inline BOOL Validate()
	{
		return ::ValidateRect(hWnd, NULL);
	}

	//
	inline BOOL GetClientCoord(LPRECT lpRect)
	{
		return ::GetClientRect(hWnd, lpRect);
	}

	inline BOOL GetClientCoord(RECT & lpRect)
	{
		return ::GetClientRect(hWnd, &lpRect);
	}

	BOOL GetClientCoord(LPPOINT Point1, LPPOINT Point2)
	{
		RECT Rect;
		if(! ::GetClientRect(hWnd, &Rect))
			return FALSE;
		*Point1 = *(LPPOINT)&Rect;
		*Point2 = *(LPPOINT)&Rect.right;
		return TRUE;
	}

	BOOL GetClientCoord(LPPOINT Point)
	{
		RECT Rect;
		if(! ::GetClientRect(hWnd, &Rect))
			return FALSE;
		*Point = *(LPPOINT)&Rect;
		return TRUE;
	}

	BOOL GetClientCoord(LPLONG x, LPLONG y)
	{
		RECT Rect;
		if(! ::GetClientRect(hWnd, &Rect))
			return FALSE;
		*x = Rect.left;
		*y = Rect.top;
		return TRUE;
	}

	BOOL GetClientCoord(LPLONG x, LPLONG y, LPLONG x2, LPLONG y2)
	{
		RECT Rect;
		if(! ::GetClientRect(hWnd, &Rect))
			return FALSE;
		*x = Rect.left;
		*y = Rect.top;
		*y2 = Rect.bottom;
		*x2  = Rect.right;
		return TRUE;
	}

	long GetClientHeight()
	{
		RECT Rect;
		if(! ::GetClientRect(hWnd, &Rect))
			return -1;
		return Rect.bottom - Rect.top;
	}

	long GetClientWidth()
	{
		RECT Rect;
		if(! ::GetClientRect(hWnd, &Rect))
			return -1;
		return Rect.left - Rect.right;
	}
	//

	BOOL GetCoord(LPRECT lpRect)
	{
		return ::GetWindowRect(hWnd, lpRect);
	}

	BOOL GetCoord(RECT & lpRect)
	{
		return ::GetWindowRect(hWnd, &lpRect);
	}

	BOOL GetCoord(LPPOINT Point1, LPPOINT Point2)
	{
		RECT Rect;
		if(! ::GetWindowRect(hWnd, &Rect))
			return FALSE;
		*Point1 = *(LPPOINT)&Rect;
		*Point2 = *(LPPOINT)&Rect.right;
		return TRUE;
	}

	BOOL GetCoord(LPPOINT Point)
	{
		RECT Rect;
		if(! ::GetWindowRect(hWnd, &Rect))
			return FALSE;
		*Point = *(LPPOINT)&Rect;
		return TRUE;
	}

	BOOL GetCoord(LPLONG x, LPLONG y)
	{
		RECT Rect;
		if(! ::GetWindowRect(hWnd, &Rect))
			return FALSE;
		*x = Rect.left;
		*y = Rect.top;
		return TRUE;
	}

	BOOL GetCoord(LPLONG x, LPLONG y, LPLONG x2, LPLONG y2)
	{
		RECT Rect;
		if(! ::GetWindowRect(hWnd, &Rect))
			return FALSE;
		*x = Rect.left;
		*y = Rect.top;
		*y2 = Rect.bottom;
		*x2  = Rect.right;
		return TRUE;
	}

	BOOL GetSize(LPLONG Width, LPLONG Height)
	{
		RECT Rect;
		if(! ::GetWindowRect(hWnd, &Rect))
			return -1;
		*Height = Rect.bottom - Rect.top;
		*Width = Rect.right - Rect.left;
	}

	long GetHeight()
	{
		RECT Rect;
		if(! ::GetWindowRect(hWnd, &Rect))
			return -1;
		return Rect.bottom - Rect.top;
	}

	long GetWidth()
	{
		RECT Rect;
		if(! ::GetWindowRect(hWnd, &Rect))
			return -1;
		return Rect.left - Rect.right;
	}

	BOOL GetRelParentCoord(LPRECT lpRect)
	{
		if(! ::GetWindowRect(hWnd, lpRect))
			return FALSE;
		MapWindowPoints(HWND_DESKTOP, ::GetParent(hWnd), (LPPOINT)lpRect, 2);
		return TRUE;
	}

	BOOL GetRelParentCoord(LPPOINT Point1, LPPOINT Point2)
	{
		RECT Rect;
		if(! ::GetWindowRect(hWnd, &Rect))
			return FALSE;
		MapWindowPoints(HWND_DESKTOP, ::GetParent(hWnd), (LPPOINT)&Rect, 2);
		*Point1 = *(LPPOINT)&Rect;
		*Point2 = *(LPPOINT)&Rect.right;
		return TRUE;
	}

	BOOL GetRelParentCoord(LPPOINT Point)
	{
		RECT Rect;
		if(! ::GetWindowRect(hWnd, &Rect))
			return FALSE;
		MapWindowPoints(HWND_DESKTOP, ::GetParent(hWnd), (LPPOINT)&Rect, 1);
		*Point = *(LPPOINT)&Rect;
		return TRUE;
	}

	BOOL GetRelParentCoord(LPLONG x, LPLONG y)
	{
		RECT Rect;
		if(! ::GetWindowRect(hWnd, &Rect))
			return FALSE;
		MapWindowPoints(HWND_DESKTOP, ::GetParent(hWnd), (LPPOINT)&Rect, 1);
		*x = Rect.left;
		*y = Rect.top;
		return TRUE;
	}

	BOOL GetRelParentCoord(LPLONG x, LPLONG y, LPLONG x2, LPLONG y2)
	{
		RECT Rect;
		if(! ::GetWindowRect(hWnd, &Rect))
			return FALSE;
		MapWindowPoints(HWND_DESKTOP, ::GetParent(hWnd), (LPPOINT)&Rect, 2);
		*x = Rect.left;
		*y = Rect.top;
		*y2 = Rect.bottom;
		*x2  = Rect.right;
		return TRUE;
	}
	///

	inline EX_WND SetFocus()
	{
		return (EX_WND)::SetFocus(hWnd);
	}

	static inline EX_WND GetFocus()
	{
		return (EX_WND)::GetFocus();
	}

	///////

	BOOL SetCoord(LPRECT lpRect)
	{
		POINT p = *(LPPOINT)lpRect;
		MapWindowPoints(HWND_DESKTOP, ::GetParent(hWnd), &p, 1);
		return MoveWindow(hWnd, p.x, p.y, lpRect->right - lpRect->left, lpRect->bottom - lpRect->top, TRUE);
	}

	BOOL SetCoord(RECT & Rect)
	{
		POINT p = *(LPPOINT)&Rect;
		MapWindowPoints(HWND_DESKTOP, ::GetParent(hWnd), &p, 1);
		return MoveWindow(hWnd, p.x, p.y, Rect.right - Rect.left, Rect.bottom - Rect.top, TRUE);
	}

	BOOL SetCoord(LPPOINT Point1, LPPOINT Point2)
	{
		POINT p = *Point1;
		MapWindowPoints(HWND_DESKTOP, ::GetParent(hWnd), &p, 1);
		return MoveWindow(hWnd, p.x, p.y, Point2->x - Point1->x, Point2->y - Point1->y, TRUE);
	}

	BOOL SetCoord(LPPOINT Point)
	{
		POINT p = *Point;
		MapWindowPoints(HWND_DESKTOP, ::GetParent(hWnd), &p, 1);
		return SetWindowPos(hWnd, NULL, p.x, p.y, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER);
	}

	BOOL SetCoord(LONG x, LONG y)
	{
		POINT p;
		p.x = x;
		p.y = y;
		MapWindowPoints(HWND_DESKTOP, ::GetParent(hWnd), &p, 1);
		return SetWindowPos(hWnd, NULL, p.x, p.y, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER);
	}

	BOOL SetCoord(LONG x, LONG y, LONG x2, LONG y2)
	{
		POINT p;
		p.x = x;
		p.y = y;
		MapWindowPoints(HWND_DESKTOP, ::GetParent(hWnd), &p, 1);
		return MoveWindow(hWnd, p.x, p.y, x2 - x, y2 - y, TRUE);
	}

	BOOL SetSize(LONG Width, LONG Height)
	{
		return SetWindowPos(hWnd, NULL, 0, 0, Width, Height, SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);
	}

	BOOL SetRelParentCoord(LPRECT lpRect)
	{
		return MoveWindow(hWnd, lpRect->left, lpRect->top, lpRect->right - lpRect->left, lpRect->bottom - lpRect->top, TRUE);
	}

	BOOL SetRelParentCoord(RECT & Rect)
	{
		return MoveWindow(hWnd, Rect.left, Rect.top, Rect.right - Rect.left, Rect.bottom - Rect.top, TRUE);
	}

	BOOL SetRelParentCoord(LPPOINT Point1, LPPOINT Point2)
	{
		return MoveWindow(hWnd, Point1->x, Point1->y, Point2->x - Point1->x, Point2->y - Point1->y, TRUE);
	}

	BOOL SetRelParentCoord(LPPOINT Point)
	{
		return SetWindowPos(hWnd, NULL, Point->x, Point->y, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER);
	}

	BOOL SetRelParentCoord(LONG x, LONG y)
	{
		return SetWindowPos(hWnd, NULL, x, y, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER);
	}

	BOOL SetRelParentCoord(LONG x, LONG y, LONG x2, LONG y2)
	{
		return MoveWindow(hWnd, x, y, x2 - x, y2 - y, TRUE);
	}


	//////

	static inline EX_WND ByPoint(LPPOINT Point)
	{
		return  (EX_WND)::WindowFromPoint(*Point);
	}

	inline bool isChild(HWND Wnd)
	{
		return ::IsChild(hWnd, Wnd) == TRUE;
	}

	////

	/*Close window.*/
	inline BOOL Close()
	{
		return ::CloseWindow(hWnd);
	}

	/*Destroys window. */
	inline BOOL Destroy()
	{
		return ::DestroyWindow(hWnd); 
	}

	/*The CreateCompatibleDC function creates a memory device context (DC) compatible with the window.*/
	inline HDC CreateCompatibleDC()
	{
		return ::CreateCompatibleDC(::GetDC(hWnd));
	}

	/*The BeginPaint function prepares the specified window for painting and fills
	a PAINTSTRUCT structure with information about the painting.*/
	inline HDC BeginPaint(LPPAINTSTRUCT ps)
	{
		return ::BeginPaint(hWnd, ps);
	}

	/*The EndPaint function marks the end of painting in the specified window.*/
	inline BOOL EndPaint(LPPAINTSTRUCT ps)
	{
		return ::EndPaint(hWnd, ps);
	}

	/*Destroys a modal dialog box, causing the system to end any processing for the dialog box.*/
	inline BOOL EndDialog(INT_PTR Result = 0)
	{
		return ::EndDialog(hWnd, Result);
	}

	/*Creates a timer with the specified time-out value.*/
	inline UINT_PTR SetTimer(UINT Time, UINT_PTR nIDEvent = NULL, TIMERPROC tp = NULL)
	{
		return ::SetTimer(hWnd,nIDEvent,Time,tp);
	}

	/*Destroys the specified timer.*/
	inline BOOL KillTimer(UINT_PTR nIDEvent = NULL)
	{
		return ::KillTimer(hWnd,nIDEvent);
	}

	inline void SwitchToThisWindow(BOOL fAltTab = FALSE)
	{
		return ::SwitchToThisWindow(hWnd, fAltTab);
	}

private:
	typedef struct
	{
		unsigned MaxElements;
		unsigned CurElement;
		HWND * Buf;
	} FOR_ENUM_WINDOWS;
	static BOOL CALLBACK EnumChildProc(HWND hwnd,LPARAM lParam)
	{
		FOR_ENUM_WINDOWS * arg = (FOR_ENUM_WINDOWS *)lParam;
		if(arg->CurElement < arg->MaxElements)
		{
			arg->Buf[arg->CurElement++] = hwnd;
			return TRUE;
		}
		return FALSE;
	}
public:

	inline BOOL GetChildWindows(HWND * hWndBuf, unsigned MaxElemInBuf)
	{
		FOR_ENUM_WINDOWS arg;
		arg.Buf = hWndBuf;
		arg.MaxElements = MaxElemInBuf;
		arg.CurElement = 0;
		return EnumChildWindows(hWnd, EnumChildProc,(LPARAM)&arg);
	}

	inline BOOL GetChildWindows(HWND * hWndBuf, unsigned MaxElemInBuf, unsigned * CountGetted)
	{
		FOR_ENUM_WINDOWS arg;
		arg.Buf = hWndBuf;
		arg.MaxElements = MaxElemInBuf;
		arg.CurElement = 0;
		BOOL Res = EnumChildWindows(hWnd, EnumChildProc,(LPARAM)&arg);
		*CountGetted = arg.CurElement;
		return Res;
	}

};



#define EX_WND_FIELD_AND_METHODS \
			EX_WND::BringToTop;\
			EX_WND::BeginPaint;\
			EX_WND::ByPoint;\
			EX_WND::CreateCompatibleDC;\
			EX_WND::Close;\
			EX_WND::Destroy;\
			EX_WND::EndDialog;\
			EX_WND::EndPaint;\
			EX_WND::EX_WND;\
			EX_WND::GetChildWindows;\
			EX_WND::GetClientCoord;\
			EX_WND::GetClientHeight;\
			EX_WND::GetClientWidth;\
			EX_WND::GetCoord;\
			EX_WND::GetFocus;\
			EX_WND::GetHeight;\
			EX_WND::GetRelParentCoord;\
			EX_WND::GetSize;\
			EX_WND::GetWidth;\
			EX_WND::Invalidate;\
			EX_WND::isChild;\
			EX_WND::KillTimer;\
			EX_WND::SetCoord;\
			EX_WND::SetFocus;\
			EX_WND::SetRelParentCoord;\
			EX_WND::SetSize;\
			EX_WND::SetTimer;\
			EX_WND::Show;\
			EX_WND::SwitchToThisWindow;\
			EX_WND::Update;\
			EX_WND::Validate;\
			EX_WND::AsCombo;\
			EX_WND::Dc;\
			EX_WND::ExStyle;\
			EX_WND::Id;\
			EX_WND::Instance;\
			EX_WND::IsUnicode;\
			EX_WND::IsWindow;\
			EX_WND::IsVisible;\
			EX_WND::IsIconic;\
			EX_WND::NameClass;\
			EX_WND::Parent;\
			EX_WND::Style;\
			EX_WND::Text;\
			EX_WND::UserData;\
			EX_WND::WndProcA;\
			EX_WND::WndProcW;\
			EX_WND::operator !;\
			EX_WND::operator !=;\
			EX_WND::operator &;\
			EX_WND::operator [];\
			EX_WND::operator ==;\
			EX_WND::operator bool;\
			EX_WND::operator EX_WND*;\
			EX_WND::operator HWND;

class WND_COMBO
{
public:

	union
	{
		HWND hWnd;
		class : public EX_WND
		{
		public:
			EX_WND_FIELD_AND_METHODS;
		};
	};

	WND_COMBO()
	{
	};

	WND_COMBO(EX_WND Wnd)
	{
		hWnd = Wnd;
	}

	operator HWND()
	{
		return hWnd;
	}

	inline int AddItemData(LPARAM Data)
	{
		return ComboBox_AddItemData(hWnd, Data);
	}

	template<typename T>
	inline int AddString(std::basic_string<T> & Str)
	{
		if(typeid(T) == typeid(WCHAR))
			return (int)SendMessageW(hWnd, CB_ADDSTRING, 0L, (LPARAM)Str.c_str());
		else
			return (int)SendMessageA(hWnd, CB_ADDSTRING, 0L, (LPARAM)Str.c_str());
	}

	inline int AddString(LPCSTR Str)
	{
		return (int)SendMessageA(hWnd, CB_ADDSTRING, 0L, (LPARAM)Str);
	}

	inline int AddString(LPCWSTR Str)
	{
		return (int)SendMessageW(hWnd, CB_ADDSTRING, 0L, (LPARAM)Str);
	}

	inline int DeleteString(int Index)
	{
		return ComboBox_DeleteString(hWnd, Index);
	}

	inline int SetCurSel(int Index)
	{
		return ComboBox_SetCurSel(hWnd, Index);
	}

	inline int GetCurSel(int Index)
	{
		return ComboBox_SetCurSel(hWnd, Index);
	}

	inline int FindString(LPCSTR Str, int StartIndex = 0)
	{
		return (int)SendMessageA(hWnd, CB_FINDSTRING, (WPARAM)StartIndex, (LPARAM)Str);
	}

	inline int FindString(LPCWSTR Str, int StartIndex = 0)
	{
		return (int)SendMessageW(hWnd, CB_FINDSTRING, (WPARAM)StartIndex, (LPARAM)Str);
	}

	template<typename T>
	inline int FindString(std::basic_string<T> & Str, int StartIndex = 0)
	{
		if(typeid(T) == typeid(WCHAR))
			return (int)SendMessageW(hWnd, CB_FINDSTRING, (WPARAM)StartIndex, (LPARAM)Str.c_str());
		else
			return (int)SendMessageA(hWnd, CB_FINDSTRING, (WPARAM)StartIndex, (LPARAM)Str.c_str());
	}

	inline int FindItemData(LPARAM data, int StartIndex = 0)
	{
		return ComboBox_FindItemData(hWnd, StartIndex, data);
	}

	int operator[](LPCWSTR FindStr)
	{
		return FindString(FindStr);
	}

	int operator[](LPCSTR FindStr)
	{
		return FindString(FindStr);
	}

	template<typename T>
	int operator[](std::basic_string<T> & Str)
	{
		return FindString(Str);
	}

	int GetCount()
	{
		return ComboBox_GetCount(hWnd);
	}

	int ResetContent()
	{
		return ComboBox_ResetContent(hWnd);
	}

	/*Adds a string to a list in a combo box at the specified location.*/
	int InsertString(LPCSTR Str, int Index = 0)
	{
		return (int)SendMessageA(hWnd, CB_INSERTSTRING, (WPARAM)Index, (LPARAM)Str);
	}


	int InsertString(LPCWSTR Str, int Index = 0)
	{
		return (int)SendMessageW(hWnd, CB_INSERTSTRING, (WPARAM)Index, (LPARAM)Str);
	}

	/*Enable control*/
	BOOL Enable()
	{
		return ComboBox_Enable(hWnd, TRUE);
	}

	/*Disable control*/
	BOOL Disable()
	{
		return ComboBox_Enable(hWnd, FALSE);
	}

	int GetLenTextItem(int Index = 0)
	{
		return ComboBox_GetLBTextLen(hWnd, Index);
	}


	/*Gets a string from a list in a combo box.*/
	inline int GetTextItem(LPSTR Str, int Index = 0)
	{
		return (int)SendMessageA(hWnd, CB_GETLBTEXT, (WPARAM)Index, (LPARAM)Str);
	}

	inline int GetTextItem(LPCWSTR Str, int Index = 0)
	{
		return (int)SendMessageW(hWnd, CB_GETLBTEXT, (WPARAM)Index, (LPARAM)Str);
	}

	template<typename T>
	inline int GetTextItem(std::basic_string<T> & Str, int Index = 0)
	{
		Str.resize(ComboBox_GetLBTextLen(hWnd, Index));
		if(typeid(T) == typeid(WCHAR))
			return (int)SendMessageW(hWnd, CB_GETLBTEXT, (WPARAM)Index, (LPARAM)Str.c_str());
		else
			return (int)SendMessageA(hWnd, CB_GETLBTEXT, (WPARAM)Index, (LPARAM)Str.c_str());
	}


	/*Limits the length of the text the user may type into the edit control of a combo box.*/
	inline int SetLimit(int cchMax)
	{
		return ComboBox_LimitText(hWnd, cchMax);
	}

	inline LRESULT GetItemData(int Index)
	{
		return ComboBox_GetItemData(hWnd, Index);
	}


};

#endif

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



#endif
