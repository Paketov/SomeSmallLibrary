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

		class
		{
			__ROW_FIELDS;
		public:
			template<class CharType>
			operator std::basic_string<CharType>() const
			{
				std::basic_string<CharType> OutStr;
				for(unsigned i = 0;i < _Fields._Count;i++)
				{
					OutStr += ((i == 0)?STR_TYPE(CharType, ""):STR_TYPE(CharType, " "));
					NumberToString(_Fields.v[i], OutStr);
				}
				return OutStr;
			}

			template<class TypeChar>
			TypeChar * operator()(TypeChar * Buf, size_t BufSize) const
			{
				TypeChar * Maxindex = Buf + BufSize, *Cur = Buf;
				for(unsigned i = 0;i < _Fields._Count;i++)
				{						
					if(i != 0 && (Cur < Maxindex))
						*(Cur++) = CHAR_TYPE(TypeChar, ' ');
					Cur += NumberToString(_Fields.v[i], Cur, Maxindex - Cur);
				}
				return Cur;
			}

			template <class TypeChar, unsigned BufSize> 
			inline TypeChar * operator()(TypeChar (&Buf)[BufSize]) 
			{
				return operator()(Buf, BufSize);
			}
		} Text;
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

		inline __SETTER<Index + 1> operator <<(T Val)
		{
			//Error: out of static limit
			typedef std::enable_if<!IsStaticArr || (Index < cj)>::type;
			(*r)[Index] = Val;
			return __SETTER<Index + 1>(r);
		}
	};

