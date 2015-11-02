#ifndef __QUERYURLHTTP_H_HAS_INCLUDED__
#define __QUERYURLHTTP_H_HAS_INCLUDED__

#include "ExQueryUrl.h"
#include "ExHashTable.h"





template<bool = true>
class __HTTP_RECIVE_QUERY
{

	typedef HASH_TABLE_STRING_KEY<char, char*, false> THASH_DATA;
#define __HTTP_RECIVE_QUERY_FIELDS \
	struct{\
	void*	Buf;\
	size_t	SizeBuf;\
	size_t	SizeData;\
	size_t	MaxSizeBuffer;\
	size_t	ReadedSizeData;\
	int		iLastErr;\
	char*			EndHeader;\
	unsigned char	TypeMethod;\
	char*			MethodString;\
	size_t			ContentLength;\
	std::def_var_in_union_with_constructor<THASH_DATA>	MethodData;\
	std::def_var_in_union_with_constructor<THASH_DATA>	Headers;\
	}


	/*
	Content Header
	*/
	static HASH_TABLE_STRING_KEY<char, unsigned char, false, unsigned char> HTTPMethods;


	bool ResizeBuffer(size_t NewSize)
	{
		if(NewSize >= TypeMethod.MaxSizeBuffer)
		{
			TypeMethod.iLastErr = ERRORS::OUT_OF_MAX_SIZE_BUFFER;
			return false;
		}
		void* NewBuf = realloc(TypeMethod.Buf, NewSize);
		if(NewBuf == nullptr)
		{
			TypeMethod.iLastErr = ERRORS::NOT_ALLOC_MEMORY;
			return false;
		}
		TypeMethod.SizeBuf = NewSize;
		TypeMethod.Buf = NewBuf;
		return true;
	}

	void InitFields()
	{		
		LastError.Clear();
		new(&TypeMethod.MethodData)  THASH_DATA();
		new(&TypeMethod.Headers)  THASH_DATA();
		TypeMethod.ContentLength = 0;
		TypeMethod.Buf = nullptr;
		TypeMethod.MaxSizeBuffer = 5000;
		ResizeBuffer(1000);
		TypeMethod.TypeMethod = METHODS::WRONG;
		TypeMethod.MethodString = nullptr;
		if(HTTPMethods.CountUsed == 0)
		{
			static struct
			{
				char* s;
				unsigned char t;
			} InitMethods[METHODS::WRONG] = 
			{
				{"HTTP",	METHODS::RESPONSE},
				{"GET",		METHODS::GET},
				{"POST",	METHODS::POST},
				{"OPTIONS", METHODS::OPT},
				{"HEAD",	METHODS::HEAD},
				{"PUT",		METHODS::PUT},
				{"PATCH",	METHODS::PATCH},
				{"DELETE",	METHODS::DEL},
				{"TRACE",	METHODS::TRACE},
				{"CONNECT",	METHODS::CONN}
			};
			for(unsigned i = 0; i < METHODS::WRONG; i++)
				*HTTPMethods.Insert(InitMethods[i].s) = InitMethods[i].t;
			
		}
	}

	unsigned char GetTypeMethod(const char* Str, int* Len = nullptr)
	{
		int LenMethod = -1;
		char MethodBuf[20];
		MethodBuf[0] = '\0';
		sscanf(Str, "%17[^ \t/]%n", MethodBuf, &LenMethod);
		unsigned char* CurMethod = HTTPMethods[MethodBuf];

		if(CurMethod == nullptr)
			return METHODS::WRONG;
		if(Len != nullptr)
			*Len = LenMethod;
		return *CurMethod;
	}

