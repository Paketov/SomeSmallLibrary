#pragma once

#include <malloc.h>
#include <type_traits>
#include <functional>

/*
	Paketov
	2015

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



/*
* For define local function.
* Not lambda way.
* Example:

	int CallLocalFunc(int(*jj)(int))
	{
	   return jj(5);
	}
	void main()
	{
		DEF_FUNC
		(
				int,LocalFunction,(int Arg),
				{
					return Arg + 1;
				}
		);
		printf("%i\n", LocalFunction(45));				//out on screen: 46
		printf("%i\n", CallLocalFunc(LocalFunction));	//out on screen: 6
		return 0;
	}
*/

#ifndef thread_local
# if __STDC_VERSION__ >= 201112 && !defined __STDC_NO_THREADS__
#  define thread_local _Thread_local
# elif defined _WIN32 && ( \
       defined _MSC_VER || \
       defined __ICL || \
       defined __DMC__ || \
       defined __BORLANDC__ )
#  define thread_local __declspec(thread) 
/* note that ICC (linux) and Clang are covered by __GNUC__ */
# elif defined __GNUC__ || \
       defined __SUNPRO_C || \
       defined __xlC__
#  define thread_local __thread
# else
#  error "Cannot define thread_local"
# endif
#endif

namespace std
{	
	template<typename TypeHaveConstructor>
	struct def_var_in_union_with_constructor
	{
	private:
	    char _Data[sizeof(TypeHaveConstructor)];
	public:
		typedef TypeHaveConstructor TVAL;

		inline TypeHaveConstructor* operator->() const { return (TypeHaveConstructor*)_Data; }

		inline operator TypeHaveConstructor&() const { return *(TypeHaveConstructor*)_Data; }

		inline TypeHaveConstructor& operator =(TypeHaveConstructor& Data)
		{
			*(TypeHaveConstructor*)_Data = Data;
		   return *(TypeHaveConstructor*)_Data;
		}

		inline TypeHaveConstructor* operator &() const {  return (TypeHaveConstructor*)_Data; }

		template<typename SetType>
		TypeHaveConstructor & operator =(SetType & Val)
		{
		    *(TypeHaveConstructor*)_Data = Val;
			return *this;
		}
	};

	struct variant_arg
	{
		template<typename RetVal>
		operator RetVal() const  { return declval<RetVal>(); }
	};

	template<typename ElemType, size_t CountElements>
	struct make_array {  typedef ElemType type[CountElements]; };

	template<typename T1, typename T2>
	struct is_equal: is_same<T1, T2> {};

	struct empty_type {};

	template<bool Cond, typename RetType>
	struct not_empty_if { typedef typename conditional<Cond, RetType, empty_type>::type type;  };

	template<bool Cond, typename RetType> 
	struct gen_err_type  	
	{ 
	private: 
		class type__ { RetType * v; }; 
	public: 
		typedef typename conditional<Cond, RetType, type__>::type type; 
	}; 

	template<long long Num>
	struct not_less_zero: integral_constant<unsigned long long, ((Num < 0)?0:Num)> {};

	/*
	*IsFraction
	*/

	template<typename TypeNumber>
	typename  enable_if<is_floating_point<TypeNumber>::value, bool>::type  
	IsFraction(TypeNumber Val) { return Val != (TypeNumber)((long long)(Val)); }

	template<typename TypeNumber>
	typename enable_if<!is_floating_point<TypeNumber>::value, bool>::type  
	IsFraction(TypeNumber Val) { return false; }

	/*
	  countof
	*/

	template <typename Type>
	inline const size_t countof(Type) { return 1; }

	template <typename Type, size_t Len>
	inline const size_t countof(Type (&)[Len]) { return Len; }

	template <typename Type>
	struct arr_count: integral_constant<size_t, 1>{};

	template <typename Type, size_t Len>
	struct arr_count<Type[Len]>: integral_constant<size_t, Len * arr_count<Type>::value>{};



	/*
	Example: 
		std::count_pointers<int****>::value equal 4.
	*/
	template<typename _T>
	struct count_pointers : integral_constant<unsigned, 0>{};

