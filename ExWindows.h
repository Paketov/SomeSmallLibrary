#ifndef __EX_WINDOWS_H__
#define __EX_WINDOWS_H__

#include <string>
#include <sstream>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <wchar.h>
#include <typeinfo>
#include <Windows.h>
#include <Windowsx.h>


#ifdef USE_WONDER_CONTROLS

#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")
#endif

template<bool>
class WND_COMBO__;
template<bool>
class WND_LIST__;
template<bool>
class EX_WND__;


typedef WND_COMBO__<true>	WND_COMBO;
typedef WND_LIST__<true>	WND_LIST;
typedef EX_WND__<true>		EX_WND;


template<bool>
class EX_WND__
{
public:
	union
	{	
		HWND hWnd;

		/*
		*std::string, LPCWSTR, LPSTR, int; Set, Get;
		*Changes the text of the specified window's title bar.
		*/
		class {
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

			operator std::basic_string<WCHAR>()
			{
				std::basic_string<WCHAR> Str;
				int Length = GetWindowTextLengthW(hWnd);
				Str.resize(Length);
				GetWindowTextW(hWnd,(LPWSTR)Str.c_str(),Length + 1);
				return Str;
			}

			operator std::basic_string<CHAR>()
			{
				std::basic_string<CHAR> Str;
				int Length = GetWindowTextLengthA(hWnd);
				Str.resize(Length);
				GetWindowTextA(hWnd,(LPSTR)Str.c_str(),Length + 1);
				return Str;
			}

			std::basic_string<WCHAR> & operator=(std::basic_string<WCHAR> & Str)
			{
				SetWindowTextW(hWnd, (LPCWSTR)Str.c_str());
				return Str;
			}

			std::basic_string<char> & operator=(std::basic_string<CHAR> & Str)
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

			template<class T>
			bool operator==(std::basic_string<T> & Str2)
			{
				return (std::basic_string<T>)*this == Str2;
			}

			bool operator==(LPWSTR Str2)
			{
				return (std::basic_string<WCHAR>)*this == Str2;
			}

			bool operator==(LPCSTR Str2)
			{
				return (std::basic_string<CHAR>)*this == Str2;
			}

			template<class T>
			bool operator!=(T Val)
			{
				return !operator==(Val);
			}

			//For int

			bool operator>(unsigned Val)
			{
				return (unsigned)*this > Val;
			}

			bool operator<(unsigned Val)
			{
				return (unsigned)*this < Val;
			}

			bool operator>=(unsigned Val)
			{
				return (unsigned)*this > Val;
			}

			bool operator<=(unsigned Val)
			{
				return (unsigned)*this < Val;
			}

			bool operator>(int Val)
			{
				return (int)*this > Val;
			}

			bool operator<(int Val)
			{
				return (int)*this < Val;
			}

			bool operator>=(int Val)
			{
				return (int)*this > Val;
			}

			bool operator<=(int Val)
			{
				return (int)*this < Val;
			}

			bool operator==(int Val)
			{
				return (int)*this == Val;
			}

			bool operator==(unsigned Val)
			{
				return (unsigned)*this == Val;
			}

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

		/*
		*int; Set, Get;
		*Sets a new identifier of the child window. The window cannot be a top-level window.
		*/
		class {
			HWND hWnd;
		public:
			inline operator int()
			{
				return ::GetDlgCtrlID(hWnd);
			}

			int operator=(int NewId)
			{
				return (int)SetWindowLongPtr(hWnd, GWLP_ID, (LONG_PTR)NewId);
			}
		} Id;

		/*
		*BOOL; Set, Get;
		*Enables or disables mouse and keyboard input to the specified window or control.
		*/
		class 
		{
			HWND hWnd;
		public:
			operator BOOL()
			{
				return ::IsWindowEnabled(hWnd);
			}

			BOOL operator=(BOOL isEnable)
			{ 
				return ::EnableWindow(hWnd, isEnable);
			}
		} Enable;

		/*
		*WNDPROC; Set, Get;
		*Sets a new address for the window procedure.
		*/
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
				return ::SendMessageW(hWnd, Msg, wParam, lParam);
			}
		} WndProcW;

		/*
		*WNDPROC; Set, Get;
		*Sets a new address for the window procedure.
		*/
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

		/*
		*<variant 4 or 8 bytes>; Set, Get;
		*Sets the user data associated with the window.
		*/
		class
		{
			HWND hWnd;
		public:
			template<class T>
			inline operator T()
			{
				return (T)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			}

