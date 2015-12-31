#ifndef __EX_DYNAMIC_BUF_H__
#define __EX_DYNAMIC_BUF_H__

#include <malloc.h>

template<typename TypeElement = char>
class DYNAMIC_BUF
{
	struct FIELDS 
	{
	    size_t count;
		size_t alloc_count;
		TypeElement*  buf;
	};

public:

	union
	{
	    class{
			friend DYNAMIC_BUF;
			FIELDS f;

			void Increase(size_t NewSize)
			{
				if(NewSize >= f.alloc_count)
				{
					size_t ReallocSize = (size_t)(NewSize * 1.61803398875f);
					TypeElement* a = (TypeElement*)realloc(f.buf, ReallocSize * sizeof(TypeElement));
					if(a == nullptr)
						return;
					f.buf = a;
					f.alloc_count = ReallocSize;
				}
				f.count = NewSize;
			}
			void Decrease(size_t NewSize)
			{
				auto CountUsed = f.count;
				if(f.alloc_count > (size_t)(NewSize *  1.61803398875f))
				{
					size_t ReallocSize = NewSize + 3;
					TypeElement* a = (TypeElement*)realloc(f.buf, ReallocSize * sizeof(TypeElement));
					if(a == nullptr)
						return;
					f.buf = a;
					f.alloc_count = ReallocSize;
				}
				f.count = NewSize;
			}
		public:
			inline operator size_t () const { return f.count; }
			size_t operator=(size_t NewSize)
			{
				if(NewSize < f.count)
					Decrease(NewSize);
				else if(NewSize > f.count)
					Increase(NewSize);
				return NewSize;
			}
		} Count;

		class{
			FIELDS f;
		public:
			inline operator size_t () const { return f.alloc_count; }
		} AllocCount;

		class{
			FIELDS f;
		public:
			inline operator TypeElement* () const { return f.buf; }
		} Buf;
	};

	inline TypeElement& operator[](size_t Index) { return Count.f.buf[Index]; }



	DYNAMIC_BUF()
	{
		Count.f.alloc_count = Count.f.count = 0;
		Count.f.buf = nullptr;
	}

	DYNAMIC_BUF(size_t NewSize)
	{
		Count.f.alloc_count = Count.f.count = 0;
		Count.f.buf = nullptr;
		Count = NewSize;
	}

	~DYNAMIC_BUF()
	{
		if(Count.f.buf != nullptr)
			free(Count.f.buf);
	}


};

#endif