	template<typename _T>
	struct count_pointers<_T *> : integral_constant<unsigned, count_pointers<_T>::value + 1>{};

	template<typename _T>
	struct count_pointers<_T * const> : integral_constant<unsigned, count_pointers<_T>::value + 1>{};

	template<typename _T>
	struct count_pointers<_T const *> : integral_constant<unsigned, count_pointers<_T>::value + 1>{};

	/*
	Example: 
		int q0 = 9, *q1 = &q0, **q2 = &q1;
		std::add_count_pointers<int, 2>::type w2 = q2;
		w2 equal q2.
	*/

	template<typename SourceType, unsigned Count>
	struct add_count_pointers { typedef typename add_count_pointers<SourceType*, Count - 1>::type type; };

	template<typename SourceType>
	struct add_count_pointers<SourceType, 0> { typedef SourceType type; };


	/*
	Example: 
		std::remove_all_pointers<int****>::type equal int.
	*/

	struct rem_mod
	{
		enum
		{
			REF = 1,
			RVREF = 2,
			POI = 4,
			ARR = 8,
			POICONST = 16,
			CONS = 32,
			VOLATILE = 64,
			POI_POICONST_VOLA_REF_RVREF = POICONST|POI|VOLATILE|REF|RVREF,
			ALL = POI_POICONST_VOLA_REF_RVREF|CONS|ARR
		};
	};

	template<typename SourceType, unsigned Param>
	struct remove_modifiers
	{
		typedef typename conditional
		<
			is_const<SourceType>::value && (Param & rem_mod::CONS), 
			typename remove_const<SourceType>::type, 
			SourceType
		>::type type;
	};

	template<typename SourceType, unsigned Param>
	struct remove_modifiers<SourceType *, Param>
	{
		typedef typename conditional
		<
			Param & rem_mod::POI, 
			typename remove_modifiers<SourceType, Param>::type, 
			typename remove_modifiers<SourceType, Param>::type *
		>::type type;
	};

	template<typename SourceType, unsigned Param>
	struct remove_modifiers<SourceType &, Param>
	{
		typedef typename conditional
		<
			Param & rem_mod::REF, 
			typename remove_modifiers<SourceType, Param>::type, 
			typename remove_modifiers<SourceType, Param>::type &
		>::type type;
	};

	template<typename SourceType, unsigned Param>
	struct remove_modifiers<SourceType &&, Param>
	{
		typedef typename conditional
		<
			Param & rem_mod::RVREF, 
			typename remove_modifiers<SourceType, Param>::type, 
			typename remove_modifiers<SourceType, Param>::type &&
		>::type type;
	};

	template<typename SourceType, unsigned Param, size_t Len>
	struct remove_modifiers<SourceType[Len], Param>
	{
		typedef typename conditional
		<
			Param & rem_mod::ARR, 
			typename remove_modifiers<SourceType, Param>::type, 
			typename remove_modifiers<SourceType, Param>::type[Len]
		>::type type;
	};

	template<typename SourceType, unsigned Param>
	struct remove_modifiers<SourceType * const, Param>
	{
		typedef typename conditional
		<
			(Param & rem_mod::POICONST) && (Param & rem_mod::POI), 
			typename remove_modifiers<SourceType, Param>::type, 
			typename conditional
			<
				Param & rem_mod::POICONST,
				typename remove_modifiers<SourceType, Param>::type *,
				typename remove_modifiers<SourceType, Param>::type * const
			>::type
		>::type type;
	};

	template<typename SourceType, unsigned Param>
	struct remove_modifiers<volatile SourceType , Param>
	{
		typedef typename conditional
		<
			Param & rem_mod::VOLATILE, 
			typename remove_modifiers<SourceType, Param>::type, 
			volatile typename remove_modifiers<SourceType, Param>::type
		>::type type;
	};

	/*
	Example: 
		char && e0 = 343434;
		std::move_pointers<double, decltype(e0)>::type dssdfdff = 0.8989; //double && dssdfdff
	*/

