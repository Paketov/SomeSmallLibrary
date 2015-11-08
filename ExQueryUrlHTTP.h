#ifndef __QUERYURLHTTP_H_HAS_INCLUDED__
#define __QUERYURLHTTP_H_HAS_INCLUDED__

/*
Warming!
This library not thread safe!
You have to manually put the critical section or block in your code.
This is made to minimize the overhead for thread synchronization.
*/


#include "ExQueryUrl.h"
#include "ExHashTable.h"


template<bool>
class __HTTP_SEND_QUERY;

typedef __HTTP_SEND_QUERY<true> HTTP_SEND_QUERY;

template<bool>
class __HTTP_RECIVE_QUERY
{

	typedef HASH_TABLE_STRING_KEY<char, char*, false> THASH_DATA;
	typedef HASH_TABLE_STRING_KEY<char, unsigned char, true, unsigned char> THASH_METHODS;
	friend HTTP_SEND_QUERY;
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
	std::def_var_in_union_with_constructor<THASH_METHODS> FilteredMethod;\
	unsigned char	TypeMethod;			\
	}
public:
	typedef THASH_DATA::TINTER TINTER;

private:
	class _HEADERS
	{
		friend HTTP_SEND_QUERY;
	public:
		class
		{
			friend HTTP_SEND_QUERY;
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

		bool DataByInterator(TINTER* Interator, const char** Key, const char** Val)
		{
			return Count.Headers->DataByInterator(Interator, Key, Val);
		}

	};

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
			for(unsigned char i = 0; i < METHODS::WRONG; i++)
				*HTTPMethods.Insert(GetMethodName(i)) = i;
		}
		new(&TypeMethod.FilteredMethod)  THASH_METHODS(HTTPMethods);
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
		unsigned i = 0;
		for( ;((Str[i] >= 'A') && (Str[i] <= 'Z')) || (Str[i] == '-'); i++);

		char c = Str[i];
		((char*)Str)[i] = '\0';
		auto CurMethod = TypeMethod.FilteredMethod->operator[](Str);
		((char*)Str)[i] = c;
		if(Len != nullptr)
			*Len = i;
		if(CurMethod == nullptr)
			return METHODS::WRONG;
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
			//Response from server
		case METHODS::RESPONSE:
			{
				unsigned i = 0;

				if(s[0] == '/')
				{
					//Read version
					for(i = 1 ;((s[i] >= '0') && (s[i] <= '9')) || (s[i] == '.'); i++);
					if(i > 1)
					{
						TypeMethod.VerProt = s + 1;
						if(s[i] == '\0')
							break;
						s[i] = '\0';
						i++;
					}
				}

				//Skip spaces
				for(;IsSpace(s[i]); i++);

				{
					unsigned Stat = 0;
					//Read status number
					for(unsigned char Digit; (Digit = s[i] - '0') <= 9; i++)
						Stat = Stat * 10 + Digit;
					TypeMethod.Status = Stat;
				}
				//Skip spaces
				for(;IsSpace(s[i]); i++);

				//If have message
				if(
					((s[i] >= 'A') && (s[i] <= 'Z')) || 
					((s[i] >= 'a') && (s[i] <= 'z'))
					)
					TypeMethod.StatusMsg = s + i;
				else
					TypeMethod.StatusMsg = (char*)GetMsgByStatus(TypeMethod.Status);
			}
			break;
			//Wrong recived method
		case METHODS::WRONG:
			TypeMethod.iLastErr = ERRORS::NOT_HAVE_METHOD;
			return false;
			//For another query methods
		default:
			{							
				unsigned i = 0, j;

				//Skip spaces
				for(;IsSpace(s[i]); i++);

				j = i;
				//Read query
				//Go to near space
				for(;(s[i] != '\0') && !IsSpace(s[i]); i++);

				if(i > j)
				{
					TypeMethod.Path = s + j;
					if(s[i] == '\0')
						break;
					s[i] = '\0';
					i++;
				}
				//Skip spaces
				for(;IsSpace(s[i]); i++);
				if((s[i] == 'H') && (s[i + 1] == 'T') && (s[i + 2] == 'T') && (s[i + 3] == 'P'))
				{
					i += 4;
					if(s[i] == '/')
					{
						//Read version
						j = ++i;
						for(;((s[i] >= '0') && (s[i] <= '9')) || (s[i] == '.'); i++);
						if(i > j)
						{
							TypeMethod.VerProt = s + j;
							s[i] = '\0';
						}
					}
				}
			}
		}
		TypeMethod.MethodString = (char*)TypeMethod.Buf;
		TypeMethod.MethodString[LenHeader] = '\0';
		return true;
	}

	template<bool IsDynamicKey, typename IndexType>
	static int ReadHeaders(char * Str, HASH_TABLE_STRING_KEY<char, char*, IsDynamicKey, IndexType>* DestHash)
	{
		unsigned i = 0, j;
		char *Key;
		while(true)
		{
lblMainLoop:
			j = i;
			for(;;i++)
			{
				switch(Str[i])
				{
				case ' ': case '\t': case '\n': case '\v': case '\f': case ':':
					goto lblOut;
				case '\r':
					if(Str[i + 1] == '\n')
					{
						i += 2;
						goto lblMainLoop;
					}
					goto lblOut;
				case '\0':
					return i;
				}
			}
lblOut:
			if(i > j)
			{
				Key = Str + j;
				Str[i] = '\0';
				i++;
			}
			for(;IsSpace(Str[i]); i++);
			j = i;
			for(;;i++)
			{							
				switch(Str[i])
				{
				case '\r':
					if(Str[i + 1] == '\n')
						goto lblOut2;
					continue;
				case '\0':
					goto lblOut2;
				}
			}
lblOut2:			
			if(i > j)
			{
				auto r = DestHash->Insert(Key);
				if(r != nullptr)
					*r = Str + j;
				if(Str[i] == '\0')
					return i;
				Str[i] = '\0';
				i += 2;
			}
		}

		return i;
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

	QUERY_URL* QueryUrl;


	/*
	Date functions;
	*/

	static long long  GmtAccuracy;

	static const size_t MaxTimeLen = sizeof("Mon, 28 Sep 1970 06:00:00 GMT");

	static bool ReadGMTTime(const char * TimeStr, tm& OutTm)
	{
		char DayOfWeekName[6], MonthName[6], MeanTime[6];
		MeanTime[0] = MonthName[0] = DayOfWeekName[0] = '\0';
		unsigned short DayOfMonth, Year, Hour, Minute, Sec;
		auto r = sscanf
			(
			TimeStr, 
			"%4[^ \r\n,\t]%*[^1234567890]%2i %4s %i %2i:%2i:%2i %4s", 
			DayOfWeekName, 
			&OutTm.tm_mday, 
			MonthName, 
			&OutTm.tm_year, 
			&OutTm.tm_hour, 
			&OutTm.tm_min, 
			&OutTm.tm_sec, 
			MeanTime
			);
		if(r < 8)
			return false;

		static const char  *Week[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
		static const char  *Months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
		OutTm.tm_wday = -1;
		OutTm.tm_year -= 1900;
		for(unsigned i = 0;i < std::countof(Week);i++)
			if(*(unsigned*)DayOfWeekName == *(unsigned*)(Week[i]))
			{
				OutTm.tm_wday = i;
				break;
			}
			if(OutTm.tm_wday == -1)
				return false;
			OutTm.tm_mon = -1;
			for(unsigned i = 0;i < std::countof(Months);i++)
				if(*(unsigned*)MonthName == *(unsigned*)(Months[i]))
				{
					OutTm.tm_mon = i;
					break;
				}	
				if(OutTm.tm_mon == -1)
					return false;
				OutTm.tm_isdst = -1;
				OutTm.tm_yday = -1;
				return true;
	}

	static bool WriteGMTTime(char * TimeStr, size_t LenBuf, tm& InTm)
	{
		static const char  *Week[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
		static const char  *Months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
		auto r = sprintf_s
			(
			TimeStr,
			LenBuf,
			"%3s, %02i %3s %i %02i:%02i:%02i GMT", 
			Week[InTm.tm_wday], 
			InTm.tm_mday, 
			Months[InTm.tm_mon], 
			InTm.tm_year + 1900, 
			InTm.tm_hour, 
			InTm.tm_min, 
			InTm.tm_sec
			);
		return r >= 0;
	}

	static void GetCurGMT(tm& OutTm)
	{
		time_t t;
		time(&t);
		TimeToGMT(t, OutTm);
	}

	static void TimeToGMT(time_t t, tm& OutTm)
	{
		t += GmtAccuracy;
		OutTm = *gmtime(&t);
	}

	inline static time_t GMTToTime(tm& InTm)
	{
#ifdef _WIN32
		return _mkgmtime(&InTm) - GmtAccuracy;
#else
		return  timegm(InTm) - GmtAccuracy;
#endif
	}
	/*
	End date functions;
	*/

	static const char* GetMsgByStatus(int Status)
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

	static const char* GetMethodName(int Number)
	{
		switch(Number)
		{
			//Response
		case METHODS::RESPONSE: return "HTTP";

			//Basic
		case METHODS::GET:		return "GET";
		case METHODS::POST:		return "POST";
		case METHODS::OPT:		return "OPTIONS";
		case METHODS::HEAD:		return "HEAD";
		case METHODS::PUT:		return "PUT";
		case METHODS::CONN:		return "CONNECT";
		case METHODS::TRACE:	return "TRACE";
		case METHODS::DEL:		return "DELETE";

			//WebDAV
			//RFC 2518
		case METHODS::PROPFIND:	return "PROPFIND";
		case METHODS::PROPPATCH:return "PROPPATCH";
		case METHODS::MKCOL:	return "MKCOL";
		case METHODS::COPY:		return "COPY";
		case METHODS::MOVE:		return "MOVE";
		case METHODS::LOCK:		return "LOCK";
		case METHODS::UNLOCK:	return "UNLOCK";

			//For version control
			//RFC 3253
		case METHODS::VERCNTRL:	return "VERSION-CONTROL";
		case METHODS::REPORT:	return "REPORT";
		case METHODS::CHKOUT:	return "CHECKOUT";
		case METHODS::CHKIN:	return "CHECKIN";
		case METHODS::UNCHKOUT:	return "UNCHECKOUT";
		case METHODS::MKWRKSPC:	return "MKWORKSPACE";
		case METHODS::UPDATE:	return "UPDATE";
		case METHODS::LABEL:	return "LABEL";
		case METHODS::MERGE:	return "MERGE";
		case METHODS::BSELNCNTRL:return "BASELINE-CONTROL";
		case METHODS::MKACTIV:	return "MKACTIVITY";

			//RFC 3648
		case METHODS::ORDPCH:	return "ORDERPATCH";

			//RFC 3744
		case METHODS::ACL:		return "ACL";

			//Another
		case METHODS::PATCH:	return "PATCH";
		case METHODS::PRI:		return "PRI";
		case METHODS::POLL:		return "POLL";
		case METHODS::SRCH:		return "SEARCH";
		case METHODS::SPACEJMP:	return "SPACEJUMP";

		case METHODS::LINK:		return "LINK";
		case METHODS::UNLINK:	return "UNLINK";

		case METHODS::MKRDCTREF:return "MKREDIRECTREF";
		case METHODS::UPRDCTREF:return "UPDATEREDIRECTREF";
		case METHODS::MKCLNDR:	return "MKCALENDAR";
		case METHODS::REBIND:	return "REBIND";
		case METHODS::UNBIND:	return "UNBIND";
		}
		return "";
	}

	static int GetMethodIndex(const char* MethodName)
	{
		auto r = HTTPMethods[MethodName];
		if(r == nullptr)
			return METHODS::WRONG;
		return *r;
	}

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
			SUCCESS,
			NOT_HAVE_METHOD,
			NOT_HTTP,
			NOT_HAVE_VER_PROTOCOL,
			OUT_OF_MAX_SIZE_BUFFER,
			NOT_ALLOC_MEMORY,
			WRONG_GET_METHOD,
			HEADER_NOT_HAVE_URI,
			NOT_READED_FROM_SOCKET,
			NOT_HAVE_DATA_IN_SOCKET
		};
	};

	struct METHODS
	{
		enum 
		{
			RESPONSE = 0,	//HTTP/VER

			//Basic
			//RFC 2616
			GET,			//GET
			POST,			//POST
			OPT,			//OPTIONS	(Get options for this server)
			HEAD,			//HEAD
			PUT,			//PUT
			CONN,			//CONNECT	(Connection with proxy server)
			TRACE,			//TRACE
			DEL,			//DELETE

			//WebDAV
			//RFC 2518
			PROPFIND,		//PROPFIND	(Get property of file)
			PROPPATCH,		//PROPPATCH	(Changing the properties of a file or directory)
			MKCOL,			//MKCOL		(Create catalog)
			COPY,			//COPY		(Copy catalog or file)
			MOVE,			//MOVE		(Move file or catalog)
			LOCK,			//LOCK
			UNLOCK,			//UNLOCK

			//For version control
			//RFC 3253
			VERCNTRL,		//VERSION-CONTROL
			REPORT,			//REPORT
			CHKOUT,			//CHECKOUT
			CHKIN,			//CHECKIN
			UNCHKOUT,		//UNCHECKOUT
			MKWRKSPC,		//MKWORKSPACE
			UPDATE,			//UPDATE
			LABEL,			//LABEL
			MERGE,			//MERGE
			BSELNCNTRL,		//BASELINE-CONTROL
			MKACTIV,		//MKACTIVITY


			//RFC 3648
			ORDPCH,			//ORDERPATCH

			//RFC 3744
			ACL,			//ACL

			//Another
			PATCH,			//PATCH
			PRI,			//PRI
			POLL,			//POLL
			SRCH,			//SEARCH
			SPACEJMP,		//SPACEJUMP

			LINK,			//LINK
			UNLINK,			//UNLINK

			MKRDCTREF,		//MKREDIRECTREF
			UPRDCTREF,		//UPDATEREDIRECTREF
			MKCLNDR,		//MKCALENDAR
			REBIND,			//REBIND
			UNBIND,			//UNBIND
			WRONG
		};
	};



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
			operator int()
			{
				return Status;
			}
		} Status;

		class
		{
			__HTTP_RECIVE_QUERY_FIELDS;
		public:
			operator const char*()
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

		class _METHOD_FILTER
		{
		public:

			class
			{
				friend _METHOD_FILTER;
				__HTTP_RECIVE_QUERY_FIELDS;
			public:
				operator size_t()
				{
					return FilteredMethod->CountUsed;
				}
			} Count;

			bool AddReciveMethod(int KnownMethodIndex)
			{
				const char* m = GetMethodName(KnownMethodIndex);	
				if(m[0] == '\0')
					return false;
				auto r = Count.FilteredMethod->Insert(m);
				if(r == nullptr)
					return false;
				*r = KnownMethodIndex;
				return true;
			}

			bool AddReciveMethod(const char* KnownMethodName)
			{
				return AddReciveMethod(GetMethodIndex(KnownMethodName));
			}

			bool AddReciveMethod(const char* UnicknownMethodName, int UnicknownMethodIndex)
			{
				auto r = Count.FilteredMethod->Insert(UnicknownMethodName);
				if(r == nullptr)
					return false;
				*r = UnicknownMethodIndex;
				return true;
			}	

			void Reset()
			{
				Count.FilteredMethod->Clear();
			}

			inline const char* In(const char* Key = nullptr)
			{
				return Count.FilteredMethod->In(Key);
			}



		} MethodFilter;


		/*
		Headers property.
		get hader val by key/
		Example get element: 
		const cahr * ContentLen = RecvQuery.Headers["Content-Length"];
		Example get count elements:
		size_t CountHeaders = RecvQuery.Headers.Count;
		Example interate:
		for(const char* Key = RecvQuery.Headers.In(); Key; Key = RecvQuery.Headers.In(Key))
		{
		const char * Val = RecvQuery.Headers[Key];
		}
		or
		for(HTTP_RECIVE_QUERY::TINTER Interator;RecvQuery.Headers.Interate(Interator) && !Interator.IsEnd;)
		{
		const char * Key, *Val;
		RecvQuery.Headers.DataByInterator(Interator, Key, Val);
		}
		*/
		_HEADERS Headers;

		/*
		Get paket date;
		string or tm structure.
		On error, all fields in tm structure equal -1.
		*/
		class
		{
			__HTTP_RECIVE_QUERY_FIELDS;
		public:

			operator const char*()
			{
				char ** r = Headers->operator[]("Date");
				if(r == nullptr)
					return nullptr;
				return *r;
			}

			operator tm()
			{
				tm RetT = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
				const char * Str = this->operator const char *();
				if(Str == nullptr)
					return RetT;
				ReadGMTTime(Str, RetT);
				return RetT;
			}
		} Date;

		class
		{
			__HTTP_RECIVE_QUERY_FIELDS;
		public:

			operator const char*()
			{
				char ** r = Headers->operator[]("Last-Modified");
				if(r == nullptr)
					return nullptr;
				return *r;
			}

			operator tm()
			{
				tm RetT = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
				const char * Str = this->operator const char *();
				if(Str == nullptr)
					return RetT;
				ReadGMTTime(Str, RetT);
				return RetT;
			}
		} LastModified;

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

	template<bool IsDynamicKeyInHash, typename TypeIndex>
	bool Recive(HASH_TABLE_STRING_KEY<char, char*, IsDynamicKeyInHash, TypeIndex>& Headers)
	{
		if(QueryUrl->IsNotHaveRecvData)
			return false;
		int CountReaded, r;
		char* EndHeader;

		ReinitFields();
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
					TypeMethod.iLastErr = ERRORS::NOT_HAVE_METHOD;
					return false;
				}
				if(!ResizeBuffer(TypeMethod.SizeBuf + 300))
					return false;
				continue;
			}
			break;
		}		

		TypeMethod.SizeHeader = (size_t)EndHeader - (size_t)TypeMethod.Buf + 4;
		char* e = (char*)StringSearch((char*)TypeMethod.Buf, "\r\n");
		*e = '\0';
		if(!ReadStartLine())
			return false;
		e += 2;
		ReadHeaders(e, &Headers);
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

	bool Recive()
	{
		if(QueryUrl->IsNotHaveRecvData)
			return false;
		int CountReaded, r;
		char* EndHeader;

		ReinitFields();
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
					TypeMethod.iLastErr = ERRORS::NOT_HAVE_METHOD;
					return false;
				}
				if(!ResizeBuffer(TypeMethod.SizeBuf + 300))
					return false;
				continue;
			}
			break;
		}		

		TypeMethod.SizeHeader = (size_t)EndHeader - (size_t)TypeMethod.Buf + 4;
		char* e = (char*)StringSearch((char*)TypeMethod.Buf, "\r\n");
		*e = '\0';
		if(!ReadStartLine())
			return false;
		e += 2;
		ReadHeaders(e, &TypeMethod.Headers);
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


