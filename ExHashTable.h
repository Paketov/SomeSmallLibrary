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

	static inline void CopyElement(CELL& Dest, const CELL& Source) { Dest.TElementStruct = Source.TElementStruct; }

protected:

#define EXHASH_TABLE_FIELDS												\
	struct																\
	{																	\
	std::def_var_in_union_with_constructor<DYNAMIC_BUF_S<CELL>> Table;	\
	TINDEX count, last_empty;											\
	}

public:

	union
	{
		class{ EXHASH_TABLE_FIELDS; friend HASH_TABLE;
		inline TINDEX operator =(TINDEX NewVal) { return count = NewVal; } public:
			inline operator TINDEX() const { return count; }
		} Count;

		class{ EXHASH_TABLE_FIELDS; friend HASH_TABLE;
		inline TINDEX operator=(TINDEX NewCount) { return Table->Count = NewCount; } public:
			inline operator TINDEX() const { return Table->Count; }
		} AllocCount;

		class{ EXHASH_TABLE_FIELDS; friend HASH_TABLE; public:
			inline operator bool() const { return last_empty == NothingIndex; }
		} IsFull;

		/*Use this property only for debug!*/
		class{	EXHASH_TABLE_FIELDS; public:
			operator TINDEX() const
			{ 
				TINDEX c = 0;
				LPCELL t = Table->BeginBuf;
				for(TINDEX i = last_empty; i != EmptyElement; i = t[i].iNext)
					c++;
				return c;
			}
		}  EmptyCount;
	};

	static const TINDEX EmptyElement = NothingIndex;

	inline LPCELL GetTable() const { return Count.Table->BeginBuf; }

	template<typename TYPE_KEY>
	inline TINDEX IndexByKey(TYPE_KEY Key) const { return TElementStruct::IndexByKey(Key, AllocCount); }

	template<typename TYPE_KEY>
	inline LPCELL ElementByKey(TYPE_KEY Key) { return GetTable() + TElementStruct::IndexByKey(Key, AllocCount); }

protected:

	template<typename TKey>
	inline LPCELL AddElement(LPCELL HashCell, TKey InitKey)
	{	
		TINDEX iRetElem;
		LPCELL lpRetElem = GetTable() + (iRetElem = Count.last_empty);
		if(!lpRetElem->SetKey(InitKey))
			return nullptr;
		Count.count++;
		Count.last_empty = lpRetElem->iNext;
		lpRetElem->iNext = HashCell->iStart;
		HashCell->iStart = iRetElem;
		return lpRetElem;
	}

	inline LPCELL AddElement(LPCELL HashCell)
	{	
		TINDEX iRetElem;
		LPCELL lpRetElem = GetTable() + (iRetElem = Count.last_empty);
		Count.count++;
		Count.last_empty = lpRetElem->iNext;
		lpRetElem->iNext = HashCell->iStart;
		HashCell->iStart = iRetElem;
		return lpRetElem;
	}

