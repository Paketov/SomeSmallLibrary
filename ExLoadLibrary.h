#ifndef __EX_LOAD_LIBRARY_H__
#define __EX_LOAD_LIBRARY_H__

#include <stdint.h>


class EXTERNAL_LIBRARY
{
public:
	typedef intptr_t HANDLE_TYPE;
private:
	HANDLE_TYPE Handle;

	class PROC
	{
		friend EXTERNAL_LIBRARY;
		void* Address;
		inline PROC(void* NewAddress) { Address = NewAddress; }
	public:
		template<typename TypeProc>
		inline operator TypeProc*() const { return (TypeProc*)Address; }
	};
public:
	EXTERNAL_LIBRARY(const char* Path);
	EXTERNAL_LIBRARY();
	PROC GetProc(const char* NameProc) const;
	bool Free();
	operator bool() const;
};



#endif