typedef __HTTP_RECIVE_QUERY<true> HTTP_RECIVE_QUERY;

template<bool>
class __HTTP_SEND_QUERY
{

	typedef HASH_TABLE_STRING_KEY<char, char*, false> THASH_DATA;

#define __HTTP_SEND_QUERY_FIELDS								\
	struct{														\
	mutable std::def_var_in_union_with_constructor<std::basic_string<char>>	VerProt;\
	mutable std::def_var_in_union_with_constructor<THASH_DATA>	ConstHeaders;\
	bool	IsAutoAddDate;										\
	}

	void InitFields()
	{
		ProtVer.IsAutoAddDate = true;
		new(&ProtVer.VerProt)		std::basic_string<char>("1.1");
		new(&ProtVer.ConstHeaders)	THASH_DATA();
		QueryUrl = nullptr;
	}

	void UninitFields()
	{
		ProtVer.VerProt->~basic_string();
		ProtVer.ConstHeaders->~THASH_DATA();
	}
public:

	__HTTP_SEND_QUERY()
	{   
		InitFields();
	}

	~__HTTP_SEND_QUERY()
	{   
		UninitFields();
	}

	QUERY_URL* QueryUrl;

	typedef THASH_DATA::TINTER TINTER;

	union
	{

		class
		{
			friend __HTTP_SEND_QUERY;
			__HTTP_SEND_QUERY_FIELDS;
		public:
			operator const char*() const
			{
				return VerProt->c_str();
			}

