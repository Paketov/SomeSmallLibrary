#pragma once

#include "ExHashTable.h"
#include "ExString.h"

template <typename CharType, typename DataType, typename IndexType, bool IsDynamicKey = true>
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

	inline static IndexType IndexByKey(const CharType* k, IndexType MaxCount) 
	{ 
		unsigned h = 0;
		for (size_t s = 0; k[s] != CHAR_TYPE(CharType, '\0'); s++) 
			h = 31 * h + k[s];
		return h % MaxCount; 
	}

	inline IndexType IndexInBound(IndexType MaxCount) const { return IndexByKey(KeyVal, MaxCount); }

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
class HASH_TABLE_STRING_KEY: private HASH_TABLE<HASH_ELEMENT_STRING<CharType, DataType, IndexType, IsDynamicKey>, IndexType>
{
	typedef HASH_ELEMENT_STRING<CharType, DataType, IndexType, IsDynamicKey> HASH_ELEMENT;
	typedef HASH_TABLE<HASH_ELEMENT, IndexType> PARENT;

	template<typename, typename, bool, typename>
	friend class HASH_TABLE_STRING_KEY;


	template<bool isdyn, typename ti>
	void CopyToThis(HASH_TABLE_STRING_KEY<CharType,DataType,isdyn,ti>& Another)
	{
		Another.EnumValues
		(
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
	}
public:

	PARENT::Count;
	PARENT::QualityInfo;
	PARENT::Interate;
	PARENT::InteratorByKey;
	PARENT::EmptyCount;

	typedef typename PARENT::TINTER TINTER, *LPTINTER;

	HASH_TABLE_STRING_KEY() {}

	template<bool isdyn, typename ti>
	HASH_TABLE_STRING_KEY(HASH_TABLE_STRING_KEY<CharType,DataType,isdyn,ti>& Another): PARENT(Another.Count)
	{
		CopyToThis(Another);
	}

	template<bool isdyn, typename ti>
	HASH_TABLE_STRING_KEY& operator =(HASH_TABLE_STRING_KEY<CharType,DataType,isdyn,ti>& Another)
	{
		PARENT::Clear();
		if(!PARENT::ResizeBeforeInsert(Another.Count))
			return *this;
		CopyToThis(Another);
		return *this;
	}

	~HASH_TABLE_STRING_KEY()
	{			
		if(IsDynamicKey)
		{
			PARENT::EnumValues
			(
				[](typename PARENT::LPTELEMENT El) 
				{
					El->DeleteKey();
					return true;
				}
			);
		}
	}

	bool DataByInterator(const LPTINTER Interator, CharType** Key, DataType* Value)
	{
		auto Cell = PARENT::CellByInterator(Interator);
		*Key = Cell->KeyVal;
		*Value = Cell->Val;
		return true;
	}

	/*
		Interate keys like in JavaScript.
	*/
	CharType* In(const CharType* Key = nullptr) const
	{
		typename PARENT::LPTELEMENT Cell;
		if(Key == nullptr)
			r = PARENT::GetStartCell();
		else
			r = PARENT::GetNextCellByKey((CharType*)Key);
		if(r == nullptr)
			return nullptr;
		return r->KeyVal;
	}

	/*
		Enum all values.
	*/
	bool Enum(bool (*EnumFunc)(void* UserData, CharType * Key, DataType* Value), void* UserData = nullptr)
	{
		struct DAT
		{ 
			decltype(EnumFunc) func;
			void* data;
		} Cur = {EnumFunc, UserData};
		return PARENT::EnumValues
		(
			[](void* Data, typename PARENT::LPTELEMENT El)
			{
				return ((DAT*)Data)->func(((DAT*)Data)->data, El->KeyVal, &(El->Val));
			},
			&Cur
		);
	}

	DataType* Insert(const CharType* NewKey)
	{
		if(PARENT::IsFull)
			PARENT::ResizeBeforeInsert((PARENT::Count < 3)? 3: (typename PARENT::TINDEX)(PARENT::Count * 1.61803398875f));
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
				[](typename PARENT::LPTELEMENT El) 
				{
					El->DeleteKey();
					return true;
				}
			);
		}
		PARENT::Clear();
	}

	/*
		Return true - is founded and deleted
		Return false - if not founded
	*/
	bool Remove(const CharType* SrchKey)
	{
		auto Res = PARENT::Remove(SrchKey);
		if(Res == nullptr)
			return false;
		Res->DeleteKey();
		if((typename PARENT::TINDEX)(PARENT::Count * 1.7f) < PARENT::AllocCount)
			PARENT::ResizeAfterRemove();
		return true;
	}

	DataType* operator [](const CharType* SearchKey)
	{
		auto Cell = Search(SearchKey);
		if(Cell == nullptr)
			return nullptr;
		return &(Cell->Val);
	}

};
