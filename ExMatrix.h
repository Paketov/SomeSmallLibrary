#ifndef __EX_MATRIX_H__
#define __EX_MATRIX_H__

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
	static const bool IsStaticArr = cj != 0;

	class FIELD_DYNAMIC
	{
	public:
		unsigned _Count;
		T * v;
		inline void SetV(T * Val)
		{   
			v = Val;
		}

		inline unsigned SetCount(const unsigned Val)
		{   
			return _Count = Val;
		}
	};

	class FIELD_STATIC
	{
	public:
		static const unsigned _Count = cj;
		T v[cj];

		inline void SetV(T * Val) const{}

		inline unsigned SetCount(const unsigned Val) const
		{   
			return Val;
		}
	};

	typedef typename std::conditional<IsStaticArr, FIELD_STATIC, FIELD_DYNAMIC>::type TFIELDS;

#define	__ROW_FIELDS \
	TFIELDS _Fields;

	template<class, unsigned, unsigned> friend class MATRIX;

public:
	union
	{
		__ROW_FIELDS;

		class
		{
			__ROW_FIELDS;
		public:
			inline operator unsigned()
			{
				return _Fields._Count;
			}
		} Count;
	};
private:
	ROW(T * Column, unsigned Count)
	{
		_Fields.SetCount(Count);
		_Fields.SetV(Column);
	}

	template <unsigned Index = 0>
	class __SETTER
	{
		ROW * r;
	public:
		inline __SETTER(ROW * r)
		{
			this->r = r;
		}

		inline typename std::enable_if
		<
			!IsStaticArr || (Index < cj), 
			__SETTER<Index + 1> 
		>::type 
		operator <<(T Val)
		{
			(*r)[Index] = Val;
			return __SETTER<Index + 1>(r);
		}
	};

