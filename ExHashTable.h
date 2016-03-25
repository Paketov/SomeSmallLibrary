#ifndef __EXHASH_TABLE_H_HAS_INCLUDED__
#define __EXHASH_TABLE_H_HAS_INCLUDED__

#include <stdio.h>
#include "ExTypeTraits.h"
#include "ExDynamicBuf.h"

/*
ExHashTable
Paketov
2015-2016

Low-level hash table.

In this table each element located in a single array.
This table fully compatible with HASH_TABLE_DYN.
You have the freedom to choose between HASH_TABLE_DYN or HASH_TABLE.


Example:

	typedef struct HASH_ELEMENT
	{
		unsigned vKey;
		double   Val;

		bool SetKey(unsigned k)
		{
			vKey = k;
			return false;
		}

		inline static unsigned short IndexByKey(unsigned k, unsigned char MaxCount) 
		{ 
			return k % MaxCount; 
		}

		inline unsigned short IndexInBound(unsigned char MaxCount) const
		{
			return IndexByKey(KeyVal, MaxCount);
		}

		inline bool CmpKey(unsigned k)
		{
			return k == vKey;
		}
	}  HASH_ELEMENT;

	HASH_TABLE<HASH_ELEMENT> HashArray(12);

	HashArray.Insert(0.000012)->Val = 0.000012;
	printf("%lf", HashArray.Search(0.000012)->Val); 

*/


template
<
	typename TElementStruct,
	typename TIndex = decltype(std::declval<TElementStruct>().IndexInBound(0)),
	TIndex NothingIndex = TIndex(-1)
>
class HASH_TABLE
{	
public:
	typedef TIndex	TINDEX, *LPTINDEX;
	typedef TElementStruct	TELEMENT, *LPTELEMENT;
	typedef struct { TINDEX iStart, iNext; } THEADCELL, *LPTHEADCELL;		
	typedef struct CELL : public THEADCELL, public TElementStruct {} CELL, *LPCELL;

	static inline void CopyElement(CELL& Dest, const CELL& Source) { *(TElementStruct*)((LPTHEADCELL)&Dest + 1) = *(TElementStruct*)((LPTHEADCELL)&Source + 1); }

protected:

#define EXHASH_TABLE_FIELDS												\
	struct																\
	{																	\
	LPCELL Table;														\
	TINDEX count, last_empty, alloc_count;								\
	}

public:

	union
	{
		class{ EXHASH_TABLE_FIELDS; friend HASH_TABLE;
		inline TINDEX operator =(TINDEX NewVal) { return count = NewVal; } public:
			inline operator TINDEX() const { return count; }
		} Count;

		class{ EXHASH_TABLE_FIELDS; friend HASH_TABLE;
		public:
			inline operator TINDEX() const { return alloc_count; }
		} AllocCount;

		class{ EXHASH_TABLE_FIELDS; friend HASH_TABLE; public:
			inline operator bool() const { return last_empty == NothingIndex; }
		} IsFull;

		/*Use this property only for debug!*/
		class{	EXHASH_TABLE_FIELDS; public:
			operator TINDEX() const
			{ 
				TINDEX c = 0;
				LPCELL t = Table;
				for(TINDEX i = last_empty; i != EmptyElement; i = t[i].THEADCELL::iNext)
					c++;
				return c;
			}
		}  EmptyCount;
	};

	static const TINDEX EmptyElement = NothingIndex;

	inline LPCELL GetTable() const { return Count.Table; }

	template<typename TYPE_KEY>
	inline TINDEX IndexByKey(TYPE_KEY Key) const { return TElementStruct::IndexByKey(Key, AllocCount); }

	template<typename TYPE_KEY>
	inline LPCELL ElementByKey(TYPE_KEY Key) { return GetTable() + TElementStruct::IndexByKey(Key, AllocCount); }

	/*
		Use this class only before ResizeAfterRemove().
	*/
	class REMOVE_POINTER
	{
		friend HASH_TABLE;
		TElementStruct* Val;
		inline REMOVE_POINTER(TElementStruct* NewVal) { Val = NewVal; }
	public:
		inline operator TElementStruct*() const { return Val; }
		inline TElementStruct* operator->() const { return Val; }
	};

protected:

