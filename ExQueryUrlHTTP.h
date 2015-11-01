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
			TypeMethod.iLastErr = OUT_OF_MAX_SIZE_BUFFER;
			return false;
		}
		void* NewBuf = realloc(TypeMethod.Buf, NewSize);
		if(NewBuf == nullptr)
		{
			TypeMethod.iLastErr = NOT_ALLOC_MEMORY;
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

		if(HTTPMethods.CountUsed == 0)
		{
			static struct
			{
				char* s;
				unsigned char t;
			} InitMethods[METHOD_MAX_COUNT] = 
			{
				{"HTTP",	METHOD_RESPONSE},
				{"GET",		METHOD_GET},
				{"POST",	METHOD_POST},
				{"OPTIONS", METHOD_OPTIONS},
				{"HEAD",	METHOD_HEAD},
				{"PUT",		METHOD_PUT},
				{"PATCH",	METHOD_PATCH},
				{"DELETE",	METHOD_DELETE},
				{"TRACE",	METHOD_TRACE},
				{"CONNECT",	METHOD_CONNECT}
			};
			for(unsigned i = 0; i < METHOD_MAX_COUNT; i++)
				*HTTPMethods.Insert(InitMethods[i].s) = InitMethods[i].t;
			
		}
	}

	bool ReadStartLine(char* Str, char* EndLine)
	{

		char* s = Str, *ms = TypeMethod.EndHeader, *p;

		while(true)
		{
			if(!IsLatter(*s))
				break;
			s++;
		}
		if(s == Str)
		{
			TypeMethod.iLastErr = ERR_NOT_HTTP;
			return false;
		}

		/*
		What is recived method?
		*/
		unsigned char* CurMethod;
		char* EndMethod; 
		{
			char c = *s;
			*s = '\0';
			EndMethod = s;
			CurMethod = HTTPMethods[Str];
			*s = c;
		}
		/*
			If not have this method
		*/
		if(CurMethod == nullptr)	
		{
			TypeMethod.iLastErr = ERR_NOT_HTTP;
			return false;	
		}

		TypeMethod.MethodString = Str;
		TypeMethod.TypeMethod = *CurMethod;

		switch(*CurMethod)
		{
		case METHOD_RESPONSE:
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
		case METHOD_POST:
		case METHOD_GET:
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
					TypeMethod.iLastErr = HEADER_NOT_HAVE_QUERY;
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

		case METHOD_OPTIONS:
			break;
		case METHOD_HEAD:
			break;
		case METHOD_PUT:
			break;
		case METHOD_PATCH:
			break;
		case METHOD_DELETE:
			break;
		case METHOD_TRACE:
			break;
		case METHOD_CONNECT:
			break;
		case METHOD_MAX_COUNT:
			break;
		}
		*EndMethod = '\0';
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
	enum
	{
		ERR_NOT_HTTP,
		NOT_HAVE_VER_PROTOCOL,
		OUT_OF_MAX_SIZE_BUFFER,
		NOT_ALLOC_MEMORY,
		WRONG_GET_METHOD,
		HEADER_NOT_HAVE_QUERY,
		NOT_READED_FROM_SOCKET
	};

	enum 
	{
		METHOD_RESPONSE = 0,
		METHOD_GET,
		METHOD_POST,
		METHOD_OPTIONS,
		METHOD_HEAD,
		METHOD_PUT,
		METHOD_PATCH,
		METHOD_DELETE,
		METHOD_TRACE,
		METHOD_CONNECT,
		METHOD_MAX_COUNT
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
		while(1)
		{
			CountReaded = QueryUrl->Recive(TypeMethod.Buf, TypeMethod.SizeBuf, MSG_PEEK);
			TypeMethod.EndHeader = (char*)StringSearch((char*)TypeMethod.Buf, "\r\n\r\n");
			if(TypeMethod.EndHeader == nullptr)
			{
				if(!ResizeBuffer(TypeMethod.SizeBuf + 300))
					return false;
				continue;
			}
			break;
		}

		TypeMethod.Headers->Clear();
		TypeMethod.MethodData->Clear();
		TypeMethod.ContentLength = 0;
		char* p = (char*)TypeMethod.Buf, *e;
		e = (char*)StringSearch(p, "\r\n");
		*e = '\0';

		if(!ReadStartLine(p, e))
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
