#ifndef __EXMATRIX_H_HAS_INCLUDED__
#define __EXMATRIX_H_HAS_INCLUDED__

/*
  MATRIX class.
  Paketov 
  2015

  Example working:

	double A[2][2] = 
	{
		1,2,
		3,4
	};

	MATRIX<double, 2, 2> j(A);      //Static version
	auto SizeMatrix = sizeof(j);	//SizeMatrix eq. 32
	MATRIX<double> b(A);			//Dynamic version
	SizeMatrix = sizeof(b);			//SizeMatrix eq. 12 on 32bit machine (CountColumn + CountRow + pointer).
	bool JIsOrtogonal = j.IsOrtogonal;

	MATRIX<double, 2, 2> L(2,2);
	MATRIX<double> U(2,2);
	j.LUDecomposition(L, U);       //LU Factorization
	auto sec_j = L * U;


	if(sec_j == j)
	{
		//This block is executed
	}
	if(j.IsSingular)
	{
		//This block is not executed
	}
	auto InverseMatrix = j.ToInverse();

	j[0] = U[1]; //Copy 1 row from U to 0 index j.

*/


#include <malloc.h>
#include <string.h>
#include <string>
#include "ExTypeTraits.h"
#include "ExString.h"

#ifdef _MATRIX_CHECK
#define matrix_check(Check, Message)  if(!(Check)) throw (Message)
#else
#define matrix_check(Check, Message)  (Check);
#endif

#ifdef _MATRIX_CHECK_INDEXES
#define matrix_check_index(Check, Message)  if(!(Check)) throw (Message)
#else
#define matrix_check_index(Check, Message)  
#endif



template<class T, unsigned cj = 0>
class ROW
{	
	template<class, unsigned, unsigned> 
	friend class MATRIX;

	static const bool IsStaticArr = (cj != 0) && (cj != unsigned(-1));

	//Static version
	class TFIELDS_STATIC
	{
	public:
		static const unsigned _Count = cj;
		T v[cj];

		inline unsigned SetCount(const unsigned Val) const { return Val; }

		inline void Init() const {}

		inline void Deinit() const {}

		inline void Allocate(const unsigned) const {}

		template<typename _T, unsigned _j>
		inline void Copy(const ROW<_T, _j> & Another)
		{
			matrix_check(Another._Fields._Count == _Count, "Count columns not equal.");
			std::arr_copy_cast(v, Another._Fields.v);
		}

		template<typename _T>
		inline void Copy(const _T * Arr, unsigned count)
		{
			matrix_check(count == _Count, "Count columns not equal.");
			std::arr_copy_cast(v, Arr);
		}

		template<typename _T>
		inline void Copy(const _T (&Arr)[cj]) { std::arr_copy_cast(v, Arr); }
	};

	//Version represent as interator
	class TFIELDS_INTERATOR
	{
	public:
		unsigned _Count;
		T * v;

		inline unsigned SetCount(const unsigned Val) { return _Count = Val; }

		inline void Init()
		{
			v = nullptr;
			_Count = 0;
		}

		inline void Deinit() const {}

		inline void Allocate(const unsigned) const {}

		template<typename _T>
		inline void Copy(const ROW<_T, 0> & Another)
		{
			matrix_check(Another._Fields._Count == _Count, "Count columns not equal.");
			std::arr_copy_cast(std::valueof(v), std::valueof(Another._Fields.v), Another._Fields._Count);
		}

		template<typename _T, unsigned _j>
		inline void Copy(const ROW<_T, _j> & Another)
		{
			matrix_check(Another._Fields._Count == _Count, "Count columns not equal.");
			std::arr_copy_cast(std::valueof(v), Another._Fields.v);
		}

		template<typename _T>
		inline void Copy(_T * Arr, unsigned count)
		{
			if(v == nullptr)
			{
				v = Arr, _Count = count;
				return;
			}
			matrix_check(count == _Count, "Count columns not equal.");
			std::arr_copy_cast(std::valueof(v), std::valueof(Arr), count);
		}

		template<typename _T, unsigned _j>
		inline void Copy(const _T (&Arr)[_j])
		{
			matrix_check(_j == _Count, "Count columns not equal.");
			std::arr_copy_cast(v, Arr);
		}
	};

	//Dynamic version
	class TFIELDS_DYNAMIC
	{
	public:
		unsigned _Count;
		T * v;

		inline unsigned SetCount(const unsigned Val) { return _Count = Val; }

		inline void Init()
		{
			v = nullptr;
			_Count = 0;
		}

		inline void Deinit()
		{
		   if(v != nullptr)
			   free(v);
		}

		inline void Allocate(const unsigned Count)
		{
			T * NewVal = (T*)realloc(v, sizeof(T) * Count);
			matrix_check(NewVal != nullptr, "Not alloc memory for row.");
			_Count = Count;
			v = NewVal;
		}

		template<typename _T>
		void Copy(const ROW<_T, 0> & Another)
		{
			Allocate(Another._Fields._Count);
			std::arr_copy_cast(std::valueof(v), std::valueof(Another._Fields.v), Another._Fields._Count);
		}

		template<typename _T, unsigned _j>
		void Copy(const ROW<_T, _j> & Another)
		{
			Allocate(Another._Fields._Count);
			std::arr_copy_cast(std::valueof(v), Another._Fields.v);
		}

		template<typename _T>
		inline void Copy(_T * Arr, unsigned count)
		{
			Allocate(count);
			std::arr_copy_cast(std::valueof(v), std::valueof(Arr), count);
		}

		template<typename _T, unsigned _j>
		void Copy(const _T (&Arr)[_j])
		{
			Allocate(_j);
			std::arr_copy_cast(std::valueof(v), Arr);
		}
	};

	typedef typename std::conditional
	<
		cj == unsigned(-1),
		TFIELDS_INTERATOR, 
		typename std::conditional
		<
			cj == 0,
			TFIELDS_DYNAMIC,
			TFIELDS_STATIC
		>::type
	>::type TFIELDS;

#define	__ROW_FIELDS TFIELDS _Fields;

	class _REMOVE_ELEMENT
	{
		__ROW_FIELDS;
	public:
		void operator()(unsigned Pos, unsigned Count = 1)
		{			
			if(Pos >= _Fields._Count)
				return;
			unsigned MaxOffset = min(Pos + Count, _Fields._Count);
			unsigned NewCount = _Fields._Count - MaxOffset;
			memmove(&_Fields.v[Pos], &_Fields.v[MaxOffset], sizeof(T) * NewCount);
			_Fields.Allocate(NewCount + Pos);
		}
	};

	class _INSERT_ELEMENT
	{
		__ROW_FIELDS;
	public:
		void operator()(unsigned Pos, unsigned Count = 1)
		{
			if(Pos > _Fields._Count)
			{
				Count += (Pos - _Fields._Count);
				Pos = _Fields._Count;
			}
			_Fields.Allocate(_Fields._Count + Count);
			memmove(&_Fields.v[Pos + Count], &_Fields.v[Pos], sizeof(T) * (_Fields._Count - Pos - Count));
		}

		void operator()(unsigned Pos, unsigned Count, T Value)
		{
			if(Pos > _Fields._Count)
			{
				Count += (Pos - _Fields._Count);
				Pos = _Fields._Count;
			}
			_Fields.Allocate(_Fields._Count + Count);
			memmove(((T*)_Fields.v) + (Pos + Count), ((T*)_Fields.v) + Pos, sizeof(T) * (_Fields._Count - Pos - Count));
			std::arr_set_elements(_Fields.v[Pos], Value, Count);
		}
	};

public:


	union
	{
		__ROW_FIELDS;

		class
		{
			__ROW_FIELDS;
			friend ROW;
			inline unsigned operator=(unsigned NewVal) { return _Fields.SetCount(NewVal); }
		public:
			inline operator unsigned() const { return _Fields._Count;}
		} Count;

		class
		{
			__ROW_FIELDS;
		public:
			operator T()
			{
				T s = T(0);
				for(unsigned i = 0;i < _Fields._Count;i++)
					s += _Fields.v[i];
				return s;
			}
		} Summ;

		class
		{
			__ROW_FIELDS;
		public:
			operator T()
			{		
				T s = T(0);
				for(unsigned i = 0;i < _Fields._Count;i++)
					s += (_Fields.v[i] * _Fields.v[i]);
				return sqrt(s);
			}
		} Mod;

		typename std::not_empty_if<cj == 0, _REMOVE_ELEMENT>::type	RemoveElement;
		typename std::not_empty_if<cj == 0, _INSERT_ELEMENT>::type	InsertElement;
	};

private:

	template <unsigned StaticSize, unsigned Index>
	class TSETTER
	{
		ROW * r;
		friend ROW;
		inline TSETTER(ROW * r) { this->r = r; }
	public:

		inline typename std::enable_if
		<
			!IsStaticArr || (Index <= cj), 
			TSETTER<StaticSize, Index + 1> 
		>::type 
		operator <<(T Val)
		{
			(*r)[Index] = Val;
			return TSETTER<StaticSize, Index + 1>(r);
		}
	};


	template <unsigned Index>
	class TSETTER<0, Index>
	{
		ROW * r;
		friend ROW;
		inline TSETTER(ROW * r) { this->r = r; }
	public:
		inline TSETTER<0, Index + 1> operator <<(T Val)
		{
			if(Index >= r->Count)
				r->_Fields.Allocate(Index + 1);
			(*r)[Index] = Val;
			return TSETTER<0, Index + 1>(r);
		}
	};

public:

	ROW() { _Fields.Init(); }

	ROW(T * Column, unsigned Count)
	{
		_Fields.Init();
		_Fields.Copy(Column, Count);
	}