	bool ReallocAndClear(TINDEX NewAllocCount)
	{
		LPCELL t = (LPCELL)___realloc(Count.Table, sizeof(CELL) * NewAllocCount);
		if(t == nullptr)
			return false;
		Count.Table = t; 
		Count.alloc_count = NewAllocCount;
		for(TINDEX i = Count.last_empty = 0; i < NewAllocCount; i++)
		{
			t[i].THEADCELL::iStart = EmptyElement;
			t[i].THEADCELL::iNext = i + 1;
		}
		t[NewAllocCount - 1].THEADCELL::iNext = EmptyElement;
		return true;
	}

	bool Realloc(TINDEX NewAllocCount)
	{
		LPCELL t = (LPCELL)___realloc(Count.Table, sizeof(CELL) * NewAllocCount);
		if(t == nullptr)
			return false;
		Count.Table = t; 
		Count.alloc_count = NewAllocCount;
		return true;
	}

public:

	/*
		After call this constructor AllocCount = NewAllocCount.
	*/
	HASH_TABLE(TINDEX NewAllocCount = 1)
	{
		Count.Table = nullptr;
		Count.count = Count.alloc_count = 0;
		if(NewAllocCount <= 0)
			NewAllocCount = 1;
		ReallocAndClear(NewAllocCount);
	}

	inline ~HASH_TABLE()
	{
		if(Count.Table != nullptr)
			___free(Count.Table);
	}

	/*
		Insert with checkin have element
	*/
	template<typename T>
	inline TElementStruct* Insert(T SearchKey)
	{
		LPCELL lpStart, p, t = GetTable();
		for(TINDEX i = (lpStart = ElementByKey(SearchKey))->THEADCELL::iStart; i != EmptyElement; i = p->THEADCELL::iNext)
			if((p = t + i)->CmpKey(SearchKey))
				return p;
		
		TINDEX iRetElem;	
		p = t + (iRetElem = Count.last_empty);
		if(!p->SetKey(SearchKey))
			return nullptr;
		Count.last_empty = p->THEADCELL::iNext;
		p->THEADCELL::iNext = lpStart->THEADCELL::iStart;
		lpStart->THEADCELL::iStart = iRetElem;
		Count.count++;
		return p;
	}

	/*
		Insert element without check availability element.
		In case collision for read multiple values use NextCollision function.
	*/
	template<typename T>
	inline TElementStruct* OnlyInsert(T SearchKey)
	{			
		TINDEX iRetElem;
		LPCELL lpStart = ElementByKey(SearchKey), lpRetElem = GetTable() + (iRetElem = Count.last_empty);
		if(!lpRetElem->SetKey(SearchKey))
			return nullptr;
		Count.last_empty = lpRetElem->THEADCELL::iNext;
		lpRetElem->THEADCELL::iNext = lpStart->THEADCELL::iStart;
		lpStart->THEADCELL::iStart = iRetElem;
		Count.count++;
		return lpRetElem;
	}

	/*
		Simple search element by various type key
	*/
	template<typename T>
	inline TElementStruct* Search(T SearchKey) const
	{
		LPCELL p, t = GetTable();
		for(TINDEX i = t[TElementStruct::IndexByKey(SearchKey, AllocCount)].THEADCELL::iStart; i != EmptyElement; i = p->THEADCELL::iNext)
			if((p = t + i)->CmpKey(SearchKey))
				return p;
		return nullptr;
	}

	/*
		Search next collision in table.
		Caution! You can not change the table between Search and NextCollision.
	*/
	template<typename T>
	inline TElementStruct* NextCollision(TElementStruct* CurElem, T SearchKey) const
	{
		LPCELL p;
		for(TINDEX i = ((LPTHEADCELL)CurElem - 1)->THEADCELL::iNext; i != EmptyElement; i = p->THEADCELL::iNext)
			if((p = GetTable() + i)->CmpKey(SearchKey))
				return p;
		return nullptr;
	}