			const char* operator= (const char* Val)
			{
				VerProt->operator=(Val);
				return Val;
			}

			std::string& operator= (std::string& Val)
			{
				VerProt->operator=(Val);
				return Val;
			}
		} ProtVer;

		class
		{
			__HTTP_SEND_QUERY_FIELDS;
		public:
			operator bool() const
			{
				return IsAutoAddDate;
			}

			bool operator =(bool Val) 
			{
				return IsAutoAddDate = Val;
			}
		} IsAutoAddDate;

		class _HEADERS
		{
		public:
			class
			{
				friend _HEADERS;
				__HTTP_SEND_QUERY_FIELDS;
			public:
				operator size_t()
				{
					return ConstHeaders->CountUsed;
				}
			} Count;

			const char* operator[](const char* Key)
			{
				char ** r = Count.ConstHeaders->operator[](Key);
				if(r == nullptr)
					return nullptr;
				return *r;
			}

			bool Insert(const char* Key, const char* Val)
			{

				char ** r = Count.ConstHeaders->operator[](Key);
				if(r == nullptr)
				{
					r = Count.ConstHeaders->Insert(Key);
					if(r == nullptr)
						return false;
				}
				*r = (char*)Val;
			}

			inline const char* In(const char* Key = nullptr) const
			{
				return Count.ConstHeaders->In(Key);
			}