			template<class T>
			inline T operator=(T NewData)
			{
				return (T)SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)NewData);
			}
		} UserData;

		/*
		*LONG_PTR; Set, Get;
		*Sets a new extended window style. 
		*/
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

		/*
		*LONG_PTR; Set, Get;
		*Sets a new window style.
		*/
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

		/*
		*HINSTANCE; Set, Get;
		*Sets a new application instance handle.
		*/
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

		/*
		*std::string, LPWSTR, LPSTR; Get;
		*Retrieves the name of the class to which the specified window belongs. 
		*/
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

			inline operator std::basic_string<CHAR>()
			{
				std::basic_string<CHAR> Str;
				Str.resize(255);
				GetClassNameA(hWnd,(LPSTR)Str.c_str(),254);
				return Str;
			}

			inline int operator()(LPSTR Buf, unsigned Len)
			{
				return GetClassNameA(hWnd, Buf, Len);
			}

			inline int operator()(LPWSTR Buf, unsigned Len)
			{
				return GetClassNameW(hWnd, Buf, Len);
			}
		} NameClass;

		/*
		*WNDCLASS, WNDCLASSEX; Get;
		*Retrieves information about a window class, including a handle to the small icon associated with the window class.
		*/
		class 
		{
			HWND hWnd;
		public:

			inline operator WNDCLASSW()
			{		
				std::basic_string<WCHAR> Name = ((EX_WND__*)this)->NameClass;
				WNDCLASSW WndClass = {0};
				GetClassInfoW(((EX_WND__*)this)->Instance,Name.c_str(),&WndClass);
				return WndClass;
			}

			inline operator WNDCLASSA()
			{		
				std::basic_string<CHAR> Name = ((EX_WND__*)this)->NameClass;
				WNDCLASSA WndClass = {0};
				GetClassInfoA(((EX_WND__*)this)->Instance,Name.c_str(),&WndClass);
				return WndClass;
			}

			inline operator WNDCLASSEXW()
			{
				std::basic_string<WCHAR> Name = ((EX_WND__*)this)->NameClass;
				WNDCLASSEXW WndClass = {0};
				WndClass.cbSize = sizeof(WndClass);
				GetClassInfoExW(((EX_WND__*)this)->Instance, Name.c_str(),&WndClass);
				return WndClass;
			}

			inline operator WNDCLASSEXA()
			{
				std::basic_string<CHAR> Name = ((EX_WND__*)this)->NameClass;
				WNDCLASSEXA WndClass = {0};
				WndClass.cbSize = sizeof(WndClass);
				GetClassInfoExA(((EX_WND__*)this)->Instance, Name.c_str(),&WndClass);
				return WndClass;
			}

			inline bool operator()(LPWNDCLASSW lpWndClass)
			{		
				std::basic_string<WCHAR> Name = ((EX_WND__*)this)->NameClass;
				return GetClassInfoW(((EX_WND__*)this)->Instance,Name.c_str(),lpWndClass) != 0;
			}

			inline bool operator()(LPWNDCLASSA lpWndClass)
			{		
				std::basic_string<CHAR> Name = ((EX_WND__*)this)->NameClass;
				return GetClassInfoA(((EX_WND__*)this)->Instance,Name.c_str(),lpWndClass) != 0;
			}

			inline bool operator()(LPWNDCLASSEXW lpWndClass)
			{		
				std::basic_string<WCHAR> Name = ((EX_WND__*)this)->NameClass;
				return GetClassInfoExW(((EX_WND__*)this)->Instance,Name.c_str(),lpWndClass) != 0;
			}

			inline bool operator()(LPWNDCLASSEXA lpWndClass)
			{		
				std::basic_string<CHAR> Name = ((EX_WND__*)this)->NameClass;
				return GetClassInfoExA(((EX_WND__*)this)->Instance,Name.c_str(),lpWndClass) != 0;
			}
		} Class;

		/*
		*HDC; Get;
		*Retrieves a handle to a device context (DC) for the client area of a specified window or for the entire screen.
		*/
		class{
			HWND hWnd;
		public:
			inline operator HDC()
			{
				return ::GetDC(hWnd);
			}
		} Dc;

		/*
		*EX_WND__, HWND; Get, Set;
		*Changes the parent window of the specified child window. 
		*/
		class
		{
			HWND hWnd;
		public:

			inline operator EX_WND__()
			{
				return ::GetParent(hWnd);
			}

			inline operator HWND()
			{
				return ::GetParent(hWnd);
			}

			inline EX_WND__ operator=(EX_WND__ NewStyle)
			{
				return ::SetParent(hWnd, NewStyle);
			}

			inline HWND operator=(HWND NewStyle)
			{
				return ::SetParent(hWnd, NewStyle);
			}

			inline EX_WND__ operator()()
			{
				return ::GetParent(hWnd);
			}
		} Parent;

		/*
		*bool; Get;
		*Determines whether the specified window handle identifies an existing window. 
		*/
		class
		{
			HWND hWnd;
		public:
			inline operator bool()
			{
				return ::IsWindow(hWnd) != 0;
			}
		} IsWindow;

		/*
		*bool; Get;
		*Determines the visibility state of the specified window. 
		*/
		class
		{
			HWND hWnd;
		public:
			inline operator bool()
			{
				return ::IsWindowVisible(hWnd);
			}
		} IsVisible;

		/*
		*bool; Get;
		*Determines whether the specified window is a native Unicode window. 
		*/
		class
		{
			HWND hWnd;
		public:
			inline operator bool()
			{
				return ::IsWindowUnicode(hWnd);
			}
		} IsUnicode;

		/*
		*bool; Get;
		*Determines whether the specified window is minimized (iconic).
		*/
		class{
			HWND hWnd;
		public:
			operator bool()
			{
				return ::IsIconic(hWnd);
			}
		} IsIconic;

		/*
		*WND_COMBO__, HWND, EX_WND__; Get;
		*Represet window as ComboBox.
		*/
		//WND_COMBO__ AsCombo;

	};

	inline operator HWND()
	{
		return hWnd;
	}

	inline EX_WND__()
	{	
		hWnd = NULL;
	}

	inline EX_WND__(HWND nhWnd)
	{
		hWnd = nhWnd;
	}

	inline operator bool()
	{
		return hWnd != NULL;
	}

	inline operator EX_WND__*()
	{
		return this;
	}

	inline bool operator !()
	{
		return hWnd == NULL;
	}

private:
	class _FOR_GET_ADDRESS
	{
		HWND hWnd;
	public:
		inline operator HWND*()
		{
			return &hWnd;
		}

		inline operator EX_WND__*()
		{
			return (EX_WND__*)&hWnd;
		}
	};
