#ifndef __EXHASH_TABLE_DYNAMIC_H_HAS_INCLUDED__
#define __EXHASH_TABLE_DYNAMIC_H_HAS_INCLUDED__

#include <stdio.h>
#include "ExTypeTraits.h"
#include "ExDynamicBuf.h"



/*
ExHashTable
Paketov
2015-2016

Low-level hash table.
This another version of table. 
This table is characterized in that it has a dynamic memory allocation for each element.
This table fully compatible with  HASH_TABLE.
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

	HASH_TABLE_DYN<HASH_ELEMENT> HashArray(12);

	HashArray.Insert(0.000012)->Val = 0.000012;
	printf("%lf", HashArray.Search(0.000012)->Val); 

*/

template
<
	typename TElementStruct,
	typename TIndex = decltype(std::declval<TElementStruct>().IndexInBound(0)),
	TIndex NothingIndex = TIndex(-1)
>
class HASH_TABLE_DYN
{	
public:	
	typedef TIndex	TINDEX, *LPTINDEX;
	typedef TElementStruct	TELEMENT, *LPTELEMENT;
	struct CELL;
	typedef CELL *LPCELL;
	typedef struct { LPCELL Next; } THEADCELL, *LPTHEADCELL;		
	struct CELL : public THEADCELL, public TElementStruct {};
	

	static inline void CopyElement(CELL& Dest, const CELL& Source) { *(TElementStruct*)((LPTHEADCELL)&Dest + 1) = *(TElementStruct*)((LPTHEADCELL)&Source + 1); }

protected:

#define EXHASH_TABLE_FIELDS												\
	struct																\
	{																	\
	LPCELL* Table;														\
	TINDEX count, alloc_count;											\
	}

public:

	union
	{
		class{ EXHASH_TABLE_FIELDS; friend HASH_TABLE_DYN;
		inline TINDEX operator =(TINDEX NewVal) { return count = NewVal; } public:
		inline operator TINDEX() const { return count; }
		} Count;

		class{ EXHASH_TABLE_FIELDS; friend HASH_TABLE_DYN;
		public:
			inline operator TINDEX() const { return alloc_count; }
		} AllocCount;

		class{ EXHASH_TABLE_FIELDS; friend HASH_TABLE_DYN; public:
		inline operator bool() const { return count >= alloc_count; }
		} IsFull;

		class{	EXHASH_TABLE_FIELDS; public:
		operator TINDEX() const { return alloc_count - count; }
		}  EmptyCount;
	};

	static const TINDEX EmptyElement = NothingIndex;


	inline LPCELL* GetTable() const { return Count.Table; }

	template<typename TYPE_KEY>
	inline TINDEX IndexByKey(TYPE_KEY Key) const { return TElementStruct::IndexByKey(Key, AllocCount); }

	template<typename TYPE_KEY>
	inline LPCELL* ElementByKey(TYPE_KEY Key) const { return GetTable() + TElementStruct::IndexByKey(Key, AllocCount); }


	/*
		Temporary pointer on delete element.
	*/
	class REMOVE_POINTER
	{
		LPCELL Val;
		friend HASH_TABLE_DYN;
		inline REMOVE_POINTER(LPCELL NewVal) { Val = NewVal; }
	public:
		inline REMOVE_POINTER(REMOVE_POINTER& NewVal) { Val = NewVal.Val; NewVal.Val = nullptr; }
		REMOVE_POINTER& operator =(REMOVE_POINTER& NewVal) 
		{ 
			if(Val != nullptr) 
				FAST_ALLOC::Delete(Val);
			Val = NewVal.Val; 
			NewVal.Val = nullptr;
			return *this;
		}
		inline operator TElementStruct*() const { return Val; }
		inline TElementStruct* operator->() const { return Val; }
		inline ~REMOVE_POINTER() { if(Val != nullptr) FAST_ALLOC::Delete(Val); }
	};

protected:

	bool ReallocAndClear(TINDEX NewAllocCount)
	{
		if(NewAllocCount <= 0)
			NewAllocCount = 1;
		LPCELL* Res = (LPCELL*)___realloc(Count.Table, sizeof(LPCELL) * NewAllocCount);
		if(Res == nullptr)
			return false;
		memset(Count.Table = Res, 0, sizeof(LPCELL) * (Count.alloc_count = NewAllocCount));
		return true;
	}

