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
		unsigned HashKey()    
		{
			return vKey;
		}

		bool SetKey(double nKey)
		{
			Val = nKey;
		}

		//Get index in hash array by key value
		static unsigned char IndexByHashKey(unsigned HashKey, unsigned char MaxCount)
		{
			return (unsigned)HashKey % MaxCount;
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
		inline bool CmpKey(double EnotherVal)
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
	typename TIndex = decltype(TElementStruct::IndexByHashKey(std::variant_arg(), std::variant_arg())),
	TIndex NothingIndex = TIndex(-1)
>
class HASH_TABLE
{	
public:
	typedef TIndex											TINDEX,		      *LPTINDEX;
	typedef decltype(std::declval<TElementStruct>().HashKey())	THASHKEY,	  *LPTHASHKEY;
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
			TINDEX		CountUsed;	\
			TINDEX		MaxCount;	\
			TINDEX		LastEmpty;	\
		}

public:

	union
	{
		class
		{
			friend HASH_TABLE;
			EXHASH_TABLE_FIELDS;
			inline TINDEX operator =(TINDEX NewVal)
			{
				return CountUsed = NewVal;
			}
		public:
			inline operator TINDEX() const
			{
				return CountUsed;
			}
		} CountUsed;


		class
		{
			friend HASH_TABLE;
			EXHASH_TABLE_FIELDS;

			inline TINDEX operator =(TINDEX NewVal)
			{
				return MaxCount = NewVal;
			}
		public:
			inline operator TINDEX() const
			{
				return MaxCount;
			}
		} MaxCount;

		class ___MAX_SIZE
		{
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

	inline TINDEX IndexByHash(THASHKEY HashKey) const
	{
		return TElementStruct::IndexByHashKey(HashKey, MaxCount);
	}

	inline TINDEX StartIndexByHash(THASHKEY HashKey) const
	{
		return (GetTable() + TElementStruct::IndexByHashKey(HashKey, MaxCount))->iStart;
	}

	inline LPCELL ElementByHash(THASHKEY HashKey)
	{
		return GetTable() + TElementStruct::IndexByHashKey(HashKey, MaxCount);
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
			LPCELL HashCell = ElementByHash(CurCell->HashKey());
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
		LPCELL HashCell = ElementByHash(TElementStruct::GenHashKey(SearchKey));	
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
		LPCELL p, HashCell = ElementByHash(TElementStruct::GenHashKey(SearchKey));
		for(TINDEX i = HashCell->iStart; i != NothingIndex; i = p->iNext)
			if((p = GetTable() + i)->CmpKey(SearchKey))
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
		THASHKEY Hash = TElementStruct::GenHashKey(SearchKey);
		LPCELL lpStart, p;

		for(TINDEX i = (lpStart = ElementByHash(Hash))->iStart; i != NothingIndex; i = p->iNext)
			if((p = GetTable() + i)->CmpKey(SearchKey))
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
		for(TINDEX i = StartIndexByHash(TElementStruct::GenHashKey(SearchKey)); i != NothingIndex; i = p->iNext)
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


	bool EnumValues(bool (*EnumFunc)(void* UserData, TElementStruct* Element), void* UserData = nullptr)
	{
		auto Elements = GetTable();
		decltype(Elements) l, m = Elements + MaxCount;
		for(auto p = Elements; p < m; p++)
			for(auto i = p->iStart; i != EmptyElement; i = l->iNext)
			{
				l = Elements + i;
				if(!EnumFunc(UserData, l))
					return false;
			}
		return true;
	}

	template<typename T>
	LPCELL Remove(T SearchKey)
	{
		LPCELL p;
		for(LPTINDEX i = &(ElementByHash(TElementStruct::GenHashKey(SearchKey))->iStart); *i != NothingIndex; i = &(p->iNext))
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

	void IncreaseTable(TINDEX NewSize)
	{
		for(LPCELL p = GetTable(), m = GetTable() + NewSize;p < m;p++)
			p->iStart = NothingIndex;

		TINDEX i, m = MaxCount;
		for(i = 0; i < m; i++)
		{
			LPCELL p = GetTable() + TElementStruct::IndexByHashKey(GetTable()[i].HashKey(), NewSize);
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
			LPCELL p = GetTable() + TElementStruct::IndexByHashKey(GetTable()[i].HashKey(), NewSize);
			GetTable()[i].iNext = p->iStart;
			p->iStart = i;
		}
		IsFull.LastEmpty = i;
		MaxCount = NewSize;
		for(NewSize--;i < NewSize;i++)
			GetTable()[i].iNext = i + 1;
		GetTable()[i].iNext = NothingIndex;
	}

	//========================================================

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
				inline operator bool() const
				{
					return CurStartList == EmptyElement;
				}
			} IsEnd;
		};

		void StartAgain()
		{
			IsEnd.CurStartList = EmptyElement;
		}

		TINTER()
		{
			StartAgain();
		}
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

	bool CheckInterator(const LPTINTER Interator) const
	{
		if(Interator->IsEnd)
			return false;
		if(Interator->IsEnd.CurElementInList >= MaxCount)
			return false;
		return true;
	}

	LPCELL CellByInterator(const LPTINTER SetInterator) const
	{
		return GetTable() + SetInterator->IsEnd.CurElementInList;
	}

	template<typename TKey>
	bool InteratorByKey(TKey SearchKey, LPTINTER Interator)
	{
		LPCELL p, Elements = GetTable();
		TINDEX s = StartIndexByHash(TElementStruct::GenHashKey(SearchKey)), i = s;
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
	LPCELL GetStartCell() const
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
		TINDEX s = IndexByHash(TElementStruct::GenHashKey(SearchKey)), i = Elements[s].iStart;
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
			return (LPCELL)&Table;
		return Table;
	}

	inline void Free()
	{
		if(!Type)
		{
			free(Table);
			Table = nullptr;
		}else
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

	static bool New(HASH_TABLE & Val)
	{
		if(!Type)
		{
			Val.Table = nullptr;
			return true;
		}
		else
			return false;
	}

	static bool New(HASH_TABLE *& Val)
	{
		HASH_TABLE * NewTable;
		if(!Type)
		{
			NewTable = (HASH_TABLE*)malloc(sizeof(HASH_TABLE));
			if(!NewTable)
				return false;
			NewTable->Table = nullptr;
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

	//Get hash key
	unsigned HashKey()    
	{
		return GenHashKey(KeyVal);
	}

	/*
		Set key value for Insert() function
	*/
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

	/*
	 For Remove function.
	*/
	inline void DeleteKey()
	{
		if(IsDynamicKey)
			free(KeyVal);
	}

	//Get index in hash array by key value
	static unsigned short IndexByHashKey(unsigned k, unsigned char MaxCount)
	{
		return k % MaxCount;
	}	

	//Get hash key by key value
	inline static unsigned GenHashKey(const CharType* vKey)	
	{
		unsigned h = 0;
		for (unsigned s = 0; vKey[s] != CHAR_TYPE(CharType, '\0'); s++) 
			h = 31 * h + vKey[s];
		return h;
	}

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
		Another.PARENT::EnumValues 
		(
			[](void* UsrData, HASH_TABLE_STRING_KEY<CharType,DataType,isdyn,ti>::HASH_ELEMENT* El) 
			{
				auto r = ((HASH_TABLE_STRING_KEY*)(UsrData))->Insert(El->KeyVal);
				if(r == nullptr)
					return false;
				*r = El->Val;
				return true;
			},
			this
		);
	}

	template<bool isdyn, typename ti>
	HASH_TABLE_STRING_KEY& operator =(HASH_TABLE_STRING_KEY<CharType,DataType,isdyn,ti>& Another)
	{
		Clear();
		Another.PARENT::EnumValues 
		(
			[](void* UsrData, HASH_TABLE_STRING_KEY<CharType,DataType,isdyn,ti>::HASH_ELEMENT* El) 
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
			EnumValues
			(
				[](void*, HASH_ELEMENT* El) 
				{
					El->DeleteKey();
					return true;
				}
			);
		}
		PARENT::Free();
	}

	bool DataByInterator(const LPTINTER Interator, CharType** Key, DataType* Value)
	{
		auto Cell = PARENT::CellByInterator(Interator);
		*Key = Cell->KeyVal;
		*Value = Cell->Val;
		return true;
	}

	/*
	Interete key like in JavaScript.
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
			[](void* Data, HASH_ELEMENT* El)
			{
				return ((DAT*)Data)->func(((DAT*)Data)->data, El->KeyVal, &(El->Val));
			},
			&Cur
		);
	}

	DataType* Insert(const CharType* NewKey)
	{
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
		if(PARENT::MaxCount < (unsigned)(CountUsed *  1.61803398875f))
			PARENT::DecreaseTable(CountUsed);
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