public:

	inline _FOR_GET_ADDRESS & operator &()
	{
		return *(_FOR_GET_ADDRESS*)this;
	}

	inline bool operator==(HWND Wnd)
	{
		return Wnd == hWnd;
	}

	inline bool operator!=(HWND Wnd)
	{
		return Wnd != hWnd;
	}

	inline bool operator==(EX_WND__ Wnd)
	{
		return Wnd.hWnd == hWnd;
	}

	inline bool operator!=(EX_WND__ Wnd)
	{
		return Wnd.hWnd != hWnd;
	}

	///
	inline EX_WND__ operator[](int nIDDlgItem)
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

	inline EX_WND__ SetFocus()
	{
		return (EX_WND__)::SetFocus(hWnd);
	}

	static inline EX_WND__ GetFocus()
	{
		return (EX_WND__)::GetFocus();
	}


	static EX_WND CreateDlg(int IdRes, DLGPROC WindowProc, HINSTANCE ProcessHandler = NULL, HWND ParentWindowHandler = NULL)
	{
		return CreateDialog(ProcessHandler, MAKEINTRESOURCE(IdRes), ParentWindowHandler, WindowProc);
	}

	static EX_WND CreateDlg(LPCSTR Name, DLGPROC WindowProc, HINSTANCE ProcessHandler = NULL, HWND ParentWindowHandler = NULL)
	{
		return CreateDialogA(ProcessHandler, Name, ParentWindowHandler, WindowProc);
	}

	static EX_WND CreateDlg(LPCWSTR Name, DLGPROC WindowProc, HINSTANCE ProcessHandler = NULL, HWND ParentWindowHandler = NULL)
	{
		return CreateDialogW(ProcessHandler, Name, ParentWindowHandler, WindowProc);
	}

	static void EnterMainLoop()
	{
		for (MSG msg; GetMessage(&msg, NULL, NULL, NULL);) 
		{  
			TranslateMessage(&msg); 
			DispatchMessage(&msg);
		}
	}

	static void EnterMainLoop(DLGPROC ProcForInterceptKeys)
	{
		for (MSG msg; GetMessage(&msg, NULL, NULL, NULL);) 
		{  
			TranslateMessage(&msg); 
			DispatchMessage(&msg);
			if((msg.message == WM_KEYDOWN) || (msg.message == WM_KEYUP))
				ProcForInterceptKeys(msg.hwnd, msg.message, msg.wParam, msg.lParam);
		}
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

	static inline EX_WND__ ByPoint(LPPOINT Point)
	{
		return  (EX_WND__)::WindowFromPoint(*Point);
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
	EX_WND::Class;\
	EX_WND::BringToTop;\
	EX_WND::BeginPaint;\
	EX_WND::ByPoint;\
	EX_WND::CreateCompatibleDC;\
	EX_WND::Close;\
	EX_WND::Destroy;\
	EX_WND::EndDialog;\
	EX_WND::EndPaint;\
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
	/*EX_WND::AsCombo;*/\
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
	EX_WND::operator EX_WND__*;\
	EX_WND::operator HWND;

template<bool>
class WND_COMBO__
{
public:
	class ITEM
	{
	public:
		union
		{
			struct
			{
				HWND hWnd;
				int Index;
			};

			class TEXT_PROP_
			{
			public:

				class 
				{
					HWND hWnd;
					int Index;
					friend TEXT_PROP_;
				public:
					inline operator int()
					{
						return ComboBox_GetLBTextLen(hWnd, Index);
					}
				} length;


				LPCSTR operator=(LPCSTR Str)
				{		
					auto Data = ComboBox_GetItemData(length.hWnd, length.Index);
					ComboBox_DeleteString(length.hWnd, length.Index);
					length.Index = SendMessageA(length.hWnd, CB_INSERTSTRING, (WPARAM)length.Index, (LPARAM)Str);
					ComboBox_SetItemData(length.hWnd, length.Index, Data);
					return Str;
				}

				LPCWSTR operator=(LPCWSTR Str)
				{
					auto Data = ComboBox_GetItemData(length.hWnd, length.Index);
					ComboBox_DeleteString(length.hWnd, length.Index);
					length.Index = SendMessageW(length.hWnd, CB_INSERTSTRING, (WPARAM)length.Index, (LPARAM)Str);
					ComboBox_SetItemData(length.hWnd, length.Index, Data);
					return Str;
				}

				template<class T>
				inline std::basic_string<T> & operator=(std::basic_string<T> & Str)
				{
					*this = Str.c_str();
					return Str;
				}

				inline operator std::basic_string<CHAR>()
				{
					std::basic_string<CHAR> Str;
					Str.resize(length + 1);
					SendMessageA(length.hWnd, CB_GETLBTEXT, (WPARAM)length.Index, (LPARAM)Str.c_str());
					return Str;
				}

				inline operator std::basic_string<WCHAR>()
				{
					std::basic_string<WCHAR> Str;
					Str.resize(length + 1);
					SendMessageW(length.hWnd, CB_GETLBTEXT, (WPARAM)length.Index, (LPARAM)Str.c_str());
					return Str;
				}

				inline int operator()(LPCWSTR Str)
				{
					return (int)SendMessageW(length.hWnd, CB_GETLBTEXT, (WPARAM)length.Index, (LPARAM)Str);
				}

				inline int operator()(LPCSTR Str)
				{
					return (int)SendMessageW(length.hWnd, CB_GETLBTEXT, (WPARAM)length.Index, (LPARAM)Str);
				}

			} Text;

			class 
			{
				HWND hWnd;
				int Index;
			public:
				template<class T>
				operator T()
				{
					return (T)ComboBox_GetItemData(hWnd, Index);
				}

				template<class T>
				T operator =(T Data)
				{
					ComboBox_SetItemData(hWnd, Index, Data);
					return Data;
				}
			} Data;
		};

		ITEM(int Index, HWND hWnd)
		{
			this->hWnd = hWnd;
			this->Index = Index;
		}

		int Delete()
		{
			return ComboBox_DeleteString(hWnd, Index);
		}
	};

	union
	{
		HWND hWnd;
		class : public EX_WND
		{
		public:
			EX_WND_FIELD_AND_METHODS;
		};

		class 
		{
			HWND hWnd;
		public:
			inline operator int()
			{
				return ComboBox_GetCurSel(hWnd);
			}

			inline int operator=(int Index)
			{
				return ComboBox_SetCurSel(hWnd, Index);
			}

			inline ITEM & operator=(ITEM & Item)
			{
				ComboBox_SetCurSel(hWnd, Item.Index);
				return Item;
			}

			inline operator ITEM()
			{
				return ITEM(operator int(), hWnd);
			}

			inline ITEM operator()()
			{
				return ITEM(operator int(), hWnd);
			}
		} CurSel;

		class 
		{
			HWND hWnd;
		public:
			inline operator int()
			{
				return ComboBox_GetCount(hWnd);
			}
		} Count;
	};

	inline ITEM operator[](int Index)
	{
		return ITEM(Index, hWnd);
	}

	inline ITEM Add()
	{
		int Index = (int)SendMessageA(hWnd, CB_ADDSTRING, 0L, (LPARAM)"");
		return ITEM(Index, hWnd);
	}

	inline ITEM Add(LPCWSTR Str)
	{
		int Index = (int)SendMessageW(hWnd, CB_ADDSTRING, 0L, (LPARAM)Str);
		return ITEM(Index, hWnd);
	}

	inline ITEM Add(LPCSTR Str)
	{
		int Index = (int)SendMessageA(hWnd, CB_ADDSTRING, 0L, (LPARAM)Str);
		return ITEM(Index, hWnd);
	}

	inline ITEM Add(std::basic_string<WCHAR> & Str)
	{
		int Index = (int)SendMessageW(hWnd, CB_ADDSTRING, 0L, (LPARAM)Str.c_str());
		return ITEM(Index, hWnd);
	}

	inline ITEM Add(std::basic_string<CHAR> & Str)
	{
		int Index = (int)SendMessageA(hWnd, CB_ADDSTRING, 0L, (LPARAM)Str.c_str());
		return ITEM(Index, hWnd);
	}

	template<class TSTR, class TDATA>
	inline ITEM Add(TSTR Str, TDATA Data)
	{
		ITEM Item = Add(Str);
		Item.Data = Data;
		return Item;
	}

	WND_COMBO__()
	{
	};

	WND_COMBO__(EX_WND Wnd)
	{
		hWnd = Wnd;
	}

	operator HWND()
	{
		return hWnd;
	}

	inline ITEM FindByString(LPCSTR Str, int StartIndex = 0)
	{
		int Index = (int)SendMessageA(hWnd, CB_FINDSTRING, (WPARAM)StartIndex, (LPARAM)Str);
		return ITEM(Index, hWnd);
	}

	inline ITEM FindByString(LPCWSTR Str, int StartIndex = 0)
	{
		int Index = (int)SendMessageW(hWnd, CB_FINDSTRING, (WPARAM)StartIndex, (LPARAM)Str);
		return ITEM(Index, hWnd);
	}

	inline ITEM FindByString(std::basic_string<CHAR> & Str, int StartIndex = 0)
	{
		int Index = (int)SendMessageA(hWnd, CB_FINDSTRING, (WPARAM)StartIndex, (LPARAM)Str.c_str());
		return ITEM(Index, hWnd);
	}

	inline ITEM FindByString(std::basic_string<WCHAR> & Str, int StartIndex = 0)
	{
		int Index = (int)SendMessageW(hWnd, CB_FINDSTRING, (WPARAM)StartIndex, (LPARAM)Str.c_str());
		return ITEM(Index, hWnd);
	}

	inline ITEM FindByItemData(LPARAM data, int StartIndex = 0)
	{
		return ITEM(ComboBox_FindItemData(hWnd, StartIndex, data), hWnd);
	}

	int ResetContent()
	{
		return ComboBox_ResetContent(hWnd);
	}

	/*Limits the length of the text the user may type into the edit control of a combo box.*/
	inline int SetLimit(int cchMax)
	{
		return ComboBox_LimitText(hWnd, cchMax);
	}
};

template<bool>
class WND_LIST__
{
public:
	WND_LIST__()
	{
	   hWnd = NULL;
	}

		
	WND_LIST__(EX_WND Wnd)
	{
		hWnd = Wnd;
	}

	class COLUMN
	{

	public:	
		union
		{
			struct
			{
				HWND hWnd;
				int Index;
			};

			class
			{
				struct
				{
					HWND hWnd;
					int Index;
				};

				inline LPSTR operator =(LPSTR Val)
				{
					LV_COLUMNA Col;
					Col.mask = LVCF_TEXT;
					Col.pszText = Val;
					SendMessageA(hWnd, LVM_SETCOLUMNA, (WPARAM)Index, (LPARAM)&Col);
					return Val;
				}

				inline LPWSTR operator =(LPWSTR Val)
				{
					LV_COLUMNW Col;
					Col.mask = LVCF_TEXT;
					Col.pszText = Val;
					SendMessageW(hWnd, LVM_SETCOLUMNW, (WPARAM)Index, (LPARAM)&Col);
					return Val;
				}

				template <class T>
				inline std::basic_string<T> & operator =(std::basic_string<T> & Val)
				{
					*this = Val.c_str();
					return Val;
				}

				inline operator std::basic_string<CHAR>()
				{
					std::basic_string<CHAR> Str;
					LV_COLUMNA Col;
					Col.mask = LVCF_TEXT;
					Col.cchTextMax = 0;
					do
					{
						Col.cchTextMax = (Col.cchTextMax | 1111) + 1;
						Str.resize(Col.cchTextMax);
						Col.pszText = (LPSTR)Str.c_str();
						SendMessageA(hWnd, LVM_GETCOLUMNA, (WPARAM)Index, (LPARAM)&Col);
					}while(Str.length() < (Col.cchTextMax - 1));
					return Str;
				}

				inline operator std::basic_string<WCHAR>()
				{
					std::basic_string<WCHAR> Str;
					LV_COLUMNW Col;
					Col.mask = LVCF_TEXT;
					Col.cchTextMax = 0;
					do
					{
						Col.cchTextMax = (Col.cchTextMax | 1111) + 1;
						Str.resize(Col.cchTextMax);
						Col.pszText = (LPWSTR)Str.c_str();
						SendMessageW(hWnd, LVM_GETCOLUMNW, (WPARAM)Index, (LPARAM)&Col);
					}while(Str.length() < (Col.cchTextMax - 1));
					return Str;
				}

				BOOL operator()(LPWSTR Buf, int LenBuf)
				{
					LV_COLUMNW Col;
					Col.mask = LVCF_TEXT;
					Col.cchTextMax = LenBuf;
					Col.pszText = Buf;
					return (BOOL)SendMessageW(hWnd, LVM_GETCOLUMNW, (WPARAM)Index, (LPARAM)&Col);
				}

				BOOL operator()(LPSTR Buf, int LenBuf)
				{
					LV_COLUMNA Col;
					Col.mask = LVCF_TEXT;
					Col.cchTextMax = LenBuf;
					Col.pszText = Buf;
					return (BOOL)SendMessageA(hWnd, LVM_GETCOLUMNA, (WPARAM)Index, (LPARAM)&Col);
				}
			} Text;

			class
			{
				HWND hWnd;
				int Index;
			public:

				inline int operator =(int Val)
				{
					LV_COLUMNW Col;
					Col.mask = LVCF_FMT;
					Col.fmt = Val;
					SendMessageW(hWnd, LVM_SETCOLUMNW, (WPARAM)Index, (LPARAM)&Col);
					return Val;
				}

				inline operator int()
				{
					LV_COLUMNW Col;
					Col.mask = LVCF_FMT;
					SendMessageW(hWnd, LVM_GETCOLUMNW, (WPARAM)Index, (LPARAM)&Col);
					return Col.fmt;
				}
			} Alignment;

			class
			{
				HWND hWnd;
				int Index;
			public:

				inline int operator =(int Val)
				{
					LV_COLUMNW Col;
					Col.mask = LVCF_WIDTH;
					Col.cx = Val;
					SendMessageW(hWnd, LVM_SETCOLUMNW, (WPARAM)Index, (LPARAM)&Col);
					return Val;
				}

				inline operator int()
				{
					LV_COLUMNW Col;
					Col.mask = LVCF_WIDTH;
					SendMessageW(hWnd, LVM_GETCOLUMNW, (WPARAM)Index, (LPARAM)&Col);
					return Col.cx;
				}
			} Width;

		};

		COLUMN(HWND hWnd, int Index)
		{
			this->hWnd = hWnd;
			this->Index = Index;
		}

		inline operator LVCOLUMNA()
		{
			LVCOLUMNA Col;
			Col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			SendMessageA(hWnd, LVM_SETCOLUMNA, (WPARAM)Index, (LPARAM)&Col);
			return Col;
		}

		inline operator LVCOLUMNW()
		{
			LVCOLUMNW Col;
			Col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			SendMessageW(hWnd, LVM_SETCOLUMNW, (WPARAM)Index, (LPARAM)&Col);
			return Col;
		}

		inline BOOL operator()(LPLVCOLUMNA Column)
		{
			return (BOOL)SendMessageA(hWnd, LVM_SETCOLUMNA, (WPARAM)Index, (LPARAM)Column);
		}

		inline BOOL operator()(LPLVCOLUMNW Column)
		{
			return (BOOL)SendMessageW(hWnd, LVM_SETCOLUMNW, (WPARAM)Index, (LPARAM)Column);
		}

		inline LPLVCOLUMNA operator=(LPLVCOLUMNA Column)
		{
			SendMessageA(hWnd, LVM_SETCOLUMNA, (WPARAM)Index, (LPARAM)Column);
			return Column;
		}

		inline LPLVCOLUMNW operator=(LPLVCOLUMNW Column)
		{
			SendMessageW(hWnd, LVM_SETCOLUMNW, (WPARAM)Index, (LPARAM)Column);
			return Column;
		}

		inline BOOL Delete()
		{
			return ListView_DeleteColumn(hWnd, Index);
		}
	};

	class SUBITEM
	{
	public:
		union
		{
			struct
			{
				HWND hWnd;
				int Index;
				int SubItem;
			};

			class
			{
				struct
				{
					HWND hWnd;
					int Index;
					int SubItem;
				};

			public:

				inline LPSTR operator =(LPSTR Val)
				{
					LV_ITEMA It;
					It.iSubItem = SubItem;
					It.pszText = Val;
					SendMessageA(hWnd, LVM_SETITEMTEXTA, (WPARAM)Index, (LPARAM)&It);
					return Val;
				}

				inline LPWSTR operator =(LPWSTR Val)
				{
					LV_ITEMW It;
					It.iSubItem = SubItem;
					It.iItem = Index;
					It.pszText = Val;
					SendMessageW(hWnd, LVM_SETITEMTEXTW, (WPARAM)Index, (LPARAM)&It);
					return Val;
				}

				template <class T>
				inline std::basic_string<T> & operator =(std::basic_string<T> & Val)
				{
					*this = Val.c_str();
					return Val;
				}

				inline operator std::basic_string<CHAR>()
				{
					std::basic_string<CHAR> Str;
					LV_ITEMA It;
					It.iSubItem = SubItem;
					It.iItem = Index;
					It.cchTextMax = 0;
					do
					{
						It.cchTextMax = (It.cchTextMax | 1111) + 1;
						Str.resize(It.cchTextMax);
						It.pszText = (LPSTR)Str.c_str();
						SendMessageA(hWnd, LVM_GETITEMTEXTA, (WPARAM)Index, (LPARAM)&It);
					}while(Str.length() < (It.cchTextMax - 1));
					return Str;
				}

				inline operator std::basic_string<WCHAR>()
				{
					std::basic_string<WCHAR> Str;
					LV_ITEMW It;
					It.iSubItem = SubItem;
					It.iItem = Index;
					It.cchTextMax = 0;
					do
					{
						It.cchTextMax = (It.cchTextMax | 1111) + 1;
						Str.resize(It.cchTextMax);
						It.pszText = (LPWSTR)Str.c_str();
						SendMessageW(hWnd, LVM_GETITEMTEXTW, (WPARAM)Index, (LPARAM)&It);
					}while(Str.length() < (It.cchTextMax - 1));
					return Str;
				}

				BOOL operator()(LPWSTR Buf, int LenBuf)
				{
					LV_ITEMW It;
					It.iSubItem = SubItem;
					It.iItem = Index;
					It.cchTextMax = LenBuf;
					It.pszText = Buf;
					return (BOOL)SendMessageW(hWnd, LVM_GETITEMTEXTW, (WPARAM)Index, (LPARAM)&It);
				}

				BOOL operator()(LPSTR Buf, int LenBuf)
				{
					LV_ITEMA It;
					It.iSubItem = SubItem;
					It.iItem = Index;
					It.cchTextMax = LenBuf;
					It.pszText = Buf;
					return (BOOL)SendMessageA(hWnd, LVM_GETITEMTEXTA, (WPARAM)Index, (LPARAM)&It);
				}
			} Text;

			class
			{
				struct
				{
					HWND hWnd;
					int Index;
					int SubItem;
				};
			public:
				inline operator RECT()
				{
					RECT Rect;
					ListView_GetSubItemRect(hWnd,Index,SubItem,LVIR_ICON,&Rect);
					return Rect;
				}
				inline BOOL operator()(LPRECT Rect)
				{
					return ListView_GetSubItemRect(hWnd,Index,SubItem,LVIR_ICON ,Rect);
				}
			} RectIcon;

			class
			{
				struct
				{
					HWND hWnd;
					int Index;
					int SubItem;
				};
			public:
				inline operator RECT()
				{
					RECT Rect;
					ListView_GetSubItemRect(hWnd,Index,SubItem,LVIR_BOUNDS ,&Rect);
					return Rect;
				}
				inline BOOL operator()(LPRECT Rect)
				{
					return ListView_GetSubItemRect(hWnd,Index,SubItem,LVIR_BOUNDS ,Rect);
				}

			} RectBonus;

			class
			{
				struct
				{
					HWND hWnd;
					int Index;
					int SubItem;
				};
			public:
				inline operator RECT()
				{
					RECT Rect;
					ListView_GetSubItemRect(hWnd,Index,SubItem,LVIR_LABEL,&Rect);
					return Rect;
				}
				inline BOOL operator()(LPRECT Rect)
				{
					return ListView_GetSubItemRect(hWnd,Index,SubItem,LVIR_LABEL ,Rect);
				}

			} RectLabel;

			class 
			{
				struct
				{
					HWND hWnd;
					int Index;
					int SubItem;
				};

			public:

				inline operator int()
				{
					LV_ITEM It;
					It.mask = LVIF_IMAGE;
					It.iSubItem = SubItem;
					It.iItem = Index;
					ListView_GetItem(hWnd,&It);
					return It.iImage;
				}

				int operator =(int iImage)
				{
					LV_ITEM It;
					It.mask = LVIF_IMAGE;
					It.iSubItem = SubItem;
					It.iItem = Index;
					It.iImage = iImage;
					ListView_SetItem(hWnd,&It);
					return iImage;
				}
			} Image;

			class 
			{
				struct
				{
					HWND hWnd;
					int Index;
					int SubItem;
				};
			public:
				template <class T>
				inline operator T()
				{
				   LVITEM It;
				   It.mask = LVIF_PARAM;
				   It.iItem = Index;
				   It.iSubItem = SubItem;
				   ListView_GetItem(hWnd,&It);
				   return (T)It.lParam;
				}

				template <class T>
				inline T operator=(T Val)
				{
				   LVITEM It;
				   It.mask = LVIF_PARAM;
				   It.iItem = Index;
				   It.iSubItem = SubItem;
				   It.lParam = (LPARAM)Val;
				   ListView_SetItem(hWnd,&It);
				   return Val;
				}
			} UserData;
		};

		inline operator LV_ITEMW()
		{
			LV_ITEMW li;
			li.mask = LVIF_IMAGE|LVIF_PARAM|LVIF_STATE|LVIF_INDENT;
			li.stateMask = LVIS_CUT|LVIS_DROPHILITED|LVIS_FOCUSED|LVIS_SELECTED|LVIS_OVERLAYMASK|LVIS_STATEIMAGEMASK;
			li.iItem = Index;
			li.iSubItem = SubItem;
		    SendMessageW(hWnd, LVM_GETITEMW, 0, (LPARAM)&li);
			return li;
		}

		inline operator LV_ITEMA()
		{
			LV_ITEMA li;
			li.mask = LVIF_IMAGE|LVIF_PARAM|LVIF_STATE|LVIF_INDENT;
			li.stateMask = LVIS_CUT|LVIS_DROPHILITED|LVIS_FOCUSED|LVIS_SELECTED|LVIS_OVERLAYMASK|LVIS_STATEIMAGEMASK;
			li.iItem = Index;
			li.iSubItem = SubItem;
		    SendMessageA(hWnd, LVM_GETITEMA, 0, (LPARAM)&li);
			return li;
		}

		inline LV_ITEMW & operator =(LV_ITEMW & Val)
		{
		   LV_ITEMW Item = Val;
		   Item.iItem = Index;
		   Item.iSubItem = SubItem;
		   SendMessageW(hWnd, LVM_SETITEMW, 0, (LPARAM)&Item);
		   return Val;
		}

		inline LV_ITEMA & operator =(LV_ITEMA & Val)
		{
		   LV_ITEMA Item = Val;
		   Item.iItem = Index;
		   Item.iSubItem = SubItem;
		   SendMessageA(hWnd, LVM_SETITEMA, 0, (LPARAM)&Item);
		   return Val;
		}

		SUBITEM(HWND hWnd, int Index, int SubItem)
		{
			this->hWnd = hWnd;
			this->Index = Index;
			this->SubItem = SubItem;
		}
	};

	class ITEM
	{
	public:
		union
		{
			struct
			{
				HWND hWnd;
				int Index;
			};

			class
			{
				struct
				{
					HWND hWnd;
					int Index;
				};
			public:
				inline operator UINT()
				{
					return ListView_GetItemState(hWnd, Index, LVIS_CUT|LVIS_DROPHILITED|LVIS_FOCUSED|LVIS_SELECTED|LVIS_OVERLAYMASK|LVIS_STATEIMAGEMASK);
				}

				inline UINT operator()(UINT Mask)
				{
					return ListView_GetItemState(hWnd, Index, Mask);
				}

				inline UINT operator=(UINT NewState)
				{
					ListView_SetItemState(hWnd, Index, NewState, LVIS_CUT|LVIS_DROPHILITED|LVIS_FOCUSED|LVIS_SELECTED|LVIS_OVERLAYMASK|LVIS_STATEIMAGEMASK);
					return NewState;
				}

				inline void operator()(UINT NewState, UINT Mask)
				{
				    ListView_SetItemState(hWnd, Index, NewState, Mask);
				}
			} State;

			class
			{
				struct
				{
					HWND hWnd;
					int Index;
				};
			public:
				operator bool()
				{
					return ListView_IsItemVisible(hWnd,Index) == TRUE;
				}
			} IsVisible;
		};

		ITEM(HWND hWnd, int Index)
		{
			this->hWnd = hWnd;
			this->Index = Index;
		}

		HWND EditLabel()
		{
			return ListView_EditLabel(hWnd, Index);
		}

		inline BOOL Delete()
		{
			return ListView_DeleteItem(hWnd, Index);
		}

		inline BOOL Update()
		{
			return ListView_Update(hWnd, Index);
		}

		inline SUBITEM operator[](int SubItem)
		{
			return SUBITEM(hWnd, Index, SubItem);
		}
	};

	union
	{
		HWND hWnd;
		class : public EX_WND
		{
		public:
			EX_WND_FIELD_AND_METHODS;
		};

		class 
		{
			HWND hWnd;
		public:
			inline operator COLORREF()
			{
				return ListView_GetBkColor(hWnd);
			}

			inline COLORREF operator=(COLORREF Val)
			{
				ListView_SetBkColor(hWnd,Val);
				return Val;
			}
		} BkColor;

		class 
		{
			HWND hWnd;
		public:
			inline operator COLORREF()
			{
				return ListView_GetTextColor(hWnd);
			}

			inline COLORREF operator=(COLORREF Val)
			{
				ListView_SetTextColor(hWnd,Val);
				return Val;
			}
		} TextColor;

		class 
		{
			HWND hWnd;
		public:
			inline operator COLORREF()
			{
				return ListView_GetTextBkColor(hWnd);
			}

			inline COLORREF operator=(COLORREF Val)
			{
				ListView_SetTextBkColor(hWnd,Val);
				return Val;
			}
		} TextBkColor;

		class
		{

		public:
			union
			{
				HWND hWnd;

				class
				{
					HWND hWnd;
				public:
					inline operator int()
					{
						LV_COLUMNW Col = {LVCF_FMT};
						int Count = 0;
						for(;(BOOL)SendMessageW(hWnd, LVM_GETCOLUMNW, (WPARAM)Count, (LPARAM)&Col) == TRUE; Count++);
						return Count;
					}
				} Count;

				class 
				{	
				public:

					union
					{
						HWND hWnd;
						class
						{
							HWND hWnd;
						public:
							operator unsigned()
							{
								return ListView_GetSelectedCount(hWnd);
							}
						} Count;
					};

					inline operator int()
					{
						return ListView_GetSelectedColumn(hWnd);
					}

					inline operator COLUMN()
					{
						int Index = ListView_GetSelectedColumn(hWnd);
						return COLUMN(hWnd, Index);
					}

					inline COLUMN operator()()
					{
						int Index = ListView_GetSelectedColumn(hWnd);
						return COLUMN(hWnd, Index);
					}

					inline int operator=(int Index)
					{
						ListView_SetSelectedColumn(hWnd, Index);
						return Index;
					}

					inline COLUMN & operator=(COLUMN & Col)
					{
						ListView_SetSelectedColumn(hWnd, Col.Index);
						return Col;
					}
				} CurSel;
			};

			inline COLUMN operator[](int Index)
			{
				return COLUMN(hWnd, Index);
			}

			inline COLUMN New()
			{
				LVCOLUMNW Col = {LVCF_FMT|LVCF_TEXT|LVCF_SUBITEM|LVCF_WIDTH,LVCFMT_CENTER,10,L"",0,0};
				int Index = (int)SendMessageW(hWnd, LVM_INSERTCOLUMNW, (WPARAM)(int)Count, (LPARAM)&Col);
				return COLUMN(hWnd, Index);
			}

			inline COLUMN New(LVCOLUMNW & Col)
			{
				int Index = (int)SendMessageW(hWnd, LVM_INSERTCOLUMNW, (WPARAM)(int)Count, (LPARAM)&Col);
				return COLUMN(hWnd, Index);
			}

			inline COLUMN New(LVCOLUMNA & Col)
			{
				int Index = (int)SendMessageA(hWnd, LVM_INSERTCOLUMNA, (WPARAM)(int)Count, (LPARAM)&Col);
				return COLUMN(hWnd, Index);
			}

			inline COLUMN New(LPSTR Str, int Width = 10, int Align = LVCFMT_CENTER, int SubItem = 0)
			{
				LVCOLUMNA Col = {LVCF_FMT|LVCF_TEXT|LVCF_SUBITEM|LVCF_WIDTH,Align,Width,Str,0,SubItem};
				int Index = (int)SendMessageA(hWnd, LVM_INSERTCOLUMNA, (WPARAM)(int)Count, (LPARAM)&Col);
				return COLUMN(hWnd, Index);
			}

			inline COLUMN New(LPWSTR Str, int Width = 10, int Align = LVCFMT_CENTER, int SubItem = 0)
			{
				LVCOLUMNW Col = {LVCF_FMT|LVCF_TEXT|LVCF_SUBITEM|LVCF_WIDTH,Align,Width,Str,0,SubItem};
				int Index = (int)SendMessageW(hWnd, LVM_INSERTCOLUMNW, (WPARAM)(int)Count, (LPARAM)&Col);
				return COLUMN(hWnd, Index);
			}

		} Columns;

		class 
		{
		public:
			union
			{
				HWND hWnd;

				class
				{
					HWND hWnd;
				public:
					inline operator int()
					{
						return ListView_GetItemCount(hWnd);
					}

					inline int operator=(int NewCount)
					{
						return ListView_SetItemCount(hWnd, NewCount);
					}
				} Count;

				class
				{
					HWND hWnd;
				public:

					inline operator int()
					{
						return  ListView_GetHotItem(hWnd);
					}

					inline int operator=(int Index)
					{
						return ListView_SetHotItem(hWnd, Index);
					}

					inline operator ITEM()
					{
						int Index = ListView_GetHotItem(hWnd);
						return ITEM(hWnd, Index);
					}

					inline ITEM operator()()
					{
						int Index = ListView_GetHotItem(hWnd);
						return ITEM(hWnd, Index);
					}

					inline ITEM & operator=(ITEM & It)
					{
						ListView_SetHotItem(hWnd, It.Index);
						return It;
					}
				} CurSel;
			};

			inline ITEM operator [](int Index)
			{
				return ITEM(hWnd, Index);
			}

			ITEM New()
			{
				LV_ITEM li = {LVIF_TEXT,0,0,0,0,TEXT(""),0,0};
				int Index = ListView_InsertItem(hWnd, &li);
				return ITEM(hWnd, Index);
			}

			ITEM New(LV_ITEMA & Item)
			{
			    int Index = (int)SendMessageA(hWnd, LVM_INSERTITEMA, 0, (LPARAM)&Item);
				return ITEM(hWnd, Index);
			}

			ITEM New(LV_ITEMW & Item)
			{
			    int Index = (int)SendMessageW(hWnd, LVM_INSERTITEMW, 0, (LPARAM)&Item);
				return ITEM(hWnd, Index);
			}

			ITEM Set(LV_ITEMA & Item)
			{
			    int Index =  (BOOL)SendMessageA(hWnd, LVM_SETITEMA, 0, (LPARAM)&Item);
				return ITEM(hWnd, Index);
			}

			ITEM Set(LV_ITEMW & Item)
			{
			    int Index =  (BOOL)SendMessageW(hWnd, LVM_SETITEMW, 0, (LPARAM)&Item);
				return ITEM(hWnd, Index);
			}

			BOOL DeleteAll()
			{
				return ListView_DeleteAllItems(hWnd);
			}
		} Items;
	};

};


#endif

