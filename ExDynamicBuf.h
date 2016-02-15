#ifndef __EX_DYNAMIC_BUF_H__
#define __EX_DYNAMIC_BUF_H__

#include <malloc.h>
#include <atomic>
#include "ExTypeTraits.h"



#if /*!defined(_DEBUG) &&*/ defined(_MSC_VER)
#define ___malloc(size) HeapAlloc(GetProcessHeap(), 0, size)
#define ___realloc(pointer, size) ((pointer == nullptr)? ___malloc(size): HeapReAlloc(GetProcessHeap(), 0, pointer, size))
#define ___free(pointer) HeapFree(GetProcessHeap(), 0, pointer)
#else
#define ___malloc(size) malloc(size)
#define ___realloc(pointer, size) realloc(pointer, size)
#define ___free(pointer) free(pointer)
#endif



template<typename TypeElement = char>
class DYNAMIC_BUF
{
	struct FIELDS { size_t count, alloc_count; TypeElement* buf; };
public:

	union{
		class{
			friend DYNAMIC_BUF;
			FIELDS f;
			void Increase(size_t NewSize)
			{
				if(NewSize >= f.alloc_count)
				{
					size_t ReallocSize = (size_t)(NewSize * 1.61803398875f);
					TypeElement* a = (TypeElement*)___realloc(f.buf, ReallocSize * sizeof(TypeElement));
					if(a == nullptr)
						return;
					f.buf = a;
					f.alloc_count = ReallocSize;
				}
				f.count = NewSize;
			}

			void Decrease(size_t NewSize)
			{
				if(f.alloc_count > (size_t)(NewSize *  1.61803398875f))
				{
					size_t ReallocSize = NewSize + 3;
					TypeElement* a = (TypeElement*)___realloc(f.buf, ReallocSize * sizeof(TypeElement));
					if(a == nullptr)
						return;
					f.buf = a;
					f.alloc_count = ReallocSize;
				}
				f.count = NewSize;
			}
		public:
			inline operator size_t () const { return f.count; }
			size_t operator=(size_t NewSize)
			{
				if(NewSize < f.count)
					Decrease(NewSize);
				else if(NewSize > f.count)
					Increase(NewSize);
				return NewSize;
			}
			size_t operator++()
			{
				Increase(f.count + 1);
				return f.count;
			}
			size_t operator--()
			{
				if(f.count <= 0)
					return 0;
				Decrease(f.count - 1);
				return f.count;
			}

			size_t operator++(int)
			{
				Increase(f.count + 1);
				return f.count;
			}
			size_t operator--(int)
			{
				if(f.count <= 0)
					return 0;
				Decrease(f.count - 1);
				return f.count;
			}
		} Count;

		class{ FIELDS f; public:
			inline operator size_t () const { return f.alloc_count; }
			size_t operator=(size_t NewAllocCount)
			{
				if(NewAllocCount == 0)
				{
					___free(f.buf);
					f.buf = nullptr;
				}else
				{
					TypeElement* a = (TypeElement*)___realloc(f.buf, NewAllocCount * sizeof(TypeElement));
					if(a == nullptr)
						return NewAllocCount;
					f.buf = a;
				}
				f.count = f.alloc_count = NewAllocCount;
				return NewAllocCount;
			}
		} AllocCount;

		class{ FIELDS f; public: 
			inline operator TypeElement*() const { return f.buf; } 
			inline operator void*() const { return (void*)f.buf; } 
		} BeginBuf;

		class{ FIELDS f; public: 
			inline operator TypeElement*() const { return f.buf + f.count; } 
			inline operator void*() const { return (void*)(f.buf + f.count); } 
		} EndBuf;
	};

	TypeElement& InsertInPosition(size_t Index)
	{
		Count++;
		memmove(Count.f.buf + (Index + 1), Count.f.buf + Index, Count - Index - 1);
		return Count.f.buf[Index];
	}

	TypeElement& InsertInPositionSubstituting(size_t Index)
	{
		Count++;
		*(EndBuf - 1) = Count.f.buf[Index];
		return Count.f.buf[Index];
	}

	inline void RemoveFromPosition(size_t Index)
	{
		memmove(Count.f.buf + Index, Count.f.buf + (Index + 1), Count - Index - 1);
		Count--;
	}

