#ifndef __EXTYPETRAITS_H__
#define __EXTYPETRAITS_H__

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

		inline Type operator()(const Type NewVal) const
		{
			typedef std::enable_if<!OnWriteErr>::type;
			return NewVal;
		}

	};

	//Template test is type equal

	template<typename T1, typename T2>
	struct is_equal: false_type {};

	template<typename T>
	struct is_equal<T, T>: true_type {};



	


};
#endif