	template<typename _T, unsigned _j>
	ROW
	(
		ROW<_T, _j> & Val,
		typename std::enable_if
		<
			std::is_convertible<_T, T>::value &&
			(!IsStaticArr || !ROW<_T, _j>::IsStaticArr || (cj == _j)), 
			std::empty_type
		>::type = std::empty_type()
	)
	{
		_Fields.Init();
		_Fields.Copy(Val);
	}

	template<typename _T, unsigned _j>
	ROW
	(
		_T (&Val)[_j],
		typename std::enable_if
		<
			std::is_convertible<_T, T>::value && !IsStaticArr || (cj == _j), 
			std::empty_type
		>::type = std::empty_type()
	)
	{
		_Fields.Init();
		_Fields.Copy(Val);
	}

	ROW(unsigned _Count)
	{
		_Fields.Init();
		_Fields.Allocate(_Count);
	}

	/////////////

	template<class TypeChar>
	STR_STAT FromText(const TypeChar * Str, size_t Len = 0xfffff)
	{
		size_t CountReaded = 0;
		for(unsigned i = 0; i <  _Fields._Count; i++)
		{		
			bool r;
			CountReaded += StringDoubleToNumber(_Fields.v + i, Str + CountReaded, NOT_LESS_Z(Len - CountReaded))(r);
			if(!r) return CountReaded;
		}
		return STR_STAT(CountReaded, true);
	}

	template<class TypeChar>
	STR_STAT FromText(std::basic_istream<TypeChar> & Stream)
	{
		size_t CountReaded = 0;
		for(unsigned i = 0;i <  _Fields._Count;i++)
		{
			bool r;
			CountReaded += StreamDoubleToNumber(_Fields.v + i, Stream)(r);
			if(!r)
				return CountReaded;
		}
		return STR_STAT(CountReaded, true);
	}

	STR_STAT FromText(FILE * Stream = stdin)
	{
		size_t CountReaded = 0;
		for(unsigned i = 0;i <  _Fields._Count;i++)
		{
			bool r;
			CountReaded += StreamDoubleToNumber(_Fields.v + i, Stream)(r);
			if(!r)
				return CountReaded;
		}
		return STR_STAT(CountReaded, true);
	}

	/////////////

	template<class TypeChar>
	STR_STAT ToText(TypeChar * Str, size_t Len = 0xfffff)
	{
		size_t CountWrited = 0;
		for(unsigned i = 0;i <  _Fields._Count;i++)
		{
			if((i > 0) && (CountWrited < Len))
				Str[CountWrited++] = CHAR_TYPE(TypeChar, ' ');
			bool r;
			CountWrited += NumberToString(_Fields.v[i], Str + CountWrited, NOT_LESS_Z(Len - CountWrited))(r);
			if(!r)
				return CountWrited;
		}
		return STR_STAT(CountWrited, true);
	}

	template<class TypeChar>
	STR_STAT ToText(std::basic_ostream<TypeChar> & Stream)
	{
		size_t CountWrited = 0;
		for(unsigned i = 0;i <  _Fields._Count;i++)
		{
			if(i > 0)
			{
				if(!__stream_io::PutChar(Stream, CHAR_TYPE(TypeChar, ' ')))
					return CountWrited;
				CountWrited++;
			}
			bool r;
			CountWrited += NumberToStream(_Fields.v[i], Stream)(r);
			if(!r)
				return CountWrited;
		}
		return STR_STAT(CountWrited, true);
	}

	STR_STAT ToText(FILE * Stream = stdout)
	{
		size_t CountWrited = 0;
		for(unsigned i = 0;i <  _Fields._Count;i++)
		{
			if(i > 0)
			{
				if(!__stream_io::PutChar(Stream, ' '))
					return CountWrited;
				CountWrited++;
			}
			bool r;
			CountWrited += NumberToStream(_Fields.v[i], Stream)(r);
			if(!r)
				return CountWrited;
		}
		return STR_STAT(CountWrited, true);
	}

	/////////////

	inline ROW & operator =(ROW & Val)
	{
		_Fields.Copy(Val);
		return *this;
	}

	template<class _T,unsigned _j>
	inline typename std::enable_if
	<
		std::is_convertible<_T, T>::value &&
		(!IsStaticArr || !ROW<_T, _j>::IsStaticArr || (cj == _j)),
		ROW &
	>::type 
	operator =(ROW<_T, _j> & Val)
	{
		_Fields.Copy(Val);
		return *this;
	}

	template<typename _T, unsigned _j>
	inline typename std::enable_if
	<
		std::is_convertible<_T, T>::value &&
		(!IsStaticArr || (cj == _j)),
		ROW &
	>::type 
	operator =(const _T (&Val)[_j])
	{
		_Fields.Copy(Val);
		return *this;
	}

	inline void Resize(unsigned NewCount) { _Fields.Allocate(NewCount); }

	template<typename _T>
	inline typename std::enable_if<std::is_convertible<_T, T>::value>::type
	SetAllVal(_T NewVal) { std::arr_set_elements(_Fields.v, NewVal, _Fields._Count); }

	inline T & operator[](const unsigned Index)
	{
		matrix_check_index(Index < _Fields._Count, "Column out of bound");
		return _Fields.v[Index];
	}

	inline TSETTER<cj, 1> operator <<(T Val)
	{
		if(1 > _Fields._Count)
			_Fields.Allocate(1);
		(*this)[0] = Val;
		return TSETTER<cj, 1>(this);
	}

	template<typename _T, unsigned _j>
	bool operator ==
	(
		ROW<_T, _j> & Val
	)
	{
		if(_Fields._Count != Val.Count)
			return false;
		for(unsigned i = 0;i < _Fields._Count;i++)
		{
		   if(_Fields.v[i] != Val._Fields.v[i])
			   return false;
		}
		return true;
	}

	template<typename _T, unsigned _j>
	inline bool operator !=(ROW<_T, _j> & Val) { return !operator==(Val); }

	template<typename _T, unsigned _j>
	inline bool operator==(_T (&Another)[_j])
	{
		if(_Fields._Count != _j)
			return false;
		for(unsigned i = 0;i < _Fields._Count;i++)
		{
		   if(_Fields.v[i] != Another[i])
			   return false;
		}
		return true;
	}

	template<typename _T, unsigned _j>
	inline bool operator!=(_T (&Another)[_j]) { return !operator==(Another); }
};

template<class T = int, unsigned ci = 0,  unsigned cj = 0>
class MATRIX
{

	template<class, unsigned, unsigned> friend class MATRIX;
public:
	static const bool IsStaticArr = ci * cj != 0;
private:

	//Static version
	class TFIELDS_STATIC
	{
	public:
		typedef ROW<T, cj> &TROW; 

		static const unsigned ni= ci;
		static const unsigned nj = cj;
		union
		{
			T				  v[ci * cj];
			T				  v2[ci][cj];
		};

		inline MATRIX & GetM() const { return *(MATRIX*)this; }

		static inline TROW GetRow(T * pRow) { return *(ROW<T, cj>*)pRow; }

		inline T & at(unsigned i = 0, unsigned j = 0) const
		{
#ifdef _MATRIX_CHECK_INDEXES
			matrix_check_index(i < ni, "i");
			matrix_check_index(j < nj, "j");
#endif
			return v2[i][j];
		}

		inline void Allocate(const unsigned, const unsigned) const {}

		template<typename _T, unsigned _i, unsigned _j>
		inline void Copy(const MATRIX<_T, _i, _j> & Another)
		{
			matrix_check((nj == Another.CountColumns) && (ni == Another.CountRows), "Count column and row not equal.");
			std::arr_copy_cast(v, Another._Fields.v);
		}

		template <typename _T> 
		inline void Copy(const _T (&Arr)[ci][cj]) { std::arr_copy_cast(v, Arr); }

		inline unsigned SetCountColumn(const unsigned Val) const { return Val; }

		inline unsigned SetCountRow(const unsigned Val) const { return Val; }

		inline void Init() const{}

		inline void Deinit() const{}
	};

	//Dynamic version
	class TFIELDS_DYNAMIC
	{
	public:
		unsigned ni;
		unsigned nj;
		T * v;

		typedef ROW<T, unsigned(-1)> TROW;
		inline MATRIX & GetM() const { return *(MATRIX*)this; }
		inline TROW GetRow(T * pRow) { return TROW(pRow, nj); }
		inline T & at(unsigned i = 0, unsigned j = 0) const
		{
#ifdef _MATRIX_CHECK_INDEXES
			matrix_check_index(i < ni, "i");
			matrix_check_index(j < nj, "j");
#endif
			return v[i * nj + j];
		}

		inline void Allocate(unsigned Row, unsigned Col)
		{
			T * NewVal = (T*)realloc(v, sizeof(T) * Col * Row);
			matrix_check(NewVal != NULL, "Not alloc memory for matrix.");
			v = NewVal;
			nj = Col;
			ni = Row;
		}

		template<typename _T>
		inline void Copy(const MATRIX<_T, 0, 0> & Another)
		{
			Allocate(Another._Fields.ni, Another._Fields.nj);
			std::arr_copy_cast(std::valueof(v), std::valueof(Another._Fields.v), Another._Fields.ni * Another._Fields.nj);
		}

		template<typename _T, unsigned _i, unsigned _j>
		inline void Copy(const MATRIX<_T, _i, _j> & Another)
		{
			Allocate(Another._Fields.ni, Another._Fields.nj);
			std::arr_copy_cast(std::valueof(v), Another._Fields.v);
		}

		template <class _T, unsigned _i, unsigned _j> 
		inline void Copy(const _T (&Arr)[_i][_j])
		{
			Allocate(_i, _j);
			std::arr_copy_cast(std::valueof(v), Arr);
		}