	inline TypeElement& Append()
	{
		Count++;
		return *(EndBuf - 1);
	}

	inline void Pop() { Count--; }

	inline void RemoveSubstituting(size_t Index)
	{
		Count.f.buf[Index] = *(EndBuf - 1);
		Count--;
	}

	int Search(TypeElement& Val)
	{
		auto buf = Count.f.buf;
		for(size_t i = 0, m = Count; i < m; i++)
			if(buf[i] == Val)
				return i;
		return -1;
	}

	void Move(DYNAMIC_BUF<TypeElement>& Dest)
	{
		if(Dest.Count.f.buf != nullptr)
			___free(Dest.Count.f.buf);
		Dest.Count.f.buf = Count.f.buf;
		Count.f.buf = nullptr;
		Dest.Count.f.count = Count.f.count;
		Dest.Count.f.alloc_count = Count.f.alloc_count;
		Count.f.alloc_count = Count.f.count = 0;
	}

	inline void SetZero() { memset(Count.f.buf, 0, sizeof(TypeElement) * Count.f.count); }

	inline TypeElement& operator[](size_t Index) { return Count.f.buf[Index]; }

	inline DYNAMIC_BUF() { Count.f.alloc_count = Count.f.count = 0; Count.f.buf = nullptr; }

	DYNAMIC_BUF(size_t NewSize)
	{
		Count.f.alloc_count = Count.f.count = 0;
		Count.f.buf = nullptr;
		Count = NewSize;
	}

	~DYNAMIC_BUF()
	{
		if(Count.f.buf != nullptr)
			___free(Count.f.buf);
	}

};


template<typename TypeElement = char>
class DYNAMIC_BUF_S
{
	struct FIELDS { size_t count; TypeElement* buf; };
public:

	union{
		class{ 
			FIELDS f; friend DYNAMIC_BUF_S;
		public:
			inline operator size_t () const { return f.count; }
			size_t operator=(size_t NewCount)
			{
				if(NewCount == 0)
				{
					___free(f.buf);
					f.buf = nullptr;
				}else
				{
					TypeElement* a = (TypeElement*)___realloc(f.buf, NewCount * sizeof(TypeElement));
					if(a == nullptr)
						return NewCount;
					f.buf = a;
				}
				f.count = NewCount;
				return NewCount;
			}

			inline size_t operator++() { return operator=(f.count + 1); }
			size_t operator--() { return operator=(f.count - 1); }
			inline size_t operator++(int) { return operator=(f.count + 1); }
			size_t operator--(int) { return operator=(f.count - 1); }
		} Count;

		class{ FIELDS f; public: 
			inline operator TypeElement*() const { return f.buf; } 
			inline operator void*() const { return (void*)f.buf; } 
		} BeginBuf;

		class{ FIELDS f; public: 
			inline operator TypeElement*() const { return f.buf + f.count; } 
			inline operator void*() const { return (void*)(f.buf + f.count); } 
		} EndBuf;
	};

	TypeElement& InsertInPosition(size_t Index)
	{
		Count++;
		memmove(Count.f.buf + (Index + 1), Count.f.buf + Index, Count - Index - 1);
		return Count.f.buf[Index];
	}

	TypeElement& InsertInPositionSubstituting(size_t Index)
	{
		Count++;
		*(EndBuf - 1) = Count.f.buf[Index];
		return Count.f.buf[Index];
	}

	inline void RemoveFromPosition(size_t Index)
	{
		memmove(Count.f.buf + Index, Count.f.buf + (Index + 1), Count - Index - 1);
		Count--;
	}

	inline TypeElement& Append()
	{
		Count++;
		return *(EndBuf - 1);
	}

	inline void Pop() { Count--; }

	inline void RemoveSubstituting(size_t Index)
	{
		Count.f.buf[Index] = *(EndBuf - 1);
		Count--;
	}

	int Search(TypeElement& Val)
	{
		auto buf = Count.f.buf;
		for(size_t i = 0, m = Count; i < m; i++)
			if(buf[i] == Val)
				return i;
		return -1;
	}

