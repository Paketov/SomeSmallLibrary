#ifndef __EXHASH_TABLE_H_HAS_INCLUDED__
#define __EXHASH_TABLE_H_HAS_INCLUDED__

#include <typeinfo>
#include <stdio.h>
#include <type_traits>
#include "ExTypeTraits.h"

/*
ExHashTable
Paketov
2015

Low level hash table.
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

HASH_TABLE<HASH_ELEMENT> HashArray;
HASH_TABLE<HASH_ELEMENT>::New(HashArray, 12);
HashArray.Init(12);

HashArray.Insert(0.000012)->Val = 0.000012;
printf("%lf", HashArray.Search(0.000012)->Val); 

*/


template
	<
	typename TElementStruct,
	bool Type = false,
	typename TIndex = decltype(TElementStruct::IndexByKey(std::variant_arg(), std::variant_arg())),
	TIndex NothingIndex = TIndex(-1)
	>
class HASH_TABLE
{	
public:
	typedef TIndex											TINDEX,		      *LPTINDEX;
	typedef TElementStruct									TPROPERTY_STRUCT, *LPTPROPERTY_STRUCT;
	typedef struct { TINDEX iStart, iNext; } THEADCELL, *LPTHEADCELL;		
	typedef struct CELL : public THEADCELL, public TElementStruct {} CELL, *LPCELL;
	static inline void CopyElement(CELL& Dest, CELL& Source) { *((TElementStruct*)(((LPTHEADCELL)&Dest) + 1)) = *((TElementStruct*)(((LPTHEADCELL)&Source) + 1)); }

protected:

#define EXHASH_TABLE_FIELDS						\
	struct										\
	{											\
	TINDEX		CountUsed, MaxCount, LastEmpty;	\
	LPCELL	   Table;							\
	}

public:

	union
	{
		class{
			friend HASH_TABLE;
			EXHASH_TABLE_FIELDS;
			inline TINDEX operator =(TINDEX NewVal) { return CountUsed = NewVal; }
		public:
			inline operator TINDEX() const { return CountUsed; }
		} CountUsed;

		class{
			friend HASH_TABLE;
			EXHASH_TABLE_FIELDS;
			inline TINDEX operator =(TINDEX NewVal) { return MaxCount = NewVal; }
		public:
			inline operator TINDEX() const { return MaxCount; }
		} MaxCount;

		class ___MAX_SIZE{
			EXHASH_TABLE_FIELDS;
		public:
			inline operator size_t() const
			{
				if(Type)
					return MaxCount * sizeof(CELL) + sizeof(HASH_TABLE) - sizeof(LPCELL);
				return MaxCount * sizeof(CELL);
			}
			inline static size_t ByCount(TINDEX CountElement)
			{
				if(Type)
					return CountElement * sizeof(CELL) + sizeof(HASH_TABLE) - sizeof(LPCELL);
				return CountElement * sizeof(CELL);
			}
		} MaxSize;

		class{
			friend HASH_TABLE;
			EXHASH_TABLE_FIELDS;
		public:
			inline operator bool() const { return LastEmpty == NothingIndex; }
		} IsFull;
	};

	typedef struct
	{
		THEADCELL				h; 
		TPROPERTY_STRUCT		v;
	} TSTATIC_VAL, *LPTSTATIC_VAL;

	static const TINDEX EmptyElement = NothingIndex;

	inline static bool IsHaveChain(LPCELL pCell) { return pCell->iStart != NothingIndex; }

	inline static bool IsHaveNextChain(LPCELL pCell) { return pCell->iNext != NothingIndex; }

	template<typename TYPE_KEY>
	inline TINDEX IndexByKey(TYPE_KEY Key) const { return TElementStruct::IndexByKey(Key, MaxCount); }

	template<typename TYPE_KEY>
	inline TINDEX StartIndexByKey(TYPE_KEY Key) const { return (GetTable() + TElementStruct::IndexByKey(Key, MaxCount))->iStart; }

	template<typename TYPE_KEY>
	inline LPCELL ElementByKey(TYPE_KEY Key) { return GetTable() + TElementStruct::IndexByKey(Key, MaxCount); }

	inline LPCELL operator[](TINDEX Index) { return GetTable() + Index; }

