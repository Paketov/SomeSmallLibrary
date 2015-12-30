#ifndef __QUERYURLHTTP_H_HAS_INCLUDED__
#define __QUERYURLHTTP_H_HAS_INCLUDED__

/*
Warning!
This library not thread safe!
You have to manually put the critical section or block in your code.
This is made to minimize the overhead for thread synchronization.
*/

#include <time.h>
#include "ExQueryUrl.h"
#include "ExHashTable.h"


template<typename>
class ___EX_HTTP
{

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
					e = i;
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

public:
	struct URI_REGEX
	{
		/*
		RegEx based on RFC 3987 https://www.ietf.org/rfc/rfc3987.txt
		*/

		static std::basic_string<char> Scheme;
		static std::basic_string<char> PctEncodedChar;
		static std::basic_string<char> UnreservedChar;
		static std::basic_string<char> SubDelimsChar;
		static std::basic_string<char> PcharChar;
		static std::basic_string<char> RegNameChar;
		static std::basic_string<char> QueryChar;
		static std::basic_string<char> QueryKeyValChar;
		static std::basic_string<char> FragmentChar;


		static std::basic_string<char> UserInfo;//iuserinfo = iunreserved | pct-encoded | sub-delims | ":"


		static std::basic_string<char> IPv4Segment;
		static std::basic_string<char> IPv4Address;
		static std::basic_string<char> IPv4RegEx;

		static std::basic_string<char> IPv6Segment;

		static std::basic_string<char> IPv6Full;
		static std::basic_string<char> IPv6Short1;    //1::
		static std::basic_string<char> IPv6Short2;
		static std::basic_string<char> IPv6Short3;
		static std::basic_string<char> IPv6Short4;
		static std::basic_string<char> IPv6Short5;
		static std::basic_string<char> IPv6Short6;
		static std::basic_string<char> IPv6Short7;
		static std::basic_string<char> IPv6Short8; //::2:3:4:5:6:7:8
		static std::basic_string<char> IPv6_IPv4Mapped; //::255.255.255.255 отображённый IPv4
		static std::basic_string<char> IPv6_IPv4Embedded; //2001:db8:3:4::192.0.2.33 встроенный IPv4

		static std::basic_string<char> IPv6Address;

		static std::basic_string<char> IPv6RegEx;

		static std::basic_string<char> SchemeRegEx;
		static std::basic_string<char> HostRegEx;//ihost = IP-literal | IPv4address | ireg-name
		static std::basic_string<char> PortRegEx;
		static std::basic_string<char> PathRegEx;
		static std::basic_string<char> ArgRegEx;
		static std::basic_string<char> QueryRegEx;
		static std::basic_string<char> FragmentRegEx;

		static std::basic_string<char> AuthorityRegEx;//iauthority = [ iuserinfo "@" ] ihost [ ":" port ]

		static std::basic_string<char> HierPart;

		static std::basic_string<char> IRIRegEx;

	};



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
	
	static int GetMethodIndex(const char* MethodName)
	{
		static HASH_TABLE_STRING_KEY<char, unsigned char, false, unsigned char> HTTPMethods(10);
		static bool a = ([](decltype(HTTPMethods)& v) -> bool 
						{					
							for(unsigned char i = 0; i < METHODS::WRONG; i++)
							{
								const char* r =  GetMethodName(i);
								if(r[0] != '\0')
									*v.Insert(r) = i;
							}
							return true;
						})(HTTPMethods);

		auto r = HTTPMethods[MethodName];
		if(r == nullptr)
			return METHODS::WRONG;
		return *r;
	}