	void Move(DYNAMIC_BUF_S<TypeElement>& Dest)
	{
		if(Dest.Count.f.buf != nullptr)
			___free(Dest.Count.f.buf);
		Dest.Count.f.buf = Count.f.buf;
		Count.f.buf = nullptr;
		Dest.Count.f.count = Count.f.count;
		Count.f.count = 0;
	}

	inline void SetZero() { memset(Count.f.buf, 0, sizeof(TypeElement) * Count); }

	inline TypeElement& operator[](size_t Index) { return Count.f.buf[Index]; }

	inline DYNAMIC_BUF_S() { Count.f.count = 0; Count.f.buf = nullptr; }

	DYNAMIC_BUF_S(size_t NewSize)
	{
		Count.f.count = 0;
		Count.f.buf = nullptr;
		Count = NewSize;
	}

	~DYNAMIC_BUF_S()
	{
		if(Count.f.buf != nullptr)
			___free(Count.f.buf);
	}

};


template<bool IsThreadSafe = false>
class __FAST_ALLOC
{
	//If size element greather then pointer
	template<size_t SizeElem>
	struct FIELDS
	{
		void* StartElement;
		size_t Count;
		size_t SizeList;

		FIELDS(): StartElement(nullptr), SizeList(80), Count(0) {}

		void* Alloc()
		{
			
			Lock<SizeElem>();
			if(StartElement != nullptr){		
				void* Ret = StartElement;
				StartElement = *(void**)Ret;
				Count--;
				Unlock<SizeElem>();
				return Ret;
			}else{
				Unlock<SizeElem>();
				return ___malloc(SizeElem);}
		}
		void Free(void* Data)
		{
			Lock<SizeElem>();
			if(Count >= SizeList){
				Unlock<SizeElem>();
				___free(Data); 
			}else{
				*(void**)Data = StartElement;
				StartElement = Data;
				Count++;
				Unlock<SizeElem>();}
		}
		void ClearList()
		{
			Lock<SizeElem>();
			void * Cur = StartElement;
			StartElement = nullptr;
			while(Cur != nullptr)
			{
				void * v = *(void**)Cur;
				___free(Cur);
				Cur = v;
			}
			Count = 0;
			Unlock<SizeElem>();
		}
		inline void SetMaxCount(size_t NewVal) { SizeList = NewVal; }
	};

	template<size_t SizeElem>
	struct FIELDS_FOR_LESS_THEN_POINTER
	{
		void** Pointers;
		size_t Count;
		size_t SizeList;

		FIELDS_FOR_LESS_THEN_POINTER() : SizeList(80), Count(0) { Pointers = (void**)___malloc(SizeList * sizeof(void*)); }

		void* Alloc() 
		{ 
			Lock<SizeElem>();
			if(Count > 0){
				auto r = Pointers[--Count];
				Unlock<SizeElem>();
				return r;
			}else{
				Unlock<SizeElem>();
				return ___malloc(SizeElem);}
		}
		void Free(void* Data)
		{
			Lock<SizeElem>();
			if(Count >= SizeList){
				Unlock<SizeElem>();
				___free(Data); 
			}else{
				Pointers[Count++] = Data;
				Unlock<SizeElem>();}
		}
		void ClearList()
		{
			Lock<SizeElem>();
			for(size_t i = 0, m = Count; i < m; i++)
				___free(Pointers[i]);
			Count = 0;
			Unlock<SizeElem>();
		}
		void SetMaxCount(size_t NewVal)
		{
			Lock<SizeElem>();
			if(Count >= NewVal)
			{
				for(size_t i = NewVal, m = Count; i < m; i++)
					___free(Pointers[i]);
				Count = NewVal;
			}
			void** b = (void**)___realloc(Pointers, NewVal * sizeof(void*));
			if(b == nullptr)
				goto lblOut;
			Pointers = b;
			SizeList = NewVal;
lblOut:
			Unlock<SizeElem>();
		}

	};

	template<class T> 
	struct VAL_TYPE: std::conditional<sizeof(T) < sizeof(void*), FIELDS_FOR_LESS_THEN_POINTER<sizeof(T)>, FIELDS<sizeof(T)>>::type {};
	
	template<size_t Len> 
	struct VAL_TYPE_: std::conditional<Len < sizeof(void*), FIELDS_FOR_LESS_THEN_POINTER<Len>, FIELDS<Len>>::type {};	
	
