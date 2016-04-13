#pragma once

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
#	pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#	pragma comment(lib, "comctl32.lib")
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
	static bool ___g;

	class __HWND_TEXT
	{
		HWND hWnd;
	public:
		void get_stl_str(std::basic_string<WCHAR>& s)
		{
			int Length = GetWindowTextLengthW(hWnd);
			s.resize(Length);
			GetWindowTextW(hWnd, (LPWSTR)s.c_str(), Length + 1);
		}

		void get_stl_str(std::basic_string<CHAR>& s)
		{
			int Length = GetWindowTextLengthA(hWnd);
			s.resize(Length);
			GetWindowTextA(hWnd, (LPSTR)s.c_str(), Length + 1);
		}
		inline int get_str_len() { return GetWindowTextLength(hWnd) }
		inline size_t get_str(char* Buffer, size_t Len) { return GetWindowTextA(hWnd, Buffer, Len); }
		inline size_t get_str(wchar_t* Buffer, size_t Len) { return GetWindowTextW(hWnd, Buffer, Len); }
		inline void set_str(char* Buffer) { SetWindowTextA(hWnd, Buffer); }
		inline void set_str(wchar_t* Buffer) { SetWindowTextW(hWnd, Buffer); }
	};


	class __HWND_NAME_CLASS
	{
		HWND hWnd;
	public:

		void get_stl_str(std::basic_string<WCHAR>& Str)
		{
			size_t l, k = 0;
			do
			{
				k = (k | 0xff) + 1;
				Str.resize(k);
				GetClassNameW(hWnd, (LPWSTR)Str.c_str(), k);
			} while((l = wcslen(Str.c_str())) >= (k - 3));
			Str.resize(l);
		}

		void get_stl_str(std::basic_string<CHAR>& Str)
		{
			size_t l, k = 0;
			do
			{
				k = (k | 0xff) + 1;
				Str.resize(k);
				GetClassNameA(hWnd, (LPSTR)Str.c_str(), k);
			} while((l = strlen(Str.c_str())) >= (k - 3));
			Str.resize(l);
		}
		inline int get_str_len() { std::string k; get_stl_str(k); return k.length(); }
		inline size_t get_str(char* Buffer, size_t Len) { return GetClassNameA(hWnd, Buffer, Len); }
		inline size_t get_str(wchar_t* Buffer, size_t Len) { return GetClassNameW(hWnd, Buffer, Len); }
	};