			bool Interate(TINTER* Interator) const
			{
				return Count.ConstHeaders->Interate(Interator);
			}

			bool DataByInterator(TINTER* Interator, const char** Key, const char** Val) const
			{
				return Count.ConstHeaders->DataByInterator(Interator, (char**)Key, (char**)Val);
			}
		} ConstHeaders;
	};

	bool SendResponse
		(
		int Stat, 
		const char* StatMsg, 
		HTTP_RECIVE_QUERY::_HEADERS& Headers
		) 
	{
		return SendResponse(Stat, StatMsg, Headers.Count.Headers.operator HTTP_RECIVE_QUERY::THASH_DATA &());
	}

	template<bool IsDynamicKeyInHash, typename TypeIndex>
	bool SendResponse
		(
		int Stat, 
		const char* StatMsg, 
		HASH_TABLE_STRING_KEY<char, char*, IsDynamicKeyInHash, TypeIndex>& Headers
		) const
	{
		if(StatMsg == nullptr)
			StatMsg = HTTP_RECIVE_QUERY::GetMsgByStatus(Stat);
		std::basic_string<char> ResponseBuf("", 50);
		unsigned s  = sprintf_s((char*)ResponseBuf.c_str(), 48, "HTTP/%s %i ", (const char*)ProtVer, Stat);
		ResponseBuf.resize(s);	
		ResponseBuf.append(StatMsg);
		ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
		if(IsAutoAddDate)
		{
			tm CurGMT;
			HTTP_RECIVE_QUERY::GetCurGMT(CurGMT);
			ResponseBuf.append("Date: ", sizeof("Date: ") - 1);
			char DateBuf[HTTP_RECIVE_QUERY::MaxTimeLen + 2];
			HTTP_RECIVE_QUERY::WriteGMTTime(DateBuf, sizeof(DateBuf), CurGMT);
			ResponseBuf.append(DateBuf, HTTP_RECIVE_QUERY::MaxTimeLen - 1);
			ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
		}
		for(TINTER i; ConstHeaders.Interate(&i) && !i.IsEnd;)
		{
			const char* Key;
			const char* Val;
			ConstHeaders.DataByInterator(&i, &Key, &Val); 
			ResponseBuf.append(Key);
			ResponseBuf.append(": ", sizeof(": ") - 1);
			ResponseBuf.append(Val);
			ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
		}

		for(TINTER i; Headers.Interate(&i) && !i.IsEnd;)
		{
			char* Key;
			char* Val;
			Headers.DataByInterator(&i, &Key, &Val);
			ResponseBuf.append(Key);
			ResponseBuf.append(": ", sizeof(": ") - 1);
			ResponseBuf.append(Val);
			ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
		}

		ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
		return QueryUrl->Send(ResponseBuf);
	}

