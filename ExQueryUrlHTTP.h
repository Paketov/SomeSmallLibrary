#ifndef __QUERYURLHTTP_H_HAS_INCLUDED__
#define __QUERYURLHTTP_H_HAS_INCLUDED__

#include "ExQueryUrl.h"
#include "ExHashTable.h"


template<bool = true>
class __HTTP_RECIVE_QUERY
{

	typedef HASH_TABLE_STRING_KEY<char, char*, false> THASH_DATA;

#define __HTTP_RECIVE_QUERY_FIELDS		\
	struct{								\
	void*			Buf;				\
	size_t			SizeBuf;			\
	size_t			MaxSizeBuffer;		\
	size_t			SizeHeader;			\
	size_t			ContentLength;		\
	char*			Path;				\
	char*			VerProt;			\
	char*			StatusMsg;			\
	char*			MethodString;		\
	int				iLastErr;			\
	int				Status;				\
	std::def_var_in_union_with_constructor<THASH_DATA>	Headers;\
	unsigned char	TypeMethod;			\
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
		new(&TypeMethod.Headers)  THASH_DATA();
		TypeMethod.ContentLength = 0;
		TypeMethod.Buf = nullptr;
		TypeMethod.MaxSizeBuffer = 5000;
		ResizeBuffer(1000);
		TypeMethod.TypeMethod = METHODS::WRONG;
		
		TypeMethod.Status = 0;
		TypeMethod.StatusMsg = (char*)GetMsgByStatus(TypeMethod.Status);
		TypeMethod.Path = "";
		TypeMethod.VerProt = "";
		TypeMethod.MethodString = "";

		if(HTTPMethods.CountUsed == 0)
		{
			static struct
			{
				char* s;
				unsigned char t;
			} InitMethods[METHODS::WRONG] = 
			{
				{"HTTP",		METHODS::RESPONSE},
				{"GET",			METHODS::GET},
				{"POST",		METHODS::POST},
				{"OPTIONS",		METHODS::OPT},
				{"HEAD",		METHODS::HEAD},
				{"PUT",			METHODS::PUT},
				{"PATCH",		METHODS::PATCH},
				{"DELETE",		METHODS::DEL},
				{"TRACE",		METHODS::TRACE},
				{"CONNECT",		METHODS::CONN},
				{"MKCOL",		METHODS::MKCOL},
				{"COPY",		METHODS::COPY},
				{"MOVE",		METHODS::MOVE},
				{"PROPFIND",	METHODS::PROPFIND},
				{"PROPPATCH",	METHODS::PROPPATCH},
				{"LOCK",		METHODS::LOCK},
				{"UNLOCK",		METHODS::UNLOCK}
			};
			for(unsigned i = 0; i < METHODS::WRONG; i++)
				*HTTPMethods.Insert(InitMethods[i].s) = InitMethods[i].t;
			
		}
	}

