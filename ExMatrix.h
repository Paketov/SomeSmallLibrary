#ifndef __EX_MATRIX_H__
#define __EX_MATRIX_H__

#include <malloc.h>
#include <string.h>
#include <string>
#include "ExString.h"

#ifdef _MATRIX_CHECK
#define matrix_check(Check, Message)  if(!(Check)) throw (Message)
#else
#define matrix_check(Check, Message)
#endif

template<class T = int>
class MATRIX
{
#define __MATRIX_FIELDS_DEF \
	struct {\
	unsigned CountRows;	\
	unsigned CountColumns;\
	T * v;\
	}
public:

	typedef T TELEMENT;

	class __ROW_INTERATOR
	{
#define	__DIMENSION2_INTERATOR_FIELDS_DEF \
		struct {\
		MATRIX * m;\
		T * Column;\
		}\

		friend MATRIX;
	public:
		union
		{
			__DIMENSION2_INTERATOR_FIELDS_DEF;

			class
			{
				__DIMENSION2_INTERATOR_FIELDS_DEF;
			public:
				operator std::string()
				{
					std::string OutStr;
					for(unsigned i = 0;i < m->CountColumns;i++)
						OutStr += ((i == 0)?"":" ") + std::to_string(Column[i]);
					return OutStr;
				}

				operator std::wstring()
				{
					std::wstring OutStr;
					for(unsigned i = 0;i < m->CountColumns;i++)
						OutStr += ((i == 0)?L"":L" ") + std::to_stringw(Column[i]);
					return OutStr;
				}

				template<class TypeChar>
				TypeChar * operator()(TypeChar * Buf, size_t BufSize)
				{
					TypeChar * Maxindex = Buf + BufSize, *Cur = Buf;
					for(unsigned i = 0;i < m->CountColumns;i++)
					{						
						if(i != 0 && (Cur < Maxindex))
							*(Cur++) = CHAR_TYPE(TypeChar, ' ');
						Cur = NumberToString(Column[i], Cur, Maxindex - Cur);
					}
					return Cur;
				}


			} AsString;
		};
	private:
		__ROW_INTERATOR(MATRIX * m, T * Column)
		{
			this->m = m;
			this->Column = Column;
		}
	public: 


		T & operator[](unsigned Index)
		{
			matrix_check(Index < m->CountColumns, "Column out of bound");
			return Column[Index];
		}

		template <unsigned Index = 0>
		class __SETTER
		{
			__ROW_INTERATOR * r;
		public:
			inline __SETTER(__ROW_INTERATOR * r)
			{
				this->r = r;
			}

			inline __SETTER<Index + 1> operator <<(T Val)
			{
			   (*r)[Index] = Val;
			   return __SETTER<Index + 1>(r);
			}

		};

		inline __SETTER<1> operator <<(T Val)
		{
			(*this)[0] = Val;
			return __SETTER<1>(this);
		}

	};

	union
	{
		__MATRIX_FIELDS_DEF;

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
		public:
			operator T()
			{
				matrix_check(CountRows == CountColumns, "Count column and row not equal.");

				MATRIX & This = *(MATRIX*)this;
				if (CountColumns == 1)
					return This[0][0];

				MATRIX add(CountColumns - 1);
				T d = T(0);
				for (unsigned i = 0; i < CountColumns; ++i) 
				{
					for (unsigned y = 1; y < CountColumns; y++)
						for (unsigned x = 0; x < CountColumns; x++) 
						{
							if (x == i) 
								continue;
							if (x < i)
								add[x][y - 1] = This[x][y];
							else
								add[x - 1][y - 1] = This[x][y];
						}
						if (i % 2) 
							This[i][0] = -This[i][0];
						d += This[i][0] * add.Determinant;
				}
				return d;
			}

		} Determinant;

		class
		{
			__MATRIX_FIELDS_DEF;
		public:
			operator std::string()
			{
				std::string OutStr;
				for(unsigned i = 0;i < m->CountRows;i++)
					OutStr += (*this)[i].AsString + "\n";
				return OutStr;
			}

			operator std::wstring()
			{
				std::wstring OutStr;
				for(unsigned i = 0;i < m->CountRows;i++)
					OutStr += (*this)[i].AsString + L"\n";
				return OutStr;
			}