public:

	template<class BaseType>
	class __TEXT_PROPERTY_SET_GET
	{
		/*
		BaseType must have this methods:

		int BaseType::get_str_len(); - get element len
		void BaseType::get_stl_str(std::basic_string<WCHAR>& ); - Get stl string wchar_t
		void BaseType::get_stl_str(std::basic_string<CHAR>& ); - Get stl string char
		size_t BaseType::get_str(char* Buffer, size_t Len); - Get c string from element
		size_t BaseType::get_str(wchar_t* Buffer, size_t Len); - Get c string from element
		void BaseType::set_str(char* Buffer); - Set c string to element
		void BaseType::set_str(wchar_t* Buffer); - Set c string to element
		*/
	public:
		class
		{
			friend __TEXT_PROPERTY_SET_GET;
			BaseType v;
		public:
			inline operator int() { return v.get_str_len(); }
		} Length;
		operator std::basic_string<WCHAR>() { std::basic_string<WCHAR> s; Length.v.get_stl_str(s); return s; }
		operator std::basic_string<CHAR>() { std::basic_string<CHAR> s; Length.v.get_stl_str(s); return s; }
		operator int() { char s[40] = {0}; Length.v.get_str(s, 39); return atoi(s); }
		operator unsigned() { char s[40] = {0}; Length.v.get_str(s, 39); unsigned r = 0; sscanf(s, "%u", &r); return r; }
		inline size_t operator()(LPWSTR Buffer, size_t Len = 0x0fffffff) { return Length.v.get_str(Buffer, Len); }
		inline size_t operator()(LPSTR Buffer, size_t Len = 0x0fffffff) { return Length.v.get_str(Buffer, Len); }
		inline std::basic_string<WCHAR> & operator=(const std::basic_string<WCHAR> & Str) { Length.v.set_str((LPWSTR)Str.c_str()); return (std::basic_string<WCHAR>&)Str; }
		inline std::basic_string<CHAR> & operator=(const std::basic_string<CHAR> & Str) { Length.v.set_str((LPSTR)Str.c_str()); return (std::basic_string<CHAR>&)Str; }
		inline LPSTR operator=(LPCSTR Str) { Length.v.set_str((LPSTR)Str); return (LPSTR)Str; }
		inline LPWSTR operator=(LPCWSTR Str) { Length.v.set_str((LPWSTR)Str); return (LPWSTR)Str; }
		int operator=(int Val) { char s[40]; itoa(Val, s, 10); operator=(s); return Val; }
		unsigned operator=(unsigned Val) { char s[40]; sprintf(s, "%u", Val); operator=(s); return Val; }
		inline bool operator==(const std::basic_string<WCHAR> & Str2) { return operator std::basic_string<WCHAR>() == Str2; }
		inline bool operator==(const std::basic_string<CHAR> & Str2) { return operator std::basic_string<CHAR>() == Str2; }
		inline bool operator==(LPCWSTR Str2) { return operator std::basic_string<WCHAR>() == Str2; }
		inline bool operator==(LPCSTR Str2) { return operator std::basic_string<CHAR>() == Str2; }
		template<class T>
		inline bool operator!=(T Val) { return !operator==(Val); }
		inline bool operator>(unsigned Val) { return (unsigned)*this > Val; }
		inline bool operator<(unsigned Val) { return operator unsigned() < Val; }
		inline bool operator>=(unsigned Val) { return operator unsigned() > Val; }
		inline bool operator<=(unsigned Val) { return operator unsigned() < Val; }
		inline bool operator>(int Val) { return operator int() > Val; }
		inline bool operator<(int Val) { return operator int() < Val; }
		inline bool operator>=(int Val) { return operator int() > Val; }
		inline bool operator<=(int Val) { return operator int() < Val; }
		inline bool operator==(int Val) { return operator int() == Val; }
		inline bool operator==(unsigned Val) { return operator int() == Val; }
	};

	template<class BaseType>
	class __TEXT_PROPERTY_GET
	{
		/*
		BaseType must have this methods:

		int BaseType::get_str_len(); - get element len
		void BaseType::get_stl_str(std::basic_string<WCHAR>& ); - Get stl string wchar_t
		void BaseType::get_stl_str(std::basic_string<CHAR>& ); - Get stl string char
		size_t BaseType::get_str(char* Buffer, size_t Len); - Get c string from element
		size_t BaseType::get_str(wchar_t* Buffer, size_t Len); - Get c string from element
		*/
	public:
		class
		{
			friend __TEXT_PROPERTY_GET;
			BaseType v;
		public:
			inline operator int() { return v.get_str_len(); }
		} Length;
		operator std::basic_string<WCHAR>() { std::basic_string<WCHAR> s; Length.v.get_stl_str(s); return s; }
		operator std::basic_string<CHAR>() { std::basic_string<CHAR> s; Length.v.get_stl_str(s); return s; }
		operator int() { char s[40] = {0}; Length.v.get_str(s, 39); return atoi(s); }
		operator unsigned() { char s[40] = {0}; Length.v.get_str(s, 39); unsigned r = 0; sscanf(s, "%u", &r); return r; }
		inline size_t operator()(LPWSTR Buffer, size_t Len = 0x0fffffff) { return Length.v.get_str(Buffer, Len); }
		inline size_t operator()(LPSTR Buffer, size_t Len = 0x0fffffff) { return Length.v.get_str(Buffer, Len); }
		inline bool operator==(const std::basic_string<WCHAR> & Str2) { return operator std::basic_string<WCHAR>() == Str2; }
		inline bool operator==(const std::basic_string<CHAR> & Str2) { return operator std::basic_string<CHAR>() == Str2; }
		inline bool operator==(LPCWSTR Str2) { return operator std::basic_string<WCHAR>() == Str2; }
		inline bool operator==(LPCSTR Str2) { return operator std::basic_string<CHAR>() == Str2; }
		template<class T>
		inline bool operator!=(T Val) { return !operator==(Val); }
		inline bool operator>(unsigned Val) { return (unsigned)*this > Val; }
		inline bool operator<(unsigned Val) { return operator unsigned() < Val; }
		inline bool operator>=(unsigned Val) { return operator unsigned() > Val; }
		inline bool operator<=(unsigned Val) { return operator unsigned() < Val; }
		inline bool operator>(int Val) { return operator int() > Val; }
		inline bool operator<(int Val) { return operator int() < Val; }
		inline bool operator>=(int Val) { return operator int() > Val; }
		inline bool operator<=(int Val) { return operator int() < Val; }
		inline bool operator==(int Val) { return operator int() == Val; }
		inline bool operator==(unsigned Val) { return operator int() == Val; }
	};

	union
	{
		HWND hWnd;

		/*
		*std::string, LPCWSTR, LPSTR, int; Set, Get;
		*Changes the text of the specified window's title bar.
		*/
		__TEXT_PROPERTY_SET_GET<__HWND_TEXT> Text;

		/*
		*int; Set, Get;
		*Sets a new identifier of the child window. The window cannot be a top-level window.
		*/
		class
		{
			HWND hWnd;
		public:
			inline operator int() const { return ::GetDlgCtrlID(hWnd); }

			int operator=(int NewId) { return (int)SetWindowLongPtr(hWnd, GWLP_ID, (LONG_PTR)NewId); }
		} Id;

		/*
		*BOOL; Set, Get;
		*Enables or disables mouse and keyboard input to the specified window or control.
		*/
		class
		{
			HWND hWnd;
		public:
			operator BOOL() const { return ::IsWindowEnabled(hWnd); }

			BOOL operator=(BOOL isEnable)
			{
				::EnableWindow(hWnd, isEnable);
				return isEnable;
			}
		} IsEnable;

		/*
		*WNDPROC; Set, Get;
		*Sets a new address for the window procedure.
		*/
		class
		{
			HWND hWnd;
		public:
			inline operator WNDPROC() const { return (WNDPROC)GetWindowLongPtrW(hWnd, GWLP_WNDPROC); }

			inline WNDPROC operator=(WNDPROC NewProc) { return (WNDPROC)SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)NewProc); }

			inline LRESULT CALLBACK operator()(UINT Msg, WPARAM wParam, LPARAM lParam) { return ::SendMessageW(hWnd, Msg, wParam, lParam); }
		} WndProcW;

		/*
		*WNDPROC; Set, Get;
		*Sets a new address for the window procedure.
		*/
		class
		{
			HWND hWnd;
		public:
			inline operator WNDPROC() const { return (WNDPROC)GetWindowLongPtrA(hWnd, GWLP_WNDPROC); }

			inline WNDPROC operator=(WNDPROC NewProc) { return (WNDPROC)SetWindowLongPtrA(hWnd, GWLP_WNDPROC, (LONG_PTR)NewProc); }

			inline LRESULT CALLBACK operator()(UINT Msg, WPARAM wParam, LPARAM lParam) { return ::SendMessageA(hWnd, Msg, wParam, lParam); }
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
			inline operator T() const { return (T)GetWindowLongPtr(hWnd, GWLP_USERDATA); }

			template<class T>
			inline T operator=(T NewData)
			{
				return (T)SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)NewData);
			}
		} UserData;

		/*
		*LONG_PTR; Set, Get;
		*Sets a new extended window style.
		*WS_EX_ ...
		*/
		class
		{
			HWND hWnd;
		public:
			inline operator LONG_PTR() const { return GetWindowLongPtr(hWnd, GWL_EXSTYLE); }

			inline LONG_PTR operator=(LONG_PTR NewStyle) { return SetWindowLongPtr(hWnd, GWL_EXSTYLE, NewStyle); }

			inline LONG_PTR operator |= (LONG_PTR AddStyle)
			{
				LONG_PTR lp = operator LONG_PTR() | AddStyle;
				operator=(lp);
				return lp;
			}

			inline LONG_PTR operator &= (LONG_PTR FilterStyle)
			{
				LONG_PTR lp = operator LONG_PTR() & FilterStyle;
				operator=(lp);
				return lp;
			}
		} ExStyle;

		/*
		*LONG_PTR; Set, Get;
		*Sets a new window style.
		* WS_ ...
		*/
		class
		{
			HWND hWnd;
		public:
			inline operator LONG_PTR() const { return GetWindowLongPtr(hWnd, GWL_STYLE); }

			inline LONG_PTR operator=(LONG_PTR NewStyle) { return SetWindowLongPtr(hWnd, GWL_STYLE, NewStyle); }

			inline LONG_PTR operator |= (LONG_PTR AddStyle)
			{
				LONG_PTR lp = operator LONG_PTR() | AddStyle;
				operator=(lp);
				return lp;
			}

			inline LONG_PTR operator &= (LONG_PTR FilterStyle)
			{
				LONG_PTR lp = operator LONG_PTR() & FilterStyle;
				operator=(lp);
				return lp;
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
			inline operator HINSTANCE() const { return (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE); }

			inline HINSTANCE operator=(HINSTANCE NewInst) { return (HINSTANCE)SetWindowLongPtr(hWnd, GWLP_HINSTANCE, (LONG_PTR)NewInst); }
		} Instance;

		/*
		*std::string, LPWSTR, LPSTR; Get;
		*Retrieves the name of the class to which the specified window belongs.
		*/
		__TEXT_PROPERTY_GET<__HWND_NAME_CLASS> NameClass;

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
				std::basic_string<WCHAR> Name = ((EX_WND*)this)->NameClass;
				WNDCLASSW WndClass = {0};
				GetClassInfoW(((EX_WND*)this)->Instance, Name.c_str(), &WndClass);
				WndClass.lpszClassName = nullptr;
				return WndClass;
			}

			inline operator WNDCLASSA()
			{
				std::basic_string<CHAR> Name = ((EX_WND*)this)->NameClass;
				WNDCLASSA WndClass = {0};
				GetClassInfoA(((EX_WND*)this)->Instance, Name.c_str(), &WndClass);
				WndClass.lpszClassName = nullptr;
				return WndClass;
			}

			inline operator WNDCLASSEXW()
			{
				std::basic_string<WCHAR> Name = ((EX_WND*)this)->NameClass;
				WNDCLASSEXW WndClass = {0};
				WndClass.cbSize = sizeof(WndClass);
				GetClassInfoExW(((EX_WND*)this)->Instance, Name.c_str(), &WndClass);
				WndClass.lpszClassName = nullptr;
				return WndClass;
			}

			inline operator WNDCLASSEXA()
			{
				std::basic_string<CHAR> Name = ((EX_WND*)this)->NameClass;
				WNDCLASSEXA WndClass = {0};
				WndClass.cbSize = sizeof(WndClass);
				GetClassInfoExA(((EX_WND*)this)->Instance, Name.c_str(), &WndClass);
				WndClass.lpszClassName = nullptr;
				return WndClass;
			}

			inline bool operator()(LPWNDCLASSW lpWndClass)
			{
				std::basic_string<WCHAR> Name = ((EX_WND*)this)->NameClass;
				bool r = GetClassInfoW(((EX_WND*)this)->Instance, Name.c_str(), lpWndClass) != 0;
				lpWndClass->lpszClassName = nullptr;
				return r;
			}

			inline bool operator()(LPWNDCLASSA lpWndClass)
			{
				std::basic_string<CHAR> Name = ((EX_WND*)this)->NameClass;
				bool r = GetClassInfoA(((EX_WND*)this)->Instance, Name.c_str(), lpWndClass) != 0;
				lpWndClass->lpszClassName = nullptr;
				return r;
			}

			inline bool operator()(LPWNDCLASSEXW lpWndClass)
			{
				std::basic_string<WCHAR> Name = ((EX_WND*)this)->NameClass;
				bool r = GetClassInfoExW(((EX_WND*)this)->Instance, Name.c_str(), lpWndClass) != 0;
				lpWndClass->lpszClassName = nullptr;
				return r;
			}

			inline bool operator()(LPWNDCLASSEXA lpWndClass)
			{
				std::basic_string<CHAR> Name = ((EX_WND*)this)->NameClass;
				bool r = GetClassInfoExA(((EX_WND*)this)->Instance, Name.c_str(), lpWndClass) != 0;
				lpWndClass->lpszClassName = nullptr;
				return r;
			}
		} Class;

		/*
		*HDC; Get;
		*Retrieves a handle to a device context (DC) for the client area of a specified window or for the entire screen.
		*/
		class
		{
			HWND hWnd;
		public:
			inline operator HDC() const { return ::GetDC(hWnd); }
		} Dc;

		/*
		*EX_WND__, HWND; Get, Set;
		*Changes the parent window of the specified child window.
		*/
		class
		{
			HWND hWnd;
		public:

			inline operator EX_WND__() const { return ::GetParent(hWnd); }

			inline operator HWND() const { return ::GetParent(hWnd); }

			inline EX_WND__ operator=(EX_WND__ NewParent) { return ::SetParent(hWnd, NewParent); }

			inline HWND operator=(HWND NewParent) { return ::SetParent(hWnd, NewParent); }

			inline EX_WND__ operator()() { return ::GetParent(hWnd); }
		} Parent;

		/*
		*bool; Get;
		*Determines whether the specified window handle identifies an existing window.
		*/
		class
		{
			HWND hWnd;
		public:
			inline operator bool() const { return ::IsWindow(hWnd) != 0; }
		} IsWindow;

		/*
		*bool; Get;
		*Determines the visibility state of the specified window.
		*/
		class
		{
			HWND hWnd;
		public:
			inline operator bool() const { return ::IsWindowVisible(hWnd); }
		} IsVisible;

		/*
		*bool; Get;
		*Determines whether the specified window is a native Unicode window.
		*/
		class
		{
			HWND hWnd;
		public:
			inline operator bool() const { return ::IsWindowUnicode(hWnd); }
		} IsUnicode;

		/*
		*bool; Get;
		*Determines whether the specified window is minimized (iconic).
		*/
		class
		{
			HWND hWnd;
		public:
			inline operator bool() const { return ::IsIconic(hWnd); }
		} IsIconic;

		/*
		*WND_COMBO__, HWND, EX_WND__; Get;
		*Represet window as ComboBox.
		*/
		//WND_COMBO__ AsCombo;


		class
		{
			HWND hWnd;
		public:
			inline operator LONG() const
			{
				RECT Rect;
				::GetClientRect(hWnd, &Rect);
				return Rect.right - Rect.left;
			}
		} ClientWidth;

		class
		{
			HWND hWnd;
		public:
			inline operator LONG() const
			{
				RECT Rect;
				::GetClientRect(hWnd, &Rect);
				return Rect.bottom - Rect.top;
			}
		} ClientHeight;

		/*
		*long; Get, Set;
		*Set or get window width.
		*/
		class
		{
			HWND hWnd;
		public:
			operator long() const
			{
				RECT Rect;
				::GetWindowRect(hWnd, &Rect);
				return Rect.right - Rect.left;
			}

			inline long operator=(long Val)
			{
				::SetWindowPos(hWnd, NULL, 0, 0, Val, EX_WND(hWnd).Height, SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);
				return Val;
			}
		} Width;

		/*
		*long; Get, Set;
		*Set or get window height.
		*/
		class
		{
			HWND hWnd;
		public:
			inline operator long() const
			{
				RECT Rect;
				::GetWindowRect(hWnd, &Rect);
				return Rect.bottom - Rect.top;
			}

			inline long operator=(long Val)
			{
				::SetWindowPos(hWnd, NULL, 0, 0, EX_WND(hWnd).Width, Val, SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);
				return Val;
			}
		} Height;

		class
		{
			HWND hWnd;
		public:
			operator LONG() const
			{
				RECT Rect;
				::GetWindowRect(hWnd, &Rect);
				MapWindowPoints(HWND_DESKTOP, ::GetParent(hWnd), (LPPOINT)&Rect, 1);
				return Rect.left;
			}

			inline LONG operator= (LONG New)
			{
				SetWindowPos(hWnd, NULL, New, EX_WND(hWnd).Top, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER);
				return New;
			}
		} Left;

		class
		{
			HWND hWnd;
		public:
			operator LONG() const
			{
				RECT Rect;
				::GetWindowRect(hWnd, &Rect);
				MapWindowPoints(HWND_DESKTOP, ::GetParent(hWnd), (LPPOINT)&Rect, 1);
				return Rect.top;
			}

			inline LONG operator= (LONG New)
			{
				SetWindowPos(hWnd, NULL, EX_WND(hWnd).Left, New, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER);
				return New;
			}
		} Top;

		class
		{
			HWND hWnd;
		public:
			inline operator LONG() const
			{
				RECT Rect;
				::GetWindowRect(hWnd, &Rect);
				MapWindowPoints(HWND_DESKTOP, ::GetParent(hWnd), ((LPPOINT)&Rect) + 1, 1);
				return Rect.right;
			}

			inline LONG operator= (LONG New)
			{
				RECT Rect;
				::GetWindowRect(hWnd, &Rect);
				MapWindowPoints(HWND_DESKTOP, ::GetParent(hWnd), (LPPOINT)&Rect, 2);
				SetWindowPos(hWnd, NULL, 0, 0, New - Rect.left, Rect.bottom - Rect.top, SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);
				return New;
			}
		} Right;

		class
		{
			HWND hWnd;
		public:
			inline operator LONG() const
			{
				RECT Rect;
				::GetWindowRect(hWnd, &Rect);
				MapWindowPoints(HWND_DESKTOP, ::GetParent(hWnd), ((LPPOINT)&Rect) + 1, 1);
				return Rect.bottom;
			}

			inline LONG operator= (LONG New)
			{
				RECT Rect;
				::GetWindowRect(hWnd, &Rect);
				MapWindowPoints(HWND_DESKTOP, ::GetParent(hWnd), (LPPOINT)&Rect, 2);
				SetWindowPos(hWnd, NULL, 0, 0, Rect.right - Rect.left, New - Rect.top, SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);
				return New;
			}
		} Bottom;

		class
		{
			HWND hWnd;
		public:
			inline operator RECT() const
			{
				RECT Rect;
				::GetWindowRect(hWnd, &Rect);
				MapWindowPoints(HWND_DESKTOP, ::GetParent(hWnd), (LPPOINT)&Rect, 2);
				return Rect;
			}

			inline RECT& operator= (RECT& New)
			{
				SetWindowPos(hWnd, NULL, New.left, New.top, New.right - New.left, New.bottom - New.top, SWP_NOOWNERZORDER | SWP_NOZORDER);
				return New;
			}

			bool operator ==(const RECT& Val) const
			{
				RECT Rect = operator RECT();
				return (Rect.bottom == Val.bottom) && (Rect.left == Val.left) && (Rect.right == Val.right) && (Rect.top == Val.top);
			}

			inline bool operator !=(const RECT& Val) const { return !operator ==(Val); }

			bool Contains(const RECT& Another)
			{
				RECT Rect = operator RECT();
				return (Another.bottom < Rect.bottom) && (Another.top > Rect.top) && (Another.left > Rect.left) && (Another.right < Rect.right);
			}

			bool Contains(const POINT& Another)
			{
				RECT Rect = operator RECT();
				return (Another.x < Rect.right) && (Another.x > Rect.left) && (Another.y < Rect.bottom) && (Another.y > Rect.top);
			}

			bool Contains(LONG x, LONG y)
			{
				RECT Rect = operator RECT();
				return (x < Rect.right) && (x > Rect.left) && (y < Rect.bottom) && (y > Rect.top);
			}
		} Rectangle;

		class
		{
			HWND hWnd;
		public:
			inline operator RECT() const
			{
				RECT Rect;
				::GetWindowRect(hWnd, &Rect);
				return Rect;
			}

			inline RECT& operator= (RECT& New)
			{
				RECT Rect = New;
				MapWindowPoints(HWND_DESKTOP, ::GetParent(hWnd), (LPPOINT)&Rect, 2);
				SetWindowPos(hWnd, NULL, Rect.left, Rect.top, Rect.right - Rect.left, Rect.bottom - Rect.top, SWP_NOOWNERZORDER | SWP_NOZORDER);
				return New;
			}

			bool operator ==(const RECT& Val) const
			{
				RECT Rect = operator RECT();
				return (Rect.bottom == Val.bottom) && (Rect.left == Val.left) && (Rect.right == Val.right) && (Rect.top == Val.top);
			}

			inline bool operator !=(const RECT& Val) const { return !operator ==(Val); }

			bool Contains(const RECT& Another)
			{
				RECT Rect = operator RECT();
				return (Another.bottom < Rect.bottom) && (Another.top > Rect.top) && (Another.left > Rect.left) && (Another.right < Rect.right);
			}

			bool Contains(const POINT& Another)
			{
				RECT Rect = operator RECT();
				return (Another.x < Rect.right) && (Another.x > Rect.left) && (Another.y < Rect.bottom) && (Another.y > Rect.top);
			}

			bool Contains(LONG x, LONG y)
			{
				RECT Rect = operator RECT();
				return (x < Rect.right) && (x > Rect.left) && (y < Rect.bottom) && (y > Rect.top);
			}
		} RectangleRelScreen;

		class
		{
			HWND hWnd;
		public:
			inline operator RECT() const
			{
				RECT Rect;
				::GetClientRect(hWnd, &Rect);
				MapWindowPoints(HWND_DESKTOP, ::GetParent(hWnd), (LPPOINT)&Rect, 2);
				return Rect;
			}

			bool operator ==(const RECT& Val) const
			{
				RECT Rect = operator RECT();
				return (Rect.bottom == Val.bottom) && (Rect.left == Val.left) && (Rect.right == Val.right) && (Rect.top == Val.top);
			}

			inline bool operator !=(const RECT& Val) const { return !operator ==(Val); }

			bool Contains(const RECT& Another)
			{
				RECT Rect = operator RECT();
				return (Another.bottom < Rect.bottom) && (Another.top > Rect.top) && (Another.left > Rect.left) && (Another.right < Rect.right);
			}

			bool Contains(const POINT& Another)
			{
				RECT Rect = operator RECT();
				return (Another.x < Rect.right) && (Another.x > Rect.left) && (Another.y < Rect.bottom) && (Another.y > Rect.top);
			}

			bool Contains(LONG x, LONG y)
			{
				RECT Rect = operator RECT();
				return (x < Rect.right) && (x > Rect.left) && (y < Rect.bottom) && (y > Rect.top);
			}
		} ClientRectangle;

		/*
		*HFONT; Get, Set;
		*Set or get current font of window.
		*/
		class
		{
			HWND hWnd;
		public:
			/*
			Use CreateFont for set new font.
			Example:
			CreateFont (14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_SWISS, L"Times New Roman");
			*/
			inline operator HFONT() const
			{
				//GetCharABCWidths
				return (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0);
			}

			inline HFONT operator=(HFONT NewFont)
			{
				SendMessage(hWnd, WM_SETFONT, (WPARAM)NewFont, TRUE);
				return NewFont;
			}
		} Font;

		/*
		*HMENU; Get, Set;
		*Set or get current menu of window.
		*/
		class
		{
			HWND hWnd;
		public:
			inline operator HMENU() const
			{
				//use CreateMenu
				return GetMenu(hWnd);
			}

			inline HMENU operator=(HMENU NewMenu)
			{
				SetMenu(hWnd, NewMenu);
				return NewMenu;
			}
		} Menu;

		/*
		*HRGN; Get, Set;
		*Set or get current menu of window.
		*/
		class
		{
			HWND hWnd;
		public:
			/*
			@RgnDest - region for copy. use CreateRectRgn
			@return
			NULLREGION
			SIMPLEREGION
			COMPLEXREGION
			ERROR
			*/
			inline int operator()(HRGN RgnDest) { return GetWindowRgn(hWnd, RgnDest); }

			inline HRGN operator=(HRGN New)
			{
				SetWindowRgn(hWnd, New, TRUE);
				return New;
			}
		} Region;
	};

	inline operator HWND() const { return hWnd; }

	inline EX_WND__() { hWnd = NULL; }

	inline EX_WND__(HWND nhWnd) { hWnd = nhWnd; }

	inline operator bool() { return hWnd != NULL; }

	inline operator EX_WND__*() { return this; }

	inline bool operator !() { return hWnd == NULL; }