	void ReinitFields()
	{		
		TypeMethod.SizeHeader = 0;
		TypeMethod.TypeMethod = METHODS::WRONG;
		TypeMethod.Status = 0;
		TypeMethod.StatusMsg = (char*)GetMsgByStatus(TypeMethod.Status);
		TypeMethod.MethodString = TypeMethod.VerProt = TypeMethod.Path = "";
		TypeMethod.Headers->Clear();
		TypeMethod.ContentLength = 0;
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
		char* s = (char*)TypeMethod.Buf;
		/*
		What is recived method?
		*/
		int LenHeader = 0;
		TypeMethod.TypeMethod = GetTypeMethod(s, &LenHeader);
		s += LenHeader;

		switch(TypeMethod.TypeMethod)
		{
		case METHODS::RESPONSE:
			{
				int PosVer = -1, StartNumMessage = -1, EndNumMessage = -1, StartMessage = -1, EndMessage = -1;
				int r = sscanf
				(
					s, 
					"/%*[1234567890.]%n%*[ \t\v\f\r]%n%i%n%*[ \t\v\f\r]%n%*s%n", 
					&PosVer, 
					&StartNumMessage, 
					&TypeMethod.Status,
					&EndNumMessage,
					&StartMessage, 
					&EndMessage
				);
				if(PosVer >= 0)
				{
					TypeMethod.VerProt = s + 1;
					s[PosVer] = '\0';
				}

				if(EndMessage >= 0)
					TypeMethod.StatusMsg = s + StartMessage;
				else
					TypeMethod.StatusMsg = (char*)GetMsgByStatus(TypeMethod.Status);
			}
			break;
		case METHODS::POST:
		case METHODS::GET:
		case METHODS::MKCOL:
		case METHODS::COPY:
		case METHODS::MOVE:
		case METHODS::DEL:
		case METHODS::PROPPATCH:
		case METHODS::PROPFIND:
		case METHODS::LOCK:
		case METHODS::UNLOCK:
		case METHODS::HEAD:
		case METHODS::PUT:
		case METHODS::PATCH:
		case METHODS::TRACE:
		case METHODS::OPT:
		case METHODS::CONN:
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
					TypeMethod.Path = s + StartQuery;
					s[EndQuery] = '\0';
				}else
				{
					TypeMethod.iLastErr = ERRORS::HEADER_NOT_HAVE_QUERY;
					return false;
				}
				if(EndVer >= 0)
				{
					TypeMethod.VerProt = s + StartVer;
					s[EndVer] = '\0';
				}
			}
			break;
		case METHODS::WRONG:
			TypeMethod.iLastErr = ERRORS::NOT_HTTP;
			return false;
		}
		TypeMethod.MethodString = (char*)TypeMethod.Buf;
		TypeMethod.MethodString[LenHeader] = '\0';
		return true;
	}

	void Copy(__HTTP_RECIVE_QUERY& Another)
	{
		if(Another.TypeMethod.Buf != nullptr)
		{
			void* NewBuf =  malloc(Another.TypeMethod.SizeBuf);
			if(NewBuf != nullptr)
			{
				TypeMethod.Buf = NewBuf;
				memcpy(TypeMethod.Buf, Another.TypeMethod.Buf, Another.TypeMethod.SizeBuf);

				if(Another.TypeMethod.Path[0] != '\0')
					TypeMethod.Path = (char*)TypeMethod.Buf + OFFSET_P(Another.TypeMethod.Buf, Another.TypeMethod.Path);
				if(Another.TypeMethod.VerProt[0] != '\0')
					TypeMethod.VerProt = (char*)TypeMethod.Buf + OFFSET_P(Another.TypeMethod.Buf, Another.TypeMethod.VerProt);
				if(Another.TypeMethod.StatusMsg[0] != '\0')
					TypeMethod.StatusMsg = (char*)TypeMethod.Buf + OFFSET_P(Another.TypeMethod.Buf, Another.TypeMethod.StatusMsg);
				if(Another.TypeMethod.MethodString[0] != '\0')
					TypeMethod.MethodString = (char*)TypeMethod.Buf + OFFSET_P(Another.TypeMethod.Buf, Another.TypeMethod.MethodString);

				TypeMethod.MaxSizeBuffer = Another.TypeMethod.MaxSizeBuffer;
				TypeMethod.SizeHeader = Another.TypeMethod.SizeHeader;
				TypeMethod.SizeBuf = Another.TypeMethod.SizeBuf;
				TypeMethod.iLastErr = Another.TypeMethod.iLastErr;
				TypeMethod.TypeMethod = Another.TypeMethod.TypeMethod;
				TypeMethod.ContentLength = Another.TypeMethod.ContentLength;
				TypeMethod.Status = Another.TypeMethod.Status;

				for(THASH_DATA::TINTER Inter; Another.TypeMethod.Headers->Interate(&Inter) && !Inter.IsEnd;)
				{
					char* Key, *Val;
					Another.TypeMethod.Headers->DataByInterator(&Inter, &Key, &Val);

					auto r = TypeMethod.Headers->Insert((char*)TypeMethod.Buf + OFFSET_P(Another.TypeMethod.Buf, Key));
					if(r != nullptr)
						*r = (char*)TypeMethod.Buf + OFFSET_P(Another.TypeMethod.Buf, Val);
				}
				QueryUrl = Another.QueryUrl;
			}
		}
	}

