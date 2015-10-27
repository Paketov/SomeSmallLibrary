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

		//Get key value
		unsigned Key()    
		{
			return vKey;
		}

		bool SetKey(double nKey)
		{
			Val = nKey;
		}

		//Get index in hash array by key value
		static unsigned char IndexByKey(unsigned Key, unsigned char MaxCount)
		{
			return (unsigned)Key % MaxCount;
		}	

		//Get key by value
		inline static unsigned GenKey(double ValForKey)	
		{
			unsigned h = 0;
			for (unsigned s = 0; s < sizeof(ValForKey); s++) 
				h = 31 * h + ((char*)&ValForKey)[s];
			return h;
		}

		//Compare values
		inline bool Cmp(double EnotherVal)
		{
			return EnotherVal == Val;
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
	typedef decltype(std::declval<TElementStruct>().Key())	TKEY,			  *LPTKEY;
	typedef TElementStruct									TPROPERTY_STRUCT, *LPTPROPERTY_STRUCT;


	typedef struct
	{
		TINDEX iStart;
		TINDEX iNext;
	} THEADCELL, *LPTHEADCELL;		

	typedef struct CELL :public THEADCELL, public TElementStruct 
	{
	} CELL, *LPCELL;

protected:

	struct
	{
		LPCELL	   Table;
	};

#define EXHASH_TABLE_FIELDS			\
		struct						\
		{							\
			unsigned	CountUsed;	\
			unsigned	MaxCount;	\
			TINDEX		LastEmpty;	\
		}

public:

	union
	{
		class
		{
			friend HASH_TABLE;
			EXHASH_TABLE_FIELDS;
			inline unsigned operator =(unsigned NewVal)
			{
				return CountUsed = NewVal;
			}
		public:
			inline operator unsigned() const
			{
				return CountUsed;
			}
		} CountUsed;


		class
		{
			friend HASH_TABLE;
			EXHASH_TABLE_FIELDS;

			inline unsigned operator =(unsigned NewVal)
			{
				return MaxCount = NewVal;
			}
		public:
			inline operator unsigned() const
			{
				return MaxCount;
			}
		} MaxCount;

		class ___MAX_SIZE
		{
			EXHASH_TABLE_FIELDS;
		public:

			inline operator unsigned() const
			{
				if(Type)
					return MaxCount * sizeof(CELL) + sizeof(HASH_TABLE) - sizeof(LPCELL);
				return MaxCount * sizeof(CELL);
			}

			inline static unsigned ByCount(unsigned CountElement)
			{
				if(Type)
					return CountElement * sizeof(CELL) + sizeof(HASH_TABLE) - sizeof(LPCELL);
				return CountElement * sizeof(CELL);
			}
		} MaxSize;


		class
		{
			friend HASH_TABLE;
			EXHASH_TABLE_FIELDS;
		public:

			inline operator bool()
			{
				return LastEmpty == NothingIndex;
			}
		} IsFull;
	};



	typedef struct
	{
		THEADCELL				h; 
		TPROPERTY_STRUCT		v;
	} TSTATIC_VAL, *LPTSTATIC_VAL;


	static const TINDEX EmptyElement = NothingIndex;

	static inline bool isHaveChain(LPCELL pCell)
	{
		return pCell->iStart != NothingIndex;
	}

	static inline bool isHaveNextChain(LPCELL pCell)
	{
		return pCell->iNext != NothingIndex;
	}

	inline TINDEX IndexByHash(TKEY Key)
	{
		return TElementStruct::IndexByKey(Key, MaxCount);
	}

	inline TINDEX StartIndexByHash(TKEY Key)
	{
		return (GetTable() + TElementStruct::IndexByKey(Key, MaxCount))->iStart;
	}

	inline LPCELL ElementByHash(TKEY Key)
	{
		return GetTable() + TElementStruct::IndexByKey(Key, MaxCount);
	}

	inline LPCELL operator[](TINDEX Index)
	{
		return GetTable() + Index;
	}

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

	inline void StaticInit(TINDEX Count, LPTSTATIC_VAL StaticTable = NULL)
	{
		if(!Type)
			Table = (LPCELL)StaticTable;
		MaxCount = Count;
		CountUsed = Count;
		IsFull.LastEmpty = 0;
		for(TINDEX i = 0;i < Count; i++)
		{
			LPCELL CurCell = GetTable() + i;
			LPCELL HashCell = ElementByHash(CurCell->Key());
			CurCell->iNext = HashCell->iStart;
			HashCell->iStart = i;
		}
	}

	inline void Init(TINDEX Count)
	{		
		MaxCount = Count;
		CountUsed = 0;
		Count--;
		TINDEX i;
		for(IsFull.LastEmpty = i = 0;i < Count;i++)
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
			return NULL;
		LPCELL HashCell = ElementByHash(TElementStruct::GenKey(SearchKey));	
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
		LPCELL p, HashCell = ElementByHash(TElementStruct::GenKey(SearchKey));
		for(TINDEX i = HashCell->iStart; i != NothingIndex; i = p->iNext)
			if((p = GetTable() + i)->Cmp(SearchKey))
				return p;
		if(IsFull)
			return NULL;
		
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
		TKEY Hash = TElementStruct::GenKey(SearchKey);
		LPCELL lpStart, p;

		for(TINDEX i = (lpStart = ElementByHash(Hash))->iStart; i != NothingIndex; i = p->iNext)
			if((p = GetTable() + i)->Cmp(SearchKey))
				return p;
		if(IsFull)
		{
			TINDEX NewSize = (TINDEX)(MaxCount * 1.61803398875f);
			if(!Realloc(*this, NewSize))
				return nullptr;
			IncreaseTable(NewSize);
			lpStart = ElementByHash(Hash);
		}
		return AddElement(lpStart, SearchKey);
	}

	template<typename T>
	inline LPCELL Search(T SearchKey)
	{
		LPCELL p;
		for(TINDEX i = StartIndexByHash(TElementStruct::GenKey(SearchKey)); i != NothingIndex; i = p->iNext)
			if((p = GetTable() + i)->Cmp(SearchKey))
				return p;
		return nullptr;
	}

	template<typename T>
	inline LPCELL NextCollision(LPCELL CurElem, T SearchKey)
	{
		LPCELL p;
		for(TINDEX i = CurElem->iNext; i != NothingIndex; i = p->iNext)
			if((p = GetTable() + i)->Cmp(SearchKey))
				return p;
		return nullptr;
	}

	template<typename T>
	LPCELL Remove(T SearchKey)
	{
		LPCELL p;
		for(LPTINDEX i = &(ElementByHash(TElementStruct::GenKey(SearchKey))->iStart); *i != NothingIndex; i = &(p->iNext))
		{
			p = GetTable() + *i;
			if(p->Cmp(SearchKey))
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

	void IncreaseTable(TINDEX NewSize)
	{
		for(LPCELL p = GetTable(), m = GetTable() + NewSize;p < m;p++)
			p->iStart = NothingIndex;

		TINDEX i, m = MaxCount;
		for(i = 0; i < m; i++)
		{
			LPCELL p = GetTable() + TElementStruct::IndexByKey(GetTable()[i].Key(), NewSize);
			GetTable()[i].iNext = p->iStart;
			p->iStart = i;
		}
		IsFull.LastEmpty = i;
		MaxCount = NewSize;
		for(NewSize--; i < NewSize; i++)
			GetTable()[i].iNext = i + 1;
		GetTable()[i].iNext = NothingIndex;

	}

	void DecreaseTable(TINDEX NewSize)
	{
		for(LPCELL p = GetTable(), m = GetTable() + MaxCount, l;p < m;p++)
		{
			for(TINDEX i = p->iStart; i != NothingIndex; i = l->iNext)
			{
				l = GetTable() + i;
				//Если элемент не входит за границы используемого диапозона
				if(i >= CountUsed) 
				{
					//Ищем в списке пустых первый элемент входящий в состав используемого диапозона
					while(IsFull.LastEmpty >= CountUsed)
						IsFull.LastEmpty = GetTable()[IsFull.LastEmpty].iNext;
					//Копируем выходящий за пределы елемент в свободный внутри массива
					IsFull.LastEmpty = GetTable()[i = IsFull.LastEmpty].iNext;
					GetTable()[i] = *l;
					l = GetTable() + i;
				}
			}
			p->iStart = NothingIndex;
		}
		TINDEX i;
		for(i = 0;i < CountUsed;i++)
		{
			LPCELL p = GetTable() + TElementStruct::IndexByKey(GetTable()[i].Key(), NewSize);
			GetTable()[i].iNext = p->iStart;
			p->iStart = i;
		}
		IsFull.LastEmpty = i;
		MaxCount = NewSize;
		for(NewSize--;i < NewSize;i++)
			GetTable()[i].iNext = i + 1;
		GetTable()[i].iNext = NothingIndex;
	}

	inline LPCELL GetTable()
	{
		if(Type)
			return (LPCELL)&Table;
		return Table;
	}

	inline void Free()
	{
		if(!Type)
			free(Table);
		else
			free(this);
	}

	static bool Realloc(HASH_TABLE & Val, TINDEX NewSize)
	{
		if(!Type)
			return (Val.Table = (LPCELL)realloc(Val.Table, NewSize * sizeof(CELL))) != NULL;
		else
			return false;
	}

	static bool Realloc(HASH_TABLE *& Val, TINDEX NewCount)
	{
		if(!Type)
		{
			LPCELL Table = (LPCELL)realloc((*Val)->Table , NewCount * sizeof(CELL));
			if(!Table)
				return false;
			Val->Table = Table;
		}else
		{
			HASH_TABLE * NewTable;
			if(!(NewTable = (HASH_TABLE*)realloc(*Val, ___MAX_SIZE::ByCount(NewCount))))
				return false;
			Val = NewTable;
		}
		return true;
	}

	static bool New(HASH_TABLE & Val, TINDEX NewCount)
	{
		if(!Type)
			return (Val.Table = (LPCELL)malloc(NewCount * sizeof(CELL))) != NULL;
		else
			return false;
	}


	static bool New(HASH_TABLE *& Val, TINDEX NewCount)
	{
		HASH_TABLE * NewTable;
		if(!Type)
		{
			NewTable = (HASH_TABLE*)malloc(sizeof(HASH_TABLE));
			if(!NewTable)
				return false;
			NewTable->Table = (LPCELL)malloc(NewCount * sizeof(CELL));
			if(!NewTable->Table)
				return false;
		}else
		{	
			if(!(NewTable = (HASH_TABLE*)malloc(___MAX_SIZE::ByCount(NewCount))))
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
			"Count elements: %u\nAlloc count elements: %u\nSize in byte: %u\nHash quality:\n", 
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

	//Get key value
	unsigned Key()    
	{
		return GenKey(KeyVal);
	}

	bool SetKey(CharType* vKey)
	{
		if(!IsDynamicKey)
		{
			KeyVal = vKey;
			return true;
		}
		KeyVal = StringDuplicate(vKey);
		if(KeyVal != nullptr)
			return true;
		return false;
	}

	void DeleteKey()
	{
		if(IsDynamicKey)
			free(KeyVal);
	}

	//Get index in hash array by key value
	static unsigned short IndexByKey(unsigned Key, unsigned char MaxCount)
	{
		return Key % MaxCount;
	}	

	//Get key by value
	inline static unsigned GenKey(CharType* vKey)	
	{
		unsigned h = 0;
		for (unsigned s = 0; vKey[s] != CHAR_TYPE(CharType, '\0'); s++) 
			h = 31 * h + vKey[s];
		return h;
	}

	//Compare values
	inline bool Cmp(CharType* EnotherKeyVal)
	{
		if(EnotherKeyVal == KeyVal)
			return true;
		return StringCompare(EnotherKeyVal, KeyVal) == 0;
	}
};


/*
	Hi level hash table using with key as string.
*/
template<typename CharType, typename DataType, bool IsDynamicKey = true>
class HASH_TABLE_STRING_KEY: private HASH_TABLE<HASH_ELEMENT_STRING<CharType, DataType, IsDynamicKey>, false, unsigned short>
{
	typedef HASH_TABLE<HASH_ELEMENT_STRING<CharType, DataType>, false, unsigned short> PARENT;
public:

	PARENT::CountUsed;
	PARENT::QualityInfo;

	HASH_TABLE_STRING_KEY(unsigned NewSize = 10)
	{
		PARENT::New(*this, NewSize);
		Init(NewSize);
	}

	~HASH_TABLE_STRING_KEY()
	{			
		auto Elements = GetTable();
		decltype(Elements) m = Elements + MaxCount;
		decltype(Elements) l;
		for(auto p = Elements; p < m; p++)
			for(auto i = p->iStart; i != PARENT::EmptyElement; i = l->iNext)
			{
				//Если элемент существует
				l = Elements + i;
				l->DeleteKey();
			}
		PARENT::Free();
	}

	DataType* Insert(CharType* NewKey)
	{
		auto Cell = PARENT::Insert(NewKey);
		if(Cell == nullptr)
			return nullptr;
		return &(Cell->Val);
	}

	void Remove(CharType* SrchKey)
	{
		auto Res = PARENT::Remove(SrchKey);
		if(Res == nullptr)
			return;
		Res->DeleteKey();
		if(MaxCount < (unsigned)(CountUsed *  1.61803398875f))
			PARENT::DecreaseTable(CountUsed);
	}

	DataType* operator [](CharType* SearchKey)
	{
	     auto Cell = Search(SearchKey);
		 if(Cell == nullptr)
			 return nullptr;
		 return &(Cell->Val);
	}

};

#endif