	struct MUTEX: public std::atomic_flag {};

	template<size_t Len>
	static inline void Lock() { if(IsThreadSafe) while(std::assoc_val<size_t, Len, MUTEX>::value.test_and_set(std::memory_order_acquire)); }
	template<size_t Len>
	static inline void Unlock() { if(IsThreadSafe) std::assoc_val<size_t, Len, MUTEX>::value.clear(std::memory_order_release); }
public:

	template<typename Type>
	inline static Type* New(){	
		auto Elem = std::assoc_val<size_t, sizeof(Type), VAL_TYPE<Type>>::value.Alloc();
		return (Elem == nullptr)? nullptr: (new(Elem) Type());}
	template<typename Type, typename A1>
	inline static Type* New(A1 arg1){	
		auto Elem = std::assoc_val<size_t, sizeof(Type), VAL_TYPE<Type>>::value.Alloc();
		return (Elem == nullptr)? nullptr: (new(Elem) Type(arg1));}
	template<typename Type, typename A1, typename A2>
	inline static Type* New(A1 arg1, A2 arg2){	
		auto Elem = std::assoc_val<size_t, sizeof(Type), VAL_TYPE<Type>>::value.Alloc();
		return (Elem == nullptr)? nullptr: (new(Elem) Type(arg1, arg2));}
	template<typename Type, typename A1, typename A2, typename A3>
	inline static Type* New(A1 arg1, A2 arg2, A3 arg3){	
		auto Elem = std::assoc_val<size_t, sizeof(Type), VAL_TYPE<Type>>::value.Alloc();
		return (Elem == nullptr)? nullptr: (new(Elem) Type(arg1, arg2, arg3));}
	template<typename Type, typename A1, typename A2, typename A3, typename A4>
	inline static Type* New(A1 arg1, A2 arg2, A3 arg3, A4 arg4){	
		auto Elem = std::assoc_val<size_t, sizeof(Type), VAL_TYPE<Type>>::value.Alloc();
		return (Elem == nullptr)? nullptr: (new(Elem) Type(arg1, arg2, arg3, arg4)); }
	template<typename Type, typename A1, typename A2, typename A3, typename A4, typename A5>
	inline static Type* New(A1 arg1, A2 arg2, A3 arg3, A4 arg4, A5 arg5){	
		auto Elem = std::assoc_val<size_t, sizeof(Type), VAL_TYPE<Type>>::value.Alloc();
		return (Elem == nullptr)? nullptr: (new(Elem) Type(arg1, arg2, arg3, arg4, arg5));}
	template<typename Type, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	inline static Type* New(A1 arg1, A2 arg2, A3 arg3, A4 arg4, A5 arg5, A6 arg6){	
		auto Elem = std::assoc_val<size_t, sizeof(Type), VAL_TYPE<Type>>::value.Alloc();
		return (Elem == nullptr)? nullptr: (new(Elem) Type(arg1, arg2, arg3, arg4, arg5, arg6));}

	/*
	Delete memory region with adding in stack regions. Late, this region takes from stack.
	*/
	template<typename Type>
	inline static typename std::enable_if<!std::is_equal<Type, void>::value>::type Delete(Type* Val)
	{
		Val->~Type();
		std::assoc_val<size_t, sizeof(Type), VAL_TYPE<Type>>::value.Free(Val);
	}

	/*
	Delete memory region without adding in stack.
	Caution! In case call this function not call destructor for type.
	*/
	inline static void JustDelete(void* Val) { ___free(Val); }

	template<typename Type>
	inline static void ClearList() {  std::assoc_val<size_t, sizeof(Type), VAL_TYPE<Type>>::value.ClearList(); }

	template<typename Type>
	inline static void SetMaxCountList(size_t NewSize) { std::assoc_val<size_t, sizeof(Type), VAL_TYPE<Type>>::value.SetMaxCount(NewSize); }

	template<typename Type>
	inline static size_t GetMaxCountList() { return std::assoc_val<size_t, sizeof(Type), VAL_TYPE<Type>>::value.SizeList; }

};


typedef __FAST_ALLOC<true> FAST_ALLOC;
typedef __FAST_ALLOC<false> FAST_ALLOC_UNSYNC;
#endif