			template<class TypeChar>
			TypeChar * operator()(TypeChar * Buf, size_t BufSize)
			{
				TypeChar * Maxindex = Buf + BufSize, *Cur = Buf;
				for(unsigned i = 0; i < CountColumns; i++)
				{			
					Cur = (*(MATRIX*)this)[i].AsString(Cur, Maxindex - Cur);
					if((Cur < Maxindex))
						*(Cur++) = CHAR_TYPE(TypeChar, '\n');
				}
				return Cur;
			}
		} AsString;
	};



public:


	__ROW_INTERATOR operator[](unsigned Index)
	{
		matrix_check(Index < CountRows, "Row out of bound");
		return __ROW_INTERATOR(this, v + CountColumns * Index); 
	}

	void SetAllVal(T NewVal)
	{
		for(unsigned i = 0, m = CountRows * CountColumns;i < m;i++)
			v[i] = NewVal;
	}

	void Remap(unsigned Col, unsigned Row)
	{
		this->CountColumns = Col;
		this->CountRows = Row;
	}

	MATRIX()
	{
		this->CountColumns = 0;
		this->CountRows = 0;
		v = NULL;
	}

	MATRIX(unsigned Col, unsigned Row, T AllVal)
	{
		this->CountColumns = Col;
		this->CountRows = Row;
		v = (T*)malloc(sizeof(T) * Col * Row);
		SetAllVal(AllVal);
	}

	MATRIX(unsigned N)
	{
		this->CountRows = this->CountColumns = N;
		v = (T*)malloc(sizeof(T) * N * N);
	}

	MATRIX(unsigned Col, unsigned Row)
	{
		this->CountColumns = Col;
		this->CountRows = Row;
		v = (T*)malloc(sizeof(T) * Col * Row);
	}

	MATRIX(MATRIX & New)
	{
		this->CountColumns = New.CountColumns;
		this->CountRows = New.CountRows;
		unsigned NewSize = sizeof(T) * CountColumns * CountRows;
		v = (T*)malloc(NewSize);
		memcpy(this->v, New.v, NewSize);
	}

	~MATRIX()
	{
		if(v)
			free(v);
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
		CountRows--;
		for (int i = Pos; i < CountRows; i++) 
		{
			for (int j = 0; j < CountColumns; j++)
				(*this)[i][j] = v[i+1][j];
		}
	}

	MATRIX & operator *=(T Val)
	{
		for(unsigned i = 0;i < CountRows;i++)
			for(unsigned j = 0;j < CountColumns;j++)
				(*this)[i][j] *= Val;
		return *this;
	}

	MATRIX operator *(MATRIX & Val)
	{
		matrix_check(CountRows == Val.CountColumns, "Count column and row not equal.");
		MATRIX m(CountColumns, Val.CountRows, T(0));
		for(unsigned i = 0;i < CountRows;i++)
			for(unsigned j = 0;j < CountColumns;j++)
				for(unsigned k = 0;k < CountRows;k++)
					m[i][j] += (*this)[i][k] * Val[k][j];
		return m;
	}



	MATRIX & operator *=(MATRIX & Val)
	{
		*this = operator *(Val);
		return *this;
	}


	void SwapRows(unsigned r1, unsigned r2)
	{
		for (int j=0; j < CountColumns; j++) 
		{
			T tmp = (*this)[r2][j];
			(*this)[r2][j] = (*this)[r1][j];
			(*this)[r1][j] = tmp;
		}
	}

	MATRIX & operator +=(MATRIX & Another)
	{
		for(unsigned i = 0;i < CountRows;i++)
			for(unsigned j = 0;j < CountColumns;j++)
				(*this)[i][j] += Another[i][j];
		return *this;
	}


	MATRIX ToTriangle()
	{
		MATRIX A = *this;
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
			return A;
		for (unsigned j = 0; j < A.CountRows; j++)
			for (unsigned u = (j + 1); u < A.CountColumns; u++) 
			{
				if (A[u][j] == 0) 
					continue;
				T M = -(A[u][j] / A[j][j]);
				for (unsigned k = 0; k < A.CountRows; k++)
					A[u][k] += (A[j][k] * M);
			}
			return A;
	}






};


#endif