		inline unsigned SetCountColumn(const unsigned Val) { return nj = Val; }
		inline unsigned SetCountRow(const unsigned Val) { return ni = Val; }
		inline void Init()
		{
			v = nullptr;
			nj = ni = 0;
		}

		inline void Deinit() { if(v != nullptr) free(v); }
	};

	typedef typename std::conditional
	<
		IsStaticArr, 
		TFIELDS_STATIC, 
		TFIELDS_DYNAMIC
	>::type TFIELDS;

#define __MATRIX_FIELDS_DEF TFIELDS _Fields

public:
	typedef T TELEMENT;
private:

	inline T & at(unsigned i = 0, unsigned j = 0)
	{
#ifdef _MATRIX_CHECK_INDEXES
	   matrix_check_index(i < _Fields.ni, "i");
	   matrix_check_index(j < _Fields.nj, "j");
#endif
	   return _Fields.v[i * _Fields.nj + j];
	}

	static inline long long gcd(long long a, long long b)
	{
		a = llabs(a);
		b = llabs(b);
		if (a*b == 0)   
			return a + b;
		if (a == b) 
			return a;
		return ((a > b)? gcd(a % b, b): gcd(a, b % a));
	}

	static inline long long lcm(long long a, long long b)
	{
		if (a*b == 0) 
			return 0;
		return (a*b) / gcd(a,b);
	}

	template<typename _T, unsigned _i, unsigned _j>
	struct TYPE_MUL_SOLUTION
	{
		typedef MATRIX
		<
			T, 
			((!IsStaticArr || !MATRIX<_T, _i, _j>::IsStaticArr)?0:ci), 
			((!IsStaticArr || !MATRIX<_T, _i, _j>::IsStaticArr)?0:_j)
		> 
		type;
	};

	class _GET_INVERSE
	{
		__MATRIX_FIELDS_DEF;
	public:
		MATRIX operator()()
		{
			matrix_check_index(_Fields.ni == _Fields.nj, "Matrix is not square");
			if((_Fields.ni == 1) && (_Fields.nj == 1))
			{
				MATRIX ret;
				ret.at() = T(1) / _Fields.at();
				return ret;
			}
			MATRIX & This = _Fields.GetM();
			T d = This.Determinant;
			matrix_check_index(d != T(0), "Determinant eq. zero");
			MATRIX RetMatrix = This.GetAdjoint();
			RetMatrix.ToTranspose();
			return RetMatrix *= (T(1) / d);
		}
	};

	class _TO_INVERSE
	{
	__MATRIX_FIELDS_DEF;
	public:
		void operator()()
		{
			matrix_check_index(_Fields.ni == _Fields.nj, "Matrix is not square");
			if((_Fields.nj == 1) && (_Fields.ni == 1))
			{
				_Fields.at() = T(1) / _Fields.at();
				return;
			}
			MATRIX & This = _Fields.GetM();
			T d = This.Determinant;
			matrix_check_index(d != T(0), "Determinant eq. zero");
			This.ToAdjoint();
			This.ToTranspose();
			This *= (T(1) / d);
		}
	};

	class _TO_TRANSPOSE
	{
		__MATRIX_FIELDS_DEF;
	public:
		void operator()()
		{
			matrix_check_index(_Fields.ni == _Fields.nj, "Matrix is not square");
			for(unsigned i = 0;i < _Fields.ni;i++)
				for(unsigned j = i;j < _Fields.nj;j++)
				{
					T temp = _Fields.at(i, j);
					_Fields.at(i, j) = _Fields.at(j, i);
					_Fields.at(j, i) = temp;
				}
		}
	};

	class _DETERMINANT
	{
		__MATRIX_FIELDS_DEF;
		template<unsigned _i>
		static T Solution(MATRIX<T, _i, _i> & Val)
		{
			if (Val.CountColumns == 1)
				return Val.at();

			MATRIX<T, std::not_less_zero<int(_i) - 1>::value, std::not_less_zero<int(_i) - 1>::value> add(Val.CountRows - 1, Val.CountColumns - 1);
			T d = T(0);
			for (unsigned i = 0; i < Val.CountColumns; ++i) 
			{
				for (unsigned y = 1; y < Val.CountColumns; y++)
					for (unsigned x = 0; x < Val.CountColumns; x++) 
					{
						if (x == i) 
							continue;
						if (x < i)
							add.at(x, y - 1) = Val.at(x, y);
						else
							add.at(x - 1, y - 1) = Val.at(x, y);
					}
				if (i % 2) 
					Val.at(i, 0) = -Val.at(i, 0);
				d += Val.at(i, 0) * Solution(add);
			}
			return d;
		}

	public:
		operator T() const
		{
			matrix_check(_Fields.ni == _Fields.nj, "Matrix is not square.");
			MATRIX This = _Fields.GetM();
			return Solution(This);
		}
	};

	class _POW_SQUARE
	{
		__MATRIX_FIELDS_DEF;
	public:
		template<typename DegreeType>
		MATRIX operator()(DegreeType n) const
		{
			matrix_check(_Fields.ni == _Fields.nj, "Matrix not square.");
			MATRIX B, A = _Fields.GetM();
			if(n < 0)
			{
				A.ToInverse();
				B = A;
			}else if(n == 0)
			{
				A.SetOnMainDiagon(1);
				return A;
			}else if(std::IsFraction(n))
			{
				T d = A.Determinant;
				if(d >= 0)
					A.SetOnMainDiagon(pow(d, n * (T(1) / A.CountRows)));
				else
				{
					matrix_check(false, "It is impossible to build a fractional power rectangular matrix.");
					return MATRIX();
				}
			}else
				B = A;
			for (unsigned r = 0, m = (abs(n) - 1); r < m; r++)
				B = A * B;
			return B;
		}
	};

	class _TO_ADJOINT
	{
		__MATRIX_FIELDS_DEF;
	public:
		void operator()()
		{
			matrix_check(_Fields.ni == _Fields.nj, "Matrix not square.");
			MATRIX<T, std::not_less_zero<(int)ci - 1>::value, std::not_less_zero<(int)cj - 1>::value> TempMatrix(_Fields.ni - 1, _Fields.nj -1);
			MATRIX Ret(_Fields.ni, _Fields.nj);
			for (unsigned i = 0; i < _Fields.ni; i++) 
				for (unsigned j = 0; j < _Fields.nj; j++) 
				{
					for (unsigned _i = 0, __i = 0; _i < _Fields.ni; _i++) 
					{
						if (_i == i) 
							continue;
						for (unsigned _j = 0, __j = 0; _j < _Fields.nj; _j++) 
						{
							if (_j == j) 
								continue;
							TempMatrix.at(__i, __j++) = _Fields.at(_i, _j); 
						}
						__i++;
					}
					Ret.at(i, j) = (((i + j) & 1)?T(-1):T(1)) * TempMatrix.Determinant;
				}
			_Fields.GetM() = Ret;
		}
	};

	class _GET_ADJOINT
	{
		__MATRIX_FIELDS_DEF;
	public:
		MATRIX operator()() //алгебраическое дополнение
		{
			matrix_check(_Fields.ni == _Fields.nj, "Matrix not square.");
			MATRIX<T, std::not_less_zero<(int)ci - 1>::value, std::not_less_zero<(int)cj - 1>::value> TempMatrix(_Fields.ni - 1, _Fields.nj -1 );
			MATRIX Ret(_Fields.ni, _Fields.nj);
			for (unsigned i = 0; i < _Fields.ni; i++) 
				for (unsigned j = 0; j < _Fields.nj; j++) 
				{
					for (unsigned _i = 0, __i = 0; _i < _Fields.ni; _i++) 
					{
						if (_i == i) 
							continue;
						for (unsigned _j = 0, __j = 0; _j < _Fields.nj; _j++) 
						{
							if (_j == j) 
								continue;
							TempMatrix.at(__i, __j++) = _Fields.at(_i, _j); 
						}
						__i++;
					}
					Ret.at(i, j) = (((i + j) & 1)?T(-1):T(1)) * TempMatrix.Determinant;
				}
			return Ret;
		}
	};

	class _GET_ALGEBRAIC_COMPLEMENT
	{
		__MATRIX_FIELDS_DEF;
	public:

		T operator()(unsigned i, unsigned j)
		{
			matrix_check_index(_Fields.ni == _Fields.nj, "Matrix is not square");
			MATRIX<T, std::not_less_zero<(int)ci - 1>::value, std::not_less_zero<(int)cj - 1>::value> TempMatrix(_Fields.ni - 1, _Fields.nj - 1);
			for (unsigned _i = 0, __i = 0; _i < _Fields.ni; _i++) 
			{
				if (_i == i) 
					continue;
				for (unsigned _j = 0, __j = 0; _j < _Fields.nj; _j++) 
				{
					if (_j == j) 
						continue;
					TempMatrix.at(__i, __j++) = _Fields.at(_i, _j); 
				}
				__i++;
			}
			return (((i + j) & 1)?T(-1):T(1)) * TempMatrix.Determinant;
		}
	};