public: 

	ROW & operator =(ROW & Val)
	{
		matrix_check(Val.Count == _Fields._Count, "Column count not equal");
		memcpy(_Fields.v, Val._Fields.v, sizeof(T) * _Fields._Count);
		return *this;
	}

	template<unsigned _j>
	ROW & operator =(ROW<T, _j>& Val)
	{
		//Error: out of static limit
		typedef typename std::enable_if<!IsStaticArr || !Val.IsStaticArr || (cj == _j)>::type;
		matrix_check(Val.Count == _Fields._Count, "Column count not equal");
		memcpy(_Fields.v, Val._Fields.v, sizeof(T) * _Fields._Count);
		return *this;
	}

	template<class _T,unsigned _j>
	ROW & operator =(ROW<_T, _j> & Val)
	{
		//Error: out of static limit
		typedef typename std::enable_if<!IsStaticArr || !Val.IsStaticArr || (cj == _j)>::type;

		//Error: type not convertible
		typedef typename std::enable_if<std::is_convertible<_T, T>::value>::type;

		matrix_check(Val.Count == _Fields._Count, "Column count not equal");
		for(unsigned i = 0;i < _Fields._Count; i++)
			_Fields.v[i] = (T)Val._Fields.v[i];
		return *this;
	}

	T & operator[](const unsigned Index)
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

	class FIELD_DYNAMIC
	{
	public:
		unsigned _CColumns;
		unsigned _CRows;
		T * v;

		inline bool Allocate(const unsigned Col, const unsigned Row)
		{
			T * NewVal = (T*)realloc(v, sizeof(T) * Col * Row);
			if(NewVal == NULL)
				return false;
			v = NewVal;
			return true;
		}

		template<unsigned _i, unsigned _j>
		inline bool Copy(const MATRIX<T, _i, _j> & Enother)
		{
			T * NewVal = (T*)realloc(v, sizeof(T) * Enother.CountColumns * Enother.CountRows);
			if(NewVal == NULL)
				return false;
			_CColumns = Enother.CountColumns;
			_CRows = Enother.CountRows;
			v = NewVal;
			memcpy(v, Enother._Fields.v, sizeof(T) * Enother.CountColumns * Enother.CountRows);
			return true;
		}

		template<class _T, unsigned _i, unsigned _j>
		inline bool Copy(const MATRIX<_T, _i, _j> & Enother)
		{
			typedef typename std::enable_if<std::is_convertible<_T, T>::value>::type;
			unsigned  m = Enother.CountColumns * Enother.CountRows;
			T * NewVal = (T*)realloc(v, sizeof(T) * m);
			if(NewVal == NULL)
				return false;
			_CColumns = Enother.CountColumns;
			_CRows = Enother.CountRows;
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
			_CColumns = _i;
			_CRows = _j;
			v = NewVal;
			memcpy(v, Enother, sizeof(T) * _i * _j);
			return true;
		}

		template <class _T, unsigned _i, unsigned _j> 
		inline bool Copy(const _T (&Enother)[_i][_j])
		{
			typedef typename std::enable_if<std::is_convertible<_T, T>::value>::type;

			T * NewVal = (T*)realloc(v, sizeof(T) * _i * _j);
			if(NewVal == NULL)
				return false;
			_CColumns = _i;
			_CRows = _j;
			v = NewVal;
			for(unsigned i = 0;i < (_i * _j);i++)
				((T*)v)[i] = ((const _T*)Enother)[i];
			return true;
		}

		inline unsigned SetCountColumn(const unsigned Val)
		{   
			return _CColumns = Val;
		}

		inline unsigned SetCountRow(const unsigned Val)
		{   
			return _CRows = Val;
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
		static const unsigned _CColumns = ci;
		static const unsigned _CRows = cj;
		T v[ci][cj];

		inline bool Allocate(const unsigned Col, const unsigned Row) const
		{
			return true;
		}

		inline bool Copy(const MATRIX<T, ci, cj> & Enother)
		{
			memcpy(v, Enother._Fields.v, sizeof(v));
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
			matrix_check((_CColumns == Enother.CountColumns) && (_CRows == Enother.CountRows), "Count column and row not equal.");
			memcpy(v, Enother._Fields.v, sizeof(v));
			return true;
		}

		template<class _T, unsigned _i, unsigned _j>
		inline bool Copy(const MATRIX<_T, _i, _j> & Enother)
		{
			typedef typename std::enable_if<(ci == _i) && (cj == _j)>::type;
			typedef typename std::enable_if<std::is_convertible<_T, T>::value>::type;
			matrix_check((_CColumns == Enother.CountColumns) && (_CRows == Enother.CountRows), "Count column and row not equal.");
			for(unsigned i = 0, m = _CColumns * _CRows;i < m;i++)
				((T*)v)[i] = ((_T*)Enother._Fields.v)[i];
			return true;
		}

		template <unsigned _i, unsigned _j> 
		inline bool Copy(const T (&Enother)[_i][_j])
		{
			typedef typename std::enable_if<(ci == _i) && (cj == _j)>::type;

			memcpy(v, Enother, sizeof(T) * _i * _j);
			return true;
		}

		template <class _T, unsigned _i, unsigned _j> 
		inline bool Copy(const _T (&Enother)[_i][_j])
		{
			typedef typename std::enable_if<(ci == _i) && (cj == _j)>::type;
			typedef typename std::enable_if<std::is_convertible<_T, T>::value>::type;
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

#define __MATRIX_FIELDS_DEF \
	TFIELDS _Fields;


	union
	{
		__MATRIX_FIELDS_DEF;
		class
		{
			__MATRIX_FIELDS_DEF;
		public:
			inline operator unsigned() const
			{
				return _Fields._CRows;
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
				return _Fields._CColumns;
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
			operator T()
			{
				var R = 0, C;
				A = Matrix.Triangle(A);
				for (var i = 0; i < A.stroc; i++) 
				{
					for (var C = 0; (C < A.stolb) && (A[i][C] == 0); C++);
					if ((A.stolb - C) != 0) 
						R++;
				}
				return R;
			}
		} Rank;

		class
		{
			__MATRIX_FIELDS_DEF;

			template<unsigned _i>
			static T Solution(MATRIX<T, _i, _i> & Val)
			{
				if (Val.CountColumns == 1)
					return Val[0][0];

				MATRIX<T, max(0, (int)_i - 1), max(0, (int)_i - 1)> add(Val.CountColumns - 1);
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
				//Error: if static count rows and columns not equal
				typedef typename std::enable_if<ci == cj>::type;
				MATRIX This = *(MATRIX*)this;
				matrix_check(This.CountRows == This.CountColumns, "Count column and row not equal.");
				return Solution(This);
			}

		} Determinant;

		class
		{
			__MATRIX_FIELDS_DEF;
		public:

			template<class TypeChar>
			operator std::basic_string<TypeChar>() const
			{
				MATRIX & This = *(MATRIX*)this;
				std::basic_string<TypeChar> OutStr;
				for(unsigned i = 0;i < This.CountRows;i++)
					OutStr += (std::basic_string<TypeChar>)This[i].Text + STR_TYPE(TypeChar, "\n");
				return OutStr;
			}

			template<class TypeChar>
			TypeChar * operator()(TypeChar * Buf, size_t BufSize) const
			{
				MATRIX & This = *(MATRIX*)this;
				TypeChar * Maxindex = Buf + BufSize, *Cur = Buf;
				for(unsigned i = 0; i < This.CountColumns; i++)
				{			
					Cur = (*(MATRIX*)this)[i].Text(Cur, Maxindex - Cur);
					if((Cur < Maxindex))
						*(Cur++) = CHAR_TYPE(TypeChar, '\n');
				}
				*Cur = CHAR_TYPE(TypeChar, '\0');
				return Cur;
			}

			template <class TypeChar, unsigned BufSize> 
			inline TypeChar * operator()(TypeChar (&Buf)[BufSize]) 
			{
				return operator()(Buf, BufSize);
			}

		} Text;
	};

public:

	typename std::conditional<IsStaticArr,ROW<T, cj> &, ROW<T, cj>>::type operator[](const unsigned Index)
	{
		matrix_check_index(Index < _Fields._CRows, "Row out of bound");
		if(IsStaticArr)
			return *(ROW<T, cj>*)((T*)_Fields.v + CountColumns * Index);
		else
			return ROW<T, cj>((T*)_Fields.v + CountColumns * Index, CountColumns); 
	}

	void SetAllVal(T NewVal)
	{
		for(unsigned i = 0, m = CountRows * CountColumns;i < m;i++)
			((T*)_Fields.v)[i] = NewVal;
	}

	MATRIX()
	{
		_Fields.Init();
		this->CountColumns = cj;
		this->CountRows = ci;
	}

	MATRIX(unsigned Col, unsigned Row, T AllVal)
	{
		_Fields.Init();
		this->CountColumns = Col;
		this->CountRows = Row;
		matrix_check(_Fields.Allocate(Col, Row), "Not alloc memory");
		SetAllVal(AllVal);
	}

	MATRIX(unsigned Col, unsigned Row)
	{
		_Fields.Init();
		this->CountColumns = Col;
		this->CountRows = Row;
		matrix_check(_Fields.Allocate(Col, Row), "Not alloc memory");
	}

	MATRIX(unsigned N)
	{
		_Fields.Init();
		this->CountRows = this->CountColumns = N;
		matrix_check(_Fields.Allocate(N, N), "Not alloc memory");
	}

	MATRIX(const MATRIX & New)
	{ 
		_Fields.Init();
		matrix_check(_Fields.Copy(New),  "Matrix not copied");
	}

	template<class _T, unsigned _i, unsigned _j>
	 MATRIX(const MATRIX<_T, _i, _j> & New)
	{ 
	    //Is types not convertible, have compile error
		typedef typename std::enable_if<std::is_convertible<_T, T>::value>::type;
		//Is matrix static, and count row or column not equal
		typedef typename std::enable_if<!IsStaticArr || !New.IsStaticArr || ((ci == _i) && (cj == _j))>::type;
		_Fields.Init();
		matrix_check(_Fields.Copy(New),  "Matrix not copied");
	}

	 
	template <unsigned _i, unsigned _j> 
	 MATRIX(const T (&New)[_i][_j]) 
	{ 
		//Is matrix static, and count row or column not equal
		typedef typename std::enable_if<!IsStaticArr || ((ci == _i) && (cj == _j))>::type;
		_Fields.Init();
		matrix_check(_Fields.Copy(New),  "Matrix not copied");
	}

	template <class _T, unsigned _i, unsigned _j> 
	 MATRIX(const _T (&New)[_i][_j]) 
	{ 
		//Is types not convertible, have compile error
		typedef typename std::enable_if<std::is_convertible<_T, T>::value>::type;
		//Is matrix static, and count row or column not equal
		typedef typename std::enable_if<!IsStaticArr || ((ci == _i) && (cj == _j))>::type;
		_Fields.Init();
		matrix_check(_Fields.Copy(New),  "Matrix not copied");
	}

	 MATRIX & operator =(const MATRIX & New)
	{
		matrix_check(_Fields.Copy(New),  "Matrix not copied");
	}

	template<class _T, unsigned _i, unsigned _j>
	 MATRIX & operator =(const MATRIX<_T, _i, _j> & New)
	{
	    //Is types not convertible, have compile error
		typedef typename std::enable_if<std::is_convertible<_T, T>::value>::type;
		//Is matrix static, and count row or column not equal
		typedef typename std::enable_if<!IsStaticArr || !New.IsStaticArr || ((ci == _i) && (cj == _j))>::type;
		matrix_check(_Fields.Copy(New),  "Matrix not copied");
	}

	template <class _T, unsigned _i, unsigned _j> 
	 MATRIX & operator =(const _T (&New)[_i][_j]) 
	{ 
		//Is types not convertible, have compile error
		typedef typename std::enable_if<std::is_convertible<_T, T>::value>::type;
		//Is matrix static, and count row or column not equal
		typedef typename std::enable_if<!IsStaticArr || ((ci == _i) && (cj == _j))>::type;
		matrix_check(_Fields.Copy(New),  "Matrix not copied");
	}

	~MATRIX()
	{
		_Fields.Deinit();
	}

	void RemoveCol(unsigned Pos)
	{
		CountColumns--;
		for (int j = pos; j < CountColumns; j++) 
		{
			for (int i = 0; i < CountRows; i++) 
				(*this)v[i][j] = v[i][j+1];
		}
	}

	void RemoveRow(unsigned Pos)
	{
		typedef typename std::enable_if<(ci * cj) == 0>::type;
		CountRows--;
		for (int i = Pos; i < CountRows; i++) 
		{
			for (int j = 0; j < CountColumns; j++)
				(*this)[i][j] = v[i+1][j];
		}
	}

	//Mul on scalar
	template<class _T>
	MATRIX & operator *=(_T Val)
	{
		typedef typename std::enable_if< std::is_convertible<_T, T>::value>::type;
		for(unsigned i = 0;i < CountRows;i++)
			for(unsigned j = 0;j < CountColumns;j++)
				(*this)[i][j] *= Val;
		return *this;
	}

	template<class _T>
	MATRIX operator *(_T Val)
	{
		typedef typename std::enable_if< std::is_convertible<_T, T>::value>::type;
		MATRIX Ret = *this;
		for(unsigned i = 0;i < CountRows;i++)
			for(unsigned j = 0;j < CountColumns;j++)
				Ret[i][j] *= Val;
		return Ret;
	}

	//////////////

	//Mul on enother matrix
	template<class _T, unsigned _i, unsigned _j>
	MATRIX<
				T, 
				((!IsStaticArr || !MATRIX<_T, _i, _j>::IsStaticArr)?0:ci), 
				((!IsStaticArr || !MATRIX<_T, _i, _j>::IsStaticArr)?0:_j)
	      >  operator *(const MATRIX<_T, _i, _j> & Val)
	{
		typedef typename std::enable_if<std::is_convertible<_T, T>::value>::type;
		typedef typename std::enable_if<!IsStaticArr || !Val.IsStaticArr || (ci == _j) && (cj == _i)>::type;
		

		matrix_check(CountRows == Val.CountColumns, "Count column and row not equal.");


		MATRIX & This = *this;
		MATRIX<
				T, 
				((!IsStaticArr || !MATRIX<_T, _i, _j>::IsStaticArr)?0:ci), 
				((!IsStaticArr || !MATRIX<_T, _i, _j>::IsStaticArr)?0:_j)
	    > Ret(CountColumns, Val.CountRows);
		for(unsigned i = 0;i < CountRows;i++)
			for(unsigned j = 0;j < CountColumns;j++)
			{
				T Summ = T(0);
				for(unsigned k = 0;k < CountRows;k++)
					Summ += This[i][k] * ((MATRIX<_T, _i, _j> &)Val)[k][j];
				Ret[i][j] = Summ;
			}
		return Ret;
	}


	MATRIX & operator *=(const MATRIX & Val)
	{
		*this = operator *(Val);
		return *this;
	}


	void SwapRows(unsigned r1, unsigned r2)
	{
		for (unsigned j = 0; j < CountColumns; j++) 
		{
			register T tmp = (*this)[r2][j];
			(*this)[r2][j] = (*this)[r1][j];
			(*this)[r1][j] = tmp;
		}
	}

	void SwapColumns(unsigned r1, unsigned r2)
	{
		for (unsigned i = 0; i < CountRows; i++) 
		{
			register T tmp = (*this)[i][r2];
			(*this)[i][r2] = (*this)[i][r1];
			(*this)[i][r2] = tmp;
		}
	}

	MATRIX & operator +=(const MATRIX & Another)
	{
		for(unsigned i = 0;i < CountRows;i++)
			for(unsigned j = 0;j < CountColumns;j++)
				(*this)[i][j] += Another[i][j];
		return *this;
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
				A.SwapRows(k, MinNull1); //A = Matrix.SwapStroc(A, k, MinNull[1], true);
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
	
	void ToTranspose()
	{
		typedef typename std::enable_if<cj == ci>::type;
		for(unsigned i = 0;i < CountRows;i++)
			for(unsigned j = i;j < CountColumns;j++)
			{
				T temp = (*this)[i][j];
				(*this)[i][j] = (*this)[j][i];
				(*this)[j][i] = temp;
			}
	}

	MATRIX GetTranspose()
	{
		MATRIX<T, cj, ci> NewMatrix(CountColumns, CountRows);
		for(unsigned i = 0;i < CountRows;i++)
			for(unsigned j = 0;j < CountColumns;j++)
				NewMatrix[j][i] = (*this)[i][j];
		return NewMatrix;
	}

	MATRIX GetInverse()
	{
		typedef typename std::enable_if<cj == ci>::type;

		MATRIX & This = *this;
		if((CountColumns == 1) && (CountRows == 1))
			return 1 / This[0][0];

		T d = This.Determinant;
		if ((CountRows != CountColumns) || (d == 0))
			return 0;

		MATRIX RetMatrix(CountRows, CountColumns);
		RetMatrix = GetСofactor();
		RetMatrix.ToTranspose();
		return RetMatrix *= ((T)1 / d);
	}

	inline void ToMinor()
	{
		*this = GetMinor();
	}

	MATRIX GetMinor()
	{
		MATRIX<T, max(0, (int)ci - 1), max(0, (int)cj - 1)> TempMatrix(CountRows - 1, CountColumns -1 );
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

	void ToСofactor()
	{
		MATRIX<T, max(0, (int)ci - 1), max(0, (int)cj - 1)> TempMatrix(CountRows - 1, CountColumns -1 );
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
				Ret[i][j] = (((i + j) & 1)?(T)-1:(T)1) * TempMatrix.Determinant;
			}
			This = Ret;
	}

	MATRIX GetСofactor()
	{
		MATRIX<T, max(0, (int)ci - 1), max(0, (int)cj - 1)> TempMatrix(CountRows - 1, CountColumns -1 );
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
				Ret[i][j] = (((i + j) & 1)?(T)-1:(T)1) * TempMatrix.Determinant;
			}
			return Ret;
	}


	void SetOnMainDiagon(T Val)
	{
		MATRIX & This = *this;
		for(unsigned i = 0, m = min(CountColumns, CountRows);i < m;i++)
			This[i][i] = Val;
	}

	void SetOnSecDiagon(T Val)
	{
		MATRIX & This = *this;
		for(unsigned i = CountColumns - 1, m = min(, CountRows);(int)i >= 0;i++)
			This[i][i] = Val;
	}

};


#endif
