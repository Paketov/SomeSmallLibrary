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

	bool CheckMethod(char* s)
	{
		size_t i = 0;
		for(;(s[i] == '\r') && (s[i + 1] == '\n'); i += 2);
		size_t j = i;
		if((s[i] < 'A') && (s[i] > 'Z'))
			return false;
		for(i++ ;((s[i] >= 'A') && (s[i] <= 'Z')) || (s[i] == '-'); i++);
		if(i <= j)
			return false;
		char c = s[i];
		s[i] = '\0';
		int CurMethod = MethodFilter[s + j];
		s[i] = c;
		if(CurMethod == METHODS::WRONG)
			return false;
		if(CurMethod == METHODS::RESPONSE) 
			return (s[i] == ' ') || (s[i] == '\t') || (s[i] == '/') && IsDigit(s[i]);
		return (s[i] == ' ') || (s[i] == '\t');
	}

	static bool CheckMethodRow(char* s)
	{
		size_t i = 0;
		for(;(s[i] == '\r') && (s[i + 1] == '\n'); i += 2);
		size_t j = i;
		if((s[i] < 'A') && (s[i] > 'Z'))
			return false;
		for(i++ ;((s[i] >= 'A') && (s[i] <= 'Z')) || (s[i] == '-'); i++);

		if(
			((i - j) == 4) && 
			(s[j] == 'H') && 
			(s[j + 1] == 'T') && 
			(s[j + 2] == 'T') &&
			(s[j + 3] == 'P')
		) 
		{
		   return (s[i] == ' ') || (s[i] == '\t') || ((s[i] == '/') && IsDigit(s[i + 1]));
		}
		return (i > j) && ((s[i] == ' ') || (s[i] == '\t'));
	}
	
	static int ReadStartLineRow
	(
		char* s,
		char** Method,
		unsigned* Status,
		char** StatusMsg_Uri,
		char** Ver,
		bool* IsResponse = std::make_default_pointer()
	)
	{
		size_t i = 0;
		for(;(s[i] == '\r') && (s[i + 1] == '\n'); i += 2);
		size_t j = i;

		//If method started not a letter
		if((s[i] < 'A') && (s[i] > 'Z'))
			return -1;
		//Read method
		for(i++ ;((s[i] >= 'A') && (s[i] <= 'Z')) || (s[i] == '-'); i++);

		if(
			((i - j) == 4) && 
			(s[j] == 'H') && 
			(s[j + 1] == 'T') && 
			(s[j + 2] == 'T') &&
			(s[j + 3] == 'P')
		) 
		{
			//If get response
			*IsResponse = true;

			size_t e = i;
			if(s[i] == '/')
			{
				//Read version
				for(i++ ;((s[i] >= '0') && (s[i] <= '9')) || (s[i] == '.'); i++);

				if(i > (e + 1))
				{					
					if((s[i] == '\r') && (s[i + 1] == '\n'))
						return -1;
					*Ver = s + (e + 1);
					s[i] = '\0';
				}
			}

			if((s[i] != ' ') && (s[i] != '\t'))
				return -1;
			s[e] = '\0';
			*Method = s + j; 
			//Skip spaces
			for(i++; (s[i] == '\t') || (s[i] == ' '); i++);
			{
				unsigned Stat = 0;
				//Read status number
				for(unsigned char Digit; (Digit = s[i] - '0') <= 9; i++)
					Stat = Stat * 10 + Digit;
				*Status = Stat;
			}
			for(; (s[i] == '\t') || (s[i] == ' '); i++);

			for(size_t j = i; ;i++)
				if((s[i] == '\r') && (s[i + 1] == '\n'))
				{
					if(i > j)
						*StatusMsg_Uri = s + j;
					else
						*StatusMsg_Uri = (char*)GetMsgByStatus(*Status);
					goto lblOut2;
				}
		}else
		{
			*IsResponse = false;
			if((s[i] != ' ') && (s[i] != '\t'))
				return -1;
			s[i] = '\0';
			*Method = s + j;
			for(i++; (s[i] == '\t') || (s[i] == ' '); i++);

			//Read URI
			{
				size_t StartQuery = i;
				for(; !IsSpace(s[i]); i++);
				if(i <= StartQuery)
					return -1;
				*StatusMsg_Uri = s + StartQuery;	
			}
			if((s[i] == '\r') && (s[i + 1] == '\n'))
				goto lblOut2;
			s[i] = '\0';		
			for(i++; (s[i] == '\t') || (s[i] == ' '); i++);

			if((s[i] == 'H') && (s[i + 1] == 'T') && (s[i + 2] == 'T') && (s[i + 3] == 'P')) 
			{
				i += 4;
				if(s[i] == '/')
				{
					size_t StartVer = ++i;
					//Read version
					for(;((s[i] >= '0') && (s[i] <= '9')) || (s[i] == '.'); i++);

					if(i > StartVer)
					{					
						*Ver = s + StartVer;
						if((s[i] == '\r') && (s[i + 1] == '\n'))
							goto lblOut2;
						s[i] = '\0';
						i++;
					}
				}
			}
		}

		for(; ;i++)
			if((s[i] == '\r') && (s[i + 1] == '\n'))
			{
lblOut2:
				s[i] = '\0';
				i += 2;
				break;
			}

		return i;
	}


	template<typename TypeUserData>
	static int ReadHeadersRow
	(
		char* s,
		TypeUserData UsrData,
		bool (*HeadersFunc)(TypeUserData UsrData, const char * Key, const char * Val)
	)
	{
		unsigned i = 0, j, k;
		char *Key;
		while(true)
		{
lblMainLoop:
			for(;;i++)
			{
				switch(s[i])
				{
				case ' ': case '\t': case '\n': case '\v': case '\f':
					continue;
				case ':':
lblSkip:
					for(;;i++)
					{
						if((s[i] == '\r') && (s[i + 1] == '\n'))
						{
							i += 2;
							goto lblMainLoop;
						}else if(s[i] == '\0')
							return i;
					}
				case '\r':
					if(s[i + 1] == '\n')
					{
						i += 2;
						goto lblMainLoop;
					}
					continue;
				case '\0':
					return i;
				}
				if((s[i] >= 'a') && (s[i] <= 'z') || (s[i] >= 'A') && (s[i] <= 'Z'))
				   goto lblReadKey;
			}	
lblReadKey:
			Key = s + i;
			//Read key
			for(i++ ;((s[i] >= 'a') && (s[i] <= 'z')) || ((s[i] >= 'A') && (s[i] <= 'Z')) || (s[i] == '-');i++);
			j = i;
			for(;(s[i] == ' ') || (s[i] == '\t'); i++);

			if((s[i] == '\r') && (s[i + 1] == '\n'))
			{
				i += 2;
				goto lblMainLoop;
			}
			else if(s[i] == '\0')
				return i;
			else if(s[i] != ':')
				goto lblSkip;

			//Skip spaces
			for(i++;(s[i] == ' ') || (s[i] == '\t'); i++);
		
			//Read val
			for(k = i;;i++)
			{
				if((s[i] == '\r') && (s[i + 1] == '\n'))
				{
					if(i > k)
					{
						s[i] = s[j] = '\0';
						if(!HeadersFunc(UsrData, Key, s + k))
							return -1;
					}
					i += 2;
					goto lblMainLoop;
				}else if(s[i] == '\0')
				{
					if(i > k)
					{
						s[j] = '\0';
						if(!HeadersFunc(UsrData, Key, s + k))
							return -1;
					}
					return i;
				}
			}
		}
		return i;
	}


	static size_t SkipInSockBuffer(QUERY_URL* Sock, void* Buf, size_t SizeBuf, size_t CountSkip)
	{
		size_t Len = CountSkip;
		do{
			size_t l = (Len > SizeBuf)? SizeBuf: Len;
			int CountReaded = Sock->Recive(Buf, l);
			if(CountReaded <= 0)
				break;
			Len -= CountReaded;
		} while(Len > 0);
		return CountSkip - Len;
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

	static bool WriteGMTTime(char * TimeStr, tm& InTm)
	{
		static const char  *Week[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
		static const char  *Months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
		auto r = sprintf
			(
			TimeStr,
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
		return  timegm(&InTm) - GmtAccuracy;
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
			NOT_HAVE_DATA_IN_SOCKET,
			INVALID_START_LINE,
			USER_INTERRUPT,
			INVALID_HEADER
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
			operator size_t() const
			{
				return MaxSizeBuffer;
			}
			size_t operator = (size_t New)
			{
				return MaxSizeBuffer = New;
			}
		} MaxSizeBuffer;

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


			int operator[](const char * Key)
			{
				auto r = Count.FilteredMethod->operator[](Key);
				if(r == nullptr)
					return METHODS::WRONG;
				return *r;
			}

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
	bool Recive(HASH_TABLE_STRING_KEY<char, char*, IsDynamicKeyInHash, TypeIndex>& Headers, bool IsPeek = false)
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
			for(char* c = (char*)TypeMethod.Buf; c[3] != '\0'; c++)
				if((c[0] == '\r') && (c[1] == '\n') && (c[2] == '\r') && (c[3] == '\n'))
				{
					EndHeader = c;
					break;
				}
			if(EndHeader == nullptr)
			{
				if(!CheckMethod((char*)TypeMethod.Buf))
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
		EndHeader[4] = '\0';
		int i;
		{
			char *Met = "", *StatMsgURI = Met, *Ver = Met;
			unsigned Stat = 0;
			bool IsResponse = false;
			
			if((i = ReadStartLineRow((char*)TypeMethod.Buf, &Met, &Stat, &StatMsgURI, &Ver, &IsResponse)) == -1)
			{
				TypeMethod.iLastErr = ERRORS::INVALID_START_LINE;
				return false;
			}
			if(IsResponse)
			{
				TypeMethod.StatusMsg = StatMsgURI;
				TypeMethod.Status = Stat;
				TypeMethod.VerProt = Ver;
			}else
			{
				TypeMethod.Path = StatMsgURI;
				TypeMethod.VerProt = Ver;
			}
			TypeMethod.MethodString = Met;
		}

		if(ReadHeadersRow((char*)TypeMethod.Buf + i, &Headers, 
			[](decltype(&Headers) Headr, const char* Key, const char* Val)
			{
				auto r = Headr->Insert(Key);
				if(r != nullptr)
					*r = (char*)Val;
				return true;
			}
		) == -1)
		{
			TypeMethod.iLastErr = ERRORS::INVALID_HEADER;
			return false;
		}
		const char * ContLength = Headers["Content-Length"];
		if(ContLength != nullptr)
		{
			unsigned l = 0;
			sscanf(ContLength, "%u", &l);
			TypeMethod.ContentLength = l;
		}
				
		if(!IsPeek)
		{
			char TmpBuf[1024];
			SkipInSockBuffer(QueryUrl, TmpBuf, sizeof(TmpBuf) - 2, TypeMethod.SizeHeader);
		}
		return true;
	}

	bool Recive(bool IsPeek = false)
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
			for(char* c = (char*)TypeMethod.Buf; c[3] != '\0'; c++)
				if((c[0] == '\r') && (c[1] == '\n') && (c[2] == '\r') && (c[3] == '\n'))
				{
					EndHeader = c;
					break;
				}
			if(EndHeader == nullptr)
			{
				if(!CheckMethod((char*)TypeMethod.Buf))
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
		EndHeader[4] = '\0';
		int i;
		{
			char *Met = "", *StatMsgURI = Met, *Ver = Met;
			unsigned Stat = 0;
			bool IsResponse = false;
			
			if((i = ReadStartLineRow((char*)TypeMethod.Buf, &Met, &Stat, &StatMsgURI, &Ver, &IsResponse)) == -1)
			{
				TypeMethod.iLastErr = ERRORS::INVALID_START_LINE;
				return false;
			}
			if(IsResponse)
			{
				TypeMethod.StatusMsg = StatMsgURI;
				TypeMethod.Status = Stat;
				TypeMethod.VerProt = Ver;
			}else
			{
				TypeMethod.Path = StatMsgURI;
				TypeMethod.VerProt = Ver;
			}
			TypeMethod.MethodString = Met;
		}

		if(ReadHeadersRow<THASH_DATA*>((char*)TypeMethod.Buf + i, &TypeMethod.Headers, 
			[](THASH_DATA* Headr, const char* Key, const char* Val)
			{
				auto r = Headr->Insert(Key);
				if(r != nullptr)
					*r = (char*)Val;
				return true;
			}
		) == -1)
		{
			TypeMethod.iLastErr = ERRORS::INVALID_HEADER;
			return false;
		}
		const char * ContLength = Headers["Content-Length"];
		if(ContLength != nullptr)
		{
			unsigned l = 0;
			sscanf(ContLength, "%u", &l);
			TypeMethod.ContentLength = l;
		}
		if(!IsPeek)
		{
			char TmpBuf[1024];
			SkipInSockBuffer(QueryUrl, TmpBuf, sizeof(TmpBuf) - 2, TypeMethod.SizeHeader);
		}
		return true;
	}

	template<typename TypeUserData>
	static int ReciveRow
	(
		QUERY_URL*  QueryUrl,
		TypeUserData UsrData,
		bool (*ResponseFunc)(TypeUserData UsrData, int Status, const char * Msg, const char* ProtoVer),
		bool (*QueryFunc)(TypeUserData UsrData, const char* Method, const char* Path, const char* ProtoVer),
		bool (*HeadersFunc)(TypeUserData UsrData, const char * Key, const char * Val),
		size_t* Readed = std::make_default_pointer(),
		bool IsPeek = false,
		size_t MaxLenBuf = 4096	
	)
	{
		if(QueryUrl->IsNotHaveRecvData)
			return false;

		int CountReaded, Result = ERRORS::SUCCESS, i;
		char* EndHeader = nullptr;
		char TmpBuf[1024];
		void* Buf = TmpBuf;
		size_t CurSizeBuf = 1024, SizeHeader;

		while(true)
		{
			CountReaded = QueryUrl->Recive(Buf, CurSizeBuf - 2, MSG_PEEK);
			if(CountReaded == -1)
			{
					Result = ERRORS::NOT_READED_FROM_SOCKET;
					goto lblOut;
			}else if(CountReaded == 0)
			{
					Result = ERRORS::NOT_HAVE_DATA_IN_SOCKET;
					goto lblOut;
			}
			((char*)Buf)[CountReaded] = '\0';
			for(char* c = (char*)Buf; c[3] != '\0'; c++)
				if((c[0] == '\r') && (c[1] == '\n') && (c[2] == '\r') && (c[3] == '\n'))
				{
					EndHeader = c;
					break;
				}
			if(EndHeader == nullptr)
			{
				if(!CheckMethodRow((char*)Buf))
				{
					Result = ERRORS::NOT_HAVE_METHOD;
					goto lblOut;
				}
				if(Buf == TmpBuf)
				{
					if((Buf = malloc(CurSizeBuf += 300)) == nullptr)
					{
						Result = ERRORS::NOT_ALLOC_MEMORY;
						goto lblOut;
					}
				}else
				{
					if((Buf = realloc(Buf, CurSizeBuf += 300)) == nullptr)
					{
						Result = ERRORS::NOT_ALLOC_MEMORY;
						goto lblOut;
					}
				}
				continue;
			}
			break;
		}		

		EndHeader[4] = '\0';
		SizeHeader = (size_t)EndHeader - (size_t)Buf + 4;
		{
			char *Met = "", *StatMsgURI = Met, *Ver = Met;
			unsigned Stat = 0;
			bool IsResponse = false;		
			if((i = ReadStartLineRow((char*)Buf, &Met, &Stat, &StatMsgURI, &Ver, &IsResponse)) == -1)
			{
				Result = ERRORS::INVALID_START_LINE;
				goto lblOut;
			}
			if(IsResponse)
			{
				if(!ResponseFunc(UsrData, Stat, StatMsgURI, Ver))
				{
					Result = ERRORS::USER_INTERRUPT;
					goto lblOut;
				}
			}else
			{
				if(!QueryFunc(UsrData, Met, StatMsgURI, Ver))
				{
					Result = ERRORS::USER_INTERRUPT;
					goto lblOut;
				}
			}
		}
		if(ReadHeadersRow((char*)Buf + i, UsrData, HeadersFunc) == -1)
		{
			Result = ERRORS::USER_INTERRUPT;
			goto lblOut;
		}

		if(!IsPeek)
			SkipInSockBuffer(QueryUrl, TmpBuf, sizeof(TmpBuf) - 2, SizeHeader);

lblOut:
		*Readed = SizeHeader;
		if((Buf != nullptr) && (Buf != TmpBuf))
			free(Buf);
		return Result;
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
		unsigned s  = sprintf((char*)ResponseBuf.c_str(), "HTTP/%.20s %i ", (const char*)ProtVer, Stat);
		ResponseBuf.resize(s);	
		ResponseBuf.append(StatMsg);
		ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
		if(IsAutoAddDate)
		{
			tm CurGMT;
			HTTP_RECIVE_QUERY::GetCurGMT(CurGMT);
			ResponseBuf.append("Date: ", sizeof("Date: ") - 1);
			char DateBuf[HTTP_RECIVE_QUERY::MaxTimeLen + 2];
			HTTP_RECIVE_QUERY::WriteGMTTime(DateBuf, CurGMT);
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
		unsigned s = sprintf((char*)ResponseBuf.c_str(), "HTTP/%.20s %i ", (const char*)ProtVer, Stat);
		ResponseBuf.resize(s);	
		ResponseBuf.append(StatMsg);
		ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
		if(IsAutoAddDate)
		{
			tm CurGMT;
			HTTP_RECIVE_QUERY::GetCurGMT(CurGMT);
			ResponseBuf.append("Date: ", sizeof("Date: ") - 1);
			char DateBuf[HTTP_RECIVE_QUERY::MaxTimeLen + 2];
			HTTP_RECIVE_QUERY::WriteGMTTime(DateBuf, CurGMT);
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
		ResponseBuf.append(" HTTP/", sizeof(" HTTP/") - 1);
		ResponseBuf.append(ProtVer.VerProt->c_str());
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
		ResponseBuf.append(" HTTP/", sizeof(" HTTP/") - 1);
		ResponseBuf.append(ProtVer.VerProt->c_str());
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

	template<typename TypeUserData>
	static int SendQueryRow
	(
		QUERY_URL*  QueryUrl,
		const char* MethodStr, 
		const char* Path, 
		TypeUserData UsrData, 
		bool (*HeadersEnumFunc)(TypeUserData UsrData, char ** Key, char ** Val),
		const char* ProtoVersion = "1.1"
	)
	{
		std::basic_string<char> ResponseBuf = MethodStr;
		ResponseBuf.append(" ", 1);
		ResponseBuf.append(Path);
		ResponseBuf.append(" HTTP/", sizeof(" HTTP/") - 1);
		ResponseBuf.append(ProtoVersion);
		ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
		for(char* Key, *Val; HeadersEnumFunc(UsrData, &Key, &Val);)
		{
			ResponseBuf.append(Key);
			ResponseBuf.append(": ", sizeof(": ") - 1);
			ResponseBuf.append(Val);
			ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
		}
		ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
		return QueryUrl->Send(ResponseBuf);
	}

	template<typename TypeUserData>
	static int SendQueryRow
	(
		QUERY_URL*  QueryUrl,
		int TypeMethod, 
		const char* Path, 
		TypeUserData UsrData, 
		bool (*HeadersEnumFunc)(TypeUserData UsrData, char ** Key, char ** Val),
		const char* ProtoVersion = "1.1"
	)
	{
		const char * StrMethod = HTTP_RECIVE_QUERY::GetMethodName(TypeMethod);
		if(StrMethod[0] == '\0')
			return false;
		return  SendQueryRow(QueryUrl, StrMethod, Path, UsrData, HeadersEnumFunc, ProtoVersion);
	}

	template<typename TypeUserData>
	static int SendResponseRow
	(
		QUERY_URL*  QueryUrl,
		int Stat, 
		const char* StatMsg, 
		TypeUserData UsrData, 
		bool (*HeadersEnumFunc)(TypeUserData UsrData, char ** Key, char ** Val),
		const char* ProtoVersion = "1.1"
	)
	{
		if(StatMsg == nullptr)
			StatMsg = HTTP_RECIVE_QUERY::GetMsgByStatus(Stat);
		std::basic_string<char> ResponseBuf("", 50);
		unsigned s = sprintf((char*)ResponseBuf.c_str(), "HTTP/%.20s %i ", ProtoVersion, Stat);
		ResponseBuf.resize(s);	
		ResponseBuf.append(StatMsg);
		ResponseBuf.append("\r\n", sizeof("\r\n") - 1);
		for(char* Key, *Val; HeadersEnumFunc(UsrData, &Key, &Val);)
		{
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