	template<typename TKey>
	inline LPCELL AddElement(LPCELL HashCell, TKey InitKey)
	{	
		TINDEX iRetElem;
		LPCELL lpRetElem = GetTable() + (iRetElem = IsFull.LastEmpty);
		if(!lpRetElem->SetKey(InitKey))
			return nullptr;
		CountUsed.CountUsed++;
		IsFull.LastEmpty = lpRetElem->iNext;
		lpRetElem->iNext = HashCell->iStart;
		HashCell->iStart = iRetElem;
		return lpRetElem;
	}

	inline LPCELL AddElement(LPCELL HashCell)
	{	
		TINDEX iRetElem;
		LPCELL lpRetElem = GetTable() + (iRetElem = IsFull.LastEmpty);
		CountUsed.CountUsed++;
		IsFull.LastEmpty = lpRetElem->iNext;
		lpRetElem->iNext = HashCell->iStart;
		HashCell->iStart = iRetElem;
		return lpRetElem;
	}

	inline void Init(TINDEX Count)
	{		
		MaxCount = Count;
		CountUsed = 0;
		Count--;
		TINDEX i;
		for(IsFull.LastEmpty = i = 0; i < Count; i++)
		{
			LPCELL p;
			(p = GetTable() + i)->iStart = NothingIndex;
			p->iNext = i + 1;
		}
		GetTable()[i].iNext = GetTable()[i].iStart = NothingIndex;
	}

	template<typename T>
	inline LPCELL InitInsert(T SearchKey)
	{		
		if(IsFull)
			return nullptr;
		LPCELL HashCell = ElementByKey(SearchKey);	
		TINDEX iRetElem;
		LPCELL lpRetElem = GetTable() + (iRetElem = IsFull.LastEmpty);
		if(!lpRetElem->SetKey(SearchKey))
			return nullptr;
		CountUsed++;
		IsFull.LastEmpty = lpRetElem->iNext;
		lpRetElem->iNext = HashCell->iStart;
		HashCell->iStart = iRetElem;
		return lpRetElem;
	}

	template<typename T>
	inline LPCELL InitInsertEx(T SearchKey)
	{
		LPCELL p, HashCell = ElementByKey(SearchKey);
		for(TINDEX i = HashCell->iStart; i != NothingIndex; i = p->iNext)
			if((p = GetTable() + i)->CmpKey(SearchKey))
				return &HaveVal;
		if(IsFull)
			return nullptr;

		TINDEX iRetElem;
		LPCELL lpRetElem = GetTable() + (iRetElem = IsFull.LastEmpty);
		if(!lpRetElem->SetKey(SearchKey))
			return nullptr;
		CountUsed++;
		IsFull.LastEmpty = lpRetElem->iNext;
		lpRetElem->iNext = HashCell->iStart;
		HashCell->iStart = iRetElem;
		return lpRetElem;
	}

	template<typename T>
	inline LPCELL Insert(T SearchKey)
	{
		LPCELL lpStart, p;
		for(TINDEX i = (lpStart = ElementByKey(SearchKey))->iStart; i != NothingIndex; i = p->iNext)
			if((p = GetTable() + i)->CmpKey(SearchKey))
				return p;
		return AddElement(lpStart, SearchKey);
	}

	template<typename T>
	inline LPCELL OnlyInsert(T SearchKey)
	{
		LPCELL lpStart = ElementByKey(SearchKey);
		return AddElement(lpStart, SearchKey);
	}

	static bool ResizeBeforeInsert(HASH_TABLE *& This)
	{
		size_t MaxCount = This->MaxCount;
		if(This->CountUsed >= MaxCount)
		{
			TINDEX NewSize = (TINDEX)(MaxCount * 1.61803398875f);
			if(!Realloc(This, NewSize))
				return false;
			This->IncreaseTable(NewSize);
		}
		return true;
	}

	static bool ResizeBeforeInsert(typename std::conditional<!Type, HASH_TABLE&, std::empty_type>::type This)
	{
		size_t MaxCount = This.MaxCount;
		if(This.CountUsed >= MaxCount)
		{
			TINDEX NewSize = (TINDEX)(MaxCount * 1.61803398875f);
			if(!Realloc(This, NewSize))
				return false;
			This.IncreaseTable(NewSize);
		}
		return true;
	}