public:

	/*
		After call this constructor AllocCount = NewAllocCount.
	*/
	HASH_TABLE(TINDEX NewAllocCount)
	{
		new(&Count.Table) DYNAMIC_BUF_S<CELL>(NewAllocCount);
		Count = 0;
		if(NewAllocCount > 0)
		{
			LPCELL t = GetTable();
			for(TINDEX i = Count.last_empty = 0; i < NewAllocCount; i++)
			{
				t[i].iStart = EmptyElement;
				t[i].iNext = i + 1;
			}
			t[NewAllocCount - 1].iNext = EmptyElement;
		}else
		{
			Count.last_empty = 0;
		}
	}

	/*
		Create empty table.
	*/
	inline HASH_TABLE()
	{
		new(&Count.Table) DYNAMIC_BUF_S<CELL>();
		Count = 0;
		Count.last_empty = EmptyElement;
	}

	inline ~HASH_TABLE()
	{
		Count.Table->~DYNAMIC_BUF_S<CELL>();
	}

	/*
		Insert with checkin have element
	*/
	template<typename T>
	inline TElementStruct* Insert(T SearchKey)
	{
		LPCELL lpStart, p, t = GetTable();
		for(TINDEX i = (lpStart = ElementByKey(SearchKey))->iStart; i != EmptyElement; i = p->iNext)
			if((p = t + i)->CmpKey(SearchKey))
				return p;
		
		TINDEX iRetElem;	
		p = t + (iRetElem = Count.last_empty);
		if(!p->SetKey(SearchKey))
			return nullptr;
		Count.last_empty = p->iNext;
		p->iNext = lpStart->iStart;
		lpStart->iStart = iRetElem;
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
		Count.last_empty = lpRetElem->iNext;
		lpRetElem->iNext = lpStart->iStart;
		lpStart->iStart = iRetElem;
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
		for(TINDEX i = t[TElementStruct::IndexByKey(SearchKey, AllocCount)].iStart; i != EmptyElement; i = p->iNext)
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
		for(TINDEX i = ((LPTHEADCELL)CurElem - 1)->iNext; i != EmptyElement; i = p->iNext)
			if((p = GetTable() + i)->CmpKey(SearchKey))
				return p;
		return nullptr;
	}

	/*
		Enumerate all elements in the table with the EnumFunc.
		Caution! When you call this function, you can not change the contents of the table!
	*/
	inline bool EnumValues(bool (*EnumFunc)(void* UserData, TElementStruct* Element), void* UserData = nullptr) const
	{
		for(LPCELL Elements = GetTable(), l, p = Elements, m = p + AllocCount; p < m; p++)
			for(auto i = p->iStart; i != EmptyElement;)
			{
				i = (l = Elements + i)->iNext;
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
			for(auto i = p->iStart; i != EmptyElement;)
			{
				i = (l = Elements + i)->iNext;
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
			for(auto i = &(p->iStart); *i != EmptyElement; )
			{	
				l = Elements + *i;
				if(IsDeleteProc(UserData, l))
				{
					auto e = *i;
					*i = l->iNext;
					l->iNext = Count.last_empty;
					Count.last_empty = e;
					Count.count--;
				}else
					i = &(l->iNext);
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
			for(auto i = &(p->iStart); *i != EmptyElement; )
			{	
				l = Elements + *i;
				if(IsDeleteProc(l))
				{
					auto e = *i;
					*i = l->iNext;
					l->iNext = Count.last_empty;
					Count.last_empty = e;
					Count.count--;
				}else
					i = &(l->iNext);
			}
	}

	/*
		Remove element by key.
		Return address element in table.
	*/
	template<typename T>
	TElementStruct* Remove(T SearchKey)
	{
		LPCELL p, t = GetTable();
		for(LPTINDEX i = &(t[TElementStruct::IndexByKey(Key, AllocCount)].iStart); *i != EmptyElement; i = &(p->iNext))
		{
			p = t + *i;
			if(p->CmpKey(SearchKey))
			{
				TINDEX j = *i;
				*i = p->iNext;
				p->iNext = Count.last_empty;
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
		for(LPTINDEX i = &(t[TElementStruct::IndexByKey(Key, AllocCount)].iStart); *i != EmptyElement; i = &(p->iNext))
		{
			p = t + *i;
			if(p->CmpKey(SearchKey))
			{
				TINDEX j = *i;
				*i = p->iNext;
				p->iNext = Count.last_empty;
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
		Count = AllocCount = 0;
		Dest.Count.last_empty = EmptyElement;
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
			for(auto i = p->iStart; i != EmptyElement; i = l->iNext)
			{
				l = Elements + i;
				if(i >= NewSize) 
				{
					for(;last_empty >= NewSize; last_empty = Elements[last_empty].iNext);
					CopyElement(Elements[last_empty], Elements[i]);
					last_empty = Elements[last_empty].iNext;
				}
			}
		for(TINDEX i = 0; i < NewSize;i++)
			Elements[i].iStart = EmptyElement;
		for(TINDEX i = 0; i < NewSize; i++)
		{
			auto j = Elements[i].IndexInBound(NewSize);
			Elements[i].iNext = Elements[j].iStart;
			Elements[j].iStart = i;
		}

		Count.last_empty = EmptyElement;
		AllocCount = NewSize;
		if(AllocCount != NewSize)
			return false;
		return true;
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
		AllocCount = NewCount;
		if(AllocCount != NewCount)
			return false;

		LPCELL Elements = GetTable();
		if(FullCount == Count)
		{	
			/*Init basic part*/
			for(LPCELL p = Elements, m = p + FullCount; p < m; p++)
				p->iStart = EmptyElement;
			
			/*Initialize the remaining part*/
			for(TINDEX p = FullCount, m = NewCount; p < m; p++)
			{
				Elements[p].iStart = EmptyElement;
				Elements[p].iNext = p + 1;
			}
			Elements[NewCount - 1].iNext = Count.last_empty;
			Count.last_empty = FullCount;
			/*Distribute all elements*/
			for(TINDEX i = 0; i < FullCount; i++)
			{
				auto j = Elements[i].IndexInBound(NewCount);
				Elements[i].iNext = Elements[j].iStart;
				Elements[j].iStart = i;
			}
		}else
		{
			TINDEX UsedList = EmptyElement;
			/*Create list used elements*/
			for(TINDEX p = 0; p < FullCount; p++)
			{			
				for(TINDEX i = Elements[p].iStart, t; i != EmptyElement; i = t)
				{
					t = Elements[i].iNext;
					Elements[i].iNext = UsedList;
					UsedList = i;
				}
				Elements[p].iStart = EmptyElement;
			}
			/*Initialize the remaining part*/
			for(TINDEX p = FullCount, m = NewCount; p < m; p++)
			{
				Elements[p].iStart = EmptyElement;
				Elements[p].iNext = p + 1;
			}
			Elements[NewCount - 1].iNext = Count.last_empty;
			Count.last_empty = FullCount;
			/*Distribute a list of used to new places*/
			for(TINDEX t; UsedList != EmptyElement; UsedList = t)
			{
				t = Elements[UsedList].iNext;
				auto i = Elements[UsedList].IndexInBound(NewCount);
				Elements[UsedList].iNext = Elements[i].iStart;
				Elements[i].iStart = UsedList;
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
		Dest.AllocCount = AllocCount;
		if(Dest.AllocCount != AllocCount)
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
	void Move(HASH_TABLE<TElementStruct, TIndex, NothingIndex>& Dest)
	{
		Count.Table->Move(Dest.Count.Table);
		Dest.Count.count = Count.count;
		Count.count = 0;
		Dest.Count.last_empty = Count.last_empty;
		Count.last_empty = EmptyElement;
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
				if(Elements[p].iStart != EmptyElement)
				{
					SetInterator.IsEnd.CurStartList = p;
					SetInterator.IsEnd.CurElementInList = Elements[p].iStart;
					return true;
				}
			SetInterator.IsEnd.CurStartList = EmptyElement;
			return false;
		}
		TINDEX i = Elements[SetInterator.IsEnd.CurElementInList].iNext;
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
	inline TElementStruct* CellByInterator(const TINTER& SetInterator) const { return GetTable() + SetInterator.IsEnd.CurElementInList; }
	
	/*
		Search key and set interator to key position.
	*/
	template<typename TKey>
	bool InteratorByKey(TKey SearchKey, TINTER& Interator)
	{
		LPCELL p, t = GetTable();
		for(TINDEX s = TElementStruct::IndexByKey(SearchKey, AllocCount), i = t[s].iStart; i != EmptyElement; i = p->iNext)
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
	TElementStruct* Remove(TINTER& Interator)
	{
		LPCELL Elements = GetTable();
		LPCELL Ret = Elements + Interator.IsEnd.CurElementInList;
		for(LPTINDEX i = &Elements[Interator.IsEnd.CurStartList].iStart; *i != EmptyElement; i = &Elements[*i].iNext)
		{
			if(Interator.IsEnd.CurElementInList == *i)
			{
				Interator.IsEnd.CurElementInList = *i = Elements[i].iNext;
				if(*i == EmptyElement)
				{
					for(TINDEX j = Interator.IsEnd.CurStartList + 1, m = AllocCount; j < m; j++)
					{
						if(Elements[j].iStart != EmptyElement)
						{
							Interator.IsEnd.CurElementInList = Elements[Interator.IsEnd.CurStartList = j].iStart;
							return Ret;
						}
					}
					Interator.IsEnd.CurStartList = EmptyElement;
				}
				break;
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
			if(Elements[p].iStart != EmptyElement)
			{
				return Elements + Elements[p].iStart;
			}
		return nullptr;
	}

	/*Next interate by key val*/
	template<typename TKey>
	TElementStruct* GetNextCellByKey(TKey SearchKey) const
	{
		LPCELL Elements = GetTable(), p;
		for(TINDEX s = IndexByKey(SearchKey), i = Elements[s].iStart; i != EmptyElement; i = p->iNext)
			if((p = Elements + i)->CmpKey(SearchKey))
			{
				if((i = Elements[i].iNext) != EmptyElement)
					return Elements + i;
				s++;
				for(TINDEX m = AllocCount; s < m; s++)
					for(i = Elements[s].iStart; i != EmptyElement; i = Elements[i].iNext)
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
			for(TINDEX p = Cur->iStart; p != EmptyElement;p = GetTable()[p].iNext)
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