	template<typename TypeUserData>
	static int Recive
	(
		QUERY_URL* QueryUrl,
		TypeUserData UsrData,
		bool (*ResponseFunc)(TypeUserData UsrData, int Status, const char* Msg, const char* ProtoVer),
		bool (*QueryFunc)(TypeUserData UsrData, const char* Method, const char* Path, const char* ProtoVer),
		bool (*HeadersFunc)(TypeUserData UsrData, const char* Key, const char* Val),
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


	template<typename TypeUserData>
	static int SendQuery
	(
		QUERY_URL* QueryUrl,
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


	/*
	Without headers
	*/
	static inline int SendQuery
	(
		QUERY_URL* QueryUrl,
		const char* MethodStr, 
		const char* Path = "/",
		const char* ProtoVersion = "1.1"
	) {return SendQuery<char>(QueryUrl, MethodStr, Path, 0, [](char, char**, char**){ return false;}, ProtoVersion); }

	template<typename TypeUserData>
	static int SendQuery
	(
		QUERY_URL* QueryUrl,
		int TypeMethod, 
		const char* Path, 
		TypeUserData UsrData, 
		bool (*HeadersEnumFunc)(TypeUserData UsrData, char ** Key, char ** Val),
		const char* ProtoVersion = "1.1"
	)
	{
		const char * StrMethod = GetMethodName(TypeMethod);
		if(StrMethod[0] == '\0')
			return false;
		return  SendQuery(QueryUrl, StrMethod, Path, UsrData, HeadersEnumFunc, ProtoVersion);
	}

	/*
	Without headers
	*/
	static inline int SendQuery
	(
		QUERY_URL* QueryUrl,
		int TypeMethod = METHODS::GET, 
		const char* Path = "/",
		const char* ProtoVersion = "1.1"
	) {return SendQuery<char>(QueryUrl, TypeMethod, Path, 0, [](char, char**, char**){ return false;}, ProtoVersion); }

	template<typename TypeUserData>
	static int SendResponse
	(
		QUERY_URL* QueryUrl,
		int Stat, 		
		TypeUserData UsrData, 
		bool (*HeadersEnumFunc)(TypeUserData UsrData, char ** Key, char ** Val),
		const char* StatMsg = nullptr, 
		const char* ProtoVersion = "1.1"
	)
	{
		if(StatMsg == nullptr)
			StatMsg = GetMsgByStatus(Stat);
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

	
	/*
	Without headers
	*/
	static inline int SendResponse
	(
		QUERY_URL* QueryUrl,
		int Stat = 200,
		const char* StatMsg = nullptr, 
		const char* ProtoVersion = "1.1"
	) {return SendResponse<char>(QueryUrl, Stat, 0, [](char, char**,char**){ return false;}, StatMsg, ProtoVersion);}
};


std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::Scheme = "(?:[:alpha:][[:alnum:]+-.]*)";
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::PctEncodedChar = "(?:%[0-9a-fA-F]{2})";
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::UnreservedChar = "[[:alnum:]-._~]";
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::SubDelimsChar = "[!&'()*+,;=\\$]";
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::PcharChar = "(?:[[:alnum:]-._~!&'()*+,;=:@\\$]|(?:%[0-9a-fA-F]{2}))";
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::RegNameChar = "(?:[[:alnum:]-._~!&'()*+,;=\\$]|(?:%[0-9a-fA-F]{2}))";
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::QueryChar = "(?:[[:alnum:]-._~!&'()/?*+,;=:@\\$]|(?:%[0-9a-fA-F]{2}))";
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::QueryKeyValChar = "(?:[[:alnum:]-._~!'()/?*+,;:@\\$]|(?:%[0-9a-fA-F]{2}))";
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::FragmentChar = "(?:[[:alnum:]-._~!&'/?()*+,;=:@\\$]|(?:%[0-9a-fA-F]{2}))";


std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::UserInfo = "(?:[[:alnum:]-._~!&'()*+,;:=\\$]|(?:%[0-9a-fA-F]{2}))";//iuserinfo = iunreserved | pct-encoded | sub-delims | ":"


std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::IPv4Segment = "(?:25[0-5]|(?:2[0-4]|1{0,1}[0-9]){0,1}[0-9])";
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::IPv4Address = "(?:(?:"+IPv4Segment+"\\.){3,3}"+IPv4Segment+")";
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::IPv4RegEx = "("+IPv4Address+")";

std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::IPv6Segment = "[0-9a-fA-F]{1,4}";

std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::IPv6Full = "(?:(?:"+IPv6Segment+":){7,7}"+IPv6Segment+")";
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::IPv6Short1 = "(?:(?:"+IPv6Segment+":){1,7}:)";    //1::
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::IPv6Short2 = "(?:(?:"+IPv6Segment+":){1,6}:"+IPv6Segment+")";
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::IPv6Short3 = "(?:(?:"+IPv6Segment+":){1,5}(?::"+IPv6Segment+"){1,2})";
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::IPv6Short4 = "(?:(?:"+IPv6Segment+":){1,4}(?::"+IPv6Segment+"){1,3})";
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::IPv6Short5 = "(?:(?:"+IPv6Segment+":){1,3}(?::"+IPv6Segment+"){1,4})";
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::IPv6Short6 = "(?:(?:"+IPv6Segment+":){1,2}(?::"+IPv6Segment+"){1,5})";
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::IPv6Short7 = "(?:"+IPv6Segment+":(?:(?::"+IPv6Segment+"){1,6}))";
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::IPv6Short8 = "(?::(?:(?::"+IPv6Segment+"){1,7}|:))"; //::2:3:4:5:6:7:8
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::IPv6_IPv4Mapped = "(?:::(?:ffff(?::0{1,4}){0,1}:){0,1}"+IPv4Address+")"; //::255.255.255.255 отображённый IPv4
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::IPv6_IPv4Embedded = "(?:(?:"+IPv6Segment+":){1,4}:"+IPv4Address+")"; //2001:db8:3:4::192.0.2.33 встроенный IPv4

std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::IPv6Address = "(?:"
	+IPv6Full+"|"
	+IPv6Short1+"|"
	+IPv6Short2+"|"
	+IPv6Short3+"|"
	+IPv6Short4+"|"
	+IPv6Short5+"|"
	+IPv6Short6+"|"
	+IPv6Short7+"|"
	+IPv6Short8+"|"
	+IPv6_IPv4Mapped+"|"
	+IPv6_IPv4Embedded+
	")";

std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::IPv6RegEx = "(?:\\[("+IPv6Address+")\\])";

std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::SchemeRegEx = "(?:("+Scheme+")://)";
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::HostRegEx = "(?:"+IPv4RegEx+"|"+IPv6RegEx+"|("+RegNameChar+"+))";//ihost = IP-literal | IPv4address | ireg-name
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::PortRegEx = "(?::(\\d{1,5}))";
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::PathRegEx = "((?:/"+PcharChar+"*)+)";
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::ArgRegEx = "(?:("+QueryKeyValChar+"+)=("+QueryKeyValChar+"+)(?:&|$))";
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::QueryRegEx = "(?:\\?("+QueryChar+"*)+)";
std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::FragmentRegEx = "(?:#("+FragmentChar+"*))";

std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::AuthorityRegEx = "(?:(?:("+UserInfo+"+)@)?"+HostRegEx+PortRegEx+"?)";//iauthority = [ iuserinfo "@" ] ihost [ ":" port ]

std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::HierPart = "(?://"+AuthorityRegEx+PathRegEx+")";

std::basic_string<char> ___EX_HTTP<char>::URI_REGEX::IRIRegEx = "(?:"+SchemeRegEx+"?"+AuthorityRegEx+")?"+PathRegEx+"?"+QueryRegEx+"?"+FragmentRegEx+"?";

typedef ___EX_HTTP<char> EX_HTTP;
#endif