	/*
		Remove element by key.
		Return address element in table.
	*/
	template<typename T>
	REMOVE_POINTER Remove(T SearchKey)
	{
		LPCELL p, t = GetTable();
		for(LPTINDEX i = &(t[TElementStruct::IndexByKey(SearchKey, AllocCount)].THEADCELL::iStart); *i != EmptyElement; i = &(p->THEADCELL::iNext))
		{
			p = t + *i;
			if(p->CmpKey(SearchKey))
			{
				TINDEX j = *i;
				*i = p->THEADCELL::iNext;
				p->THEADCELL::iNext = Count.last_empty;
				Count.last_empty = j;
				Count.count--;
				return p;
			}
		}
		return nullptr;
	}

	/*
		Remove all collision element by key.
		To find out the number of deleted items, check property Count.
	*/
	template<typename T>
	void RemoveAllCollision(T SearchKey)
	{
		LPCELL p, t = GetTable();
		for(LPTINDEX i = &(t[TElementStruct::IndexByKey(Key, AllocCount)].THEADCELL::iStart); *i != EmptyElement; i = &(p->THEADCELL::iNext))
		{
			p = t + *i;
			if(p->CmpKey(SearchKey))
			{
				TINDEX j = *i;
				*i = p->THEADCELL::iNext;
				p->THEADCELL::iNext = Count.last_empty;
				Count.last_empty = j;
				Count.count--;
			}	
		}
	}


	/*
		Clear all table.
		Deletes all element from table.
	*/
	void Clear()
	{
		Count = 0;
		ReallocAndClear(1);
	}

	/*
		Enumerate all elements in the table with the EnumFunc.
		Caution! When you call this function, you can not change the contents of the table!
	*/
	inline bool EnumValues(bool (*EnumFunc)(void* UserData, TElementStruct* Element), void* UserData = nullptr) const
	{
		for(LPCELL Elements = GetTable(), l, p = Elements, m = p + AllocCount; p < m; p++)
			for(auto i = p->THEADCELL::iStart; i != EmptyElement;)
			{
				i = (l = Elements + i)->THEADCELL::iNext;
				if(!EnumFunc(UserData, l))
					return false;
			}
		return true;
	}

	/*
		Enumerate all elements in the table with the EnumFunc.
		Caution! When you call this function, you can not change the contents of the table!
	*/
	inline bool EnumValues(bool (*EnumFunc)(TElementStruct* Element))
	{
		for(LPCELL Elements = GetTable(), l, p = Elements, m = p + AllocCount; p < m; p++)
			for(auto i = p->THEADCELL::iStart; i != EmptyElement;)
			{
				i = (l = Elements + i)->THEADCELL::iNext;
				if(!EnumFunc(l)) 
					return false;
			}
		return true;
	}

	/*
		Enumerate all elements in the table with the EnumFunc.
		Is function return true element will be removed.
		Caution! When you call this function, you can not change the contents of the table!
	*/
	inline void EnumDelete(bool (*IsDeleteProc)(void* UserData, TElementStruct* Element), void* UserData = nullptr)
	{
		for(LPCELL Elements = GetTable(), l, p = Elements, m = p + AllocCount; p < m; p++)
			for(auto i = &(p->THEADCELL::iStart); *i != EmptyElement; )
			{	
				l = Elements + *i;
				if(IsDeleteProc(UserData, l))
				{
					auto e = *i;
					*i = l->THEADCELL::iNext;
					l->THEADCELL::iNext = Count.last_empty;
					Count.last_empty = e;
					Count.count--;
				}else
					i = &(l->THEADCELL::iNext);
			}
	}
		
