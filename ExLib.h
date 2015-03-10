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
	T Buf[200];
	if(typeid(T) == typeid(wchar_t))
		swprintf((wchar_t*)Buf,200,L"%i", Val);
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


};

#ifdef WIN32

class EX_WND
{
private:
	HWND hWnd;

public:

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

	inline BOOL SetText(LPCWSTR NewText)
	{
		return SetWindowTextW(hWnd, NewText);	
	}

	inline BOOL SetText(LPCSTR NewText)
	{
		return SetWindowTextA(hWnd, NewText);
	}

	inline BOOL SetTextItem(LPCWSTR NewText, int nIDDlgItem)
	{
		return SetDlgItemTextW(hWnd,nIDDlgItem, NewText);	
	}

	inline BOOL SetTextItem(LPCSTR NewText, int nIDDlgItem)
	{
		return SetDlgItemTextA(hWnd,nIDDlgItem,NewText);
	}

	template<typename T>
	inline BOOL SetText(std::basic_string<T> & Str)
	{
		if(typeid(T) == typeid(wchar_t))
		{
			return SetWindowTextW(hWnd, (LPCWSTR)Str.c_str());
		}else
		{
			return SetWindowTextA(hWnd, (LPCSTR)Str.c_str());
		}
	}

	template<typename T>
	inline BOOL SetTextItem(std::basic_string<T> & Str, int nIDDlgItem)
	{
		if(typeid(T) == typeid(wchar_t))
		{
			return SetDlgItemTextW(hWnd,nIDDlgItem,(LPCWSTR)Str.c_str());
		}else
		{
			return SetDlgItemTextA(hWnd,nIDDlgItem,(LPCSTR)Str.c_str());
		}
	}

	//////

	inline int GetTextLength()
	{
		return GetWindowTextLengthW(hWnd);
	}

	inline int GetText(LPWSTR Buffer, int Len = 0x0fffffff)
	{
		return GetWindowTextW(hWnd, Buffer, Len);	
	}

	inline int GetText(LPSTR Buffer, int Len = 0x0fffffff)
	{
		return GetWindowTextA(hWnd, Buffer, Len);
	}

	inline UINT GetTextItem(LPWSTR Buffer, int nIDDlgItem, int Len = 0x0fffffff)
	{
		return GetDlgItemTextW(hWnd,nIDDlgItem, Buffer, Len);	
	}

	inline UINT GetTextItem(LPSTR Buffer, int nIDDlgItem, int Len = 0x0fffffff)
	{
		return GetDlgItemTextA(hWnd,nIDDlgItem, Buffer, Len);	
	}

	template<typename T>
	inline int GetText(std::basic_string<T> & Str)
	{
		if(typeid(T) == typeid(wchar_t))
		{
			int Length = GetWindowTextLengthW(hWnd);
			Str.resize(Length);
			return GetWindowTextW(hWnd,(LPWSTR)Str.c_str(),Length + 1);
		}else
		{
			int Length = GetWindowTextLengthA(hWnd);
			Str.resize(Length);
			return GetWindowTextA(hWnd,(LPSTR)Str.c_str(),Length + 1);
		}
	}

	template<typename T>
	inline int GetTextItem(std::basic_string<T> & Str, int nIDDlgItem)
	{		
		HWND Wnd = GetDlgItem(hWnd, nIDDlgItem);
		if(Wnd == NULL)
		{
			Str.clear();
			return 0;
		}
		if(typeid(T) == typeid(wchar_t))
		{
			int Length = GetWindowTextLengthW(Wnd);
			Str.resize(Length);
			return GetWindowTextW(Wnd,(LPWSTR)Str.c_str(),Length + 1);
		}else
		{
			int Length = GetWindowTextLengthA(Wnd);
			Str.resize(Length);
			return GetWindowTextA(Wnd,(LPSTR)Str.c_str(),Length + 1);
		}
	}
	//

	inline unsigned GetUnsigned(BOOL * lpTranslated = NULL)
	{
		return GetDlgItemInt(::GetParent(hWnd),GetDlgCtrlID(hWnd), lpTranslated, FALSE);
	}

	inline int GetInt(BOOL * lpTranslated = NULL)
	{
		return GetDlgItemInt(::GetParent(hWnd),GetDlgCtrlID(hWnd), lpTranslated, TRUE);
	}

	inline unsigned GetUnsignedItem(int nIDDlgItem,BOOL * lpTranslated = NULL)
	{
		return GetDlgItemInt(hWnd,nIDDlgItem, lpTranslated, FALSE);
	}

	inline int GetIntItem(int nIDDlgItem, BOOL * lpTranslated = NULL)
	{
		return GetDlgItemInt(hWnd,nIDDlgItem, lpTranslated, TRUE);
	}