	class _GET_SIMPLEX_MIN
	{
		__MATRIX_FIELDS_DEF;
	public:
		/*
		Parameters
		  tab - matrix for retransform
		  return: 0 - if success, 1 - is overflow limited of loops, -1 - is function not limited
		*/
		static int Solution(MATRIX & tab, unsigned MaxLoop = 20)
		{
			for(int loop = 0; loop < MaxLoop;loop++) 
			{
				int pivot_col, pivot_row;
				pivot_col = _GET_SIMPLEX_MAX::SearchMaxCol(tab);
				if(pivot_col < 0)
					return 0;
				pivot_row = _GET_SIMPLEX_MAX::SearchMinRatioRow(tab, pivot_col);
				if(pivot_row < 0) //Функция не ограничена
					return -1;
				_GET_SIMPLEX_MAX::RetransformMatrix(tab, pivot_row, pivot_col);
			}
			return 1;
		}
		MATRIX<T, ((ci == 0)?0:1), cj> operator()(bool & IsSuccess = std::make_default_reference()) 
		{
			MATRIX tab = _Fields.GetM();
			MATRIX<T, ((ci == 0)?0:1),cj> Res(1, _Fields.ni, T(0));
			if(Solution(tab) != 0)
			{
				IsSuccess = false;
				return Res;
			}
			_GET_SIMPLEX_MAX::SearchBasicVars(Res, tab);
			IsSuccess = true;
			return Res;
		}
	};

	class _GET_SIMPLEX_MAX
	{
		__MATRIX_FIELDS_DEF;

		friend _GET_SIMPLEX_MIN;
		static int SearchMinCol(MATRIX & tab)
		{
			T min = T(0);
			int pivot_col = -1;
			for(int j = 1; j < tab._Fields.nj; j++) 
				if (tab.at(0, j) < min) 
					min = tab.at(0, pivot_col = j);
			return pivot_col;
		}

		static int SearchMaxCol(MATRIX & tab)
		{
			T Max = T(0);
			int pivot_col = -1;
			for(int j = 1; j < tab._Fields.nj; j++) 
				if (tab.at(0, j) >= Max) 
					Max = tab.at(0, pivot_col = j);
			return pivot_col;
		}

		static int SearchMinRatioRow(MATRIX & tab, int column)
		{
			T minimal = 999e+100 * 999e+100 * 999e+100;
			int pivot_row = -1;
			for(int i = 1; i < tab._Fields.ni; i++) 
				if ((tab.at(i, column) >= 0.0) && (abs(tab.at(i, 0) / tab.at(i, column)) < minimal))
					minimal = abs(tab.at(i, 0) / tab.at(pivot_row = i, column));
			return pivot_row;
		}

		static void RetransformMatrix(MATRIX & tab, unsigned row, unsigned col)
		{
			long double ratio = tab.at(row, col);
			for(unsigned i = 0;i < tab._Fields.ni;i++)
			{
				T multiplier = tab.at(i, col);
				if(i != row)
				{
					for(unsigned j = 0;j < tab._Fields.nj;j++)
						tab.at(i, j) -= multiplier * tab.at(row, j) / ratio;
				}
			}
			for(unsigned i = 0;i < tab._Fields.nj;i++)
				tab.at(row, i) /= ratio;
		}

		static void SearchBasicVars(MATRIX<T, ((ci == 0)?0:1),cj> & Out, MATRIX & In)
		{
			Out.at() = In.at();
			for(unsigned j = 1, jo = 1;j < In._Fields.nj;j++)
			{
				int xi = -1;
				for(int i = 1; i < In._Fields.ni; i++) 
				{
					if (In.at(i, j) == T(1)) 
					{
						if (xi == -1)
							xi = i;
						else
							xi = -1;
					} else if(In.at(i, j) != T(0))
						xi = -1;
				}
				Out.at(0, jo++) = (xi != -1)? In.at(xi, 0):T(0);
			}
		}

	public:

		/*
		Parameters
		  @tab - matrix for retransform
		  return: 0 - if success, 1 - is overflow limited of loops, -1 - is function not limited
		*/
		static int Solution(MATRIX & tab, unsigned MaxLoop = 20)
		{
			for(unsigned loop = 0; loop < MaxLoop; loop++) 
			{
				int pivot_col, pivot_row;
				pivot_col = SearchMinCol(tab);
				if(pivot_col < 0)
					return 0;
				pivot_row = SearchMinRatioRow(tab, pivot_col);
				if(pivot_row < 0) //Function not limited
					return -1;
				RetransformMatrix(tab, pivot_row, pivot_col);	
			}
			return 1;
		}

		inline MATRIX<T, ((ci == 0)?0:1), cj> operator()() 
		{
			bool IsSuccess;
			return operator()(IsSuccess);
		}

		MATRIX<T, ((ci == 0)?0:1), cj> operator()(bool & IsSuccess) 
		{
			MATRIX tab = *(MATRIX*)this;
			MATRIX<T, ((ci == 0)?0:1),cj> Res(1, _Fields.ni, T(0));
			if(Solution(tab) != 0)
			{
				IsSuccess = false;
				return Res;
			}
			SearchBasicVars(Res, tab);
			IsSuccess = true;
			return Res;
		}
	};

	class _LU_DECOMPOSITION
	{
		__MATRIX_FIELDS_DEF;
	public:
		template
		<
			   typename uT, typename lT,
			   unsigned li, unsigned ui
		>
		typename std::enable_if
		<
		   std::is_convertible<T, uT>::value &&
		   std::is_convertible<T, lT>::value &&
		   (!IsStaticArr ||
			   (!MATRIX<lT,li, li>::IsStaticArr || (li == ci)) &&
			   (!MATRIX<uT,ui, ui>::IsStaticArr || (ui == ci))
		   )
		>::type 
		operator()(MATRIX<lT, li, li> & l, MATRIX<uT, ui, ui> & u)
		{
			matrix_check(_Fields.ni == _Fields.nj, "Matrix is not square.");
			matrix_check(l._Fields.ni == l._Fields.nj, "L matrix is not square.");
			matrix_check(u._Fields.ni == u._Fields.nj, "U matrix is not square.");
			for (unsigned i = 0; i < _Fields.ni; i++)
			{
				for (unsigned j = 0; j < _Fields.ni; j++)
				{
					if (j < i)
						l.at(j, i) = 0;
					else
					{
						l.at(j, i) = _Fields.at(j, i);
						for (unsigned k = 0; k < i; k++)
							l.at(j, i) = l.at(j, i) - l.at(j, k) * u.at(k, i);
					}
				}
				for (unsigned j = 0; j < _Fields.nj; j++)
				{
					if (j < i)
						u.at(i, j) = 0;
					else if (j == i)
						u.at(i, j) = 1;
					else
					{
						u.at(i, j) = _Fields.at(i, j) / l.at(i, i);
						for (unsigned k = 0; k < i; k++)
							u.at(i, j) = u.at(i, j) - ((l.at(i, k) * u.at(k, j)) / l.at(i, i));
					}
				}
			}
		}
	};

	class _SOLVE_LIN_EQ_WITH_ARG
	{
		__MATRIX_FIELDS_DEF;
	public:
		/*
		  Solve for linear equalation;
		  Example:
			double v[2][2] = 
			{
				  1,2,
				  4,5
			};

			double e[2][1] = 
			{
				  3,
				  6
			};

			MATRIX<double, 2, 3> x = v;
			MATRIX<double, 2, 1> x = e;
			MATRIX<double, 2, 1> solve = x.SolveLinEq(x);
			solve eq. 
			{
				-1,
				2
			}
		*/
		template<unsigned _i, unsigned _j>
		inline typename std::enable_if
		<
			!MATRIX<T, _i, _j>::IsStaticArr || 
			!IsStaticArr ||
			(ci == _i) && (_j == 1),
			MATRIX<T, ci, ((cj == 0)?0: 1)>
		>::type
		operator()(MATRIX<T, _i, _j> & RightValMatrix)
		{
			matrix_check(_Fields.GetM().IsSquare, "Matrix coefficients is not square.");
			matrix_check(RightValMatrix._Fields.ni == _Fields.ni, "Matrix \"RightValMatrix\" is not \
																  equal by count rows with matrix coefficients");
			matrix_check(RightValMatrix._Fields.nj == 1, "Matrix \"RightValMatrix\" is not \
														 equal by columns with 1");
			return _Fields.GetM().GetInverse() * RightValMatrix;
		}
	};

	class _SOLVE_LIN_EQ_WITHOUT_ARG
	{
	protected:
		__MATRIX_FIELDS_DEF;
	public:
		/*
		  Solve for linear equalation;
		  Example:
			double v[2][3] = 
			{
				  1,2,3,
				  4,5,6
			};
			MATRIX<double, 2, 3> x = v;
			MATRIX<double, 2, 1> solve = x.SolveLinEq();
			solve eq. 
			{
				-1,
				2
			}
		*/
		inline MATRIX<T, ci, ((cj == 0)?0:1)> operator()()
		{
			MATRIX & This = _Fields.GetM();
			matrix_check((_Fields.ni + 1) == _Fields.nj, "Matrix coefficients is not square.");
			MATRIX<T, ci, ci> CoefMatr(_Fields.ni, _Fields.ni);
			This.GetMiniMap(CoefMatr);
			MATRIX<T, ci, ((cj == 0)?0:1)> RightValMatrix(_Fields.ni, 1);
			This.GetMiniMap(RightValMatrix, 0, _Fields.ni);
			CoefMatr.ToInverse();
			return CoefMatr * RightValMatrix;
		}
	};

	class _SOLVE_LIN_EQ_WITHOUT_ARG_: public _SOLVE_LIN_EQ_WITH_ARG
	{
	public:
		_SOLVE_LIN_EQ_WITH_ARG::operator();
		inline MATRIX<T, ci, ((cj == 0)?0:1)> operator()()
		{
			_SOLVE_LIN_EQ_WITHOUT_ARG & This = *(_SOLVE_LIN_EQ_WITHOUT_ARG*)this;
			return This.operator()();
		}
	};
	