public:

	static const char * GetMsgByStatus(int Status)
	{
		switch(Status)
		{
			//Informational
		case 100:	return "Continue";
		case 101:	return "Switching Protocols";
		case 102:	return "Processing";
		case 105:	return "Name Not Resolved";

			//Success
		case 200:	return "OK";
		case 201:	return "Created";
		case 202:	return "Accepted";
		case 203:   return "Non-Authoritative Information"; 
		case 204:	return "No Content";
		case 205:	return "Reset Content";
		case 206:	return "Partial Content";
		case 207:	return "Multi-Status";
		case 226:	return "IM Used";

			//Redirection 
		case 301:	return "Moved Permanently";
		case 302:	return "Moved Temporarily";
		case 303:	return "See Other";
		case 304:	return "Not Modified";
		case 305:	return "Use Proxy";
		case 307:	return "Temporary Redirect";

			//Client Error 
		case 400:	return "Bad Request";
		case 401:	return "Unauthorized";
		case 402:	return "Payment Required";
		case 403:	return "Forbidden";
		case 404:	return "Not Found";
		case 405:	return "Method Not Allowed";
		case 406:	return "Not Acceptable";
		case 407:	return "Proxy Authentication Required";
		case 408:	return "Request Timeout";
		case 409:	return "Conflict";
		case 410:	return "Gone";
		case 411:	return "Length Required";
		case 412:	return "Precondition Failed";
		case 413:	return "Request Entity Too Large";
		case 414:	return "Request-URI Too Large";
		case 415:	return "Unsupported Media Type";
		case 416:	return "Requested Range Not Satisfiable";
		case 417:	return "Expectation Failed";
		case 418:	return "I'm a teapot"; //:)
		case 422:	return "Unprocessable Entity";
		case 423:	return "Locked";
		case 424:	return "Failed Dependency";
		case 425:	return "Unordered Collection";
		case 426:	return "Upgrade Required";
		case 428:	return "Precondition Required";
		case 429:	return "Too Many Requests";
		case 431:	return "Request Header Fields Too Large";
		case 434:	return "Requested host unavailable";
		case 449:	return "Retry With";
		case 451:	return "Unavailable For Legal Reasons";
		case 456:	return "Unrecoverable Error";

			//Server Error
		case 500:	return "Internal Server Error";
		case 501:	return "Not Implemented";
		case 502:	return "Bad Gateway";
		case 503:	return "Service Unavailable";
		case 504:	return "Gateway Timeout";
		case 505:	return "HTTP Version Not Supported";
		case 506:	return "Variant Also Negotiates";
		case 507:	return "Insufficient Storage";
		case 508:	return "Loop Detected";
		case 509:	return "Bandwidth Limit Exceeded";
		case 510:	return "Not Extended";
		case 511:	return "Network Authentication Required";
		}
		return "";
	}

	QUERY_URL* QueryUrl;

	__HTTP_RECIVE_QUERY()
	{
		InitFields();
	}

	__HTTP_RECIVE_QUERY(__HTTP_RECIVE_QUERY& Another)
	{
		InitFields();
		Copy(Another);
	}
	__HTTP_RECIVE_QUERY& operator =(__HTTP_RECIVE_QUERY& Another)
	{
		Copy(Another);
		return *this;
	}

		
	~__HTTP_RECIVE_QUERY()
	{
		if(TypeMethod.Buf != nullptr)
			free(TypeMethod.Buf);
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
			OPT,			//OPTIONS	(Get options for this server)
			HEAD,			//HEAD
			PUT,			//PUT
			PATCH,			//PATCH
			DEL,			//DELETE
			TRACE,			//TRACE
			CONN,			//CONNECT	(Connection with proxy server)
			MKCOL,			//MKCOL		(Create catalog)
			COPY,			//COPY		(Copy catalog or file)
			MOVE,			//MOVE		(Move file or catalog)
			PROPFIND,		//PROPFIND	(Get property of file)
			PROPPATCH,		//PROPPATCH	(Changing the properties of a file or directory)
			LOCK,			//LOCK
			UNLOCK,			//UNLOCK
			WRONG
		};
	};


	typedef THASH_DATA::TINTER TINTER;

	union
	{

		class
		{
			__HTTP_RECIVE_QUERY_FIELDS;
		public:
			operator const char*()
			{
				return VerProt;
			}
		} ProtVer;

		/*
		When recived METHODS::POST, METHODS::GET, or another.
		*/
		class
		{
			__HTTP_RECIVE_QUERY_FIELDS;
		public:
			operator const char*()
			{
				return Path;
			}
		} Path;

		class
		{
			__HTTP_RECIVE_QUERY_FIELDS;
		public:
			operator const char*()
			{
				return Status;
			}
		} Status;

		class
		{
			__HTTP_RECIVE_QUERY_FIELDS;
		public:
			operator const int()
			{
				return StatusMsg;
			}
		} StatusMsg;

		class
		{
			__HTTP_RECIVE_QUERY_FIELDS;
		public:
			operator size_t()
			{
				return SizeHeader;
			}
		} Size;

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
		ReinitFields();
		char* EndHeader;
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
			EndHeader = (char*)StringSearch((char*)TypeMethod.Buf, "\r\n\r\n");
			if(EndHeader == nullptr)
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
		
		TypeMethod.SizeHeader = (size_t)EndHeader - (size_t)TypeMethod.Buf + 4;

		char* p = (char*)TypeMethod.Buf, *e;
		e = (char*)StringSearch(p, "\r\n");
		*e = '\0';

		if(!ReadStartLine())
			return false;

		while(true)
		{
			p = e + 2;
			if(p >= EndHeader)
				break;
			e = (char*)StringSearch(p, "\r\n");
			*e = '\0';
			int EndKey = -1, StartVal = -1;
			sscanf(p, "%*[^: \t]%n%*[: \t]%n", &EndKey, &StartVal);
			if((EndKey >= 0) && (StartVal >= 0))
			{
				p[EndKey] = '\0';
				auto r = TypeMethod.Headers->Insert(p);
				if(r != nullptr)
					*r = p + StartVal;
			}	
		}
		
		const char * ContLength = Headers["Content-Length"];
		if(ContLength != nullptr)
		{
			unsigned l = 0;
			sscanf(ContLength, "%u", &l);
			TypeMethod.ContentLength = l;
		}

		{		
			size_t LenHeader = TypeMethod.SizeHeader;
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