	int SendResponse
		(
		int Stat, 
		const char* StatMsg = nullptr
		) const
	{
		if(StatMsg == nullptr)
			StatMsg = HTTP_RECIVE_QUERY::GetMsgByStatus(Stat);
		std::basic_string<char> ResponseBuf("", 50);
		unsigned s = sprintf_s((char*)ResponseBuf.c_str(), 48, "HTTP/%s %i ", (const char*)ProtVer, Stat);
		ResponseBuf.resize(s);	
		ResponseBuf.append(StatMsg);
		ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
		if(IsAutoAddDate)
		{
			tm CurGMT;
			HTTP_RECIVE_QUERY::GetCurGMT(CurGMT);
			ResponseBuf.append("Date: ", sizeof("Date: ") - 1);
			char DateBuf[HTTP_RECIVE_QUERY::MaxTimeLen + 2];
			HTTP_RECIVE_QUERY::WriteGMTTime(DateBuf, sizeof(DateBuf), CurGMT);
			ResponseBuf.append(DateBuf, HTTP_RECIVE_QUERY::MaxTimeLen - 1);
			ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
		}
		for(TINTER i; ConstHeaders.Interate(&i) && !i.IsEnd;)
		{
			const char* Key;
			const char* Val;
			ConstHeaders.DataByInterator(&i, &Key, &Val); 
			ResponseBuf.append(Key);
			ResponseBuf.append(": ", sizeof(": ") - 1);
			ResponseBuf.append(Val);
			ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
		}
		ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
		return QueryUrl->Send(ResponseBuf);
	}