	class _SOLVE_LIN_EQ: 
	public std::conditional
	< 
		!IsStaticArr,
		_SOLVE_LIN_EQ_WITHOUT_ARG_,
		typename std::not_empty_if
		<
			((ci == cj) || ((ci + 1) == cj)), 
			typename std::conditional
			<
				ci == cj,
				_SOLVE_LIN_EQ_WITH_ARG,
				_SOLVE_LIN_EQ_WITHOUT_ARG
			>::type
		>::type
	>::type {};

	class _TO_ALL_MINORS
	{
		__MATRIX_FIELDS_DEF;
	public:
		inline void operator()() { _Fields.GetM() = _Fields.GetM().GetAllMinors(); }
	};

	class _GET_MINOR
	{
		__MATRIX_FIELDS_DEF;
	public:
		T operator()(unsigned i, unsigned j)
		{
			matrix_check(_Fields.ni == _Fields.nj, "Matrix is not square.");
			MATRIX<T, std::not_less_zero<(int)ci - 1>::value, std::not_less_zero<(int)cj - 1>::value> TempMatrix(_Fields.ni - 1, _Fields.nj -1);
			for (unsigned _i = 0, __i = 0; _i < _Fields.ni; _i++) 
			{
				if (_i == i) 
					continue;
				for (unsigned _j = 0, __j = 0; _j < _Fields.nj; _j++) 
				{
					if (_j == j) 
						continue;
					TempMatrix.at(__i, __j++) = _Fields.at(_i,_j); 
				}
				__i++;
			}
			return TempMatrix.Determinant;
		}
	};

	class _GET_ALL_MINORS
	{
		__MATRIX_FIELDS_DEF;
	public:
		MATRIX operator()()
		{
			matrix_check(_Fields.ni == _Fields.nj, "Matrix is not square.");
			MATRIX<T, std::not_less_zero<(int)ci - 1>::value, std::not_less_zero<(int)cj - 1>::value> TempMatrix(_Fields.ni - 1, _Fields.nj -1);
			MATRIX Ret(_Fields.ni, _Fields.nj);
			for (unsigned i = 0; i < _Fields.ni; i++) 
				for (unsigned j = 0; j < _Fields.nj; j++) 
				{
					for (unsigned _i = 0, __i = 0; _i < _Fields.ni; _i++) 
					{
						if (_i == i) 
							continue;
						for (unsigned _j = 0, __j = 0; _j < _Fields.nj; _j++) 
						{
							if (_j == j) 
								continue;
							TempMatrix.at(__i, __j++) = _Fields.at(_i, _j); 
						}
						__i++;
					}
					Ret.at(i, j) = TempMatrix.Determinant;
				}
			return Ret;
		}
	};

	//For square matrix
	class _IS_SINGULAR_SQUARE
	{
		__MATRIX_FIELDS_DEF;
	public:
		//Вырожденная ли матрица
		operator bool() { return T(0) == T(_Fields.GetM().Determinant); }
	};

	//For rect matrix
	class _IS_SINGULAR_RECT
	{
	public:
		//Вырожденная ли матрица
		operator bool() const { return false; }
	};

	//Remove
	class _REMOVE_COLUMN
	{
		__MATRIX_FIELDS_DEF;
		void RemoveCol(unsigned Pos, unsigned Count)
		{
			for (int j = 0, _j = 0, h = Pos, m = _Fields.ni * _Fields.nj; ;) 
			{		
				if(_j == h)
					_j += Count, h += _Fields.nj;

				if(_j >= m)
					return;
				_Fields.v[j++] = _Fields.v[_j++];
			}
		}

	public:
		void operator()(unsigned Pos, unsigned Count = 1)
		{
			if(Pos >= _Fields.nj)
				return;
			if((Pos + Count) >= _Fields.nj)
				Count = _Fields.nj - Pos;
			RemoveCol(Pos, Count);
			_Fields.Allocate(_Fields.ni, _Fields.nj - Count);
		}
	};

	class _REMOVE_ROW
	{		
		__MATRIX_FIELDS_DEF;

		void RemoveRow(unsigned Pos, unsigned Count)
		{
			T* selem = ((T*)_Fields.v) + Pos * _Fields.nj;
			memmove
			(
				selem, 
				selem + _Fields.nj * Count, 
				sizeof(T) * (_Fields.ni * _Fields.nj  - _Fields.nj * Count - Pos * _Fields.nj)
			);
		}
	public:
		void operator()(unsigned Pos, unsigned Count = 1)
		{
			if(Pos >= _Fields.ni)
				return;
			if((Pos + Count) >= _Fields.ni)
				Count = _Fields.ni - Pos;
			RemoveRow(Pos, Count);
			_Fields.Allocate(_Fields.ni - Count, _Fields.nj);
		}
	};

	//Insert
	class _INSERT_COLUMN
	{
		__MATRIX_FIELDS_DEF;
		void InsertCol(unsigned Pos, unsigned Count)
		{
			unsigned OldNj = _Fields.nj - Count;
			for (int i = _Fields.ni - 1; i >= 0; i--)
				memmove
				(
					&_Fields.v[i * _Fields.nj + Pos + Count], 
					&_Fields.v[i * OldNj + Pos], 
					((i == _Fields.ni - 1)?(OldNj - Pos): OldNj) * sizeof(T)
				);
		}

	public:
		void operator()(unsigned Pos, unsigned Count = 1)
		{
			if(Pos > _Fields.nj)
			{
			   Count += (Pos - _Fields.nj);
			   Pos = _Fields.nj;
			}
			_Fields.Allocate(_Fields.ni, _Fields.nj + Count);
			InsertCol(Pos, Count);
		}

		void operator()(unsigned Pos, unsigned Count, T InitVal)
		{
			if(Pos > _Fields.nj)
			{
			   Count += (Pos - _Fields.nj);
			   Pos = _Fields.nj;
			}
			_Fields.Allocate(_Fields.ni, _Fields.nj + Count);
			InsertCol(Pos, Count);
			for(unsigned i = 0;i < Count;i++)
				_Fields.GetM().SetColumnVal(Pos + i, InitVal);
		}

		template <typename ArrType>
		typename std::enable_if<std::is_convertible<ArrType, T>::value>::type
		operator()(unsigned Pos, unsigned Count, ArrType * PasteCol)
		{
			operator()(Pos, Count);
			for(unsigned j = Pos, m = j + Count, r = 0; j < m; j++)
				for(unsigned i = 0; i < _Fields.ci; i++, r++)
				   _Fields.at(i, j) = PasteCol[r];
		}
	};

	class _INSERT_ROW
	{		
		__MATRIX_FIELDS_DEF;
		void InsertRow(unsigned Pos, unsigned Count)
		{
			T* selem = ((T*)_Fields.v) + Pos * _Fields.nj;
			memmove
			(
				selem + _Fields.nj * Count, 
				selem, 
				sizeof(T) * (_Fields.ni * _Fields.nj  - _Fields.nj * Count - Pos * _Fields.nj)
			);
		}
	public:
		void operator()(unsigned Pos, unsigned Count = 1)
		{
			if(Pos > _Fields.ni)
			{
			   Count += (Pos - _Fields.ni);
			   Pos = _Fields.ni;
			}
			_Fields.Allocate(_Fields.ni + Count, _Fields.nj);
			InsertRow(Pos, Count);
		}

		void operator()(unsigned Pos, unsigned Count, T InitVal)
		{
			if(Pos > _Fields.ni)
			{
			   Count += (Pos - _Fields.ni);
			   Pos = _Fields.ni;
			}
			_Fields.Allocate(_Fields.ni + Count, _Fields.nj);
			InsertRow(Pos, Count);
			for(unsigned i = 0;i < Count;i++)
				_Fields.GetM().SetRowVal(Pos + i, InitVal);
		}

		template <typename ArrType>
		typename std::enable_if<std::is_convertible<ArrType, T>::value>::type
		operator()(unsigned Pos, unsigned Count, ArrType * PasteRow)
		{
			operator()(Pos, Count);
			for(unsigned i = Pos, m = i + Count, r = 0; i < m;i++)
				for(unsigned j = 0;j < _Fields.cj; j++, r++)
				   _Fields.at(i, j) = PasteRow[r];
		}
	};

public:
	union
	{
		__MATRIX_FIELDS_DEF;

		class
		{
			__MATRIX_FIELDS_DEF;
			friend MATRIX;
			inline unsigned operator =(unsigned Val) { return _Fields.SetCountRow(Val); }
		public:
			inline operator unsigned() const { return _Fields.ni; }
		} CountRows;

		class
		{
			__MATRIX_FIELDS_DEF;
			friend MATRIX;
			inline unsigned operator =(unsigned Val) { return _Fields.SetCountColumn(Val); }
		public:
			inline operator unsigned() const { return _Fields.nj; }
		} CountColumns;

		class
		{
			__MATRIX_FIELDS_DEF;
		public:
			operator unsigned()
			{
				MATRIX re = _Fields.GetM(); 
				re.ToRowEchelon();
				unsigned null = 0;
				for (unsigned i = 0; i < re._Fields.ni; i++) 
				{
					T sum = 0;
					for (unsigned j = 0; j < re._Fields.nj; j++)
						sum += abs(re.at(i, j));
					if (sum == 0) 
						null++;
				}
				return re.CountRows - null;
			}
		} Rank;

		class 
		{
			__MATRIX_FIELDS_DEF;
		public:
			operator bool()
			{
			   if(ci != cj)
				   return false;
			   for (unsigned i = 0; i < _Fields.ni; i++)
				   for (unsigned j = 0; j < _Fields.nj; j++) 
					   if(_Fields.at(i, j) != _Fields.at(j, i))
						   return false;
				return true;
			}
		} IsSymmetric;

