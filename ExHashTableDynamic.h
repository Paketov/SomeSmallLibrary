#ifndef __EXHASH_TABLE_DYNAMIC_H_HAS_INCLUDED__
#define __EXHASH_TABLE_DYNAMIC_H_HAS_INCLUDED__

#include <stdio.h>
#include "ExTypeTraits.h"
#include "ExDynamicBuf.h"


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
	

	static inline void CopyElement(CELL& Dest, const CELL& Source) { Dest.TElementStruct = Source.TElementStruct; }

protected:

#define EXHASH_TABLE_FIELDS												\
	struct																\
	{																	\
	std::def_var_in_union_with_constructor<DYNAMIC_BUF_S<LPCELL>> Table;\
	TINDEX count;											\
	}

public:

	union
	{
		class{ EXHASH_TABLE_FIELDS; friend HASH_TABLE_DYN;
		inline TINDEX operator =(TINDEX NewVal) { return count = NewVal; } public:
			inline operator TINDEX() const { return count; }
		} Count;

		class{ EXHASH_TABLE_FIELDS; friend HASH_TABLE_DYN;
		inline TINDEX operator=(TINDEX NewCount) { return Table->Count = NewCount; } public:
			inline operator TINDEX() const { return Table->Count; }
		} AllocCount;

		class{ EXHASH_TABLE_FIELDS; friend HASH_TABLE_DYN; public:
			inline operator bool() const { return Table->Count >= count; }
		} IsFull;

		class{	EXHASH_TABLE_FIELDS; public:
			operator TINDEX() const { return Table->Count - count; }
		}  EmptyCount;
	};

	static const TINDEX EmptyElement = NothingIndex;


	inline LPCELL* GetTable() const { return Count.Table->BeginBuf; }

	template<typename TYPE_KEY>
	inline TINDEX IndexByKey(TYPE_KEY Key) const { return TElementStruct::IndexByKey(Key, AllocCount); }

	template<typename TYPE_KEY>
	inline LPCELL* ElementByKey(TYPE_KEY Key) { return GetTable() + TElementStruct::IndexByKey(Key, AllocCount); }

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
	HASH_TABLE_DYN(TINDEX NewAllocCount)
	{
		new(&Count.Table) DYNAMIC_BUF_S<LPCELL>(NewAllocCount);
		Count = 0;
		if(NewAllocCount > 0)
			memset(GetTable(), 0, sizeof(LPCELL) * NewAllocCount);
	}

	/*
		Create empty table.
	*/
	inline HASH_TABLE_DYN()
	{
		new(&Count.Table) DYNAMIC_BUF_S<LPCELL>();
		Count = 0;
	}

	inline ~HASH_TABLE_DYN()
	{
		Count.Table->~DYNAMIC_BUF_S<LPCELL>();
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
		for(LPCELL lpStart = GetTable()[TElementStruct::IndexByKey(Key, AllocCount)]; lpStart != nullptr; lpStart = lpStart->Next)
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
					*i = (*i)->Next;
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

	/*
		Remove element by key.
		Return address element in table.
	*/
	template<typename T>
	TElementStruct* Remove(T SearchKey)
	{
		for(LPCELL *lpStart = ElementByKey(SearchKey); *lpStart != nullptr; lpStart = &(*lpStart)->Next)
		{
			if((*lpStart)->CmpKey(SearchKey))
			{
				LPCELL DelElem = *lpStart;
				*lpStart = DelElem->Next;
				FAST_ALLOC::Delete(DelElem);
				Count.count--;
				return std::make_default_pointer();
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
		Count = AllocCount = 0;
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
		{
			return ResizeBeforeInsert(NewCount);
		}
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
		AllocCount = NewCount;
		LPCELL *t = GetTable();
		memset(t, 0, sizeof(LPCELL) * AllocCount);
		while(UsedList != nullptr)
		{
			LPCELL* j = t + UsedList->IndexInBound(AllocCount);
			LPCELL c = *j;
			*j = UsedList;
			UsedList = UsedList->Next;
			(*j)->Next = c;
		}
		return AllocCount == NewCount;
	}


	/*========================================================*/
	/*
		Clone this table to another
	*/
	bool Clone(HASH_TABLE_DYN<TElementStruct, TIndex, NothingIndex>& Dest) const
	{
		LPCELL UsedList = nullptr;
		for(LPCELL *s = Dest.GetTable(), *m = s + Dest.AllocCount; s < m; s++)
			for(LPCELL i = *s; i != nullptr; )
			{
				LPCELL j = UsedList;
				UsedList = i;
				i = i->Next;
				UsedList->Next = j;
			}

		Dest.AllocCount = AllocCount;
		LPCELL* dt = Dest.GetTable();
		if(Dest.AllocCount != AllocCount)
		{
			/*If not alloc memory, then return all back*/
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
		memset(t, 0, sizeof(LPCELL) * Dest.AllocCount);
		LPCELL* st = GetTable();
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
						if(k < m) break;
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
		Dest.Count = Count;
		return true;
	}
	/*
		Move this table to another.
		After call Count == 0.
	*/
	void Move(HASH_TABLE_DYN<TElementStruct, TIndex, NothingIndex>& Dest)
	{
		Count.Table->Move(Dest.Count.Table);
		Dest.Count.count = Count.count;
		Count.count = 0;
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
	static inline TElementStruct* CellByInterator(const TINTER& SetInterator) { return SetInterator.IsEnd.CurElementInList; }
	
	/*
		Search key and set interator to key position.
	*/
	template<typename TKey>
	bool InteratorByKey(TKey SearchKey, TINTER& Interator)
	{
		LPCELL *t = GetTable();
		TINDEX s = TElementStruct::IndexByKey(SearchKey, AllocCount);
		for(LPCELL i = t[s]; i != EmptyElement; i = i->Next)
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
	//TElementStruct* Remove(TINTER& Interator)
	//{
	//	LPCELL Elements = GetTable();
	//	LPCELL Ret = Elements + Interator.IsEnd.CurElementInList;
	//	for(LPTINDEX i = &Elements[Interator.IsEnd.CurStartList].iStart; *i != EmptyElement; i = &Elements[*i].iNext)
	//	{
	//		if(Interator.IsEnd.CurElementInList == *i)
	//		{
	//			Interator.IsEnd.CurElementInList = *i = Elements[i].iNext;
	//			if(*i == EmptyElement)
	//			{
	//				for(TINDEX j = Interator.IsEnd.CurStartList + 1, m = AllocCount; j < m; j++)
	//				{
	//					if(Elements[j].iStart != EmptyElement)
	//					{
	//						Interator.IsEnd.CurElementInList = Elements[Interator.IsEnd.CurStartList = j].iStart;
	//						return Ret;
	//					}
	//				}
	//				Interator.IsEnd.CurStartList = EmptyElement;
	//			}
	//			break;
	//		}
	//	}
	//	return Ret;
	//}

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

	/*Next interate by key val*/
	template<typename TKey>
	TElementStruct* GetNextCellByKey(TKey SearchKey) const
	{
		LPCELL *t = GetTable(), *i = ElementByKey(Key);
		for(LPCELL e = *i; e != nullptr; e = e->Next)
		{
			if(e->CmpKey(SearchKey))
			{
				if(e->Next != nullptr)
					return e->Next;
				i++;
				for(LPCELL *m = t + AllocCount; i < m; i++)
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