private:
	class _FOR_GET_ADDRESS
	{
		HWND hWnd;
	public:
		inline operator HWND*() { return &hWnd; }

		inline operator EX_WND__*() { return (EX_WND__*)&hWnd; }
	};
public:

	inline _FOR_GET_ADDRESS & operator &() { return *(_FOR_GET_ADDRESS*)this; }

	inline bool operator==(HWND Wnd) const { return Wnd == hWnd; }

	inline bool operator!=(HWND Wnd) const { return Wnd != hWnd; }

	inline bool operator==(EX_WND__ Wnd) const { return Wnd.hWnd == hWnd; }

	inline bool operator!=(EX_WND__ Wnd) const { return Wnd.hWnd != hWnd; }

	inline EX_WND__ operator[](int nIDDlgItem) { return GetDlgItem(hWnd, nIDDlgItem); }

	inline bool BringToTop() { return BringWindowToTop(hWnd) != FALSE; }

	inline bool Show(int nCmdShow = SW_SHOW) { return ShowWindow(hWnd, nCmdShow) != FALSE; }

	inline bool Update() { return UpdateWindow(hWnd) != FALSE; }

	inline bool Invalidate() { return InvalidateRect(hWnd, NULL, FALSE) != FALSE; }

	inline bool Invalidate(const RECT *lpRect) { return InvalidateRect(hWnd, lpRect, FALSE) != FALSE; }

	inline bool Validate(RECT * lpRect) { return ::ValidateRect(hWnd, lpRect) != FALSE; }

	inline bool Validate() { return ::ValidateRect(hWnd, NULL) != FALSE; }

	inline EX_WND__ Focus() { return (EX_WND__)::SetFocus(hWnd); }

	static inline EX_WND__ GetFocus() { return (EX_WND__)::GetFocus(); }

	static inline EX_WND CreateDlg(int IdRes, DLGPROC WindowProc, HINSTANCE ProcessHandler = NULL, HWND ParentWindowHandler = NULL)
	{
		return CreateDialog(ProcessHandler, MAKEINTRESOURCE(IdRes), ParentWindowHandler, WindowProc);
	}

	static inline EX_WND CreateDlg(LPCSTR Name, DLGPROC WindowProc, HINSTANCE ProcessHandler = NULL, HWND ParentWindowHandler = NULL)
	{
		return CreateDialogA(ProcessHandler, Name, ParentWindowHandler, WindowProc);
	}

	static inline EX_WND CreateDlg(LPCWSTR Name, DLGPROC WindowProc, HINSTANCE ProcessHandler = NULL, HWND ParentWindowHandler = NULL)
	{
		return CreateDialogW(ProcessHandler, Name, ParentWindowHandler, WindowProc);
	}

	static void EnterMainLoop()
	{
		for(MSG msg; GetMessage(&msg, NULL, NULL, NULL);)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	static void EnterMainLoop(DLGPROC ProcForInterceptKeys)
	{
		for(MSG msg; GetMessage(&msg, NULL, NULL, NULL);)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if((msg.message == WM_KEYDOWN) || (msg.message == WM_KEYUP))
				ProcForInterceptKeys(msg.hwnd, msg.message, msg.wParam, msg.lParam);
		}
	}


	/////////
	inline bool ShowContextMenu(HMENU hMenu, UINT Flags = TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY)
	{
		//Use CreatePopupMenu for creating menu
		POINT Poi;
		GetCursorPos(&Poi);
		return TrackPopupMenu(hMenu, Flags, Poi.x, Poi.y, 0, hWnd, NULL) != FALSE;
	}

	inline bool ShowContextMenu(HMENU hMenu, UINT Flags, int x, int y) { return TrackPopupMenu(hMenu, Flags, x, y, 0, hWnd, NULL) != FALSE; }

	static inline EX_WND__ ByPoint(LPPOINT Point) { return  (EX_WND__)::WindowFromPoint(*Point); }

	inline bool IsChild(HWND Wnd) const { return ::IsChild(hWnd, Wnd) == TRUE; }

	////

	/*Close window.*/
	inline bool Close() { return ::CloseWindow(hWnd) != FALSE; }

	/*Destroys window. */
	inline bool Destroy() { return ::DestroyWindow(hWnd) != FALSE; }

	/*The CreateCompatibleDC function creates a memory device context (DC) compatible with the window.*/
	inline HDC CreateCompatibleDC() { return ::CreateCompatibleDC(::GetDC(hWnd)); }

	/*The BeginPaint function prepares the specified window for painting and fills
	a PAINTSTRUCT structure with information about the painting.*/
	inline HDC BeginPaint(LPPAINTSTRUCT ps) { return ::BeginPaint(hWnd, ps); }

	/*The EndPaint function marks the end of painting in the specified window.*/
	inline bool EndPaint(LPPAINTSTRUCT ps) { return ::EndPaint(hWnd, ps) != FALSE; }

	/*Destroys a modal dialog box, causing the system to end any processing for the dialog box.*/
	inline bool EndDialog(INT_PTR Result = 0) { return ::EndDialog(hWnd, Result) != FALSE; }

	/*Creates a timer with the specified time-out value.*/
	inline UINT_PTR SetTimer(UINT Time, UINT_PTR nIDEvent = NULL, TIMERPROC tp = NULL) { return ::SetTimer(hWnd, nIDEvent, Time, tp); }

	/*Destroys the specified timer.*/
	inline BOOL KillTimer(UINT_PTR nIDEvent = NULL) { return ::KillTimer(hWnd, nIDEvent); }

	inline void SwitchToThisWindow(BOOL fAltTab = FALSE) { return ::SwitchToThisWindow(hWnd, fAltTab); }