		class 
		{
			__MATRIX_FIELDS_DEF;
		public:
			operator bool()
			{
			   if(_Fields.ni != _Fields.nj)
				   return false;
			   for (unsigned i = 0; i < _Fields.ni; i++)
				   for (unsigned j = 0; j < _Fields.nj; j++) 
					   if(_Fields.at(i, j) != -_Fields.at(j, i))
						   return false;
				return true;
			}
		} IsAntiSymmetric;

		class 
		{
			__MATRIX_FIELDS_DEF;
		public:
			operator bool()
			{
			   for (unsigned i = 0; i < _Fields.ni; i++)
				   for (unsigned j = 0; j < _Fields.nj; j++) 
				   {
					   if(i == j)
					   {
						  if(_Fields.at(i, j) == T(0))
							  return false;
					   }else
					   {
						   if(_Fields.at(i, j) != T(0))
							  return false;
					   }
				   }
				return true;
			}
		} IsDiagonal;

		class 
		{
			__MATRIX_FIELDS_DEF;
		public:
			//Единичная матрица
			operator bool()
			{
			   for (unsigned i = 0; i < _Fields.ni; i++)
				   for (unsigned j = 0; j < _Fields.nj; j++) 
				   {
					   if(i == j)
					   {
						  if(_Fields.at(i, j) != T(1))
							  return false;
					   }else
					   {
						   if(_Fields.at(i, j) != T(0))
							  return false;
					   }
				   }
				return true;
			}
		} IsIdentity;

		class 
		{
			__MATRIX_FIELDS_DEF;
		public:
			inline operator T()
			{
				T Res = T(0);
				for (unsigned i = 0; (i < _Fields.ni) && (i < _Fields.nj); i++)
					Res += _Fields.at(i, i);
				return Res;
			}
		} Track;

		class 
		{
			__MATRIX_FIELDS_DEF;
		public:
			operator bool() const { return _Fields.nj == _Fields.ni; }
		} IsSquare;

		class 
		{			
			__MATRIX_FIELDS_DEF;
		public:
			operator bool() { return (_Fields.GetM() * _Fields.GetM().GetTranspose()).IsIdentity; }
		} IsOrtogonal;

		typename std::not_empty_if<ci == cj, _DETERMINANT>::type			Determinant;
		typename std::not_empty_if<ci == cj, _GET_INVERSE>::type			GetInverse;
		typename std::not_empty_if<ci == cj, _TO_INVERSE>::type				ToInverse;
		typename std::not_empty_if<ci == cj, _TO_TRANSPOSE>::type			ToTranspose;
		typename std::not_empty_if<ci == cj,  _POW_SQUARE>::type			Pow;
		typename std::not_empty_if<ci == cj, _TO_ADJOINT>::type				ToAdjoint;
		typename std::not_empty_if<ci == cj, _GET_ADJOINT>::type			GetAdjoint;
		typename std::not_empty_if<ci == cj, _GET_ALGEBRAIC_COMPLEMENT>::type	GetAlgebraicComplement;
		typename std::not_empty_if<ci == cj, _LU_DECOMPOSITION>::type       LUDecomposition;
		typename std::not_empty_if<ci == cj, _TO_ALL_MINORS>::type			ToAllMinors;
		typename std::not_empty_if<ci == cj, _GET_MINOR>::type				GetMinor;
		typename std::not_empty_if<ci == cj, _GET_ALL_MINORS>::type			GetAllMinors;
		typename std::not_empty_if<!IsStaticArr, _REMOVE_ROW>::type			RemoveRow;
		typename std::not_empty_if<!IsStaticArr, _REMOVE_COLUMN>::type		RemoveCol;
		typename std::not_empty_if<!IsStaticArr, _INSERT_COLUMN>::type		InsertCol;
		typename std::not_empty_if<!IsStaticArr, _INSERT_ROW>::type			InsertRow;
		

		typename std::conditional<ci == cj, _IS_SINGULAR_SQUARE, _IS_SINGULAR_RECT >::type IsSingular;
		_SOLVE_LIN_EQ														SolveLinEq;
		/*
		Example:
			double tab[4][7]  = 
			{ // bas.  a  b  c  d  e  f
				{ 0,  -2,-3, 0, 1, 0, 0},  // F = 2a + 3b - d -> max
				{ 16,  2,-1, 0,-2, 1, 0},  // 2a - b - 2d + e = 16
				{ 18,  3, 2, 1,-3, 0, 0},  // 3a - 2b + c - 3d = 18
				{ 24, -1, 3, 0, 4, 0, 1}   // -a + 3b + 4d + f = 24
			};

			MATRIX<double, 4, 7> matr(tab); 
			bool IsSuccess; 
														 //              F           a           b       c  d        e         f
			auto Result = hdh.GetSimplexMax(IsSuccess); // Result Eq. {25.636363, 0.545454, 8.18181818, 0, 0, 23.09090909090, 0}
														// IsSuccess Eq. true
		
		*/
		_GET_SIMPLEX_MAX													GetSimplexMax; //Maximization via simplex method
		_GET_SIMPLEX_MIN													GetSimplexMin; //Minimization via simplex method
	};

public:


	void Resize(unsigned i, unsigned j)
	{
		if(IsStaticArr)
			matrix_check((ci == i) && (cj == j), "Not alloc mem for static matrix");
		_Fields.Allocate(i, j);
	}

	template<class TypeChar>
	STR_STAT ReadSize(const TypeChar * Str, size_t Len = 0xfffff)
	{

		size_t CountReaded = 0;
		MATRIX & This = *this;
		unsigned i, j;
		if(!StringToNumber(&i, Str, Len)(CountReaded))
			return CountReaded;
		bool r;
		CountReaded += StringToNumber(&j, Str + CountReaded, NOT_LESS_Z(Len - CountReaded))(r);
		if(!r)
			return CountReaded;
		Resize(i, j);
		return STR_STAT(CountReaded, true);
	}

	template<class TypeChar>
	STR_STAT ReadSize(std::basic_istream<TypeChar> & Stream)
	{
		size_t CountReaded = 0;
		MATRIX & This = *this;
		unsigned i, j;
		if(!StreamToNumber(&i, Stream)(CountReaded))
			return CountReaded;
		bool r;
		CountReaded += StreamToNumber(&j, Stream)(r);
		if(!r)
			return CountReaded;
		Resize(i, j);
		return STR_STAT(CountReaded, true);
	}
	
	STR_STAT ReadSize(FILE * Stream = stdin)
	{
		size_t CountReaded = 0;
		MATRIX & This = *this;
		unsigned i, j;
		if(!StreamToNumber(&i, Stream)(CountReaded))
			return CountReaded;
		bool r;
		CountReaded += StreamToNumber(&j, Stream)(r);
		if(!r)
			return CountReaded;
		Resize(i, j);
		return STR_STAT(CountReaded, true);
	}

	template<class TypeChar>
	STR_STAT FromText(const TypeChar * Str, size_t Len = 0xfffff, bool IsReadSize = false)
	{
		size_t CountReaded = 0;
		MATRIX & This = *this;
		if(IsReadSize)
		{
			if(!ReadSize(Str, Len)(CountReaded))
				return CountReaded;
		}
		for(unsigned i = 0;i <  _Fields.ni;i++)
		{
			bool r;
			CountReaded += This[i].FromText(Str + CountReaded, NOT_LESS_Z(Len -  CountReaded))(r);
			if(!r)
				return STR_STAT(CountReaded, false);
		}
		return STR_STAT(CountReaded, true);
	}

	template<class TypeChar>
	STR_STAT FromText(std::basic_istream<TypeChar> & Stream, bool IsReadSize = false)
	{
		size_t CountReaded = 0;
		MATRIX & This = *this;
		if(IsReadSize)
		{
			if(!ReadSize(Stream)(CountReaded))
				return CountReaded;
		}
		for(unsigned i = 0; i <  _Fields.ni; i++)
		{
			bool r;
			CountReaded += This[i].FromText(Stream)(r);
			if(!r)
				return STR_STAT(CountReaded, false);
		}
		return STR_STAT(CountReaded, true);
	}

	STR_STAT FromText(FILE * Stream = stdin, bool IsReadSize = false)
	{
		size_t CountReaded = 0;
		MATRIX & This = *this;
		if(IsReadSize)
		{
			if(!ReadSize(Stream)(CountReaded))
				return CountReaded;
		}
		for(unsigned i = 0; i <  _Fields.ni; i++)
		{
			bool r;
			CountReaded += This[i].FromText(Stream)(r);
			if(!r)
				return STR_STAT(CountReaded, false);
		}
		return STR_STAT(CountReaded, true);
	}

	/////////////

	template<class TypeChar>
	STR_STAT ToText(TypeChar * Str, size_t Len = 0xfffff, bool IsWriteSize = false)
	{
		size_t CountWrited = 0;
		MATRIX & This = *this;
		for(unsigned i = 0;i <  _Fields.ni;i++)
		{			
			if((i > 0) && (CountWrited < Len))
				Str[CountWrited++] = CHAR_TYPE(TypeChar, '\n');
			bool r;
			CountWrited += This[i].ToText(Str + CountWrited, NOT_LESS_Z(Len - CountWrited))(r);
			if(!r)
				return CountWrited;
		}
		return STR_STAT(CountWrited, true);
	}

	template<class TypeChar>
	STR_STAT ToText(std::basic_ostream<TypeChar> & Stream)
	{
		size_t CountWrited = 0;
		MATRIX & This = *this;
		for(unsigned i = 0;i <  _Fields.ni;i++)
		{
			if(i > 0)
			{
				if(!__stream_io::PutChar(Stream, CHAR_TYPE(TypeChar, '\n')))
					return CountWrited;
				CountWrited++;
			}
			bool r;
			CountWrited += This[i].ToText(Stream)(r);
			if(!r)
				return CountWrited;
		}
		return STR_STAT(CountWrited, true);
	}