public: 

	//////
	template<class TypeChar>
	STR_STAT FromText(const TypeChar * Str, size_t Len = 0xfffff)
	{
		size_t CountReaded = 0;
		for(unsigned i = 0;i <  _Fields._Count;i++)
		{		
			bool r;
			CountReaded += StringDoubleToNumber(_Fields.v + i, Str + CountReaded, NOT_LESS_Z(Len - CountReaded))(r);
			if(!r)
				return CountReaded;
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

	///////////

	inline ROW & operator =(ROW & Val)
	{
		matrix_check(Val._Fields._Count == _Fields._Count, "Column count not equal");
		if(IsStatic)
			std::val_copy(_Fields.v, Val._Fields.v);
		else
			memcpy(_Fields.v, Val._Fields.v, sizeof(T) * _Fields._Count);
		return *this;
	}

	template<unsigned _j>
	inline typename std::enable_if
	<
		!IsStaticArr || !ROW<T, _j>::IsStaticArr || (cj == _j), 
		ROW &
	>::type 
	operator =(ROW<T, _j>& Val)
	{
		matrix_check(Val._Fields._Count == _Fields._Count, "Column count not equal");
		memcpy(_Fields.v, Val._Fields.v, sizeof(T) * _Fields._Count);
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
		matrix_check(Val._Fields._Count  == _Fields._Count, "Column count not equal");
		for(unsigned i = 0;i < _Fields._Count; i++)
			_Fields.v[i] = (T)Val._Fields.v[i];
		return *this;
	}

	template<unsigned _j>
	inline typename std::enable_if
	<
		!IsStaticArr || (cj == _j),
		ROW &
	>::type 
	operator =(const T (&New)[_j])
	{
		matrix_check_index(_j == _Fields._Count, "In array size not equal column count");
	   std::val_copy(_Fields.v, New);
	   return *this;
	}

	template<typename _T, unsigned _j>
	inline typename std::enable_if
	<
		std::is_convertible<_T, T>::value &&
		(!IsStaticArr || (cj == _j)),
		ROW &
	>::type 
	operator =(const _T (&New)[_j])
	{
		matrix_check_index(_j == _Fields._Count, "In array size not equal column count");
		for(unsigned i = 0;i < _j;i++)
			_Fields.v[i] = New[i];
		return *this;
	}

	inline T & operator[](const unsigned Index)
	{
		matrix_check_index(Index < _Fields._Count, "Column out of bound");
		return _Fields.v[Index];
	}

	inline __SETTER<1> operator <<(T Val)
	{
		(*this)[0] = Val;
		return __SETTER<1>(this);
	}
};

template<class T = int, unsigned ci = 0,  unsigned cj = 0>
class MATRIX
{

	template<class, unsigned, unsigned> friend class MATRIX;
public:
	static const bool IsStaticArr = ci * cj != 0;

private:

#define __MATRIX_FIELDS_DEF TFIELDS _Fields;

	class FIELD_DYNAMIC
	{
	public:
		unsigned ni;
		unsigned nj;
		T * v;

		inline T & at(unsigned i = 0, unsigned j = 0)
		{
#ifdef _MATRIX_CHECK_INDEXES
			matrix_check_index(i < ni, "i");
			matrix_check_index(j < nj, "j");
#endif
			return v[i * nj + j];
		}

		inline bool Allocate(const unsigned Row, const unsigned Col)
		{
			T * NewVal = (T*)realloc(v, sizeof(T) * Col * Row);
			if(NewVal == NULL)
				return false;
			v = NewVal;
			return true;
		}

		inline bool Copy(const MATRIX<T, 0, 0> & Enother)
		{
			T * NewVal = (T*)realloc(v, sizeof(T) * Enother.CountColumns * Enother.CountRows);
			if(NewVal == NULL)
				return false;
			nj = Enother._Fields.nj;
			ni = Enother._Fields.ni;
			v = NewVal;
			memcpy(v, Enother._Fields.v, sizeof(T) * Enother.CountColumns * Enother.CountRows);
			return true;
		}

		template<unsigned _i, unsigned _j>
		inline bool Copy(const MATRIX<T, _i, _j> & Enother)
		{
			T * NewVal = (T*)realloc(v, sizeof(T) * Enother.CountColumns * Enother.CountRows);
			if(NewVal == NULL)
				return false;
			nj = Enother._Fields.nj;
			ni = Enother._Fields.ni;
			v = NewVal;
			std::val_copy(*v, Enother._Fields.v);
			return true;
		}

		template<class _T, unsigned _i, unsigned _j>
		inline typename std::enable_if
		<
			std::is_convertible<_T, T>::value, 
			bool
		>::type Copy(const MATRIX<_T, _i, _j> & Enother)
		{
			unsigned  m = Enother.CountColumns * Enother.CountRows;
			T * NewVal = (T*)realloc(v, sizeof(T) * m);
			if(NewVal == NULL)
				return false;
			nj = Enother._Fields.nj;
			ni = Enother._Fields.ni;
			v = NewVal;
			for(unsigned i = 0;i < m;i++)
				((T*)v)[i] = ((_T*)Enother._Fields.v)[i];
			return true;
		}

		template <unsigned _i, unsigned _j> 
		inline bool Copy(const T (&Enother)[_i][_j])
		{
			T * NewVal = (T*)realloc(v, sizeof(T) * _i * _j);
			if(NewVal == NULL)
				return false;
			nj = _j;
			ni = _i;
			v = NewVal;
			std::val_copy(*v, Enother);
			return true;
		}

		template <class _T, unsigned _i, unsigned _j> 
		inline typename std::enable_if
		<
			std::is_convertible<_T, T>::value, 
			bool
		>::type 
		Copy(const _T (&Enother)[_i][_j])
		{
			T * NewVal = (T*)realloc(v, sizeof(T) * _i * _j);
			if(NewVal == NULL)
				return false;
			nj = _j;
			ni = _i;
			v = NewVal;
			for(unsigned i = 0;i < (_i * _j);i++)
				((T*)v)[i] = ((const _T*)Enother)[i];
			return true;
		}

		inline unsigned SetCountColumn(const unsigned Val)
		{   
			return nj = Val;
		}

		inline unsigned SetCountRow(const unsigned Val)
		{   
			return ni = Val;
		}

		inline void Init()
		{
			v = NULL;
		}


		inline void Deinit()
		{
			if(v != NULL)
				free(v);
		}
	};

	class FIELD_STATIC
	{
	public:
		static const unsigned ni= ci;
		static const unsigned nj = cj;
		union
		{
			T				  v[ci * cj];
			T				  v2[ci][cj];
		};

		inline T & at(unsigned i = 0, unsigned j = 0)
		{
#ifdef _MATRIX_CHECK_INDEXES
			matrix_check_index(i < ni, "i");
			matrix_check_index(j < nj, "j");
#endif
			return v2[i][j];
		}

		inline bool Allocate(const unsigned, const unsigned) const
		{
			return true;
		}

		inline bool Copy(const MATRIX<T, ci, cj> & Enother)
		{
			std::val_copy(v, Enother._Fields.v);
			return true;
		}

		template<unsigned _i, unsigned _j>
		inline bool Copy(const MATRIX<T, _i, _j> & Enother)
		{
			typedef typename std::enable_if<false>::type;
			return true;
		}

		template<>
		inline bool Copy(const MATRIX<T, 0, 0> & Enother)
		{
			matrix_check((nj == Enother.CountColumns) && (ni == Enother.CountRows), "Count column and row not equal.");
			std::val_copy(v, *Enother._Fields.v);
			return true;
		}

		template<class _T, unsigned _i, unsigned _j>
		inline typename std::enable_if
		<
			(ci == _i) && (cj == _j) && 
			std::is_convertible<_T, T>::value,
			bool
		>::type 
		Copy(const MATRIX<_T, _i, _j> & Enother)
		{
			matrix_check((nj == Enother.CountColumns) && (ni == Enother.CountRows), "Count column and row not equal.");
			for(unsigned i = 0, m = nj * ni;i < m;i++)
				((T*)v)[i] = ((_T*)Enother._Fields.v)[i];
			return true;
		}

		template <unsigned _i, unsigned _j> 
		inline typename std::enable_if
		<
			(ci == _i) && (cj == _j), 
			bool
		>::type 
		Copy(const T (&Enother)[_i][_j])
		{
			std::val_copy(v, Enother);
			return true;
		}

		template <class _T, unsigned _i, unsigned _j> 
		inline typename std::enable_if
		<
			(ci == _i) && (cj == _j) && std::is_convertible<_T, T>::value, 
			bool
		>::type
		Copy(const _T (&Enother)[_i][_j])
		{
			for(unsigned i = 0;i < (_i * _j);i++)
				((T*)v)[i] = ((const _T*)Enother)[i];
			return true;
		}

		inline unsigned SetCountColumn(const unsigned Val) const
		{   
			return Val;
		}

		inline unsigned SetCountRow(const unsigned Val) const
		{   
			return Val;
		}

		inline void Init() const{}

		inline void Deinit() const{}
	};
public:
	typedef T TELEMENT;

	typedef typename std::conditional<IsStaticArr, FIELD_STATIC, FIELD_DYNAMIC>::type TFIELDS;
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
	class PARENT_FIELDS
	{
	protected:
	   __MATRIX_FIELDS_DEF;
	};

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

	class _GET_INVERSE: PARENT_FIELDS
	{
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
			MATRIX & This = *(MATRIX*)this;
			T d = This.Determinant;
			matrix_check_index(d != T(0), "Determinant eq. zero");
			MATRIX RetMatrix = This.GetAdjoint();
			RetMatrix.ToTranspose();
			return RetMatrix *= (T(1) / d);
		}
	};

	class _TO_INVERSE: PARENT_FIELDS
	{
	public:
		void operator()()
		{
			matrix_check_index(_Fields.ni == _Fields.nj, "Matrix is not square");
			if((_Fields.nj == 1) && (_Fields.ni == 1))
			{
				_Fields.at() = T(1) / _Fields.at();
				return;
			}
			MATRIX & This = *(MATRIX*)this;
			T d = This.Determinant;
			matrix_check_index(d != T(0), "Determinant eq. zero");
			This.ToAdjoint();
			This.ToTranspose();
			This *= (T(1) / d);
		}
	};

	class _TO_TRANSPOSE: PARENT_FIELDS
	{
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

	class _DETERMINANT: PARENT_FIELDS
	{
		template<unsigned _i>
		static T Solution(MATRIX<T, _i, _i> & Val)
		{
			if (Val.CountColumns == 1)
				return Val.at();

			MATRIX<T, max(0, (int)_i - 1), max(0, (int)_i - 1)> add(Val.CountRows - 1, Val.CountColumns - 1);
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
			MATRIX This = *(MATRIX*)this;
			return Solution(This);
		}
	};

	class _POW_SQUARE: PARENT_FIELDS
	{
	public:
		template<typename DegreeType>
		MATRIX operator()(DegreeType n) const
		{
			matrix_check(_Fields.ni == _Fields.nj, "Matrix not square.");
			MATRIX B, A = *(MATRIX*)this;
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

	class _TO_ADJOINT: PARENT_FIELDS
	{
	public:
		void operator()()
		{
			matrix_check(_Fields.ni == _Fields.nj, "Matrix not square.");
			MATRIX<T, max(0, (int)ci - 1), max(0, (int)cj - 1)> TempMatrix(_Fields.ni - 1, _Fields.nj -1);
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
			*(MATRIX*)this = Ret;
		}
	};

	class _GET_ADJOINT: PARENT_FIELDS
	{
	public:
		MATRIX operator()() //алгебраическое дополнение
		{
			matrix_check(_Fields.ni == _Fields.nj, "Matrix not square.");
			MATRIX<T, max(0, (int)ci - 1), max(0, (int)cj - 1)> TempMatrix(_Fields.ni - 1, _Fields.nj -1 );
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

	class _GET_ALGEBRAIC_COMPLEMENT: PARENT_FIELDS
	{
	public:

		T operator()(unsigned i, unsigned j)
		{
			matrix_check_index(_Fields.ni == _Fields.nj, "Matrix is not square");
			MATRIX<T, max(0, (int)ci - 1), max(0, (int)cj - 1)> TempMatrix(_Fields.ni - 1, _Fields.nj - 1);
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

	class _GET_SIMPLEX_MIN: PARENT_FIELDS
	{
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
			_GET_SIMPLEX_MAX::SearchBasicVars(Res, tab);
			IsSuccess = true;
			return Res;
		}
	};

	class _GET_SIMPLEX_MAX: PARENT_FIELDS
	{

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
			T max = T(0);
			int pivot_col = -1;
			for(int j = 1; j < tab._Fields.nj; j++) 
				if (tab.at(0, j) >= max) 
					max = tab.at(0, pivot_col = j);
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
			for(unsigned loop = 0; loop < MaxLoop;loop++) 
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

	class _LU_DECOMPOSITION: PARENT_FIELDS
	{
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

	class _SOLVE_LIN_EQ_WITH_ARG: public PARENT_FIELDS
	{
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
			MATRIX & This = *(MATRIX*)this;
			matrix_check(This.IsSquare, "Matrix coefficients is not square.");
			matrix_check(RightValMatrix._Fields.ni == _Fields.ni, "Matrix \"RightValMatrix\" is not \
																  equal by count rows with matrix coefficients");
			matrix_check(RightValMatrix._Fields.nj == 1, "Matrix \"RightValMatrix\" is not \
														 equal by columns with 1");
			return This.GetInverse() * RightValMatrix;
		}
	};

	class _SOLVE_LIN_EQ_WITHOUT_ARG: public PARENT_FIELDS
	{
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
			MATRIX & This = *(MATRIX*)this;
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
	>::type
	{
	};

	class _TO_ALL_MINORS: PARENT_FIELDS
	{
	public:
		inline void operator()()
		{
			MATRIX & This = *(MATRIX*)this;
			This = This.GetAllMinors();
		}
	};

	class _GET_MINOR : PARENT_FIELDS
	{
	public:
		T operator()(unsigned i, unsigned j)
		{
			matrix_check(_Fields.ni == _Fields.nj, "Matrix is not square.");
			MATRIX<T, max(0, (int)ci - 1), max(0, (int)cj - 1)> TempMatrix(_Fields.ni - 1, _Fields.nj -1);
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

	class _GET_ALL_MINORS : PARENT_FIELDS
	{
	public:
		MATRIX operator()()
		{
			matrix_check(_Fields.ni == _Fields.nj, "Matrix is not square.");
			MATRIX<T, max(0, (int)ci - 1), max(0, (int)cj - 1)> TempMatrix(_Fields.ni - 1, _Fields.nj -1);
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

	class _IS_SINGULAR_SQUARE : PARENT_FIELDS
	{
	public:
		//Вырожденная ли матрица
		operator bool()
		{
			MATRIX & This = *(MATRIX*)this;
			return T(0) == T(This.Determinant);
		}
	};

	class _IS_SINGULAR_RECT
	{
	public:
		//Вырожденная ли матрица
		operator bool()
		{
			return false;
		}
	};

public:
	union
	{
		__MATRIX_FIELDS_DEF;
		class
		{
			__MATRIX_FIELDS_DEF;
		public:
			inline operator unsigned() const
			{
				return _Fields.ni;
			}

			inline unsigned operator =(unsigned Val)
			{
				return _Fields.SetCountRow(Val);
			}

		} CountRows;

		class
		{
			__MATRIX_FIELDS_DEF;
		public:
			inline operator unsigned() const
			{
				return _Fields.nj;
			}

			inline unsigned operator =(unsigned Val)
			{
				return _Fields.SetCountColumn(Val);
			}

		} CountColumns;

		class
		{
			__MATRIX_FIELDS_DEF;
		public:
			operator unsigned()
			{
				MATRIX re = *(MATRIX*)this; 
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
		} IsAntySymmetric;

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
			operator bool()
			{
				return _Fields.nj == _Fields.ni;
			}
		} IsSquare;

		class 
		{			
			__MATRIX_FIELDS_DEF;
		public:
			operator bool()
			{
				MATRIX & This = *(MATRIX*)this;
				return (This * This.GetTranspose()).IsIdentity;
			}
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


	bool Resize(unsigned i, unsigned j)
	{
		if(IsStaticArr)
			return (ci == i) && (cj == j);
		CountRows = i;
		CountColumns = j;
		return _Fields.Allocate(i, j);
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
		return STR_STAT(CountReaded, Resize(i, j));
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
		return STR_STAT(CountReaded, Resize(i, j));
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
		return STR_STAT(CountReaded, Resize(i, j));
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

	inline typename std::conditional
	<
		IsStaticArr,
		ROW<T, cj> &, 
		ROW<T, cj>
	>::type 
	operator[](const unsigned Index)
	{
		matrix_check_index(Index < _Fields.ni, "Row out of bound");
		if(IsStaticArr)
			return *(ROW<T, cj>*)((T*)_Fields.v + _Fields.nj * Index);
		else
			return ROW<T, cj>((T*)_Fields.v + _Fields.nj * Index, _Fields.nj); 
	}

	void SetAllVal(T NewVal)
	{
		for(unsigned i = 0, m = CountRows * CountColumns;i < m;i++)
			((T*)_Fields.v)[i] = NewVal;
	}

	MATRIX()
	{
		_Fields.Init();
		CountColumns = cj;
		CountRows = ci;
	}

	MATRIX(unsigned ni, unsigned nj, T AllVal)
	{
		_Fields.Init();
		CountColumns = nj;
		CountRows = ni;
		matrix_check(_Fields.Allocate(ni, nj), "Not alloc memory");
		SetAllVal(AllVal);
	}

	MATRIX(unsigned ni, unsigned nj)
	{
		_Fields.Init();
		CountColumns = nj;
		CountRows = ni;
		matrix_check(_Fields.Allocate(ni, nj), "Not alloc memory");
	}

	MATRIX(unsigned N)
	{
		_Fields.Init();
		CountRows = CountColumns = N;
		matrix_check(_Fields.Allocate(N, N), "Not alloc memory");
	}

	MATRIX(const MATRIX & New)
	{ 
		_Fields.Init();
		matrix_check(_Fields.Copy(New),  "Matrix not copied");
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
		>::type _d = std::empty_type()
	)
	{ 
		_Fields.Init();
		matrix_check(_Fields.Copy(New),  "Matrix not copied");
	}


	template <unsigned _i, unsigned _j> 
	MATRIX
	(
		T (&New)[_i][_j], 
		typename std::enable_if
		<
			!IsStaticArr || ((ci == _i) && (cj == _j)), 
			std::empty_type
		>::type _d = std::empty_type()
	)
	{ 
		_Fields.Init();
		matrix_check(_Fields.Copy(New),  "Matrix not copied");
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
		>::type _d = std::empty_type()
	) 
	{
		_Fields.Init();
		matrix_check(_Fields.Copy(New),  "Matrix not copied");
	}

	MATRIX & operator =(const MATRIX & New)
	{
		matrix_check(_Fields.Copy(New),  "Matrix not copied");
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
		matrix_check(_Fields.Copy(New),  "Matrix not copied");
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
		matrix_check(_Fields.Copy(New),  "Matrix not copied");
	}

	~MATRIX()
	{
		_Fields.Deinit();
	}

	void RemoveCol(unsigned Pos)
	{
		CountColumns = CountColumns - 1;
		for (int j = Pos; j < _Fields.nj; j++) 
		{
			for (int i = 0; i < _Fields.ni; i++) 
				at(i, j) = at(i, j+1);
		}
	}

	//typename std::enable_if<((ci * cj) == 0)>::type 
	void RemoveRow(unsigned Pos)
	{
		CountRows = CountRows - 1;
		for (int i = Pos; i < _Fields.ni; i++) 
		{
			for (int j = 0; j < _Fields.nj; j++)
				at(i, j) = at(i+1, j);
		}
	}

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
		return *this
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
		for(unsigned i = StartRow, _i = 0; (i < _Fields.ni) && (_i < Dest._Fields.ni);i++, _i++)
			for(unsigned j = StartColumn, _j = 0;(j < _Fields.nj) && (_j < Dest._Fields.nj);j++, _j++)
				Dest.at(_i, _j) = at(i, j);
	}	


	MATRIX<T, cj, ci> GetTranspose()
	{
		MATRIX & This = *this;
		MATRIX<T, cj, ci> NewMatrix(_Fields.nj, _Fields.ni);
		for(unsigned i = 0;i < _Fields.ni;i++)
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

	template<typename _T, unsigned _i, unsigned _j>
	bool operator==(MATRIX<_T, _i, _j> & Enother)
	{
	   if(
		   !std::is_convertible<T, _T>::value || 
		   (_Fields.ni != Enother._Fields.ni) || 
		   (_Fields.nj != Enother._Fields.nj)
		 )
		   return false;
	   for(unsigned i = 0, m = _Fields.nj * _Fields.ni;i < m;i++)
			if(_Fields.v[i] != Enother._Fields.v[i])
				return false;
	   return true;
	}

	template<typename _T, unsigned _i, unsigned _j>
	bool operator==(_T (&Enother)[_i][_j])
	{
	   if(
		   !std::is_convertible<T, _T>::value || 
		   (_Fields.ni != _i) || 
		   (_Fields.nj != _j)
		 )
		   return false;
	   for(unsigned i = 0, m = _i * _j;i < m;i++)
			if(_Fields.v[i] != ((_T*)Enother)[i])
				return false;
	   return true;
	}

	template<typename _T, unsigned _i, unsigned _j>
	inline bool operator!=(MATRIX<_T, _i, _j> & Enother)
	{
	   return !operator==(Enother);
	}

	template<typename _T, unsigned _i, unsigned _j>
	inline bool operator!=(_T (&Enother)[_i][_j])
	{
	   return !operator==(Enother);
	}
};


#endif