private:
	typedef struct
	{
		unsigned MaxElements;
		unsigned CurElement;
		HWND * Buf;
	} FOR_ENUM_WINDOWS;

	static BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam)
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

	inline bool GetChildWindows(HWND * hWndBuf, unsigned MaxElemInBuf)
	{
		FOR_ENUM_WINDOWS arg;
		arg.Buf = hWndBuf;
		arg.MaxElements = MaxElemInBuf;
		arg.CurElement = 0;
		return EnumChildWindows(hWnd, EnumChildProc, (LPARAM)&arg) != FALSE;
	}

	inline bool GetChildWindows(HWND * hWndBuf, unsigned MaxElemInBuf, unsigned * CountGetted)
	{
		FOR_ENUM_WINDOWS arg;
		arg.Buf = hWndBuf;
		arg.MaxElements = MaxElemInBuf;
		arg.CurElement = 0;
		BOOL Res = EnumChildWindows(hWnd, EnumChildProc, (LPARAM)&arg);
		*CountGetted = arg.CurElement;
		return Res != FALSE;
	}

};

#ifdef USE_WONDER_CONTROLS
template<bool l>
bool EX_WND__<l>::___g = ([]() { InitCommonControls(); return true; })();
#endif

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
	EX_WND::ClientHeight;\
	EX_WND::ClientWidth;\
	EX_WND::GetFocus;\
	EX_WND::Height;\
	EX_WND::Width;\
	EX_WND::Left;\
	EX_WND::Right;\
	EX_WND::Top;\
	EX_WND::Bottom;\
	EX_WND::Rectangle;\
	EX_WND::RectangleRelScreen;\
	EX_WND::ClientRectangle;\
	EX_WND::Font;\
	EX_WND::Invalidate;\
	EX_WND::IsChild;\
	EX_WND::KillTimer;\
	EX_WND::Focus;\
	EX_WND::SetTimer;\
	EX_WND::Show;\
	EX_WND::SwitchToThisWindow;\
	EX_WND::Update;\
	EX_WND::Validate;\
	EX_WND::Menu;\
	EX_WND::Region;\
	EX_WND::ShowContextMenu;\
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
	class __ELEMENT_TEXT
	{
		struct
		{
			HWND hWnd;
			int Index;
		};
	public:
		inline int get_str_len() { return ComboBox_GetLBTextLen(hWnd, Index); }

		void get_stl_str(std::basic_string<WCHAR>& s)
		{
			s.resize(get_str_len());
			SendMessageW(hWnd, CB_GETLBTEXT, (WPARAM)Index, (LPARAM)s.c_str());
		}

		void get_stl_str(std::basic_string<CHAR>& s)
		{
			s.resize(get_str_len());
			SendMessageA(hWnd, CB_GETLBTEXT, (WPARAM)Index, (LPARAM)s.c_str());
		}

		inline size_t get_str(char* Buffer, size_t Len) { return (int)SendMessageA(hWnd, CB_GETLBTEXT, (WPARAM)Index, (LPARAM)Buffer); }

		inline size_t get_str(wchar_t* Buffer, size_t Len) { return (int)SendMessageW(hWnd, CB_GETLBTEXT, (WPARAM)Index, (LPARAM)Buffer); }

		void set_str(char* Str)
		{
			auto Data = ComboBox_GetItemData(hWnd, Index);
			ComboBox_DeleteString(hWnd, Index);
			Index = SendMessageA(hWnd, CB_INSERTSTRING, (WPARAM)Index, (LPARAM)Str);
			ComboBox_SetItemData(hWnd, Index, Data);
		}

		void set_str(wchar_t* Str)
		{
			auto Data = ComboBox_GetItemData(hWnd, Index);
			ComboBox_DeleteString(hWnd, Index);
			Index = SendMessageW(hWnd, CB_INSERTSTRING, (WPARAM)Index, (LPARAM)Str);
			ComboBox_SetItemData(hWnd, Index, Data);
		}
	};

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

			EX_WND::__TEXT_PROPERTY_SET_GET<__ELEMENT_TEXT> Text;

			class
			{
				HWND hWnd;
				int Index;
			public:
				template<class T>
				operator T() const { return (T)ComboBox_GetItemData(hWnd, Index); }

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

		int Delete() { return ComboBox_DeleteString(hWnd, Index); }
	};

	union
	{
		HWND hWnd;
		class: public EX_WND
		{
		public:
			EX_WND_FIELD_AND_METHODS;
		};

		class
		{
			HWND hWnd;
		public:
			inline operator int() const { return ComboBox_GetCurSel(hWnd); }

			inline int operator=(int Index) { return ComboBox_SetCurSel(hWnd, Index); }

			inline ITEM & operator=(ITEM & Item)
			{
				ComboBox_SetCurSel(hWnd, Item.Index);
				return Item;
			}

			inline operator ITEM() { return ITEM(operator int(), hWnd); }

			inline ITEM operator()() { return ITEM(operator int(), hWnd); }
		} CurSel;

		class
		{
			HWND hWnd;
		public:
			inline operator int() { return ComboBox_GetCount(hWnd); }
		} Count;
	};

	inline ITEM operator[](int Index) { return ITEM(Index, hWnd); }

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

	WND_COMBO__() {};

	WND_COMBO__(EX_WND Wnd) { hWnd = Wnd; }

	operator HWND() const { return hWnd; }

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

	inline ITEM FindByItemData(LPARAM data, int StartIndex = 0) { return ITEM(ComboBox_FindItemData(hWnd, StartIndex, data), hWnd); }

	int ResetContent() { return ComboBox_ResetContent(hWnd); }

	/*Limits the length of the text the user may type into the edit control of a combo box.*/
	inline int SetLimit(int cchMax) { return ComboBox_LimitText(hWnd, cchMax); }
};