	template<typename DestType, typename>
	struct move_pointers_ref_arr { typedef DestType type; };
	template<typename DestType, typename SourceType>
	struct move_pointers_ref_arr<DestType, SourceType&> { typedef typename move_pointers_ref_arr<DestType, SourceType>::type  &type; };
	template<typename DestType, typename SourceType>
	struct move_pointers_ref_arr<DestType, SourceType&&> { typedef typename move_pointers_ref_arr<DestType, SourceType>::type  &&type; };
	template<typename DestType, typename SourceType>
	struct move_pointers_ref_arr<DestType, SourceType*> { typedef typename move_pointers_ref_arr<DestType, SourceType>::type  *type; };
	template<typename DestType, typename SourceType>
	struct move_pointers_ref_arr<DestType, SourceType * const> { typedef typename move_pointers_ref_arr<DestType, SourceType>::type  * const type; };
	template<typename DestType, typename SourceType>
	struct move_pointers_ref_arr<DestType, SourceType const *> { typedef typename move_pointers_ref_arr<DestType, SourceType>::type  const * type; };
	template<typename DestType, typename SourceType, size_t Len>
	struct move_pointers_ref_arr<DestType, SourceType[Len]> { typedef typename move_pointers_ref_arr<DestType, SourceType>::type  type[Len]; };

	/*
		Example: 
		int (& arr)[12];
		std::remove_ref_pointers_arr<decltype(arr)>::type eq. int

		int ***& v;
		std::remove_ref_pointers_arr<decltype(v)>::type eq. int
	*/
	template<typename Type>
	struct sizeof_value { static const size_t value = sizeof(typename remove_modifiers<Type, rem_mod::POI_POICONST_VOLA_REF_RVREF>::type); };


	/*
	* arr_type
	*  Return last pointer on val or array vals;
	*/

	template<typename Type> struct arr_type {typedef Type type;};
	template<typename Type, size_t Len> struct arr_type<Type[Len]>{typedef Type type[Len];};
	template<typename Type, size_t Len> struct arr_type<const Type[Len]>{typedef Type type[Len];};
	template<typename Type> struct arr_type<Type&>{typedef typename arr_type<Type>::type type;};
	template<typename Type> struct arr_type<Type&&>{typedef typename arr_type<Type>::type type;};
	template<typename Type> struct arr_type<volatile Type>{typedef typename arr_type<Type>::type type;};
	template<typename Type> struct arr_type<Type * const>{typedef typename arr_type<Type*>::type type;};
	template<typename Type> struct arr_type<const Type>{typedef Type type;};

	template<typename Type> struct arr_type<Type*>
	{
	   typedef 
	   typename conditional
	   <
		   !is_pointer<Type>::value && !is_reference<Type>::value && !is_array<Type>::value, Type*, Type
	   >::type type;
	};

	template<typename Type> struct arr_type<const Type*>
	{
	   typedef const
	   typename conditional
	   <
		   !is_pointer<Type>::value && !is_reference<Type>::value && !is_array<Type>::value, Type*,Type
	   >::type type;
	};

	/*
	* arr_to_single_dimension
	*/

	template<typename TypeArr, bool IsConvertPointer = false, size_t Count = 1>
	struct arr_to_single_dimension { typedef TypeArr type[Count]; };

	template<typename TypeArr>
	struct arr_to_single_dimension<TypeArr*, true> { typedef typename arr_to_single_dimension<TypeArr, true, 1>::type type; };

	template<typename TypeArr,  bool IsConvertPointer ,size_t Count, size_t Len>
	struct arr_to_single_dimension<TypeArr[Len], IsConvertPointer, Count> { typedef typename arr_to_single_dimension<TypeArr, IsConvertPointer, Count * Len>::type type; };

	/*
	* arr_count_dimension
	*/

	template<typename TypeArr>
	struct arr_count_dimension: integral_constant<size_t, 0>{};

	template<typename TypeArr, size_t Len>
	struct arr_count_dimension<TypeArr[Len]>: integral_constant<size_t, arr_count_dimension<TypeArr>::value + 1>{};