	template<typename T>
	inline LPCELL Search(T SearchKey)
	{
		LPCELL p;
		for(TINDEX i = StartIndexByKey(SearchKey); i != NothingIndex; i = p->iNext)
			if((p = GetTable() + i)->CmpKey(SearchKey))
				return p;
		return nullptr;
	}

	template<typename T>
	inline LPCELL NextCollision(LPCELL CurElem, T SearchKey)
	{
		LPCELL p;
		for(TINDEX i = CurElem->iNext; i != NothingIndex; i = p->iNext)
			if((p = GetTable() + i)->CmpKey(SearchKey))
				return p;
		return nullptr;
	}

	inline bool EnumValues(bool (*EnumFunc)(void* UserData, TElementStruct* Element), void* UserData = nullptr)
	{
		for(decltype(GetTable()) Elements = GetTable(), l, p = Elements, m = p + MaxCount; p < m; p++)
			for(auto i = p->iStart; i != EmptyElement;)
			{
				i = (l = Elements + i)->iNext;
				if(!EnumFunc(UserData, l))
					return false;
			}
		return true;
	}

	inline bool EnumValues(bool (*EnumFunc)(TElementStruct* Element))
	{
		for(decltype(GetTable()) Elements = GetTable(), l, p = Elements, m = p + MaxCount; p < m; p++)
			for(auto i = p->iStart; i != EmptyElement;)
			{
				i = (l = Elements + i)->iNext;
				if(!EnumFunc(l)) 
					return false;
			}
		return true;
	}

	inline void EnumDelete(bool (*IsDeleteProc)(void* UserData, TElementStruct* Element), void* UserData = nullptr)
	{
		for(decltype(GetTable()) Elements = GetTable(), l, p = Elements, m = p + MaxCount; p < m; p++)
			for(auto i = &(p->iStart); *i != EmptyElement; )
			{	
				l = Elements + *i;
				if(IsDeleteProc(UserData, l))
				{
					auto e = *i;
					*i = l->iNext;
					l->iNext = IsFull.LastEmpty;
					IsFull.LastEmpty = e;
					IsFull.CountUsed--;
				}else
					i = &(l->iNext);
			}
	}

	inline void EnumDelete(bool (*IsDeleteProc)(TElementStruct* Element))
	{
		for(decltype(GetTable()) Elements = GetTable(), l, p = Elements, m = p + MaxCount; p < m; p++)
			for(auto i = &(p->iStart); *i != EmptyElement; )
			{	
				l = Elements + *i;
				if(IsDeleteProc(l))
				{
					auto e = *i;
					*i = l->iNext;
					l->iNext = IsFull.LastEmpty;
					IsFull.LastEmpty = e;
					IsFull.CountUsed--;
				}else
					i = &(l->iNext);
			}
	}

	template<typename T>
	LPCELL Remove(T SearchKey)
	{
		LPCELL p;
		for(LPTINDEX i = &(ElementByKey(SearchKey)->iStart); *i != NothingIndex; i = &(p->iNext))
		{
			p = GetTable() + *i;
			if(p->CmpKey(SearchKey))
			{
				TINDEX j = *i;
				*i = p->iNext;
				p->iNext = IsFull.LastEmpty;
				IsFull.LastEmpty = j;
				IsFull.CountUsed--;
				return p;
			}
		}
		return nullptr;
	}

	template<typename T>
	void RemoveAll(T SearchKey)
	{
		LPCELL p;
		for(LPTINDEX i = &(ElementByKey(SearchKey)->iStart); *i != NothingIndex; i = &(p->iNext))
		{
			p = GetTable() + *i;
			if(p->CmpKey(SearchKey))
			{
				TINDEX j = *i;
				*i = p->iNext;
				p->iNext = IsFull.LastEmpty;
				IsFull.LastEmpty = j;
				IsFull.CountUsed--;
			}	
		}
	}

	static bool ResizeAfterRemove(HASH_TABLE *& This)
	{
		size_t CountUsed = This->CountUsed;
		if((This->MaxCount > (size_t)(CountUsed *  1.61803398875f)) && (CountUsed > 3))
		{
			This->DecreaseTable();
			return Realloc(This, CountUsed);
		}
		return true;
	}