	bool Realloc(TINDEX NewAllocCount)
	{
		TINDEX c = NewAllocCount;
		if(NewAllocCount <= 0)
			c = 1;
		LPCELL *Res = (LPCELL*)___realloc(Count.Table, sizeof(LPCELL) * c);
		if(Res == nullptr)
			return false;
		if(NewAllocCount <= 0)
			*Res = nullptr;
		Count.Table = Res; 
		Count.alloc_count = c;
		return true;
	}
public:

	/*
		After call this constructor AllocCount = NewAllocCount.
	*/
	HASH_TABLE_DYN(TINDEX NewAllocCount = 1)
	{
		Count.Table = nullptr;
		Count.alloc_count = Count.count = 0;
		ReallocAndClear(NewAllocCount);
	}

	inline ~HASH_TABLE_DYN()
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
		LPCELL *lpStart = ElementByKey(SearchKey);
		for(; *lpStart != nullptr; lpStart = &(*lpStart)->Next)
		{
			if((*lpStart)->CmpKey(SearchKey))
				return *lpStart;
		}
		if((*lpStart = FAST_ALLOC::New<CELL>()) == nullptr)
			return nullptr;
		(*lpStart)->Next = nullptr;
		(*lpStart)->SetKey(SearchKey);
		Count.count++;
		return *lpStart;
	}

	/*
		Insert element without check availability element.
		In case collision for read multiple values use NextCollision function.
	*/
	template<typename T>
	inline TElementStruct* OnlyInsert(T SearchKey)
	{			
		LPCELL *lpStart = ElementByKey(SearchKey), NewElem;
		if((NewElem = FAST_ALLOC::New<CELL>()) == nullptr)
			return nullptr;
		NewElem->Next = *lpStart;
		(*lpStart = NewElem)->SetKey(SearchKey);
		Count.count++;
		return NewElem;
	}

	/*
		Simple search element by various type key
	*/
	template<typename T>
	inline TElementStruct* Search(T SearchKey) const
	{
		for(LPCELL lpStart = GetTable()[TElementStruct::IndexByKey(SearchKey, AllocCount)]; lpStart != nullptr; lpStart = lpStart->Next)
		{
			if(lpStart->CmpKey(SearchKey))
				return lpStart;
		}
		return nullptr;
	}

	/*
		Search next collision in table.
		Caution! You can not change the table between Search and NextCollision.
	*/
	template<typename T>
	inline TElementStruct* NextCollision(TElementStruct* CurElem, T SearchKey) const
	{
		for(LPCELL lpNext = ((LPTHEADCELL)CurElem - 1)->Next; lpNext != nullptr; lpNext = lpNext->Next)
		{
			if(lpNext->CmpKey(SearchKey))
				return lpNext;
		}
		return nullptr;
	}

	/*
		Remove element by key.
		Return address element in table.
	*/
	template<typename T>
	REMOVE_POINTER Remove(T SearchKey)
	{
		for(LPCELL *lpStart = ElementByKey(SearchKey); *lpStart != nullptr; lpStart = &(*lpStart)->Next)
		{
			if((*lpStart)->CmpKey(SearchKey))
			{
				REMOVE_POINTER DelElem = *lpStart;
				*lpStart = DelElem.Val->Next;
				Count.count--;
				return DelElem;
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
		for(LPCELL *i = ElementByKey(SearchKey); *i != nullptr; )
		{
			if((*i)->CmpKey(SearchKey))
			{
				LPCELL DelElem = *i;
				*i = DelElem->Next;
				FAST_ALLOC::Delete(DelElem);
				Count.count--;
			}else
				i = &((*i)->Next);
		}
	}

	/*
		Clear all table.
		Deletes all element from table.
	*/
	void Clear()
	{		
		for(LPCELL *s = GetTable(), *m = s + AllocCount; s < m; s++)
			for(LPCELL i = *s; i != nullptr; )
			{
				LPCELL DelElem = i;
				i = DelElem->Next;
				FAST_ALLOC::Delete(DelElem);
			}
		ReallocAndClear(1);
		Count = 0;
	}

	/*
		Enumerate all elements in the table with the EnumFunc.
		Caution! When you call this function, you can not change the contents of the table!
	*/
	inline bool EnumValues(bool (*EnumFunc)(void* UserData, TElementStruct* Element), void* UserData = nullptr) const
	{
		for(LPCELL *s = GetTable(), *m = s + AllocCount; s < m; s++)
			for(LPCELL i = *s; i != nullptr; i = i->Next)
			{
				if(!EnumFunc(UserData, i))
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
		for(LPCELL *s = GetTable(), *m = s + AllocCount; s < m; s++)
			for(LPCELL i = *s; i != nullptr; i = i->Next)
			{
				if(!EnumFunc(i))
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
		for(LPCELL *s = GetTable(), *m = s + AllocCount; s < m; s++)
			for(LPCELL *i = s; *i != nullptr; )
			{
				if(IsDeleteProc(UserData, *i))
				{
					LPCELL DelElem = *i;
					*i = (*i)->Next;
					FAST_ALLOC::Delete(DelElem);
					Count.count--;
				}else
					i = &((*i)->Next);
			}
	}
		
	/*
		Enumerate all elements in the table with the EnumFunc.
		Is function return true element will be removed.
		Caution! When you call this function, you can not change the contents of the table!
	*/
	inline void EnumDelete(bool (*IsDeleteProc)(TElementStruct* Element))
	{
		for(LPCELL *s = GetTable(), *m = s + AllocCount; s < m; s++)
			for(LPCELL* i = s; *i != nullptr; )
			{
				if(IsDeleteProc(*i))
				{
					LPCELL DelElem = *i;
					*i = DelElem->Next;
					FAST_ALLOC::Delete(DelElem);
					Count.count--;
				}else
					i = &((*i)->Next);
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
	inline bool ResizeAfterRemove() { return ResizeAfterRemove(Count); }

	bool ResizeAfterRemove(TINDEX NewCount)
	{
		if(NewCount <= 0)
		{
			Clear();
			return true;
		}else
			return ResizeBeforeInsert(NewCount);
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
		LPCELL UsedList = nullptr;
		for(LPCELL *s = GetTable(), *m = s + AllocCount; s < m; s++)
			for(LPCELL i = *s; i != nullptr; )
			{
				LPCELL j = UsedList;
				UsedList = i;
				i = i->Next;
				UsedList->Next = j;
			}
		bool r = ReallocAndClear(NewCount);
		LPCELL *t = GetTable();
		while(UsedList != nullptr)
		{
			LPCELL* j = t + UsedList->IndexInBound(AllocCount), c = *j;
			UsedList = (*j = UsedList)->Next;
			(*j)->Next = c;
		}
		return r;
	}


	/*========================================================*/
	/*
		Clone this table to another
	*/
	bool Clone(HASH_TABLE_DYN<TElementStruct, TIndex, NothingIndex>& Dest) const
	{
		LPCELL UsedList = nullptr;
		if(Dest.Count >= 0)
		{
			for(LPCELL *s = Dest.GetTable(), *m = s + Dest.AllocCount; s < m; s++)
				for(LPCELL i = *s; i != nullptr; )
				{
					LPCELL j = UsedList;
					UsedList = i;
					i = i->Next;
					UsedList->Next = j;
				}
		}		
		if(!Dest.ReallocAndClear(AllocCount))
		{
			/*If not alloc memory, then return all back*/
			LPCELL* dt = Dest.GetTable();
			for(; UsedList != nullptr;)
			{
				LPCELL* j = dt + UsedList->IndexInBound(Dest.AllocCount);
				LPCELL c = *j;
				*j = UsedList;
				UsedList = UsedList->Next;
				(*j)->Next = c;
			}
			return false;
		}	
		LPCELL *st = GetTable(), *dt = Dest.GetTable();
		for(TINDEX k = 0, m = AllocCount; k < m; k++)
			for(LPCELL i = st[k]; i != nullptr; i = i->Next)
			{
				if(UsedList == nullptr)
				{
					/*If used list == null, then for faster jump to cilcle without checkin*/
					while(true)
					{
						for(; i != nullptr; i = i->Next)
						{
							LPCELL n = FAST_ALLOC::New<CELL>();
							CopyElement(*n, *i);
							n->Next = dt[k];
							dt[k] = n;
						}
						k++;
						if(k >= m) break;
						i = st[k];
					};
					goto lblOut;
				}
				LPCELL n = UsedList;
				UsedList = UsedList->Next;
				CopyElement(*n, *i);
				n->Next = dt[k];
				dt[k] = n;
			}
		while(UsedList != nullptr)
		{
			LPCELL DelElem = UsedList;
			UsedList = UsedList->Next;
			FAST_ALLOC::Delete(DelElem);
		}
lblOut:
		Dest.Count.count = Count.count;
		return true;
	}
	/*
		Move this table to another.
		After call Count == 0.
	*/
	bool Move(HASH_TABLE_DYN<TElementStruct, TIndex, NothingIndex>& Dest)
	{
		if(!Dest.ReallocAndClear(1))
			return false;
		LPCELL* t = Dest.Count.Table;
		Dest.Count.Table = Count.Table;
		Count.Table = t;
		Dest.Count.alloc_count = Count.alloc_count;
		Count.alloc_count = 1; 
		Dest.Count.count = Count.count;
		Count.count = 0;
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
		friend HASH_TABLE_DYN;
	public:
		union
		{
			class
			{
				friend TINTER;
				friend HASH_TABLE_DYN;
				TINDEX CurStartList;
				LPCELL CurElementInList;
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
		LPCELL *t = GetTable();
		if(SetInterator.IsEnd)
		{
			p = 0;
lblSearchStart:
			for(TINDEX m = AllocCount; p < m; p++)
				if(t[p] != nullptr)
				{
					SetInterator.IsEnd.CurStartList = p;
					SetInterator.IsEnd.CurElementInList = t[p];
					return true;
				}
			SetInterator.IsEnd.CurStartList = EmptyElement;
			return false;
		}
		if(SetInterator.IsEnd.CurElementInList->Next != nullptr)
		{
			SetInterator.IsEnd.CurElementInList = SetInterator.IsEnd.CurElementInList->Next;
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
		return true;
	}
	/*
		Get element by interator.
	*/
	static inline TElementStruct* ElementByInterator(const TINTER& SetInterator) { return SetInterator.IsEnd.CurElementInList; }
	
	/*
		Search key and set interator to key position.
	*/
	template<typename TKey>
	bool InteratorByKey(TKey SearchKey, TINTER& Interator)
	{
		LPCELL *t = GetTable();
		TINDEX s = TElementStruct::IndexByKey(SearchKey, AllocCount);
		for(LPCELL i = t[s]; i != nullptr; i = i->Next)
			if(i->CmpKey(SearchKey))
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
		for(LPCELL *DelElem = GetTable() + Interator.IsEnd.CurStartList; *DelElem != nullptr; DelElem = &(*DelElem)->Next)
		{
			if(*DelElem == Interator.IsEnd.CurElementInList)
			{
				Interate(Interator);
				REMOVE_POINTER El2 = *DelElem;
				*DelElem = El2.Val->Next;
				Count.count--;
				return El2;
			}
		}
		return nullptr;
	}

	/*========================================================*/
	/*Interate by key val*/
	inline TElementStruct* GetStartCell() const
	{
		for(LPCELL *p = GetTable(), *m = p + AllocCount; p < m; p++)
			if(*p != nullptr)
			{
				return *p;
			}
		return nullptr;
	}

	/*
		Next interate by key val
		Use only for not collision values! To do this, use only Insert function.
	*/
	template<typename TKey>
	TElementStruct* GetNextCellByKey(TKey SearchKey) const
	{
		const LPCELL *t = GetTable(), *i = ElementByKey(SearchKey);
		for(LPCELL e = *i; e != nullptr; e = e->Next)
		{
			if(e->CmpKey(SearchKey))
			{
				if(e->Next != nullptr)
					return e->Next;
				i++;
				for(const LPCELL *m = t + AllocCount; i < m; i++)
					if(*i != nullptr) 
						return *i;
				return nullptr;
			}
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
			AllocCount * sizeof(CELL) + unsigned(AllocCount) * (LPCELL));
		if(Len2 < 0)
			Len2 = 0;
		CurLen -= Len2;
		Buffer += Len2;
		for(LPCELL* c = GetTable(), *m = c + AllocCount; c < m; c++)
		{
			unsigned CountInCurIndex = 0;
			for(LPCELL p = *c; p != nullptr; p = p->Next)
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