template<bool>
class WND_LIST__
{



public:
	WND_LIST__() { hWnd = NULL; }

	WND_LIST__(EX_WND Wnd) { hWnd = Wnd; }

	operator EX_WND() { return hWnd; }

	class COLUMN
	{
		class __COLUMN_TEXT
		{
			struct
			{
				HWND hWnd;
				int Index;
			};
		public:
			inline int get_str_len() { std::basic_string<CHAR> s; get_stl_str(s); return s.length(); }

			void get_stl_str(std::basic_string<WCHAR>& Str)
			{
				LV_COLUMNW Col;
				Col.mask = LVCF_TEXT;
				Col.cchTextMax = 0;
				size_t l;
				do
				{
					Col.cchTextMax = (Col.cchTextMax | 0xff) + 1;
					Str.resize(Col.cchTextMax);
					Col.pszText = (LPWSTR)Str.c_str();
					SendMessageA(hWnd, LVM_GETCOLUMNW, (WPARAM)Index, (LPARAM)&Col);
				} while((l = wcslen(Str.c_str())) >= (Col.cchTextMax - 3));
				Str.resize(l);
			}

			void get_stl_str(std::basic_string<CHAR>& Str)
			{
				LV_COLUMNA Col;
				Col.mask = LVCF_TEXT;
				Col.cchTextMax = 0;
				size_t l;
				do
				{
					Col.cchTextMax = (Col.cchTextMax | 0xff) + 1;
					Str.resize(Col.cchTextMax);
					Col.pszText = (LPSTR)Str.c_str();
					SendMessageA(hWnd, LVM_GETCOLUMNA, (WPARAM)Index, (LPARAM)&Col);
				} while((l = strlen(Str.c_str())) >= (Col.cchTextMax - 3));
				Str.resize(l);
			}