	/*
		Enumerate all elements in the table with the EnumFunc.
		Is function return true element will be removed.
		Caution! When you call this function, you can not change the contents of the table!
	*/
	inline void EnumDelete(bool (*IsDeleteProc)(TElementStruct* Element))
	{
		for(LPCELL Elements = GetTable(), l, p = Elements, m = p + AllocCount; p < m; p++)
			for(auto i = &(p->THEADCELL::iStart); *i != EmptyElement; )
			{	
				l = Elements + *i;
				if(IsDeleteProc(l))
				{
					auto e = *i;
					*i = l->THEADCELL::iNext;
					l->THEADCELL::iNext = Count.last_empty;
					Count.last_empty = e;
					Count.count--;
				}else
					i = &(l->THEADCELL::iNext);
			}
	}



	/*========================================================*/
	/*Resize table functional*/
	/*
		Resize table to Count size. After call AllocCount == Count. 
		Since it is a low-level(!) hash table, a change in the size you have to do manually.
		This is made to maximize the flexibility of the algorithm table.
		After removal of the items, you are free to choose whether to reduce the table to the number of existing elements.
	*/
	bool ResizeAfterRemove()
	{
		size_t NewSize = Count.count;
		auto last_empty = Count.last_empty;
		LPCELL Elements = GetTable(), l, m = Elements + AllocCount;
		for(auto p = Elements; p < m; p++)
			for(auto i = p->THEADCELL::iStart; i != EmptyElement; i = l->THEADCELL::iNext)
			{
				l = Elements + i;
				if(i >= NewSize) 
				{
					for(;last_empty >= NewSize; last_empty = Elements[last_empty].THEADCELL::iNext);
					CopyElement(Elements[last_empty], Elements[i]);
					last_empty = Elements[last_empty].THEADCELL::iNext;
				}
			}
		for(TINDEX i = 0; i < NewSize;i++)
			Elements[i].THEADCELL::iStart = EmptyElement;
		for(TINDEX i = 0; i < NewSize; i++)
		{
			auto j = Elements[i].IndexInBound(NewSize);
			Elements[i].THEADCELL::iNext = Elements[j].THEADCELL::iStart;
			Elements[j].THEADCELL::iStart = i;
		}

		if(NewSize <= 0)
			return ReallocAndClear(1);
		Count.last_empty = EmptyElement;
		return Realloc(NewSize);
	}

	/*
		Resize table AllocCount to NewCount. After call AllocCount == NewCount.
		If before call NewCount < AllocCount, then behavior is unpredictable!
		Since it is a low-level(!) hash table, a change in the size you have to do manually.
		To verify the necessity of calling this function, use the property IsFull, next 
		calculate NewCount and call this function.
	*/
	bool ResizeBeforeInsert(TINDEX NewCount)
	{
		TINDEX FullCount = AllocCount;

		if(!Realloc(NewCount))
			return false;

		LPCELL Elements = GetTable();
		if(FullCount == Count)
		{	
			/*Init basic part*/
			for(LPCELL p = Elements, m = p + FullCount; p < m; p++)
				p->THEADCELL::iStart = EmptyElement;
			
			/*Initialize the remaining part*/
			for(TINDEX p = FullCount, m = NewCount; p < m; p++)
			{
				Elements[p].THEADCELL::iStart = EmptyElement;
				Elements[p].THEADCELL::iNext = p + 1;
			}
			Elements[NewCount - 1].THEADCELL::iNext = Count.last_empty;
			Count.last_empty = FullCount;
			/*Distribute all elements*/
			for(TINDEX i = 0; i < FullCount; i++)
			{
				auto j = Elements[i].IndexInBound(NewCount);
				Elements[i].THEADCELL::iNext = Elements[j].THEADCELL::iStart;
				Elements[j].THEADCELL::iStart = i;
			}
		}else
		{
			TINDEX UsedList = EmptyElement;
			/*Create list used elements*/
			for(TINDEX p = 0; p < FullCount; p++)
			{			
				for(TINDEX i = Elements[p].THEADCELL::iStart, t; i != EmptyElement; i = t)
				{
					t = Elements[i].THEADCELL::iNext;
					Elements[i].THEADCELL::iNext = UsedList;
					UsedList = i;
				}
				Elements[p].THEADCELL::iStart = EmptyElement;
			}
			/*Initialize the remaining part*/
			for(TINDEX p = FullCount, m = NewCount; p < m; p++)
			{
				Elements[p].THEADCELL::iStart = EmptyElement;
				Elements[p].THEADCELL::iNext = p + 1;
			}
			Elements[NewCount - 1].THEADCELL::iNext = Count.last_empty;
			Count.last_empty = FullCount;
			/*Distribute a list of used to new places*/
			for(TINDEX t; UsedList != EmptyElement; UsedList = t)
			{
				t = Elements[UsedList].THEADCELL::iNext;
				auto i = Elements[UsedList].IndexInBound(NewCount);
				Elements[UsedList].THEADCELL::iNext = Elements[i].THEADCELL::iStart;
				Elements[i].THEADCELL::iStart = UsedList;
			}
		}
		return true;
	}