	static bool ResizeAfterRemove(typename std::conditional<!Type, HASH_TABLE&, std::empty_type>::type This)
	{
		size_t CountUsed = This.CountUsed;
		if((This.MaxCount > (size_t)(CountUsed *  1.61803398875f/*Golden ratio*/)) && (CountUsed > 3))
		{
			This.DecreaseTable();
			return Realloc(This, CountUsed);
		}
		return true;
	}

	void IncreaseTable(TINDEX NewSize)
	{
		decltype(GetTable()) Elements = GetTable();
		MaxCount = NewSize;
		for(LPCELL p = Elements, m = p + NewSize; p < m; p++)
			p->iStart = NothingIndex;
		TINDEX FullCount = CountUsed;
		for(TINDEX i = 0; i < FullCount; i++)
		{
			auto j = Elements[i].IndexInBound(NewSize);
			Elements[i].iNext = Elements[j].iStart;
			Elements[j].iStart = i;
		}
		TINDEX i = IsFull.LastEmpty = FullCount;
		for(NewSize--; i < NewSize; i++)
			Elements[i].iNext = i + 1;
		Elements[i].iNext = NothingIndex;
	}

	void DecreaseTable()
	{
		TINDEX NewSize = CountUsed;
		auto LastEmpty = IsFull.LastEmpty;
		decltype(GetTable()) Elements = GetTable(), l, m = Elements + MaxCount;
		for(auto p = Elements; p < m; p++)
			for(auto i = p->iStart; i != EmptyElement; i = l->iNext)
			{
				l = Elements + i;
				if(i >= NewSize) 
				{
					for(;LastEmpty >= NewSize; LastEmpty = Elements[LastEmpty].iNext);
					CopyElement(Elements[LastEmpty], Elements[i]);
					LastEmpty = Elements[LastEmpty].iNext;
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
		IsFull.LastEmpty = EmptyElement;
		MaxCount = NewSize;
	}

	//========================================================

	/*Interator type*/
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
		TINTER() { StartAgain(); }
	} TINTER, *LPTINTER;

	bool Interate(LPTINTER SetInterator) const
	{
		LPCELL Elements = GetTable();
		if(SetInterator->IsEnd)
		{
			for(TINDEX p = 0, m = MaxCount; p < m; p++)
				if(Elements[p].iStart != EmptyElement)
				{
					SetInterator->IsEnd.CurStartList = p;
					SetInterator->IsEnd.CurElementInList = Elements[p].iStart;
					return true;
				}
			SetInterator->IsEnd.CurStartList = EmptyElement;
			return false;
		}
		TINDEX m = MaxCount, i = SetInterator->IsEnd.CurElementInList;
		if(i >= m)
			return false;
		i = Elements[i].iNext;
		if(i != EmptyElement)
		{
			SetInterator->IsEnd.CurElementInList = i;
			return true;
		}
		for(TINDEX p = SetInterator->IsEnd.CurStartList + 1; p < m; p++)
		{			
			for(i = Elements[p].iStart; i != EmptyElement; i = Elements[i].iNext)
			{
				SetInterator->IsEnd.CurStartList = p;
				SetInterator->IsEnd.CurElementInList = i;
				return true;
			}
		}
		SetInterator->IsEnd.CurStartList = EmptyElement;
		return false;
	}

	inline bool CheckInterator(const LPTINTER Interator) const
	{
		if(Interator->IsEnd)
			return false;
		if(Interator->IsEnd.CurElementInList >= MaxCount)
			return false;
		return true;
	}

	inline LPCELL CellByInterator(const LPTINTER SetInterator) const { return GetTable() + SetInterator->IsEnd.CurElementInList; }

	template<typename TKey>
	bool InteratorByKey(TKey SearchKey, LPTINTER Interator)
	{
		LPCELL p, Elements = GetTable();
		TINDEX s = StartIndexByKey(SearchKey), i = s;
		for(; i != NothingIndex; i = p->iNext)
			if((p = Elements + i)->CmpKey(SearchKey))
			{
				Interator->IsEnd.CurStartList = s;
				Interator->IsEnd.CurElementInList = i;
				return true;
			}
		return false;
	}

	//Interate by key val
	inline LPCELL GetStartCell() const
	{
		LPCELL Elements = GetTable();
		for(TINDEX p = 0, m = MaxCount; p < m; p++)
			if(Elements[p].iStart != EmptyElement)
			{
				return Elements + Elements[p].iStart;
			}
		return nullptr;
	}


	template<typename TKey>
	LPCELL GetNextCellByKey(TKey SearchKey) const
	{
		LPCELL Elements = GetTable(), p;
		TINDEX s = IndexByKey(SearchKey), i = Elements[s].iStart;
		for(; i != NothingIndex; i = p->iNext)
			if((p = Elements + i)->CmpKey(SearchKey))
				break;
		if(i == NothingIndex)
			return nullptr;			
		i = Elements[i].iNext;
		if(i != EmptyElement)
			return Elements + i;
		s++;
		for(TINDEX m = MaxCount; s < m; s++)
			for(i = Elements[s].iStart; i != EmptyElement; i = Elements[i].iNext)
			{
				return Elements + i;
			}
		return nullptr;
	}


	//========================================================

	template<bool TypeAnother>
	static bool Copy(HASH_TABLE *& This,  HASH_TABLE<TElementStruct, TypeAnother, TIndex, NothingIndex>& Another)
	{
		LPCELL Tab = Another.GetTable();
		if(Tab == nullptr)
			return false;

		size_t NewSize = Another.MaxSize;
		size_t NewCount = Another.MaxCount;

		if(!Realloc(This, NewCount))
			return false;
		memcpy(This->GetTable(), Tab, NewSize);
		This->CountUsed.LastEmpty = Another.CountUsed.LastEmpty;
		This->CountUsed.MaxCount = Another.CountUsed.MaxCount;
		This->CountUsed.CountUsed = Another.CountUsed.CountUsed;
		return true;
	}

	template<bool TypeAnother>
	static bool Copy(HASH_TABLE & This,  HASH_TABLE<TElementStruct, TypeAnother, TIndex, NothingIndex>& Another)
	{
		LPCELL Tab = Another.GetTable();
		if(Tab == nullptr)
			return false;

		size_t NewSize = Another.MaxSize;
		size_t NewCount = Another.MaxCount;

		if(!Realloc(This, NewCount))
			return false;
		memcpy(This.GetTable(), Tab, NewSize);
		This.CountUsed.LastEmpty = Another.CountUsed.LastEmpty;
		This.CountUsed.MaxCount = Another.CountUsed.MaxCount;
		This.CountUsed.CountUsed = Another.CountUsed.CountUsed;
		return true;
	}

	inline LPCELL GetTable() const
	{
		if(Type)
			return (LPCELL)&CountUsed.Table;
		return CountUsed.Table;
	}

	inline static void Free(typename std::conditional<!Type, HASH_TABLE&, std::empty_type>::type This)
	{
		if(This.CountUsed.Table != nullptr)
		{
			free(This.CountUsed.Table);
			This.CountUsed.Table = nullptr;
		}
	}

	inline static void Free(HASH_TABLE* This)
	{
		if(!Type)
		{
			if(This->CountUsed.Table != nullptr)
			{
				free(This->CountUsed.Table);
				This->CountUsed.Table = nullptr;
			}
		}else
		{
			if(This != nullptr)
				free(This);
		}
	}

	static inline bool Realloc(typename std::conditional<!Type, HASH_TABLE&, std::empty_type>::type Val, TINDEX NewSize)
	{
		return (Val.CountUsed.Table = (LPCELL)realloc(Val.CountUsed.Table, NewSize * sizeof(CELL))) != nullptr;
	}

	static bool Realloc(HASH_TABLE *& Val, TINDEX NewCount)
	{
		if(!Type)
		{
			LPCELL Table = (LPCELL)realloc((*Val)->CountUsed.Table , NewCount * sizeof(CELL));
			if(!Table)
				return false;
			Val->CountUsed.Table = Table;
		}else
		{
			HASH_TABLE * NewTable;
			if(!(NewTable = (HASH_TABLE*)realloc(*Val, ___MAX_SIZE::ByCount(NewCount))))
				return false;
			Val = NewTable;
		}
		return true;
	}

	static inline bool New(typename std::conditional<!Type, HASH_TABLE&, std::empty_type>::type Val, TINDEX NewCount)
	{
		return (Val.CountUsed.Table = (LPCELL)malloc(NewCount * sizeof(CELL))) != nullptr;
	}

	static bool New(HASH_TABLE *& Val, TINDEX NewCount)
	{
		HASH_TABLE * NewTable;
		if(!Type)
		{
			NewTable = (HASH_TABLE*)malloc(sizeof(HASH_TABLE));
			if(!NewTable)
				return false;
			NewTable->CountUsed.Table = (LPCELL)malloc(NewCount * sizeof(CELL));
			if(!NewTable->CountUsed.Table)
				return false;
		}else
		{	
			if(!(NewTable = (HASH_TABLE*)malloc(___MAX_SIZE::ByCount(NewCount))))
				return false;
		}
		Val = NewTable;
		return true;
	}

	static inline bool New(typename std::conditional<!Type, HASH_TABLE&, std::empty_type>::type Val)
	{
		Val.CountUsed.Table = nullptr;
		return true;
	}

	static bool New(HASH_TABLE *& Val)
	{
		HASH_TABLE * NewTable;
		if(!Type)
		{
			NewTable = (HASH_TABLE*)malloc(sizeof(HASH_TABLE));
			if(!NewTable)
				return false;
			NewTable->CountUsed.Table = nullptr;
		}else
		{	
			if(NewTable = (HASH_TABLE*)malloc(sizeof(HASH_TABLE)))
				return false;
		}
		Val = NewTable;
		return true;
	}

	unsigned QualityInfo(char * Buffer, unsigned LenBuf)
	{
		unsigned CurLen = LenBuf, Len2, CurIndex = 0;
		Len2 = sprintf_s(
			Buffer, 
			CurLen, 
			"Count elements: %u\nAlloc count elements: %u\nSize(in bytes): %u\nHash quality:\n", 
			unsigned(CountUsed), 
			unsigned(MaxCount), 
			MaxCount * sizeof(CELL));
		if(Len2 < 0)
			Len2 = 0;
		CurLen -= Len2;
		Buffer += Len2;
		for(LPCELL Cur = GetTable(), Max = Cur + MaxCount;Cur < Max;Cur++)
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

template <typename CharType, typename DataType, bool IsDynamicKey = true>
struct HASH_ELEMENT_STRING
{
	CharType* KeyVal;
	DataType  Val;
	/*
	For Remove function.
	*/
	inline void DeleteKey()
	{
		if(IsDynamicKey)
			free(KeyVal);
	}

	///////////////////

	bool SetKey(const CharType* vKey)
	{
		if(!IsDynamicKey)
		{
			KeyVal = (decltype(KeyVal))vKey;
			return true;
		}
		KeyVal = StringDuplicate(vKey);
		if(KeyVal != nullptr)
			return true;
		return false;
	}

	inline static unsigned short IndexByKey(const CharType* k, unsigned char MaxCount) 
	{ 
		unsigned h = 0;
		for (unsigned s = 0; k[s] != CHAR_TYPE(CharType, '\0'); s++) 
			h = 31 * h + k[s];
		return h % MaxCount; 
	}

	inline unsigned short IndexInBound(unsigned char MaxCount) const { return IndexByKey(KeyVal, MaxCount); }

	//Compare values
	inline bool CmpKey(const CharType* EnotherKeyVal)
	{
		if(EnotherKeyVal == KeyVal)
			return true;
		return StringCompare(EnotherKeyVal, KeyVal) == 0;
	}
};


/*
Hi level hash table using with key as string.
*/
template<typename CharType, typename DataType, bool IsDynamicKey = true, typename IndexType = unsigned short>
class HASH_TABLE_STRING_KEY: private HASH_TABLE<HASH_ELEMENT_STRING<CharType, DataType, IsDynamicKey>, false, IndexType>
{
	typedef HASH_ELEMENT_STRING<CharType, DataType, IsDynamicKey> HASH_ELEMENT;
	typedef HASH_TABLE<HASH_ELEMENT, false, IndexType> PARENT;

	template<typename, typename, bool, typename>
	friend class HASH_TABLE_STRING_KEY;
public:

	PARENT::CountUsed;
	PARENT::QualityInfo;
	PARENT::Interate;
	PARENT::InteratorByKey;

	typedef typename PARENT::TINTER TINTER, *LPTINTER;


	HASH_TABLE_STRING_KEY(unsigned NewSize = 10)
	{
		PARENT::New(*this, NewSize);
		Init(NewSize);
	}

	template<bool isdyn, typename ti>
	HASH_TABLE_STRING_KEY(HASH_TABLE_STRING_KEY<CharType,DataType,isdyn,ti>& Another)
	{
		PARENT::New(*this, Another.MaxCount);
		Init(Another.MaxCount);
		Clear();
		Another.EnumValues(
			[](void* UsrData, typename HASH_TABLE_STRING_KEY<CharType,DataType,isdyn,ti>::HASH_ELEMENT* El) 
			{
				auto r = ((HASH_TABLE_STRING_KEY*)(UsrData))->Insert(El->KeyVal);
				if(r == nullptr)
					return false;
				*r = El->Val;
				return true;
			},
			this);
	}

	template<bool isdyn, typename ti>
	HASH_TABLE_STRING_KEY& operator =(HASH_TABLE_STRING_KEY<CharType,DataType,isdyn,ti>& Another)
	{
		Clear();
		Another.EnumValues(
			[](void* UsrData, typename HASH_TABLE_STRING_KEY<CharType,DataType,isdyn,ti>::HASH_ELEMENT* El) 
			{
				auto r = ((HASH_TABLE_STRING_KEY*)(UsrData))->Insert(El->KeyVal);
				if(r == nullptr)
					return false;
				*r = El->Val;
				return true;
			},
			this
			);
		return *this;
	}

	~HASH_TABLE_STRING_KEY()
	{			
		if(IsDynamicKey)
		{
			EnumValues(
				[](void*, HASH_ELEMENT* El) 
				{
					El->DeleteKey();
					return true;
				});
		}
		PARENT::Free(this);
	}

	bool DataByInterator(const LPTINTER Interator, CharType** Key, DataType* Value)
	{
		auto Cell = PARENT::CellByInterator(Interator);
		*Key = Cell->KeyVal;
		*Value = Cell->Val;
		return true;
	}

	/*
	Interate key like in JavaScript.
	*/
	CharType* In(const CharType* Key = nullptr) const
	{
		typename PARENT::LPCELL Cell;
		if(Key == nullptr)
			Cell = PARENT::GetStartCell();
		else
			Cell = PARENT::GetNextCellByKey((CharType*)Key);
		if(Cell == nullptr)
			return nullptr;
		return Cell->KeyVal;
	}

	bool Enum(bool (*EnumFunc)(void* UserData, CharType * Key, DataType* Value), void* UserData = nullptr)
	{
		struct DAT
		{ 
			decltype(EnumFunc) func;
			void* data;
		};
		DAT Cur = {EnumFunc, UserData};
		return EnumValues
		(
			[](void* Data, HASH_ELEMENT* El){return ((DAT*)Data)->func(((DAT*)Data)->data, El->KeyVal, &(El->Val));},
			&Cur
		);
	}

	DataType* Insert(const CharType* NewKey)
	{
		PARENT::ResizeBeforeInsert(*this);
		auto Cell = PARENT::Insert(NewKey);
		if(Cell == nullptr)
			return nullptr;
		return &(Cell->Val);
	}

	void Clear()
	{
		if(IsDynamicKey)
		{
			EnumValues
			(
				[](void*, HASH_ELEMENT* El) 
				{
					El->DeleteKey();
					return true;
				}
			);
		}
		if(PARENT::MaxCount > 10)
		{
			Realloc(*this, 10);
			Init(10);
		}else
			Init(PARENT::MaxCount);
	}

	void Remove(const CharType* SrchKey)
	{
		auto Res = PARENT::Remove(SrchKey);
		if(Res == nullptr)
			return;
		Res->DeleteKey();
		PARENT::ResizeAfterRemove(*this);
	}

	DataType* operator [](const CharType* SearchKey)
	{
		auto Cell = Search(SearchKey);
		if(Cell == nullptr)
			return nullptr;
		return &(Cell->Val);
	}

};

#endif
