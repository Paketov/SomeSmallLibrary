#ifndef __EX_ARG_H__
#define __EX_ARG_H__

#include "ExString.h"


/*
     ExArg
	 Paketov
	 2015.

	 Library for optimize work with input aruments.
	 Example:
	 cmd>programm  --help --val=123 C:\temp

	 C++

	 int main(int argc, char * argv[])
	 {
		EX_ARG<char> Argv(argc, argv, "--", "="); 
		if(Argv["val"].IsNotHave)
		  return 1;							//this code not execute
	    int InputNumber = Argv["val"];		//InputNumber eq. 123
		char * InputString = Argv["val"];	//InputNumber eq. "123"

		if(Argv.GetAnother(1).IsNotHave)
			return 2;						//this code not execute
		InputString = Argv.GetAnother(1);	//InputNumber eq. "C:\temp"
		if(Argv["help"].IsNotHave)
		  printf("--help not input");		//this code not execute
	    return 0;	    
	 }
*/

template<typename TypeChar = char>
class EX_ARG
{

#define _EX_ARG_FIELDS	\
	struct{\
	TypeChar  **arg;\
	unsigned	_Count;\
	unsigned	SeparatorLen;\
	unsigned	PrefixLen;\
	TypeChar	Prefix[100];\
	TypeChar	Separator[100];\
	bool i;\
	}
public:
	union{
		class{
			friend EX_ARG;
			_EX_ARG_FIELDS;
		public:
			inline operator unsigned() const { return _Count; }
		} Count;
	};

private:

	bool CmpNameValues(const TypeChar * BaseStr, const TypeChar * SubStr, size_t Len)
	{
		if(Count.i)
			if(StringICompare(BaseStr, SubStr, Len) != 0)
				return false;
		if(StringCompare(BaseStr, SubStr, Len) != 0)
			return false;
		return StringCompare(BaseStr + Len, Count.Prefix, Count.PrefixLen) != 0;
	}

	int Search(const TypeChar * _Name, unsigned _NameLen,TypeChar *& NameArg, TypeChar *& Value)
	{
		for(unsigned i = 0;i < Count._Count; i++)
		{
			TypeChar *Name, *Val;
			if(!Get(Count.arg[i], StringLength(Count.arg[i]), Name, Val))
				continue;

			if(CmpNameValues(Name, _Name, _NameLen))
			{
				Value = Val;
				NameArg = Name;
				return i;
			}
		}
		return -1;
	}

	int SearchVal(const TypeChar * SrchValue, unsigned SrchValueLen, TypeChar *& NameArg, TypeChar *& Value)
	{
		for(int i = 0;i < Count._Count; i++)
		{
			TypeChar *Name, *Val;
			if(!Get(Count.arg[i], StringLength(Count.arg[i]), Name, Val))
				continue;

			if(StringCompare(Val, SrchValue, SrchValueLen) == 0)
			{
				Value = Val;
				NameArg = Name;
				return i;
			}
		}
		return -1;
	}

	bool Get(TypeChar * Arg, unsigned Len, TypeChar *& Name, TypeChar *& Value)
	{
		if(Len <= Count.PrefixLen)
			return false;
		if(StringCompare(Arg, Count.Prefix, Count.PrefixLen) != 0)
			return false;
		Name = Arg + Count.PrefixLen;
		const TypeChar * Sep = StringSearch(Name, Count.Separator);
		if(Sep != NULL)
			Value = (TypeChar*)Sep + Count.SeparatorLen;
		else 
			Value = Arg + Len;
		return true;
	}

public:

	class INTERATOR
	{
#define __INTERATOR_FIELDS struct{TypeChar * arg; TypeChar * Value;TypeChar * NameArg;}

	public:
		union
		{
			class{
				__INTERATOR_FIELDS;
			public:
				inline operator TypeChar*() { return arg; }
				inline TypeChar * operator()() { return  arg; }
			} FullString;

			class{
				__INTERATOR_FIELDS;
				friend INTERATOR;
			public:
				inline operator TypeChar*() { return NameArg; }
				inline TypeChar * operator()() { return  NameArg; }
			} NameArg;

			class{
				__INTERATOR_FIELDS;
			public:
				inline operator TypeChar*() { return Value; }
				inline TypeChar * operator()() { return  Value; }
			} Value;

			class{
				__INTERATOR_FIELDS;
			public:
				inline operator bool() { return *arg == CHAR_TYPE(TypeChar, '\0'); }
			} IsNotHave;
		};