	/*========================================================*/
	/*
		Clone this table to another
	*/
	bool Clone(HASH_TABLE<TElementStruct, TIndex, NothingIndex>& Dest) const
	{
		if(!Dest.Realloc(AllocCount))
			return false;
		Dest.Count.count = Count.count;
		Dest.Count.last_empty = Count.last_empty;
		memcpy(Dest.GetTable(), GetTable(), AllocCount * sizeof(CELL));
		return true;
	}
	/*
		Move this table to another.
		After call Count == 0.
	*/

	bool Move(HASH_TABLE<TElementStruct, TIndex, NothingIndex>& Dest)
	{
		if(!Dest.ReallocAndClear(1))
			return false;
		LPCELL t = Dest.Count.Table;
		Dest.Count.Table = Count.Table;
		Count.Table = t;
		Dest.Count.alloc_count = Count.alloc_count;
		Count.alloc_count = 1; 
		Dest.Count.count = Count.count;
		Count.count = 0;
		TINDEX le;
		le = Dest.Count.last_empty;
		Dest.Count.last_empty = Count.last_empty;
		Count.last_empty = le;
		return true;
	}

	/*========================================================*/
	/*
		While you use interator, you can't use ResizeAfterRemove or ResizeBeforeInsert functions.
		You can use Remove or RemoveAllCollision functions, but if you use Insert or OnlyInsert
		inserted elements may be listed or not.
		You can't remove iterate element via Remove or RemoveAllCollision functions.

	*/
	/*
		Interator type 
	*/
	typedef struct TINTER
	{
		friend HASH_TABLE;
	public:
		union
		{
			class
			{
				friend TINTER;
				friend HASH_TABLE;
				TINDEX CurStartList;
				TINDEX CurElementInList;
			public:
				inline operator bool() const { return CurStartList == EmptyElement; }
			} IsEnd;
		};
		void StartAgain() { IsEnd.CurStartList = EmptyElement; }
		inline TINTER() { StartAgain(); }
	} TINTER, *LPTINTER;

	/*
		Start or continue interate table.
	*/
	bool Interate(TINTER& SetInterator) const
	{
		TINDEX p;
		LPCELL Elements = GetTable();
		if(SetInterator.IsEnd)
		{
			p = 0;
lblSearchStart:
			for(TINDEX m = AllocCount; p < m; p++)
				if(Elements[p].THEADCELL::iStart != EmptyElement)
				{
					SetInterator.IsEnd.CurStartList = p;
					SetInterator.IsEnd.CurElementInList = Elements[p].THEADCELL::iStart;
					return true;
				}
			SetInterator.IsEnd.CurStartList = EmptyElement;
			return false;
		}
		TINDEX i = Elements[SetInterator.IsEnd.CurElementInList].THEADCELL::iNext;
		if(i != EmptyElement)
		{
			SetInterator.IsEnd.CurElementInList = i;
			return true;
		}
		p = SetInterator.IsEnd.CurStartList + 1;
		goto lblSearchStart;
	}
	/*
		Check interator is correct.
	*/
	inline bool InteratorCheck(const TINTER& Interator) const
	{
		if(Interator.IsEnd)
			return false;
		if(Interator.IsEnd.CurElementInList >= AllocCount)
			return false;
		return true;
	}
	/*
		Get element by interator.
	*/
	inline TElementStruct* ElementByInterator(const TINTER& SetInterator) const { return GetTable() + SetInterator.IsEnd.CurElementInList; }
	
