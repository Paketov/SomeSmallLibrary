#ifndef __EXTYPETRAITS_H__
#define __EXTYPETRAITS_H__

#include <malloc.h>
#include <type_traits>


/*
	is_equal<t1, t2>::value
	empty_type
	not_empty_if<bool, t>::type
	gen_err_type<bool, t>::type
	IsFraction(Number)
	countof(array)
	count_pointers<t>::value
	add_count_pointers<t, count_add>::true
	remove_pointers<t>::type
	remove_pointers_ref<t>::type
	remove_pointers_ref_arr<t>::type
	move_pointers_ref_arr<t1, t2>::type
	sizeof_value<type>::value
	valueof(variable)
	val_copy(var_dest, var_source)
*/

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

		inline typename  enable_if<!OnWriteErr,Type>::type 
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

	struct empty_type
	{
	};


	template<bool Cond, typename RetType>
	struct not_empty_if
	{
		typedef typename conditional<Cond, RetType, empty_type>::type type; 
	};

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
	typename  enable_if<is_floating_point<TypeNumber>::value,bool>::type  
	IsFraction(TypeNumber Val)
	{
		return Val != (TypeNumber)((long long)(Val));
	}

	template<typename TypeNumber>
	typename enable_if<!is_floating_point<TypeNumber>::value,bool>::type  
	IsFraction(TypeNumber Val)
	{
		return false;
	}

	template <typename Type>
	inline const size_t countof(Type)
	{ 	
		return 1;
	}

	template <typename Type, size_t Len>
	inline const size_t countof(Type (&)[Len])
	{ 	
		return Len;
	}

	/*
	Example: 
		std::count_pointers<int****>::value equal 4.
	*/
	template<typename _T>
	struct count_pointers
	{
		static const unsigned value = 0;
	};


	template<typename _T>
	struct count_pointers<_T*>
	{
		static const unsigned value = count_pointers<_T>::value + 1;
	};

	/*
	Example: 
		int q0 = 9, *q1 = &q0, **q2 = &q1;
		std::add_count_pointers<int, 2>::type w2 = q2;
		w2 equal q2.
	*/

	template<typename _T, unsigned Count>
	struct add_count_pointers
	{
	    typedef typename add_count_pointers<_T*, Count - 1>::type type;
	};

	template<typename _T>
	struct add_count_pointers<_T, 0>
	{
	   typedef _T type;
	};

	/*
	Example: 
		std::remove_all_pointers<int****>::type equal int.
	*/
	template<typename _T>
	struct remove_pointers
	{
		typedef _T type;
	};

	template<typename _T>
	struct remove_pointers<_T*>
	{
		typedef typename remove_pointers<_T>::type type;
	};

	/*
	Example: 
		int (& arr)[12];
		std::remove_ref_pointers<decltype(arr)>::type eq. int[12]

	    int ***& v;
		std::remove_ref_pointers<decltype(v)>::type eq. int
	*/
	template<typename SourceType>
	struct remove_pointers_ref
	{
	    typedef SourceType type;
	};

	template<typename SourceType>
	struct remove_pointers_ref<SourceType&>
	{
	    typedef typename remove_pointers_ref<SourceType>::type  type;
	};

	template<typename SourceType>
	struct remove_pointers_ref<SourceType&&>
	{
	    typedef typename remove_pointers_ref<SourceType>::type  type;
	};

	template<typename SourceType>
	struct remove_pointers_ref<SourceType*>
	{
	    typedef typename remove_pointers_ref<SourceType>::type  type;
	};


	/*
	Example: 
		int (& arr)[12];
		std::remove_ref_pointers_arr<decltype(arr)>::type eq. int

	    int ***& v;
		std::remove_ref_pointers_arr<decltype(v)>::type eq. int
	*/

	template<typename SourceType>
	struct remove_pointers_ref_arr
	{
	    typedef SourceType type;
	};

	template<typename SourceType>
	struct remove_pointers_ref_arr<SourceType&>
	{
	    typedef typename remove_pointers_ref_arr<SourceType>::type  type;
	};

	template<typename SourceType>
	struct remove_pointers_ref_arr<SourceType&&>
	{
	    typedef typename remove_pointers_ref_arr<SourceType>::type  type;
	};

	template<typename SourceType>
	struct remove_pointers_ref_arr<SourceType*>
	{
	    typedef typename remove_pointers_ref_arr<SourceType>::type  type;
	};

	template<typename SourceType, size_t Len>
	struct remove_pointers_ref_arr<SourceType[Len]>
	{
	    typedef typename remove_pointers_ref_arr<SourceType>::type  type;
	};


	/*
	Example: 
		char && e0 = 343434;
		std::move_pointers<double, decltype(e0)>::type dssdfdff = 0.8989; //double && dssdfdff
	*/

	template<typename DestType, typename>
	struct move_pointers_ref_arr
	{
	    typedef DestType type;
	};

	template<typename DestType, typename SourceType>
	struct move_pointers_ref_arr<DestType, SourceType&>
	{
	    typedef typename move_pointers_ref_arr<DestType, SourceType>::type  &type;
	};

	template<typename DestType, typename SourceType>
	struct move_pointers_ref_arr<DestType, SourceType&&>
	{
	    typedef typename move_pointers_ref_arr<DestType, SourceType>::type  &&type;
	};

	template<typename DestType, typename SourceType>
	struct move_pointers_ref_arr<DestType, SourceType*>
	{
	    typedef typename move_pointers_ref_arr<DestType, SourceType>::type  *type;
	};

	template<typename DestType, typename SourceType, size_t Len>
	struct move_pointers_ref_arr<DestType, SourceType[Len]>
	{
	    typedef typename move_pointers_ref_arr<DestType, SourceType>::type  type[Len];
	};


	/*
		Example: 
		int (& arr)[12];
		std::remove_ref_pointers_arr<decltype(arr)>::type eq. int

	    int ***& v;
		std::remove_ref_pointers_arr<decltype(v)>::type eq. int
	*/
	template<typename Type>
	struct sizeof_value
	{
	    static const size_t value = sizeof(remove_pointers_ref<Type>::type);
	};

	/*
	Example: 
	 int q0 = 9, *q1 = &q0, **q2 = &q1;
	 std::valueof(q2) = 12;
	 auto a = std::valueof(q2); // a == 12
	 char w0[12];
	 std::valueof(w0)[0] = '0'; //w0[0] == '0'
	 q0 equal 12.
	*/
	template <typename Type>
	inline 
	typename enable_if<is_pointer<Type>::value, typename remove_pointers<Type>::type&>::type  
	valueof(Type & Pointer)
	{
		return valueof(*Pointer);
	}

	template <typename Type>
	inline 
	typename enable_if<!is_pointer<Type>::value, Type &>::type  
	valueof(Type & Value)
	{
		return Value;
	}

	/*
	Example: 
	 double a = 123.456, b, *c = &b;
	 std::val_copy(c, a); //b eq. 123.456

	 int d[3] = {1,2,3}, e[3];
	 std::val_copy(e, d); //e eq. {1,2,3}
	*/

	template<typename DestType, typename SourceType>
    inline typename enable_if
	<
		 (sizeof_value<DestType>::value == sizeof_value<SourceType>::value) &&
		 (!is_scalar<SourceType>::value || !is_scalar<DestType>::value) &&
		 !is_const<DestType>::value
	>::type 
	val_copy(DestType & Dest, SourceType & Source)
	{
		struct COP_STRUCT{DestType __val;};
		(COP_STRUCT&)valueof(Dest) = (COP_STRUCT&)valueof(Source);
	}

	template<typename DestType, typename SourceType>
    inline typename enable_if
	<
		 (sizeof_value<DestType>::value != sizeof_value<SourceType>::value) &&
		 !is_const<DestType>::value
	>::type 
	val_copy(DestType & Dest, SourceType & Source, size_t LenCopy)
	{
		memcpy(&Dest, &Source, LenCopy);
	}

	template<typename DestType, typename SourceType>
    inline typename enable_if
	<
		 (sizeof_value<DestType>::value != sizeof_value<SourceType>::value) &&
		 (!is_scalar<SourceType>::value || !is_scalar<DestType>::value) &&
		 !is_const<DestType>::value
	>::type 
	val_copy(DestType & Dest, SourceType & Source)
	{
		struct COP_STRUCT{char __val[max(sizeof_value<DestType>::value, sizeof_value<SourceType>::value)];};
		(COP_STRUCT&)valueof(Dest) = (COP_STRUCT&)valueof(Source);
	}

	template<typename DestType, typename SourceType>
    inline typename enable_if
	<
		 is_scalar<SourceType>::value &&
		 is_scalar<DestType>::value &&
		 !is_const<DestType>::value
	>::type
	val_copy(DestType & Dest, SourceType & Source)
	{
		valueof(Dest) = valueof(Source);
	}

};
#endif