	/*
	* is_arr_value
	*/

	template<typename Type>
	struct is_arr_value : integral_constant<unsigned, 
		is_array<typename remove_const<typename arr_type<Type>::type>::type>::value ||
		is_pointer<typename remove_const<typename arr_type<Type>::type>::type>::value
	>{};

	/*
	* arr_value_element
	*/

	template<typename Type> struct arr_value_element 
	{
		typedef typename remove_modifiers
		<
			typename arr_to_single_dimension<
			typename remove_pointer<
			typename arr_type<Type>::type
			>::type
			>::type, 
			rem_mod::ARR
		>::type type;
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
	inline typename enable_if<!is_pointer<Type>::value, Type &>::type  
	valueof(Type & Value);

	template <typename Type>
	inline typename enable_if
	<
		is_pointer<Type>::value, 
		typename remove_modifiers<Type, rem_mod::POI_POICONST_VOLA_REF_RVREF>::type&
	>::type  
	valueof(Type & Pointer) { return valueof(*Pointer); }

	template <typename Type>
	inline typename enable_if<!is_pointer<Type>::value, Type &>::type  
	valueof(Type & Value) { return Value; }

	/*
	* arr_valueof_singd
	* array value as single dimension
	* represent pointer or val as array or pointer independence of last modifier
	* Example:
	* int arr[2] = {1,2}, * arr1, (*arr3)[2];
	* arr_valueof(arr1); //eq. int &* {1, 2}
	* int ***** arr4 ;
	* arr_valueof(arr4); //eq. int &* {1, 2}
	* arr_valueof(arr); //eq.  int&[2] {1, 2}
	* arr_valueof(arr3); //eq. int&[2] {1, 2}
	*/

	template <typename Type>
	typename arr_type<Type>::type & arr_valueof(Type& Pointer)
	{
		typedef typename arr_type<Type>::type RET_TYPE;
		return (RET_TYPE&)valueof(Pointer);
	}
		
	/*
	* arr_val_to_single_dimension
	* array value as single dimension
	* represent pointer or val as single dimension array
	* int[2][2] => int[4]
	* double[2][4][3] => double[48]
	* Example: 
	* int k[2][4];
	* auto r = arr_val_to_single_dimension(k);
	* for(unsigned i = 0; i < std::countof(r); i++)
	*	r[i] = 0;
	*/
	template <typename Type>
	typename arr_to_single_dimension<
		typename remove_pointer<
			typename arr_type<Type>::type
		>::type
	>::type & 
	arr_val_to_single_dimension(Type& Array)
	{
		typedef typename arr_to_single_dimension<
		typename remove_pointer<typename arr_type<Type>::type>::type>::type  RET_TYPE;
		return (RET_TYPE&)Array;
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
		 (sizeof(DestType) == sizeof(SourceType)) &&
		 (!is_scalar<SourceType>::value || !is_scalar<DestType>::value) &&
		 !is_const<DestType>::value
	>::type 
	val_copy(DestType & Dest, SourceType & Source)
	{
		struct COP_STRUCT {char __val[sizeof(DestType)];};
		(COP_STRUCT&)Dest = (COP_STRUCT&)Source;
	}

	template<typename DestType, typename SourceType>
	inline void val_copy(DestType & Dest, SourceType & Source, size_t LenCopy) { memcpy(&Dest, &Source, LenCopy); }

	template<typename DestType, typename SourceType>
	inline typename enable_if
	<
		 (sizeof(DestType) != sizeof(SourceType)) &&
		 (!is_scalar<SourceType>::value || !is_scalar<DestType>::value) &&
		 !is_const<DestType>::value
	>::type 
	val_copy(DestType & Dest, SourceType & Source)
	{
		struct COP_STRUCT { char __val [(sizeof(DestType) < sizeof(SourceType))? sizeof(SourceType): sizeof(DestType)];};
		(COP_STRUCT&)Dest = (COP_STRUCT&)Source;
	}

	template<typename DestType, typename SourceType>
	inline typename enable_if
	<
		 is_scalar<SourceType>::value &&
		 is_scalar<DestType>::value &&
		 !is_const<DestType>::value
	>::type
	val_copy(DestType & Dest, SourceType & Source) { Dest = Source; }

	template<size_t CountBytes>
	inline void zero_memory(void* Region)
	{
		struct __copy_struct{ unsigned char __zeroval[CountBytes]; };
		const __copy_struct __zeroval = {0};
		*(__copy_struct*)Region = __zeroval;
	}

	template<typename DestType>
	inline typename enable_if
	<!is_const<DestType>::value>::type zero_val(DestType & DestVal) { zero_memory<sizeof(DestType)>(&DestVal); }

	/*
	*arr_copy_cast
	*/
	template<typename DestType, typename SourceType>
	inline typename enable_if
	<
		 is_equal
		 <
			typename arr_value_element<DestType>::type,
			typename remove_const<typename arr_value_element<SourceType>::type>::type
		 >::value
	>::type 
	arr_copy_cast(DestType & Dest, SourceType & Source) { val_copy(Dest, Source); }

	template<typename DestType, typename SourceType>
	inline typename enable_if
	<
		 !is_equal
		 <
			typename arr_value_element<DestType>::type,
			typename remove_const<typename arr_value_element<SourceType>::type>::type
		 >::value &&
		 is_convertible
		 <
			typename arr_value_element<DestType>::type,
			typename arr_value_element<SourceType>::type
		 >::value
	>::type 
	arr_copy_cast(DestType & Dest, SourceType & Source)
	{
		auto& rSource = arr_val_to_single_dimension(Source);
		auto& rDest = arr_val_to_single_dimension(Dest);
		for(size_t i = 0, m = min(countof(rDest), countof(rSource)); i < m; i++)
			val_copy(rDest[i], rSource[i]);
	}

	template<typename DestType, typename SourceType>
	inline typename enable_if
	<
		 is_equal
		 <
			typename arr_value_element<DestType>::type,
			typename remove_const<typename arr_value_element<SourceType>::type>::type
		 >::value 
	>::type 
	arr_copy_cast(DestType & Dest, SourceType & Source, size_t Count) { val_copy(Dest, Source, Count * sizeof(Dest[0])); }

	template<typename DestType, typename SourceType>
	inline typename enable_if
	<
		 !is_equal<
			typename arr_value_element<DestType>::type,
			typename remove_const<typename arr_value_element<SourceType>::type>::type
		 >::value &&
		 is_convertible<
			typename arr_value_element<DestType>::type,
			typename arr_value_element<SourceType>::type
		 >::value
	>::type 
	arr_copy_cast(DestType & Dest, SourceType & Source, size_t Count)
	{
		auto& rSource = arr_val_to_single_dimension(Source);
		auto& rDest = arr_val_to_single_dimension(Dest);
		for(size_t i = 0, m = Count; i < m; i++)
			val_copy(rDest[i], rSource[i]);
	}

	/*
	*	inverse_bytes
	*/
	template<typename Type>
	Type inverse_bytes(const Type& Val)
	{
		Type r;
		const char* c = (const char*)&Val;
		char* b = (char*)(&r + 1) - 1;
		for(; b >= (char*)&r; b--, c++) *b = *c;
		return r;
	}

	inline void inverse_bytes(void* Dest, const void* Sourse, size_t Len)
	{
		const char* c = (const char*)Sourse;
		char* b = (char*)Dest + Len - 1;
		for(; b >= (char*)Dest; b--, c++) *b = *c;
	}

	/*
	* arr_set_elements
	*/
	template<typename DestType, typename SourceType>
	inline typename enable_if
	<
		 is_convertible
		 <
			typename arr_value_element<DestType>::type,
			typename remove_const<SourceType>::type
		 >::value
	>::type 
	arr_set_elements(DestType & Dest, SourceType & Val, size_t Count)
	{
		auto& rDest = arr_val_to_single_dimension(Dest);
		for(size_t i = 0, m = Count; i < m; i++)
			rDest[i] = Val;
	}

	template<typename DestType, typename SourceType>
	inline typename enable_if
	<
		 is_convertible
		 <
			typename arr_value_element<DestType>::type,
			typename remove_const<SourceType>::type
		 >::value
	>::type 
	arr_set_elements(DestType & Dest, SourceType & Val) { arr_set_elements(Dest, Val, countof(Dest)); }

	/*
	Get dynamic unique id for any type.
	Warning! On each running function generate different id. 
	Not use this function for save id type in file or other external storage.
	*/	
	template<typename>
	unsigned __get_id_for_type_id_generator() { static unsigned i = 0; return i++; }


	template<typename>
	unsigned get_id_for_type() { static unsigned Id = __get_id_for_type_id_generator<empty_type>(); return Id; }

	/*
	Associate some data with type
	*/

	template<typename TYPE, typename ASSOC_TYPE>
	struct assoc_type { static ASSOC_TYPE value; };

	template<typename TYPE,  typename ASSOC_TYPE>
	ASSOC_TYPE assoc_type<TYPE, ASSOC_TYPE>::value;

	template<typename TYPE, TYPE Val,  typename ASSOC_TYPE>
	struct assoc_val { static ASSOC_TYPE value; };

	template<typename TYPE, TYPE Val,  typename ASSOC_TYPE>
	ASSOC_TYPE assoc_val<TYPE, Val, ASSOC_TYPE>::value;

	struct make_default_pointer{
		template<typename RetVal>
		inline operator RetVal*() const { struct s{}; return &assoc_type<s, RetVal>::value; } 
	};

	struct make_default_reference{
		template<typename RetVal>
		inline operator RetVal&() const { struct s{}; return assoc_type<s, RetVal>::value; } 
	};

	template<typename RefType>
	bool is_default_ref(RefType& ChekRef) { return &(RefType&)make_default_reference() == &ChekRef; }

	struct on_scope_out_caller{
		std::function<void()> Func;
		inline on_scope_out_caller(std::function<void()> f): Func(f) {}
		inline ~on_scope_out_caller() { Func(); }
	};


	template<size_t CountChar, typename CharType>
	inline CharType* copy_const_string(CharType* Dest, const CharType (&Source)[CountChar])
	{
		struct copy_struct{ CharType __g[CountChar]; };
		*(copy_struct*)Dest = *(copy_struct*)&Source;
		return Dest + (CountChar - 1);
	}

	template<typename TypeInt>
	inline TypeInt not_less_zero_val(TypeInt v) { return (v < 0)? 0: v; }




};	

/*
* TYPE_VAL must have CountPointers field for counter;
*/
template<typename T>
void SHARED_POINTERDeleteProc(T* p) { delete p; }

template<typename TYPE_VAL, void (*DeleteProc)(TYPE_VAL*) = SHARED_POINTERDeleteProc>
class SHARED_POINTER
{
	TYPE_VAL* p;

	inline void Deinit()
	{	
		if(p == nullptr) return;
		p->CountPointers--;
		if(p->CountPointers <= 0) DeleteProc(p);
	}
public:
	inline SHARED_POINTER(): p(nullptr) {};
	inline SHARED_POINTER(TYPE_VAL* Pointer): p(Pointer) { if(p != nullptr) p->CountPointers++; }
	inline SHARED_POINTER(const SHARED_POINTER& a): p(a.p) { if(p != nullptr) p->CountPointers++; }
	inline ~SHARED_POINTER() { Deinit(); }
	inline SHARED_POINTER& operator=(const SHARED_POINTER& a) 
	{ 
		Deinit();
		p = a.p;
		if(p != nullptr) p->CountPointers++;
		return *this;
	}

	inline TYPE_VAL* operator->() const { return p; }
	inline TYPE_VAL* Get() const { return p; }
	inline bool operator ==(const SHARED_POINTER& Worker2) const { return p == Worker2.p; } 
	inline bool operator !=(const SHARED_POINTER& Worker2) const { return p != Worker2.p; }
	inline bool operator ==(decltype(nullptr) n) const { return p == nullptr; } 
	inline bool operator !=(decltype(nullptr) n) const { return p != nullptr; } 
};