	/*
		Search key and set interator to key position.
	*/
	template<typename TKey>
	bool InteratorByKey(TKey SearchKey, TINTER& Interator)
	{
		LPCELL p, t = GetTable();
		for(TINDEX s = TElementStruct::IndexByKey(SearchKey, AllocCount), i = t[s].THEADCELL::iStart; i != EmptyElement; i = p->THEADCELL::iNext)
			if((p = t + i)->CmpKey(SearchKey))
			{
				Interator.IsEnd.CurStartList = s;
				Interator.IsEnd.CurElementInList = i;
				return true;
			}
		return false;
	}
	/*
		Remove by interator.
	*/
	REMOVE_POINTER RemoveByInterator(TINTER& Interator)
	{
		LPCELL t = GetTable(), Ret = t + Interator.IsEnd.CurElementInList;
		for(LPTINDEX i = &t[Interator.IsEnd.CurStartList].THEADCELL::iStart; *i != EmptyElement; i = &t[*i].THEADCELL::iNext)
		{
			if(Interator.IsEnd.CurElementInList == *i)
			{
				Interate(Interator);	
				LPCELL El2 = t + *i;
				*i = El2->THEADCELL::iNext;
				Count.count--;
				return El2;
			}
		}
		return Ret;
	}

	/*========================================================*/
	/*Interate by key val*/
	inline TElementStruct* GetStartCell() const
	{
		LPCELL Elements = GetTable();
		for(TINDEX p = 0, m = AllocCount; p < m; p++)
			if(Elements[p].THEADCELL::iStart != EmptyElement)
			{
				return Elements + Elements[p].THEADCELL::iStart;
			}
		return nullptr;
	}

	/*Next interate by key val*/
	template<typename TKey>
	TElementStruct* GetNextCellByKey(TKey SearchKey) const
	{
		LPCELL Elements = GetTable(), p;
		for(TINDEX s = IndexByKey(SearchKey), i = Elements[s].THEADCELL::iStart; i != EmptyElement; i = p->THEADCELL::iNext)
			if((p = Elements + i)->CmpKey(SearchKey))
			{
				if((i = Elements[i].THEADCELL::iNext) != EmptyElement)
					return Elements + i;
				s++;
				for(TINDEX m = AllocCount; s < m; s++)
					for(i = Elements[s].THEADCELL::iStart; i != EmptyElement; i = Elements[i].THEADCELL::iNext)
					{
						return Elements + i;
					}
					return nullptr;
			}
			return nullptr;
	}

	/*========================================================*/
	/*For debug*/
	unsigned QualityInfo(char * Buffer, unsigned LenBuf)
	{
		unsigned CurLen = LenBuf, Len2, CurIndex = 0;
		Len2 = sprintf_s(
			Buffer, 
			CurLen, 
			"Count elements: %u\nAlloc count elements: %u\nSize(in bytes): %u\nHash quality:\n", 
			unsigned(Count), 
			unsigned(AllocCount), 
			AllocCount * sizeof(CELL));
		if(Len2 < 0)
			Len2 = 0;
		CurLen -= Len2;
		Buffer += Len2;
		for(LPCELL Cur = GetTable(), Max = Cur + AllocCount;Cur < Max;Cur++)
		{

			unsigned CountInCurIndex = 0;
			for(TINDEX p = Cur->THEADCELL::iStart; p != EmptyElement;p = GetTable()[p].THEADCELL::iNext)
				CountInCurIndex++;

			Len2 = sprintf_s(Buffer, CurLen, "%u:%u,", CurIndex, CountInCurIndex);
			if(Len2 < 0)
				Len2 = 0;
			CurLen -= Len2;
			Buffer += Len2;
			CurIndex++;
		}
		return LenBuf - CurLen;
	}
};

#endif