	///

	inline BOOL SetUnsigned(unsigned Val)
	{
		return SetDlgItemInt(::GetParent(hWnd),GetDlgCtrlID(hWnd), Val, FALSE);
	}

	inline BOOL SetInt(int Val)
	{
		return SetDlgItemInt(::GetParent(hWnd),GetDlgCtrlID(hWnd), Val, TRUE);
	}

	inline BOOL SetUnsignedItem(unsigned Val, int nIDDlgItem)
	{
		return SetDlgItemInt(hWnd,nIDDlgItem, Val, FALSE);
	}

	inline BOOL SetIntItem(int Val, int nIDDlgItem)
	{
		return SetDlgItemInt(hWnd,nIDDlgItem, Val, TRUE);
	}

	///

	inline EX_WND operator[](int nIDDlgItem)
	{
		return GetItem(nIDDlgItem);
	}

	inline EX_WND GetItem(int nIDDlgItem)
	{
		return (EX_WND)GetDlgItem(hWnd,nIDDlgItem);
	}

	inline EX_WND GetParent()
	{
		return (EX_WND)::GetParent(hWnd);
	}

	inline EX_WND SetParent(HWND NewParent)
	{
		return (EX_WND)::SetParent(hWnd, NewParent);
	}

	inline int GetId()
	{
		return GetDlgCtrlID(hWnd);
	}

	inline int SetId(int NewId)
	{
		return (int)SetWindowLongPtr(hWnd, GWLP_ID, (LONG_PTR)NewId);
	}

	////

	inline WNDPROC GetProc()
	{
		return (WNDPROC)GetWindowLongPtr(hWnd, GWLP_WNDPROC);
	}

	inline WNDPROC SetProc(WNDPROC NewProc)
	{
		return (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)NewProc);
	}

	// 

	inline LONG_PTR GetUserData()
	{
		return GetWindowLongPtr(hWnd, GWL_USERDATA);
	}

	inline LONG_PTR SetProc(LONG_PTR NewData)
	{
		return SetWindowLongPtr(hWnd, GWL_USERDATA, NewData);
	}

	//

	inline HINSTANCE GetInstance()
	{
		return (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	}

	inline HINSTANCE SetInstance(HINSTANCE NewInst)
	{
		return (HINSTANCE)SetWindowLongPtr(hWnd, GWLP_HINSTANCE, (LONG_PTR)NewInst);
	}
	///

	inline LONG_PTR GetStyle()
	{
		return GetWindowLongPtr(hWnd, GWL_STYLE);
	}

	inline LONG_PTR SetStyle(LONG_PTR NewStyle)
	{
		return SetWindowLongPtr(hWnd, GWL_STYLE, NewStyle);
	}

	//
	inline LONG_PTR GetExStyle()
	{
		return GetWindowLongPtr(hWnd, GWL_EXSTYLE);
	}

	inline LONG_PTR SetExStyle(LONG_PTR NewStyle)
	{
		return SetWindowLongPtr(hWnd, GWL_EXSTYLE, NewStyle);
	}

	//
	template<typename T>
	inline int GetClassName(std::basic_string<T> & Str)
	{
		Str.resize(255);
		if(typeid(T) == typeid(wchar_t))
		{
			return GetClassNameW(hWnd,(LPWSTR)Str.c_str(),254);
		}else
		{
			return GetClassNameA(hWnd,(LPSTR)Str.c_str(),254);
		}
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


	inline bool isWindow()
	{
		return ::IsWindow(hWnd);
	}

	bool isVisible()
	{
		return ::IsWindowVisible(hWnd);
	}

	////

	inline BOOL Close()
	{
		return ::CloseWindow(hWnd);
	}

	inline BOOL Destoy()
	{
		return ::DestroyWindow(hWnd); 
	}

    inline HDC GetDC()
	{
		return ::GetDC(hWnd);
	}

	inline HDC CreateCompatibleDC()
	{
		return ::CreateCompatibleDC(::GetDC(hWnd));
	}

	inline HDC BeginPaint(LPPAINTSTRUCT ps)
	{
		return ::BeginPaint(hWnd, ps);
	}

	inline BOOL EndPaint(LPPAINTSTRUCT ps)
	{
		return ::EndPaint(hWnd, ps);
	}

	inline BOOL EndDialog(INT_PTR Result = 0)
	{
		return ::EndDialog(hWnd, Result);
	}

	inline UINT_PTR SetTimer(UINT Time, UINT_PTR nIDEvent = NULL, TIMERPROC tp = NULL)
	{
		return ::SetTimer(hWnd,nIDEvent,Time,tp);
	}

	inline BOOL KillTimer(UINT_PTR nIDEvent = NULL)
	{
		return ::KillTimer(hWnd,nIDEvent);
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