	bool ReadStartLine()
	{

		char* s = (char*)TypeMethod.Buf, *ms = TypeMethod.EndHeader;
		/*
		What is recived method?
		*/
		int LenHeader = -1;
		TypeMethod.TypeMethod = GetTypeMethod(s, &LenHeader);
		
		switch(TypeMethod.TypeMethod)
		{
		case METHODS::RESPONSE:
			{
				int PosVer = -1, StartNumMessage = -1, EndNumMessage = -1, StartMessage = -1, EndMessage = -1;
				int r = sscanf
				(
					s, 
					"/%*[1234567890.]%n%*[ \t\v\f\r]%n%*[1234567890]%n%*[ \t\v\f\r]%n%*s%n", 
					&PosVer, 
					&StartNumMessage, 
					&EndNumMessage,
					&StartMessage, 
					&EndMessage
				);
				if(PosVer >= 0)
				{
					static const char * Key = "ver_prot";
					*TypeMethod.MethodData->Insert((char*)Key) = s + 1;
					s[PosVer] = '\0';
				}
				if(EndNumMessage >= 0)
				{
					static const char * Key = "status";
					*TypeMethod.MethodData->Insert((char*)Key) = s + StartNumMessage;
					s[EndNumMessage] = '\0';
				}					
				if(EndMessage >= 0)
				{
					static const char * Key = "status_msg";
					*TypeMethod.MethodData->Insert((char*)Key) = s + StartMessage;
					s[EndMessage] = '\0';
				}
			}
			break;
		case METHODS::POST:
		case METHODS::GET:
			{							
				int StartQuery = -1, EndQuery = -1, StartVer = -1, EndVer = -1;
				int r = sscanf
				(
					s, 
					"%*[ \t\v\f\r]%n%*[^ \t\v\f\r]%n%*[ \t\v\f\r]HTTP/%n%*[1234567890.]%n", 
					&StartQuery, 
					&EndQuery,
					&StartVer,
					&EndVer
				);
				if(EndQuery >= 0)
				{
					static const char * Key = "query";
					*TypeMethod.MethodData->Insert((char*)Key) = s + StartQuery;
					s[EndQuery] = '\0';
				}else
				{
					TypeMethod.iLastErr = ERRORS::HEADER_NOT_HAVE_QUERY;
					return false;
				}

				if(EndVer >= 0)
				{
				   	static const char * Key = "ver_prot";
					*TypeMethod.MethodData->Insert((char*)Key) = s + StartVer;
					s[EndVer] = '\0';
				}
			}
			break;

		case METHODS::OPT:
			break;
		case METHODS::HEAD:
			break;
		case METHODS::PUT:
			break;
		case METHODS::PATCH:
			break;
		case METHODS::DEL:
			break;
		case METHODS::TRACE:
			break;
		case METHODS::CONN:
			break;
		case METHODS::WRONG:
			TypeMethod.iLastErr = ERRORS::NOT_HTTP;
			return false;
		}
		TypeMethod.MethodString = s;
		s[LenHeader] = '\0';
		return true;
	}

public:
	QUERY_URL* QueryUrl;

	__HTTP_RECIVE_QUERY()
	{
		InitFields();
	}
		
	~__HTTP_RECIVE_QUERY()
	{
		if(TypeMethod.Buf != nullptr)
			free(TypeMethod.Buf);
		TypeMethod.MethodData->~THASH_DATA();
		TypeMethod.Headers->~THASH_DATA();
	}

	struct ERRORS
	{
		enum
		{
			NOT_HTTP,
			NOT_HAVE_VER_PROTOCOL,
			OUT_OF_MAX_SIZE_BUFFER,
			NOT_ALLOC_MEMORY,
			WRONG_GET_METHOD,
			HEADER_NOT_HAVE_QUERY,
			NOT_READED_FROM_SOCKET,
			NOT_HAVE_DATA_IN_SOCKET
		};
	};

	struct METHODS
	{
		enum 
		{
			RESPONSE = 0,	//RESPONSE
			GET,			//GET
			POST,			//POST
			OPT,			//OPTIONS
			HEAD,			//HEAD
			PUT,			//PUT
			PATCH,			//PATCH
			DEL,			//DELETE
			TRACE,			//TRACE
			CONN,			//CONNECT
			WRONG
		};
	};


	typedef THASH_DATA::TINTER TINTER;

	union
	{
		class _METHOD_DATA
		{
		public:
			class
			{
			   friend _METHOD_DATA;
			   __HTTP_RECIVE_QUERY_FIELDS;
			public:
				operator size_t()
				{
				   return MethodData->CountUsed;
				}
			} Count;

			const char* operator[](const char* Key)
			{
				char ** r = Count.MethodData->operator[](Key);
				if(r == nullptr)
					return nullptr;
				return *r;
			}

			inline const char* In(const char* Key = nullptr)
			{
			    return Count.MethodData->In(Key);
			}

			bool Interate(TINTER* Interator)
			{
			    return Count.MethodData->Interate(Interator);
			}
		} MethodData;
	
		class _HEADERS
		{
		public:
			class
			{
			   friend _HEADERS;
			   __HTTP_RECIVE_QUERY_FIELDS;
			public:
				operator size_t()
				{
				   return Headers->CountUsed;
				}
			} Count;