			inline size_t get_str(char* Buffer, size_t Len)
			{
				LV_COLUMNA Col;
				Col.mask = LVCF_TEXT;
				Col.cchTextMax = Len;
				Col.pszText = Buffer;
				SendMessageA(hWnd, LVM_GETCOLUMNA, (WPARAM)Index, (LPARAM)&Col);
				return strnlen(Buffer, Len);
			}

			inline size_t get_str(wchar_t* Buffer, size_t Len)
			{
				LV_COLUMNW Col;
				Col.mask = LVCF_TEXT;
				Col.cchTextMax = Len;
				Col.pszText = Buffer;
				SendMessageW(hWnd, LVM_GETCOLUMNW, (WPARAM)Index, (LPARAM)&Col);
				return wcsnlen(Buffer, Len);
			}

			inline void set_str(char* Val)
			{
				LV_COLUMNA Col;
				Col.mask = LVCF_TEXT;
				Col.pszText = Val;
				SendMessageA(hWnd, LVM_SETCOLUMNA, (WPARAM)Index, (LPARAM)&Col);
			}

			inline void set_str(wchar_t* Val)
			{
				LV_COLUMNW Col;
				Col.mask = LVCF_TEXT;
				Col.pszText = Val;
				SendMessageW(hWnd, LVM_SETCOLUMNW, (WPARAM)Index, (LPARAM)&Col);
			}
		};
	public:
		union
		{
			struct
			{
				HWND hWnd;
				int Index;
			};

			EX_WND::__TEXT_PROPERTY_SET_GET<__COLUMN_TEXT> Text;

			/*
			int; Get, Set;
			LVCFMT_ ...
			*/
			class
			{
				struct
				{
					HWND hWnd;
					int Index;
				};
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
				struct
				{
					HWND hWnd;
					int Index;
				};
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

		inline BOOL operator()(LPLVCOLUMNA Column) { return (BOOL)SendMessageA(hWnd, LVM_SETCOLUMNA, (WPARAM)Index, (LPARAM)Column); }

		inline BOOL operator()(LPLVCOLUMNW Column) { return (BOOL)SendMessageW(hWnd, LVM_SETCOLUMNW, (WPARAM)Index, (LPARAM)Column); }

		inline LPLVCOLUMNA operator=(LPLVCOLUMNA Column) { SendMessageA(hWnd, LVM_SETCOLUMNA, (WPARAM)Index, (LPARAM)Column); return Column; }

		inline LPLVCOLUMNW operator=(LPLVCOLUMNW Column)
		{
			SendMessageW(hWnd, LVM_SETCOLUMNW, (WPARAM)Index, (LPARAM)Column);
			return Column;
		}

		inline BOOL Delete() { return ListView_DeleteColumn(hWnd, Index); }
	};

	class SUBITEM
	{
		class __SUBITEM_TEXT
		{
			struct
			{
				HWND hWnd;
				int Index;
				int SubItem;
			};
		public:
			inline int get_str_len() { std::basic_string<CHAR> s; get_stl_str(s); return s.length(); }