		inline INTERATOR(TypeChar* Arg = "", TypeChar * Name = "", TypeChar * Val = "")
		{
			NameArg.arg = Arg;
			NameArg.Value = Val;
			NameArg.NameArg = Name;
		}

		inline operator bool() const { return NameArg.arg != NULL; } 

		inline operator TypeChar *() { return NameArg.Value; }

		bool operator==(TypeChar * Enother) const
		{
			if(NameArg.arg == NULL)
				return false;
			if(StringCompare(NameArg.Value, Enother) == 0)
				return true;
			return false;
		}

		inline bool operator!=(TypeChar * Enother) const { return !operator==()(Enother); }

		operator int() const
		{
			int Res = 0;
			StringDoubleToNumber(&Res, NameArg.Value, 0xffff);
			return Res;
		}

		operator double() const
		{
			double Res = 0;
			StringToNumber(&Res, NameArg.Value, 0xffff);
			return Res;
		}

		INTERATOR & operator()(TypeChar *& ArgName, TypeChar *& Value)
		{
			ArgName = NameArg.NameArg;
			Value = NameArg.Value;
			return *this;
		}
	};


	EX_ARG
	(
		unsigned nCount, 
		TypeChar * argv[], 
		const TypeChar * nPrefix = STR_TYPE(TypeChar,"--"), 
		const TypeChar * nSeparator = STR_TYPE(TypeChar,"=")
	)
	{
		Count.arg = argv;
		Count._Count = nCount;
		Count.PrefixLen = StringLength(nPrefix);
		Count.SeparatorLen = StringLength(nSeparator);
		StringCopy(Count.Prefix, nPrefix);
		StringCopy(Count.Separator, nSeparator);
		Count.i = false;
	}

	EX_ARG()
	{
		Count.arg = NULL;
		Count._Count = NULL;
		Count.PrefixLen = 0;
		Count.SeparatorLen = 0;
		Count.Prefix[0] = 0;
		Count.Separator[0] = 0;
		Count.i = false;
	}

	void Set
	(
		unsigned nCount, 
		TypeChar * argv[], 
		const TypeChar * nPrefix = STR_TYPE(TypeChar,"--"), 
		const TypeChar * nSeparator = STR_TYPE(TypeChar,"=")
	)
	{
		Count.arg = argv;
		Count._Count = nCount;
		Count.PrefixLen = StringLength(nPrefix);
		Count.SeparatorLen = StringLength(nSeparator);
		StringCopy(Count.Prefix, nPrefix);
		StringCopy(Count.Separator, nSeparator);
	}

	/*
	Get arg by name.
	*/
	inline INTERATOR operator[](TypeChar * _Name)
	{
		TypeChar * Value = STR_TYPE(TypeChar, ""), *Name = STR_TYPE(TypeChar, "");
		int Index = Search(_Name, StringLength(_Name), Name, Value);
		return INTERATOR(((Index == -1)?STR_TYPE(TypeChar, ""): Count.arg[Index]), Name, Value);
	}

	/*
	Get arg by index.
	*/
	inline INTERATOR operator[](unsigned Index)
	{
		if(Index >= Count._Count)
			throw "Index out of bound";

		TypeChar *Name = STR_TYPE(TypeChar, ""), *Value = STR_TYPE(TypeChar, "");
		Get(Count.arg[Index], StringLength(Count.arg[Index]), Name, Value);
		return INTERATOR(Count.arg[Index], Name, Value);
	}

	/*
	Search arg by val.
	*/
	inline INTERATOR SearchByVal(TypeChar * SrchValue)
	{
		TypeChar *Name = STR_TYPE(TypeChar, ""), *Value = STR_TYPE(TypeChar, "");
		int Index = SearchVal(SrchValue, StringLength(SrchValue), Name, Value);
		return INTERATOR(((Index == -1)? STR_TYPE(TypeChar, ""): Count.arg[Index]), Name, Value);
	}


	/*
	Get another args.
	*/
	inline INTERATOR GetAnother(unsigned Index = 0)
	{
		for(int i = 0, j = 0;i < Count._Count; i++)
		{
			TypeChar *Name = NULL, *Value = NULL;
			if(!Get(Count.arg[i], StringLength(Count.arg[i]), Name, Value))
			{
				if(j == Index)
					return INTERATOR(Count.arg[i], STR_TYPE(TypeChar, ""), Count.arg[i]);
				j++;
			}
		}
		return INTERATOR();
	}

};


#endif