	bool SendQuery
	(
		unsigned char TypeMethod, 
		const char * Path,
		HTTP_RECIVE_QUERY::_HEADERS& Headers
	) 
	{
		return SendQuery(TypeMethod, Path, Headers.Count.Headers.operator HTTP_RECIVE_QUERY::THASH_DATA &());
	}

	bool SendQuery
	(
		const char* MethodStr, 
		const char * Path,
		HTTP_RECIVE_QUERY::_HEADERS& Headers
	) 
	{
		return SendQuery(MethodStr, Path, Headers.Count.Headers.operator HTTP_RECIVE_QUERY::THASH_DATA &());
	}

	int SendQuery
		(
		unsigned char TypeMethod, 
		const char * Path = "/"
		) const
	{
		const char * StrMethod = HTTP_RECIVE_QUERY::GetMethodName(TypeMethod);
		if(StrMethod[0] == '\0')
			return false;
		return SendQuery(StrMethod, Path);
	}

	template<bool IsDynamicKeyInHash, typename TypeIndex>
	int SendQuery
		(
		unsigned char TypeMethod, 
		const char * Path, 
		HASH_TABLE_STRING_KEY<char, char*, IsDynamicKeyInHash, TypeIndex>& Headers
		) const
	{
		const char * StrMethod = HTTP_RECIVE_QUERY::GetMethodName(TypeMethod);
		if(StrMethod[0] == '\0')
			return false;
		return SendQuery(StrMethod, Path, Headers);
	}