			void get_stl_str(std::basic_string<WCHAR>& Str)
			{
				LV_ITEMW It;
				It.iSubItem = SubItem;
				It.iItem = Index;
				It.cchTextMax = 0;
				size_t l;
				do
				{
					It.cchTextMax = (It.cchTextMax | 0xff) + 1;
					Str.resize(It.cchTextMax);
					It.pszText = (LPWSTR)Str.c_str();
					SendMessageW(hWnd, LVM_GETITEMTEXTW, (WPARAM)Index, (LPARAM)&It);
				} while((l = wcslen(Str.c_str())) >= (It.cchTextMax - 3));
				Str.resize(l);
			}

			void get_stl_str(std::basic_string<CHAR>& Str)
			{
				LV_ITEMA It;
				It.iSubItem = SubItem;
				It.iItem = Index;
				It.cchTextMax = 0;
				size_t l;
				do
				{
					It.cchTextMax = (It.cchTextMax | 0xff) + 1;
					Str.resize(It.cchTextMax);
					It.pszText = (LPSTR)Str.c_str();
					SendMessageA(hWnd, LVM_GETITEMTEXTA, (WPARAM)Index, (LPARAM)&It);
				} while((l = strlen(Str.c_str())) >= (It.cchTextMax - 3));
				Str.resize(l);
			}

			inline size_t get_str(char* Buffer, size_t Len)
			{
				LV_ITEMA It;
				It.iSubItem = SubItem;
				It.iItem = Index;
				It.cchTextMax = Len;
				It.pszText = Buffer;
				SendMessageA(hWnd, LVM_GETITEMTEXTA, (WPARAM)Index, (LPARAM)&It);
				return strnlen(Buffer, Len);
			}

			inline size_t get_str(wchar_t* Buffer, size_t Len)
			{
				LV_ITEMW It;
				It.iSubItem = SubItem;
				It.iItem = Index;
				It.cchTextMax = Len;
				It.pszText = Buffer;
				SendMessageW(hWnd, LVM_GETITEMTEXTW, (WPARAM)Index, (LPARAM)&It);
				return wcsnlen(Buffer, Len);
			}

			inline void set_str(char* Val)
			{
				LV_ITEMA It;
				It.iSubItem = SubItem;
				It.iItem = Index;
				It.pszText = Val;
				SendMessageW(hWnd, LVM_SETITEMTEXTA, (WPARAM)Index, (LPARAM)&It);
			}

			inline void set_str(wchar_t* Val)
			{
				LV_ITEMW It;
				It.iSubItem = SubItem;
				It.iItem = Index;
				It.pszText = Val;
				SendMessageW(hWnd, LVM_SETITEMTEXTW, (WPARAM)Index, (LPARAM)&It);
			}
		};
	public:
		union
		{
			struct
			{
				HWND hWnd;
				int Index;
				int SubItem;
			};

			EX_WND::__TEXT_PROPERTY_SET_GET<__SUBITEM_TEXT> Text;

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
					ListView_GetSubItemRect(hWnd, Index, SubItem, LVIR_ICON, &Rect);
					return Rect;
				}
				inline BOOL operator()(LPRECT Rect) { return ListView_GetSubItemRect(hWnd, Index, SubItem, LVIR_ICON, Rect); }
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
					ListView_GetSubItemRect(hWnd, Index, SubItem, LVIR_BOUNDS, &Rect);
					return Rect;
				}
				inline BOOL operator()(LPRECT Rect) { return ListView_GetSubItemRect(hWnd, Index, SubItem, LVIR_BOUNDS, Rect); }

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
					ListView_GetSubItemRect(hWnd, Index, SubItem, LVIR_LABEL, &Rect);
					return Rect;
				}
				inline BOOL operator()(LPRECT Rect) { return ListView_GetSubItemRect(hWnd, Index, SubItem, LVIR_LABEL, Rect); }
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
					ListView_GetItem(hWnd, &It);
					return It.iImage;
				}

