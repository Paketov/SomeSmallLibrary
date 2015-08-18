#ifndef __EXTYPETRAITS_H__
#define __EXTYPETRAITS_H__

#include <malloc.h>
#include <type_traits>

namespace std
{		

	template <typename Type, unsigned Offset = 0>
	struct decl_spec_val
	{
	private:
		char __a[Offset];
	public:
		Type Val;

		inline Type operator()() const
		{
			return Val;
		}

		inline Type operator()(const Type NewVal)
		{
			return Val = NewVal;
		}
	};

	template <typename Type>
	struct decl_spec_val<Type, 0>
	{ 	
		Type Val;

		inline Type operator()() const
		{
			return Val;
		}

		inline Type operator()(const Type NewVal)
		{
			return Val = NewVal;
		}
	};

	template <typename Type, unsigned Offset, size_t Len>
	struct decl_spec_val<Type[Len], Offset>
	{
	private:
		char __a[Offset];
	public:

		Type Val[Len];

		inline Type * operator()()
		{
			return Val;
		}

		inline Type * operator()(Type * NewVal)
		{
			memcpy(Val, NewVal, sizeof(Val));
			return NewVal;
		}
	};

	template <typename Type, size_t Len>
	struct decl_spec_val<Type[Len], 0>
	{ 	
		Type Val[Len];

		inline Type * operator()()
		{
			return Val;
		}

		inline Type * operator()(Type * NewVal)
		{
			memcpy(Val, NewVal, sizeof(Val));
			return NewVal;
		}
	};

	template <typename Type, Type InitVal, bool OnWriteErr = false>
	struct decl_const_val
	{	
		static const Type Val = InitVal;

		inline Type operator()() const
		{
			return Val;
		}

		inline typename std::enable_if<!OnWriteErr,Type>::type 
		operator()(const Type NewVal) const
		{
			return NewVal;
		}

	};

	//Template test is type equal

	template<typename T1, typename T2>
	struct is_equal: false_type {};

	template<typename T>
	struct is_equal<T, T>: true_type {};

	template<bool Cond, typename RetType> 
	struct gen_err_type  	
	{ 
	private: 
		class type__ 
		{ 
			RetType * v; 
		}; 
	public: 
		typedef typename conditional<Cond, RetType, type__>::type type; 
	}; 




	template<typename TypeNumber>
	bool IsFraction(TypeNumber Val)
	{
		return std::is_floating_point<TypeNumber>::value && (Val != (TypeNumber)((long long)(Val)));
	}

	template<typename DestType, typename SourceType>
    inline typename std::enable_if
	<
		 (sizeof(DestType) == sizeof(SourceType)) &&
		 (!std::is_scalar<SourceType>::value || !std::is_scalar<DestType>::value) &&
		 !std::is_const<DestType>::value
	>::type 
	ValCopy(DestType & Dest, SourceType & Source)
	{
		struct COP_STRUCT{DestType __val;};
		(COP_STRUCT&)Dest = (COP_STRUCT&)Source;
	}

	template<typename DestType, typename SourceType>
    inline typename std::enable_if
	<
		 (sizeof(DestType) != sizeof(SourceType)) &&
		 !std::is_const<DestType>::value
	>::type 
	ValCopy(DestType & Dest, SourceType & Source, size_t LenCopy)
	{
		memcpy(&Dest, &Source, LenCopy);
	}

	template<typename DestType, typename SourceType>
    inline typename std::enable_if
	<
		 (sizeof(DestType) != sizeof(SourceType)) &&
		 (!std::is_scalar<SourceType>::value || !std::is_scalar<DestType>::value) &&
		 !std::is_const<DestType>::value
	>::type 
	ValCopy(DestType & Dest, SourceType & Source)
	{
		struct COP_STRUCT{char __val[max(sizeof(DestType), sizeof(SourceType))];};
		(COP_STRUCT&)Dest = (COP_STRUCT&)Source;
	}

	template<typename DestType, typename SourceType>
    inline typename std::enable_if
	<
		 std::is_scalar<SourceType>::value &&
		 std::is_scalar<DestType>::value &&
		 !std::is_const<DestType>::value
	>::type 
	ValCopy(DestType & Dest, SourceType & Source)
	{
		Dest = Source;
	}

};
#endif