	int SendQuery
		(
		const char* MethodStr, 
		const char* Path = "/"
		) const
	{
		std::basic_string<char> ResponseBuf = MethodStr;
		ResponseBuf.append(" ", 1);
		ResponseBuf.append(Path);
		ResponseBuf.append(" ", 1);
		if(!ProtVer.VerProt->empty())
		{
			ResponseBuf.append("HTTP/", sizeof("HTTP/") - 1);
			ResponseBuf.append(ProtVer.VerProt->c_str());
		}
		ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
		for(TINTER i; ConstHeaders.Interate(&i) && !i.IsEnd;)
		{
			const char* Key;
			const char* Val;
			ConstHeaders.DataByInterator(&i, &Key, &Val); 
			ResponseBuf.append(Key);
			ResponseBuf.append(": ", sizeof(": ") - 1);
			ResponseBuf.append(Val);
			ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
		}
		ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
		return QueryUrl->Send(ResponseBuf);
	}

	template<bool IsDynamicKeyInHash, typename TypeIndex>
	int SendQuery
		(
		const char* MethodStr, 
		const char* Path, 
		HASH_TABLE_STRING_KEY<char, char*, IsDynamicKeyInHash, TypeIndex>& Headers
		) const
	{
		std::basic_string<char> ResponseBuf = MethodStr;
		ResponseBuf.append(" ", 1);
		ResponseBuf.append(Path);
		ResponseBuf.append(" ", 1);
		if(!ProtVer.VerProt->empty())
		{
			ResponseBuf.append("HTTP/", sizeof("HTTP/") - 1);
			ResponseBuf.append(ProtVer.VerProt->c_str());
		}
		ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
		for(TINTER i; ConstHeaders.Interate(&i) && !i.IsEnd;)
		{
			const char* Key;
			const char* Val;
			ConstHeaders.DataByInterator(&i, &Key, &Val); 
			ResponseBuf.append(Key);
			ResponseBuf.append(": ", sizeof(": ") - 1);
			ResponseBuf.append(Val);
			ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
		}
		for(TINTER i; Headers.Interate(&i) && !i.IsEnd;)
		{
			char* Key, *Val;
			Headers.DataByInterator(&i, &Key, &Val);
			ResponseBuf.append(Key);
			ResponseBuf.append(": ", sizeof(": ") - 1);
			ResponseBuf.append(Val);
			ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
		}
		ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
		return QueryUrl->Send(ResponseBuf);
	}

};


template<bool d>
long long __HTTP_RECIVE_QUERY<d>::GmtAccuracy = 0;

template<bool d>
HASH_TABLE_STRING_KEY<char, unsigned char, false, unsigned char> __HTTP_RECIVE_QUERY<d>::HTTPMethods;



#endif
