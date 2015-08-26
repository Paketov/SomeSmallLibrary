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

inline long long gcd(long long a, long long b)
{
	a = llabs(a);
	b = llabs(b);
	if (a*b == 0)   
		return a + b;
	if (a == b) 
		return a;
	return ((a > b)? gcd(a % b, b): gcd(a, b % a));
}

inline long long lcm(long long a, long long b)
{
	if (a*b == 0) 
		return 0;
	return (a*b) / gcd(a,b);
}



//Retrive, row as multi reference

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
			std::ValCopy(_Fields.v, Val._Fields.v);
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
	   std::ValCopy(_Fields.v, New);
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
			std::ValCopy(*v, Enother._Fields.v);
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
			std::ValCopy(*v, Enother);
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
		T					  v[ci][cj];

		inline bool Allocate(const unsigned, const unsigned) const
		{
			return true;
		}

		inline bool Copy(const MATRIX<T, ci, cj> & Enother)
		{
			std::ValCopy(v, Enother._Fields.v);
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
			std::ValCopy(v, *Enother._Fields.v);
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
			std::ValCopy(v, Enother);
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
			MATRIX & This = *(MATRIX*)this;
			if((This._Fields.ni == 1) && (This._Fields.nj == 1))
				return 1 / This[0][0];
			matrix_check_index(This._Fields.ni == This._Fields.nj, "Matrix is not square");
			T d = This.Determinant;
			matrix_check_index(d != T(0), "Determinant eq. zero");
			MATRIX RetMatrix(This._Fields.ni, This._Fields.nj);
			RetMatrix = This.GetAdjoint();
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
			MATRIX & This = *(MATRIX*)this;
			if((This._Fields.nj == 1) && (This._Fields.ni == 1))
			{
				This[0][0] = 1 / This[0][0];
				return;
			}
			matrix_check_index(This._Fields.ni == This._Fields.nj, "Matrix is not square");
			T d = This.Determinant;
			matrix_check_index(d != T(0), "Determinant eq. zero");
			This.ToAdjoint();
			This.ToTranspose();
			This *= ((T)1 / d);
		}
	};

	class _TO_TRANSPOSE
	{
		__MATRIX_FIELDS_DEF;
	public:
		void operator()()
		{
			MATRIX & This = *(MATRIX*)this;
			for(unsigned i = 0;i < This._Fields.ni;i++)
				for(unsigned j = i;j < This._Fields.nj;j++)
				{
					T temp = This[i][j];
					This[i][j] = This[j][i];
					This[j][i] = temp;
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
				return Val[0][0];

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
							add[x][y - 1] = Val[x][y];
						else
							add[x - 1][y - 1] = Val[x][y];
					}
				if (i % 2) 
					Val[i][0] = -Val[i][0];
				d += Val[i][0] * Solution(add);
			}
			return d;
		}

	public:

		operator T() const
		{
			MATRIX This = *(MATRIX*)this;
			matrix_check(This._Fields.ni == This._Fields.nj, "Matrix is not square.");
			return Solution(This);
		}

	};

	class _POW_RECT
	{
		__MATRIX_FIELDS_DEF;
	public:
		template<typename DegreeType>
		MATRIX operator()(DegreeType n) const
		{
			MATRIX B, A = *(MATRIX*)this;
			if(n < 0)
			{
				matrix_check(false, "It is impossible to build a negative power rectangular matrix.");
				return MATRIX();
			}else if(n == 0)
			{
				A.SetOnMainDiagon(T(1));
				return A;
			}else if(std::IsFraction(n))
			{
				matrix_check(false, "It is impossible to build a fractional power rectangular matrix.");
				return MATRIX();
			}else
				B = A;
			for (unsigned r = 0, m = (abs(n) - 1); r < m; r++)
				B = A * B;
			return B;
		}
	};

	class _POW_SQUARE
	{
		__MATRIX_FIELDS_DEF;
	public:
		template<typename DegreeType>
		MATRIX operator()(DegreeType n) const
		{
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

	class _TO_ADJOINT
	{
		__MATRIX_FIELDS_DEF;
	public:
		void operator()()
		{
			MATRIX<T, max(0, (int)ci - 1), max(0, (int)cj - 1)> TempMatrix(_Fields.ni - 1, _Fields.nj -1);
			MATRIX Ret(_Fields.ni, _Fields.nj);
			MATRIX & This = *(MATRIX*)this;
			matrix_check(_Fields.ni == _Fields.nj, "Count column and row not equal.");
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
							TempMatrix[__i][__j++] = This[_i][_j]; 
						}
						__i++;
					}
					Ret[i][j] = (((i + j) & 1)?T(-1):T(1)) * TempMatrix.Determinant;
				}
				This = Ret;
		}
	};

	class _GET_ADJOINT
	{
		__MATRIX_FIELDS_DEF;
	public:
		MATRIX operator()() //алгебраическое дополнение
		{
			MATRIX<T, max(0, (int)ci - 1), max(0, (int)cj - 1)> TempMatrix(_Fields.ni - 1, _Fields.nj -1 );
			MATRIX Ret(_Fields.ni, _Fields.nj);
			MATRIX & This = *(MATRIX*)this;
			matrix_check(_Fields.ni == _Fields.nj, "Count column and row not equal.");
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
							TempMatrix[__i][__j++] = This[_i][_j]; 
						}
						__i++;
					}
					Ret[i][j] = (((i + j) & 1)?T(-1):T(1)) * TempMatrix.Determinant;
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
			MATRIX & This = *this;
			MATRIX<T, max(0, (int)ci - 1), max(0, (int)cj - 1)> TempMatrix(_Fields.ni - 1, _Fields.nj - 1);
			for (unsigned _i = 0, __i = 0; _i < _Fields.ni; _i++) 
			{
				if (_i == i) 
					continue;
				for (unsigned _j = 0, __j = 0; _j < _Fields.nj; _j++) 
				{
					if (_j == j) 
						continue;
					TempMatrix[__i][__j++] = This[_i][_j]; 
				}
				__i++;
			}
			return (((i + j) & 1)?T(-1):T(1)) * TempMatrix.Determinant;
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
						sum += abs(re[i][j]);
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
			   MATRIX & This = *(MATRIX*)this;
			   for (unsigned i = 0; i < _Fields.ni; i++)
				   for (unsigned j = 0; j < _Fields.nj; j++) 
					   if(This[i][j] != This[j][i])
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
			   if(ci != cj)
				   return false;
			   MATRIX & This = *(MATRIX*)this;
			   for (unsigned i = 0; i < _Fields.ni; i++)
				   for (unsigned j = 0; j < _Fields.nj; j++) 
					   if(This[i][j] != -This[j][i])
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
			   MATRIX & This = *(MATRIX*)this;
			   for (unsigned i = 0; i < _Fields.ni; i++)
				   for (unsigned j = 0; j < _Fields.nj; j++) 
				   {
					   if(i == j)
					   {
					      if(This[i][j] == T(0))
							  return false;
					   }else
					   {
					       if(This[i][j] != T(0))
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
			operator bool()
			{
			   MATRIX & This = *(MATRIX*)this;
			   for (unsigned i = 0; i < _Fields.ni; i++)
				   for (unsigned j = 0; j < _Fields.nj; j++) 
				   {
					   if(i == j)
					   {
					      if(This[i][j] != T(1))
							  return false;
					   }else
					   {
					       if(This[i][j] != T(0))
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
			operator T()
			{
				MATRIX & This = *(MATRIX*)this;
				T Res = T(0);
				for (unsigned i = 0; (i < _Fields.ni) && (i < _Fields.nj); i++)
					Res += This[i][i];
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
		typename std::conditional<ci == cj, _POW_SQUARE, _POW_RECT>::type	Pow;
		typename std::not_empty_if<ci == cj, _TO_ADJOINT>::type				ToAdjoint;
		typename std::not_empty_if<ci == cj, _GET_ADJOINT>::type			GetAdjoint;
		typename std::not_empty_if<ci == cj, _GET_ALGEBRAIC_COMPLEMENT>::type	GetAlgebraicComplement;
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
		for (int j = pos; j < _Fields.nj; j++) 
		{
			for (int i = 0; i < _Fields.ni; i++) 
				(*this)v[i][j] = v[i][j+1];
		}
	}

	//typename std::enable_if<((ci * cj) == 0)>::type 
	void RemoveRow(unsigned Pos)
	{
		CountRows = CountRows - 1;
		for (int i = Pos; i < _Fields.ni; i++) 
		{
			for (int j = 0; j < _Fields.nj; j++)
				(*this)[i][j] = v[i+1][j];
		}
	}

	//Mul
	template<class _T>
	typename std::enable_if<std::is_convertible<_T, T>::value, MATRIX&>::type 
	operator *=(_T Val)
	{
		for(unsigned i = 0;i < _Fields.ni;i++)
		{
			for(unsigned j = 0;j < _Fields.nj;j++)
				(*this)[i][j] *= Val;
		}
		return *this;
	}	
	
	template<class _T>
	typename std::enable_if<std::is_convertible<_T, T>::value, MATRIX>::type 
	operator *(_T Val)
	{
		MATRIX Ret = *this;
		for(unsigned i = 0;i < _Fields.ni;i++)
		{
			for(unsigned j = 0;j < _Fields.nj;j++)
				Ret[i][j] *= Val;
		}
		return Ret;
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
		matrix_check(_Fields.ni == Val._Fields.nj, "Count column and row not equal.");
		MATRIX & This = *this;
		typename TYPE_MUL_SOLUTION<_T, _i, _j>::type Ret(_Fields.ni, Val._Fields.nj);
		for(unsigned i = 0;i < Ret._Fields.ni;i++)
			for(unsigned j = 0;j < Ret._Fields.nj;j++)
			{
				T Summ = T(0);
				for(unsigned k = 0;k < _Fields.nj;k++)
					Summ += This[i][k] * Val[k][j];
				Ret[i][j] = Summ;
			}
		return Ret;
	}

	//Div
	template<class _T>
	typename std::enable_if<std::is_convertible<_T, T>::value, MATRIX&>::type 
	operator /=(_T Val)
	{
		for(unsigned i = 0; i < _Fields.ni; i++)
		{
			for(unsigned j = 0; j < _Fields.nj; j++)
				(*this)[i][j] /= Val;
		}
		return *this;
	}	
	
	template<class _T>
	typename std::enable_if<std::is_convertible<_T, T>::value, MATRIX>::type 
	operator /(_T Val)
	{
		MATRIX Ret = *this;
		for(unsigned i = 0;i < _Fields.ni;i++)
		{
			for(unsigned j = 0;j < _Fields.nj;j++)
				Ret[i][j] /= Val;
		}
		return Ret;
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
		MATRIX & This = *this;
		for(unsigned i = 0;i < CountRows;i++)
			for(unsigned j = 0;j < CountColumns;j++)
				This[i][j] += Value;
		return This;
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
		MATRIX & This = *this;
		for(unsigned i = 0;i < CountRows;i++)
			for(unsigned j = 0;j < CountColumns;j++)
				This[i][j] += Another[i][j];
		return This;
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
		for(unsigned i = 0;i < CountRows;i++)
			for(unsigned j = 0;j < CountColumns;j++)
				This[i][j] += Value;
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
		for(unsigned i = 0;i < CountRows;i++)
			for(unsigned j = 0;j < CountColumns;j++)
				This[i][j] += Another[i][j];
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
		MATRIX & This = *this;
		for(unsigned i = 0;i < CountRows;i++)
			for(unsigned j = 0;j < CountColumns;j++)
				This[i][j] -= Value;
		return This;
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
		MATRIX & This = *this;
		for(unsigned i = 0;i < CountRows;i++)
			for(unsigned j = 0;j < CountColumns;j++)
				This[i][j] -= Another[i][j];
		return This;
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
		for(unsigned i = 0;i < CountRows;i++)
			for(unsigned j = 0;j < CountColumns;j++)
				This[i][j] -= Value;
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
		for(unsigned i = 0;i < CountRows;i++)
			for(unsigned j = 0;j < CountColumns;j++)
				This[i][j] -= Another[i][j];
		return This;
	}
	///////

	void SwapRows(unsigned r1, unsigned r2)
	{
		MATRIX & This = *this;
		for (unsigned j = 0; j < CountColumns; j++) 
		{
			register T tmp = This[r2][j];
			This[r2][j] = This[r1][j];
			This[r1][j] = tmp;
		}
	}

	void SwapColumns(unsigned r1, unsigned r2)
	{
		MATRIX & This = *this;
		for (unsigned i = 0; i < CountRows; i++) 
		{
			register T tmp = This[i][r2];
			This[i][r2] = This[i][r1];
			This[i][r2] = tmp;
		}
	}

	void ToTriangle()
	{
		MATRIX & A = *this;
		unsigned MinNull0 = A.CountColumns, MinNull1 = 0;
		for (unsigned k = 0; k < (A.CountRows - 1); k++) 
		{
			MinNull0 = A.CountColumns;
			for (unsigned v = k; v < A.CountRows; v++) 
			{
				unsigned CountNull = 0;
				for (;(CountNull < A.CountRows) && (A[v][CountNull] == 0); CountNull++);
				if (CountNull < MinNull0)
				{
					MinNull0 = CountNull;
					MinNull1 = v;
				}
			}
			if (k != MinNull1) 
				A.SwapRows(k, MinNull1);
		}
		if (A[0][0] == 0) 
			return;
		for (unsigned j = 0; j < A.CountRows; j++)
			for (unsigned u = (j + 1); u < A.CountColumns; u++) 
			{
				if (A[u][j] == 0) 
					continue;
				T M = -(A[u][j] / A[j][j]);
				for (unsigned k = 0; k < A.CountRows; k++)
					A[u][k] += (A[j][k] * M);
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
		MATRIX & A = *this;
		int  l, factor1;
		for (unsigned i = 0,r = 0, k; i < min(_Fields.nj, _Fields.ni); i++)  
		{
			if (A[r][i] == 0) 
			{
				for (k = r + 1; k < _Fields.ni; k++) 
					if (A[k][i] != 0) 
						break;
				if (k < _Fields.ni)   
					A.SwapRows(r, k);
			}
			if (A[r][i] != 0) 
			{
				for (k=r+1; k < _Fields.ni; k++) 
				{
					if (A[k][i] == 0) 
						continue;
					l = lcm(A[k][i], A[r][i]);
					factor1 = l / A[k][i];
					for (unsigned j = i; j < _Fields.nj; j++) 
						A[k][j] = A[k][j] * factor1 - A[r][j] * (l / A[r][i]);
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

	template<class _T, unsigned _i, unsigned _j>
	void GetMiniMap(MATRIX<_T, _i, _j> & Result, unsigned StartRow, unsigned StartColumn)
	{
		MATRIX & This = *this;
		for(unsigned i = StartRow, _i = 0; i < CountRows;i++, _i++)
			for(unsigned j = StartColumn, _j = 0;j < CountColumns;j++, _j++)
				Result[_i][_j] = This[i][j];
	}	

	MATRIX<T, cj, ci> GetTranspose()
	{
		MATRIX & This = *this;
		MATRIX<T, cj, ci> NewMatrix(_Fields.nj, _Fields.ni);
		for(unsigned i = 0;i < _Fields.ni;i++)
		{
			for(unsigned j = 0;j < _Fields.nj;j++)
			{
				NewMatrix[j][i] = This[i][j];
			}
		}
		return NewMatrix;
	}

	inline void ToAllMinors()
	{
		*this = GetAllMinors();
	}

	T GetMinor(unsigned i, unsigned j)
	{
		MATRIX & This = *this;
		MATRIX<T, max(0, (int)ci - 1), max(0, (int)cj - 1)> TempMatrix(CountRows - 1, CountColumns -1);
		
		for (unsigned _i = 0, __i = 0; _i < CountRows; _i++) 
		{
			if (_i == i) 
				continue;
			for (unsigned _j = 0, __j = 0; _j < CountColumns; _j++) 
			{
				if (_j == j) 
					continue;
				TempMatrix[__i][__j++] = This[_i][_j]; 
			}
			__i++;
		}
		return TempMatrix.Determinant;
	}

	MATRIX GetAllMinors()
	{
		MATRIX<T, max(0, (int)ci - 1), max(0, (int)cj - 1)> TempMatrix(CountRows - 1, CountColumns -1);
		MATRIX Ret(CountRows, CountColumns);
		MATRIX & This = *this;

		for (unsigned i = 0; i < CountRows; i++) 
			for (unsigned j = 0; j < CountColumns; j++) 
			{
				for (unsigned _i = 0, __i = 0; _i < CountRows; _i++) 
				{
					if (_i == i) 
						continue;
					for (unsigned _j = 0, __j = 0; _j < CountColumns; _j++) 
					{
						if (_j == j) 
							continue;
						TempMatrix[__i][__j++] = This[_i][_j]; 
					}
					__i++;
				}
				Ret[i][j] = TempMatrix.Determinant;
			}
		return Ret;
	}

	void SetOnMainDiagon(T Val)
	{
		MATRIX & A = *this;
		for(unsigned i = 0;i < A.CountRows;i++)
			for(unsigned j = 0;j < A.CountColumns;j++)
					A[i][j] = (i == j)?Val:T(0);
	}

	void SetOnSecDiagon(T Val)
	{
		MATRIX & A = *this;
		for(unsigned i = A.CountRows - 1;(int)i >= 0;i--)
			for(unsigned j = A.CountColumns - 1;(int)j >= 0;j--)
				A[i][j] = (i == j)?Val:T(0);
	}

};


#endif