				int operator =(int iImage)
				{
					LV_ITEM It;
					It.mask = LVIF_IMAGE;
					It.iSubItem = SubItem;
					It.iItem = Index;
					It.iImage = iImage;
					ListView_SetItem(hWnd, &It);
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
					ListView_GetItem(hWnd, &It);
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
					ListView_SetItem(hWnd, &It);
					return Val;
				}
			} UserData;
		};

		inline operator LV_ITEMW()
		{
			LV_ITEMW li;
			li.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_STATE | LVIF_INDENT;
			li.stateMask = LVIS_CUT | LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED | LVIS_OVERLAYMASK | LVIS_STATEIMAGEMASK;
			li.iItem = Index;
			li.iSubItem = SubItem;
			SendMessageW(hWnd, LVM_GETITEMW, 0, (LPARAM)&li);
			return li;
		}

		inline operator LV_ITEMA()
		{
			LV_ITEMA li;
			li.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_STATE | LVIF_INDENT;
			li.stateMask = LVIS_CUT | LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED | LVIS_OVERLAYMASK | LVIS_STATEIMAGEMASK;
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
				inline operator UINT() const
				{
					return ListView_GetItemState(hWnd, Index, LVIS_CUT | LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED | LVIS_OVERLAYMASK | LVIS_STATEIMAGEMASK);
				}

				inline UINT operator()(UINT Mask)
				{
					return ListView_GetItemState(hWnd, Index, Mask);
				}

				inline UINT operator=(UINT NewState)
				{
					ListView_SetItemState(hWnd, Index, NewState, LVIS_CUT | LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED | LVIS_OVERLAYMASK | LVIS_STATEIMAGEMASK);
					return NewState;
				}

				inline void operator()(UINT NewState, UINT Mask) { ListView_SetItemState(hWnd, Index, NewState, Mask); }
			} State;

			class
			{
				struct
				{
					HWND hWnd;
					int Index;
				};
			public:
				operator bool() const { return ListView_IsItemVisible(hWnd, Index) == TRUE; }
			} IsVisible;
		};

		ITEM(HWND hWnd, int Index)
		{
			this->hWnd = hWnd;
			this->Index = Index;
		}

		HWND EditLabel() { return ListView_EditLabel(hWnd, Index); }

		inline BOOL Delete() { return ListView_DeleteItem(hWnd, Index); }

		inline BOOL Update() { return ListView_Update(hWnd, Index); }

		inline SUBITEM operator[](int SubItem) { return SUBITEM(hWnd, Index, SubItem); }
	};

	union
	{
		HWND hWnd;
		class: public EX_WND
		{
		public:
			EX_WND_FIELD_AND_METHODS;
		};

		class
		{
			HWND hWnd;
		public:
			inline operator COLORREF() const { return ListView_GetBkColor(hWnd); }

			inline COLORREF operator=(COLORREF Val)
			{
				ListView_SetBkColor(hWnd, Val);
				return Val;
			}
		} BkColor;

		class
		{
			HWND hWnd;
		public:
			inline operator COLORREF() const { return ListView_GetTextColor(hWnd); }

			inline COLORREF operator=(COLORREF Val)
			{
				ListView_SetTextColor(hWnd, Val);
				return Val;
			}
		} TextColor;

		class
		{
			HWND hWnd;
		public:
			inline operator COLORREF() const { return ListView_GetTextBkColor(hWnd); }

			inline COLORREF operator=(COLORREF Val)
			{
				ListView_SetTextBkColor(hWnd, Val);
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
					inline operator int() const
					{
						LV_COLUMNW Col = {LVCF_FMT};
						int Count = 0;
						for(; (BOOL)SendMessageW(hWnd, LVM_GETCOLUMNW, (WPARAM)Count, (LPARAM)&Col) == TRUE; Count++);
						return Count;
					}
				} Count;

				class __CUR_SEL
				{
				public:

					class
					{
						friend __CUR_SEL;
						HWND hWnd;
					public:
						operator unsigned() const { return ListView_GetSelectedCount(hWnd); }
					} Count;

					inline operator int() const { return ListView_GetSelectedColumn(Count.hWnd); }

					inline operator COLUMN()
					{
						int Index = ListView_GetSelectedColumn(Count.hWnd);
						return COLUMN(hWnd, Index);
					}

					inline COLUMN operator()()
					{
						int Index = ListView_GetSelectedColumn(Count.hWnd);
						return COLUMN(hWnd, Index);
					}

					inline int operator=(int Index)
					{
						ListView_SetSelectedColumn(Count.hWnd, Index);
						return Index;
					}

					inline COLUMN & operator=(COLUMN & Col)
					{
						ListView_SetSelectedColumn(Count.hWnd, Col.Index);
						return Col;
					}
				} CurSel;
			};

			inline COLUMN operator[](int Index) { return COLUMN(hWnd, Index); }

			inline COLUMN New()
			{
				LVCOLUMNW Col = {LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH,LVCFMT_CENTER,10,L"",0,0};
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
				LVCOLUMNA Col = {LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH,Align,Width,Str,0,SubItem};
				int Index = (int)SendMessageA(hWnd, LVM_INSERTCOLUMNA, (WPARAM)(int)Count, (LPARAM)&Col);
				return COLUMN(hWnd, Index);
			}

			inline COLUMN New(LPWSTR Str, int Width = 10, int Align = LVCFMT_CENTER, int SubItem = 0)
			{
				LVCOLUMNW Col = {LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH,Align,Width,Str,0,SubItem};
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
					inline operator int() const { return ListView_GetItemCount(hWnd); }

					inline int operator=(int NewCount) { return ListView_SetItemCount(hWnd, NewCount); }
				} Count;

				class
				{
					HWND hWnd;
				public:

					inline operator int() const { return  ListView_GetHotItem(hWnd); }

					inline int operator=(int Index) { return ListView_SetHotItem(hWnd, Index); }

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

			inline ITEM operator [](int Index) { return ITEM(hWnd, Index); }

			ITEM New()
			{
				LV_ITEM li = {LVIF_TEXT,0,0,0,0,TEXT(""),0,0};
				int Index = ListView_InsertItem(hWnd, &li);
				return ITEM(hWnd, Index);
			}

			ITEM New(int IndexInsert, int SubItemIndex = 0, LPWSTR Text = L"")
			{
				LV_ITEMW li = {LVIF_TEXT,0,0,0,0};
				li.iItem = IndexInsert;
				li.iSubItem = SubItemIndex;
				li.pszText = Text;
				return New(li);
			}

			ITEM New(int IndexInsert, int SubItemIndex, LPSTR Text, int iImage = 0)
			{
				LV_ITEMA li = {LVIF_TEXT,0,0,0,0};
				li.iItem = IndexInsert;
				li.iSubItem = SubItemIndex;
				li.pszText = Text;
				return New(li);
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
				int Index = (BOOL)SendMessageA(hWnd, LVM_SETITEMA, 0, (LPARAM)&Item);
				return ITEM(hWnd, Index);
			}

			ITEM Set(LV_ITEMW & Item)
			{
				int Index = (BOOL)SendMessageW(hWnd, LVM_SETITEMW, 0, (LPARAM)&Item);
				return ITEM(hWnd, Index);
			}

			BOOL DeleteAll() { return ListView_DeleteAllItems(hWnd); }
		} Items;

		/*
		set, get;
		LVS_EX_ ...
		*/
		class
		{
			HWND hWnd;
		public:
			operator DWORD() const { return ListView_GetExtendedListViewStyle(hWnd); }

			DWORD operator=(DWORD Val)
			{
				ListView_SetExtendedListViewStyle(hWnd, Val);
				return Val;
			}

			DWORD operator|=(DWORD Val)
			{
				ListView_SetExtendedListViewStyle(hWnd, Val | ListView_GetExtendedListViewStyle(hWnd));
				return Val;
			}

			DWORD operator&=(DWORD Val)
			{
				ListView_SetExtendedListViewStyle(hWnd, Val & ListView_GetExtendedListViewStyle(hWnd));
				return Val;
			}
		} ExtendedStyle;


		class
		{
			HWND hWnd;
		public:
			operator bool() const { return ListView_GetExtendedListViewStyle(hWnd) & LVS_EX_GRIDLINES; }

			bool operator=(bool Val)
			{

				DWORD r = ListView_GetExtendedListViewStyle(hWnd);
				if(Val)
					r |= LVS_EX_GRIDLINES;
				else
					r &= (~LVS_EX_GRIDLINES);
				ListView_SetExtendedListViewStyle(hWnd, r);
				return Val;
			}
		} IsShowGrid;

	};

};

/*
Show dialog for select file in file system.
*/
inline bool OpenFileDialog
(
	LPWSTR bufFileName,
	DWORD sizeBuf,
	LPWSTR Filter = nullptr,
	LPWSTR Title = nullptr,
	LPWSTR InitialDir = nullptr,
	HWND hwnd = NULL,
	DWORD Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST
)
{
	OPENFILENAMEW ofn = {0};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = bufFileName;
	ofn.lpstrFile[0] = L'\0';
	ofn.nMaxFile = sizeBuf;
	ofn.lpstrFilter = (Filter == nullptr) ? L"All\0*.*\0" : Filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = Title;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = InitialDir;
	ofn.Flags = Flags;
	return GetOpenFileNameW(&ofn) != FALSE;
}

inline bool OpenFileDialog
(
	LPSTR bufFileName,
	DWORD sizeBuf,
	LPSTR Filter = nullptr,
	LPSTR Title = nullptr,
	LPSTR InitialDir = nullptr,
	HWND hwnd = NULL,
	DWORD Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST
)
{
	OPENFILENAMEA ofn = {0};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = bufFileName;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeBuf;
	ofn.lpstrFilter = (Filter == nullptr) ? "All\0*.*\0" : Filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = Title;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = InitialDir;
	ofn.Flags = Flags;
	return GetOpenFileNameA(&ofn) != FALSE;
}


inline bool SaveFileDialog
(
	LPWSTR bufFileName,
	DWORD sizeBuf,
	LPWSTR Filter = nullptr,
	LPWSTR Title = nullptr,
	LPWSTR InitialDir = nullptr,
	HWND hwnd = NULL,
	DWORD Flags = 0
)
{
	OPENFILENAMEW ofn = {0};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = bufFileName;
	ofn.lpstrFile[0] = L'\0';
	ofn.nMaxFile = sizeBuf;
	ofn.lpstrFilter = (Filter == nullptr) ? L"All\0*.*\0" : Filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = Title;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = InitialDir;
	ofn.Flags = 0;
	return GetSaveFileNameW(&ofn) != FALSE;
}


inline bool SaveFileDialog
(
	LPSTR bufFileName,
	DWORD sizeBuf,
	LPSTR Filter = nullptr,
	LPSTR Title = nullptr,
	LPSTR InitialDir = nullptr,
	HWND hwnd = NULL,
	DWORD Flags = 0
)
{

	OPENFILENAMEA ofn = {0};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = bufFileName;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeBuf;
	ofn.lpstrFilter = (Filter == nullptr) ? "All\0*.*\0" : Filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = Title;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = InitialDir;
	ofn.Flags = 0;
	return GetSaveFileNameA(&ofn) != FALSE;
}