	STR_STAT ToText(FILE * Stream = stdout)
	{
		size_t CountWrited = 0;
		MATRIX & This = *this;
		for(unsigned i = 0;i <  _Fields.ni;i++)
		{
			if(i > 0)
			{
				if(!__stream_io::PutChar(Stream, '\n'))
					return CountWrited;
				CountWrited++;
			}
			bool r;
			CountWrited += This[i].ToText(Stream)(r);
			if(!r)
				return CountWrited;
		}
		return STR_STAT(CountWrited, true);
	}

	inline typename TFIELDS::TROW operator[](const unsigned Index)
	{
		matrix_check_index(Index < _Fields.ni, "Row out of bound");
		return _Fields.GetRow((T*)_Fields.v + _Fields.nj * Index);
	}

	void SetAllVal(T NewVal)
	{
		for(unsigned i = 0, m = _Fields.ni * _Fields.nj;i < m;i++)
			((T*)_Fields.v)[i] = NewVal;
	}

	MATRIX() { _Fields.Init(); }

	MATRIX(unsigned ni, unsigned nj, T AllVal)
	{
		_Fields.Init();
		_Fields.Allocate(ni, nj);
		SetAllVal(AllVal);
	}

	MATRIX(unsigned ni, unsigned nj)
	{
		_Fields.Init();
		_Fields.Allocate(ni, nj);
	}

	MATRIX(unsigned N)
	{
		_Fields.Init();
		_Fields.Allocate(N, N);
	}

	MATRIX(const MATRIX & New)
	{ 
		_Fields.Init();
		_Fields.Copy(New);
	}

	template<typename _T, unsigned _i, unsigned _j>
	MATRIX
	(
		const MATRIX<_T, _i, _j> & New, 
		typename std::enable_if
		<
			std::is_convertible<_T, T>::value &&
			(!IsStaticArr || !MATRIX<_T, _i, _j>::IsStaticArr || ((ci == _i) && (cj == _j))), 
			std::empty_type
		>::type = std::empty_type()
	)
	{ 
		_Fields.Init();
		_Fields.Copy(New);
	}

	template <typename _T, unsigned _i, unsigned _j> 
	MATRIX
	(
		_T (&New)[_i][_j], 
		typename std::enable_if
		<
			std::is_convertible<_T, T>::value &&
			(!IsStaticArr || ((ci == _i) && (cj == _j))), 
			std::empty_type
		>::type = std::empty_type()
	) 
	{
		_Fields.Init();
		_Fields.Copy(New);
	}

	MATRIX & operator =(const MATRIX & New)
	{
		_Fields.Copy(New);
		return *this;
	}

	template<class _T, unsigned _i, unsigned _j>
	typename std::enable_if
	<
		std::is_convertible<_T, T>::value &&
		(!IsStaticArr || !MATRIX<_T, _i, _j>::IsStaticArr || ((ci == _i) && (cj == _j))), 
		MATRIX &
	>::type
	operator =(const MATRIX<_T, _i, _j> & New)
	{
		_Fields.Copy(New);
		return *this;
	}

	template <class _T, unsigned _i, unsigned _j> 
	typename std::enable_if
	<
		std::is_convertible<_T, T>::value &&
		(!IsStaticArr || ((ci == _i) && (cj == _j))), 
		MATRIX &
	>::type
	operator =(const _T (&New)[_i][_j]) 
	{ 
		_Fields.Copy(New);
		return *this;
	}

	~MATRIX() { _Fields.Deinit(); }

	//Mul
	template<class _T>
	typename std::enable_if<std::is_convertible<_T, T>::value, MATRIX&>::type 
	operator *=(_T Val)
	{
		for(unsigned i = 0, m = _Fields.nj * _Fields.ni;i < m;i++)
			_Fields.v[i] *= Val;
		return *this;
	}	
	
	template<class _T>
	typename std::enable_if<std::is_convertible<_T, T>::value, MATRIX>::type 
	operator *(_T Val)
	{
		MATRIX This = *this;
		for(unsigned i = 0, m = This._Fields.nj * This._Fields.ni;i < m;i++)
			This._Fields.v[i] *= Val;
		return This;
	}

	template<typename _T, unsigned _i, unsigned _j>
	inline typename std::enable_if
	<
		std::is_convertible<_T, T>::value && 
		(
			!IsStaticArr || 
			!MATRIX<_T, _i, _j>::IsStaticArr || 
			std::is_equal<typename TYPE_MUL_SOLUTION<_T, _i, _j>::type, MATRIX>::value &&
			(cj == _i)
		), 
		MATRIX&
	>::type
	operator *=(MATRIX<_T, _i, _j> & Val)
	{
		*this = operator *(Val);
		return *this;
	}

	template<class _T, unsigned _i, unsigned _j>
	inline typename std::enable_if
	<
		std::is_convertible<_T, T>::value && 
		(!IsStaticArr || !MATRIX<_T, _i, _j>::IsStaticArr || (cj == _i)),
		typename TYPE_MUL_SOLUTION<_T, _i, _j>::type
	>::type
	operator *(MATRIX<_T, _i, _j> & Val)
	{
		matrix_check(_Fields.nj == Val._Fields.ni, "Count column and row not equal.");
		MATRIX & This = *this;
		typename TYPE_MUL_SOLUTION<_T, _i, _j>::type Ret(_Fields.ni, Val._Fields.nj);
		for(unsigned i = 0;i < Ret._Fields.ni;i++)
			for(unsigned j = 0;j < Ret._Fields.nj;j++)
			{
				T Summ = T(0);
				for(unsigned k = 0;k < _Fields.nj;k++)
					Summ += at(i, k) * Val.at(k, j);
				Ret.at(i, j) = Summ;
			}
		return Ret;
	}

	//Div
	template<class _T>
	typename std::enable_if<std::is_convertible<_T, T>::value, MATRIX&>::type 
	operator /=(_T Val)
	{
		for(unsigned i = 0, m = _Fields.nj * _Fields.ni;i < m;i++)
			_Fields.v[i] /= Val;
		return *this;
	}	
	
	template<class _T>
	typename std::enable_if<std::is_convertible<_T, T>::value, MATRIX>::type 
	operator /(_T Val)
	{
		MATRIX This = *this;
		for(unsigned i = 0, m = This._Fields.nj * This._Fields.ni;i < m;i++)
			This._Fields.v[i] /= Val;
		return This;
	}

	template<typename _T, unsigned _n>
	inline typename std::enable_if
	<
		std::is_convertible<_T, T>::value && 
		(
			!IsStaticArr || 
			!MATRIX<_T, _n, _n>::IsStaticArr || 
			std::is_equal<typename TYPE_MUL_SOLUTION<_T, _n, _n>::type, MATRIX>::value &&
			(cj == _n)
		), 
		MATRIX&
	>::type
	operator /=(MATRIX<_T, _n, _n> & Val)
	{
		matrix_check(_Fields.ni == Val._Fields.nj, "Count column and row not equal.");
		MATRIX & This = *this;
		return This *= Val.GetInverse();
	}

	template<class _T, unsigned _n>
	inline typename std::enable_if
	<
		std::is_convertible<_T, T>::value && 
		(!IsStaticArr || !MATRIX<_T, _n, _n>::IsStaticArr || (cj == _n)),
		typename TYPE_MUL_SOLUTION<_T, _n, _n>::type
	>::type
	operator /(MATRIX<_T, _n, _n> & Val)
	{
		matrix_check(_Fields.ni == Val._Fields.nj, "Count column and row not equal.");
		MATRIX & This = *this;
		return This * Val.GetInverse();
	}

	//Add
	template<typename _T>
	typename std::enable_if
	<
		std::is_convertible<_T, T>::value, 
		MATRIX &
	>::type
	operator +=(_T Value)
	{
		for(unsigned i = 0, m = _Fields.nj * _Fields.ni;i < m;i++)
			_Fields.v[i] += Value;
		return *this;
	}

	template<typename _T, unsigned _i, unsigned _j>
	typename std::enable_if
	<
		std::is_convertible<_T, T>::value && 
		(!IsStaticArr || !MATRIX<_T, _i, _j>::IsStaticArr || ((ci == _i) && (cj == _j))), 
		MATRIX &
	>::type
	operator +=(MATRIX<_T, _i, _j> & Another)
	{
		matrix_check
		(
			(_Fields.nj == Another._Fields.nj) && 
			(_Fields.ni == Another._Fields.ni), 
			"Count column and row not equal."
		);
		for(unsigned i = 0, m = _Fields.nj * _Fields.ni;i < m;i++)
			_Fields.v[i] += Another._Fields.v[i];
		return *this;
	}

	template<typename _T>
	typename std::enable_if
	<
		std::is_convertible<_T, T>::value, 
		MATRIX &
	>::type
	operator +(_T Value)
	{
		MATRIX This = *this;
		for(unsigned i = 0, m = This._Fields.nj * This._Fields.ni;i < m;i++)
			This._Fields.v[i] += Value;
		return This;
	}

	template<typename _T, unsigned _i, unsigned _j>
	typename std::enable_if
	<
		std::is_convertible<_T, T>::value && 
		(!IsStaticArr || !MATRIX<_T, _i, _j>::IsStaticArr || ((ci == _i) && (cj == _j))), 
		MATRIX &
	>::type
	operator +(MATRIX<_T, _i, _j> & Another)
	{
		matrix_check
		(
			(_Fields.nj == Another._Fields.nj) && 
			(_Fields.ni == Another._Fields.ni), 
			"Count column and row not equal."
		);

		MATRIX This = *this;
		for(unsigned i = 0, m = This._Fields.nj * This._Fields.ni;i < m;i++)
			This._Fields.v[i] += Another._Fields.v[i];
		return This;
	}
	