			const char* operator[](const char* Key)
			{
				char ** r = Count.Headers->operator[](Key);
				if(r == nullptr)
					return nullptr;
				return *r;
			}

			inline const char* In(const char* Key = nullptr)
			{
			    return Count.Headers->In(Key);
			}

			bool Interate(TINTER* Interator)
			{
			    return Count.Headers->Interate(Interator);
			}
		} Headers;

		class
		{
			friend __HTTP_RECIVE_QUERY;
			 __HTTP_RECIVE_QUERY_FIELDS;
		public:
			operator int()
			{
			   return TypeMethod;
			}
		} TypeMethod;

		class
		{
			__HTTP_RECIVE_QUERY_FIELDS;
		public:
			operator const char*()
			{
			   return MethodString;
			}
		} Method;

		class
		{
			 __HTTP_RECIVE_QUERY_FIELDS;
		public:
			operator int()
			{
			   return iLastErr;
			}

			void Clear()
			{
			   iLastErr = 0;
			}
		} LastError;

		class
		{
			__HTTP_RECIVE_QUERY_FIELDS;
		public:
			operator size_t()
			{
			   return ContentLength;
			}
		} ContentLength;
	};

	bool Read()
	{
		if(QueryUrl->IsNotHaveRecvData)
			return false;

		int CountReaded, r;
		while(true)
		{
			CountReaded = QueryUrl->Recive(TypeMethod.Buf, TypeMethod.SizeBuf - 2, MSG_PEEK);
			if(CountReaded == -1)
			{
				TypeMethod.iLastErr = ERRORS::NOT_READED_FROM_SOCKET;
				return false;
			}else if(CountReaded == 0)
			{
				TypeMethod.iLastErr = ERRORS::NOT_HAVE_DATA_IN_SOCKET;
				return false;
			}
			((char*)TypeMethod.Buf)[CountReaded] = '\0';
			TypeMethod.EndHeader = (char*)StringSearch((char*)TypeMethod.Buf, "\r\n\r\n");
			if(TypeMethod.EndHeader == nullptr)
			{
				if(GetTypeMethod((char*)TypeMethod.Buf) == METHODS::WRONG)
				{
					TypeMethod.iLastErr = ERRORS::NOT_HTTP;
					return false;
				}
				if(!ResizeBuffer(TypeMethod.SizeBuf + 300))
					return false;
				continue;
			}
			break;
		}

		TypeMethod.Headers->Clear();
		TypeMethod.MethodData->Clear();
		TypeMethod.ContentLength = 0;
		TypeMethod.TypeMethod = METHODS::WRONG;
		char* p = (char*)TypeMethod.Buf, *e;
		e = (char*)StringSearch(p, "\r\n");
		*e = '\0';

		if(!ReadStartLine())
			return false;

		while(true)
		{
			p = e + 2;
			if(p >= TypeMethod.EndHeader)
				break;
			e = (char*)StringSearch(p, "\r\n");
			*e = '\0';
			int EndKey = -1, StartVal = -1;
			sscanf(p, "%*[^: \t]%n%*[: \t]%n", &EndKey, &StartVal);
			if((EndKey >= 0) && (StartVal >= 0))
			{
				p[EndKey] = '\0';
				*TypeMethod.Headers->Insert(p) = p + StartVal;
			}	
		}
		
		const char * ContLength = Headers["Content-Length"];
		if(ContLength != nullptr)
		{
			unsigned l = 0;
			sscanf(ContLength, "%u", &l);
			TypeMethod.ContentLength = l;
		}

		size_t LenHeader = ((unsigned long long)TypeMethod.EndHeader - (unsigned long long)TypeMethod.Buf) + 4;
		{
			char RecBuf[1000];
			do{
				size_t l = (LenHeader > 1000)?1000:LenHeader;
				CountReaded = QueryUrl->Recive(RecBuf, l);
				if(CountReaded <= 0)
					break;
				LenHeader -= CountReaded;
			} while(LenHeader > 0);
		}
		return true;
	}

};

template<bool d>
HASH_TABLE_STRING_KEY<char, unsigned char, false, unsigned char> __HTTP_RECIVE_QUERY<d>::HTTPMethods;


typedef __HTTP_RECIVE_QUERY<true> HTTP_RECIVE_QUERY;
#endif