	//Sub

	template<typename _T>
	typename std::enable_if
	<
		std::is_convertible<_T, T>::value, 
		MATRIX &
	>::type
	operator-=(_T Value)
	{
		for(unsigned i = 0, m = _Fields.nj * _Fields.ni;i < m;i++)
			_Fields.v[i] -= Value;
		return *this;
	}

	template<typename _T, unsigned _i, unsigned _j>
	typename std::enable_if
	<
		std::is_convertible<_T, T>::value && 
		(!IsStaticArr || !MATRIX<_T, _i, _j>::IsStaticArr || ((ci == _i) && (cj == _j))), 
		MATRIX &
	>::type
	operator -=(MATRIX<_T, _i, _j> & Another)
	{
		matrix_check
		(
			(_Fields.nj == Another._Fields.nj) && 
			(_Fields.ni == Another._Fields.ni), 
			"Count column and row not equal."
		);
		for(unsigned i = 0, m = _Fields.nj * _Fields.ni;i < m;i++)
			_Fields.v[i] -= Another._Fields.v[i];
		return *this;
	}

	template<typename _T>
	typename std::enable_if
	<
		std::is_convertible<_T, T>::value, 
		MATRIX &
	>::type
	operator -(_T Value)
	{
		MATRIX This = *this;
		for(unsigned i = 0, m = This._Fields.nj * This._Fields.ni;i < m;i++)
			This._Fields.v[i] -= Value;
		return This;
	}

	template<typename _T, unsigned _i, unsigned _j>
	typename std::enable_if
	<
		std::is_convertible<_T, T>::value && 
		(!IsStaticArr || !MATRIX<_T, _i, _j>::IsStaticArr || ((ci == _i) && (cj == _j))), 
		MATRIX &
	>::type
	operator -(MATRIX<_T, _i, _j> & Another)
	{
		matrix_check
		(
			(_Fields.nj == Another._Fields.nj) && 
			(_Fields.ni == Another._Fields.ni), 
			"Count column and row not equal."
		);
		MATRIX This = *this;
		for(unsigned i = 0, m = This._Fields.nj * This._Fields.ni;i < m;i++)
			This._Fields.v[i] -= Another._Fields.v[i];
		return This;
	}
	///////

	void SwapRows(unsigned r1, unsigned r2)
	{
		MATRIX & This = *this;
		for (unsigned j = 0; j < _Fields.nj; j++) 
		{
			register T tmp = at(r2, j);
			at(r2, j) = at(r1, j);
			at(r1, j) = tmp;
		}
	}

	void SwapColumns(unsigned r1, unsigned r2)
	{
		for (unsigned i = 0; i < _Fields.ni; i++) 
		{
			register T tmp = at(i, r2);
			at(i, r2) = at(i, r1);
			at(i, r2) = tmp;
		}
	}

	void ToTriangle()
	{
		unsigned MinNull0 = _Fields.nj, MinNull1 = 0;
		for (unsigned k = 0; k < (_Fields.ni - 1); k++) 
		{
			MinNull0 = _Fields.nj;
			for (unsigned v = k; v < _Fields.ni; v++) 
			{
				unsigned CountNull = 0;
				for (;(CountNull < _Fields.ni) && (at(v, CountNull) == 0); CountNull++);
				if (CountNull < MinNull0)
				{
					MinNull0 = CountNull;
					MinNull1 = v;
				}
			}
			if (k != MinNull1) 
				SwapRows(k, MinNull1);
		}
		if (at() == 0) 
			return;
		for (unsigned j = 0; j < _Fields.ni; j++)
			for (unsigned u = (j + 1); u < _Fields.nj; u++) 
			{
				if (at(u, j) == 0) 
					continue;
				T M = -(at(u, j) / at(j, j));
				for (unsigned k = 0; k < _Fields.ni; k++)
					at(u, k) += (at(j, k) * M);
			}
	}

	MATRIX GetRowEchelon()
	{
	   MATRIX This = *this;
	   This.ToRowEchelon();
	   return This;
	}

	/*
		Ступенчатый вид по строкам
	*/
	void ToRowEchelon()
	{
		int  l, factor1;
		for (unsigned i = 0, r = 0, k; i < min(_Fields.nj, _Fields.ni); i++)  
		{
			if (at(r, i) == 0) 
			{
				for (k = r + 1; k < _Fields.ni; k++) 
					if (at(k, i) != 0) 
						break;
				if (k < _Fields.ni)   
					SwapRows(r, k);
			}
			if (at(r, i) != 0) 
			{
				for (k = r + 1; k < _Fields.ni; k++) 
				{
					if (at(k, i) == 0) 
						continue;
					l = lcm(at(k, i), at(r, i));
					factor1 = l / at(k, i);
					for (unsigned j = i; j < _Fields.nj; j++) 
						at(k, j) = at(k, j) * factor1 - at(r, j) * (l / at(r, i));
				}
				r++;
			}
		}
	}

	MATRIX GetTriangle() const
	{
		MATRIX A = *this;
		A.ToTriangle();
		return A;
	}

	template<typename _T, unsigned _i, unsigned _j>
	typename std::enable_if<std::is_convertible<T, _T>::value>::type
	GetMiniMap(MATRIX<_T, _i, _j> & Dest, unsigned StartRow = 0, unsigned StartColumn = 0)
	{
		for(unsigned i = StartRow, ie = 0; (i < _Fields.ni) && (ie < Dest._Fields.ni);i++, ie++)
			for(unsigned j = StartColumn, je = 0;(j < _Fields.nj) && (je < Dest._Fields.nj);j++, je++)
				Dest.at(ie, je) = at(i, j);
	}

	template<typename _T, unsigned _i, unsigned _j>
	typename std::enable_if<std::is_convertible<T, _T>::value>::type
	GetMiniMap(_T (&Dest)[_i][_j], unsigned StartRow = 0, unsigned StartColumn = 0)
	{
		GetMiniMap((MATRIX<_T, _i, _j>&)Dest, StartRow, StartColumn);
	}

	template<typename _T, unsigned _i, unsigned _j>
	typename std::enable_if<std::is_convertible<_T, T>::value>::type
	SetMiniMap(MATRIX<_T, _i, _j> & Source, unsigned StartRow = 0, unsigned StartColumn = 0)
	{
		for(unsigned i = StartRow, ie = 0; (i < _Fields.ni) && (ie < Source._Fields.ni);i++, ie++)
			for(unsigned j = StartColumn, je = 0;(j < _Fields.nj) && (je < Source._Fields.nj);j++, je++)
				at(i, j) = Source.at(ie, je);
	}

	template<typename _T, unsigned _i, unsigned _j>
	typename std::enable_if<std::is_convertible<_T, T>::value>::type
	SetMiniMap(_T (&Source)[_i][_j], unsigned StartRow = 0, unsigned StartColumn = 0)
	{
		SetMiniMap((MATRIX<_T, _i, _j>&)Source, StartRow, StartColumn);
	}

	MATRIX<T, cj, ci> GetTranspose()
	{
		MATRIX & This = *this;
		MATRIX<T, cj, ci> NewMatrix(_Fields.nj, _Fields.ni);
		for(unsigned i = 0; i < _Fields.ni; i++)
		{
			for(unsigned j = 0;j < _Fields.nj;j++)
				NewMatrix.at(j, i) = This.at(i, j);
		}
		return NewMatrix;
	}

	void SetOnMainDiagon(T Val)
	{
		for(unsigned i = 0;i < _Fields.ni;i++)
			for(unsigned j = 0;j < _Fields.nj;j++)
					at(i, j) = (i == j)?Val:T(0);
	}

	void SetOnSecDiagon(T Val)
	{
		for(unsigned i = _Fields.ni - 1;(int)i >= 0;i--)
			for(unsigned j = _Fields.nj - 1;(int)j >= 0;j--)
				at(i, j) = (i == j)?Val:T(0);
	}

	void SetColumnVal(unsigned IndexColumn, T Val)
	{
		for(unsigned i = 0;i < _Fields.ni;i++)
			at(i, IndexColumn) = Val;
	}

	void SetRowVal(unsigned IndexRow, T Val)
	{
		for(unsigned j = 0;j < _Fields.nj;j++)
			at(IndexRow, j) = Val;
	}


	template<typename _T, unsigned _i, unsigned _j>
	bool operator==(MATRIX<_T, _i, _j> & Another)
	{
	   if(
		   !std::is_convertible<T, _T>::value || 
		   (_Fields.ni != Another._Fields.ni) || 
		   (_Fields.nj != Another._Fields.nj)
		 )
		   return false;
	   for(unsigned i = 0, m = _Fields.nj * _Fields.ni;i < m;i++)
			if(_Fields.v[i] != Another._Fields.v[i])
				return false;
	   return true;
	}

	template<typename _T, unsigned _i, unsigned _j>
	bool operator==(_T (&Another)[_i][_j])
	{
	   if(
		   !std::is_convertible<T, _T>::value || 
		   (_Fields.ni != _i) || 
		   (_Fields.nj != _j)
		 )
		   return false;
	   for(unsigned i = 0, m = _i * _j;i < m;i++)
			if(_Fields.v[i] != ((_T*)Another)[i])
				return false;
	   return true;
	}

	template<typename _T, unsigned _i, unsigned _j>
	inline bool operator!=(MATRIX<_T, _i, _j> & Another) { return !operator==(Another); }

	template<typename _T, unsigned _i, unsigned _j>
	inline bool operator!=(_T (&Another)[_i][_j]) { return !operator==(Another); }
};


#endif
