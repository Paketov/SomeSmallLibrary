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


class EX_HTTP
{

	static bool CheckMethodRow(char* s);
	
	static int ReadStartLineRow
	(
		char* s,
		char** Method,
		unsigned* Status,
		char** StatusMsg_Uri,
		char** Ver,
		bool* IsResponse = std::make_default_pointer()
	);

	static int ReadHeadersRow
	(
		char* s,
		void* UsrData,
		bool (*HeadersFunc)(void* UsrData, const char * Key, const char * Val)
	);


	static size_t SkipInSockBuffer(QUERY_URL* Sock, void* Buf, size_t SizeBuf, size_t CountSkip);

public:
	struct URI_REGEX
	{
		/*
			RegEx based on RFC 3987 https://www.ietf.org/rfc/rfc3987.txt
		*/

		static const std::basic_string<char> Scheme;
		static const std::basic_string<char> PctEncodedChar;
		static const std::basic_string<char> UnreservedChar;
		static const std::basic_string<char> SubDelimsChar;
		static const std::basic_string<char> PcharChar;
		static const std::basic_string<char> RegNameChar;
		static const std::basic_string<char> QueryChar;
		static const std::basic_string<char> QueryKeyChar;
		static const std::basic_string<char> QueryValChar;
		static const std::basic_string<char> FragmentChar;


		static const std::basic_string<char> UserInfo;//iuserinfo = iunreserved | pct-encoded | sub-delims | ":"


		static const std::basic_string<char> IPv4Segment;
		static const std::basic_string<char> IPv4Address;
		static const std::basic_string<char> IPv4RegEx;

		static const std::basic_string<char> IPv6Segment;

		static const std::basic_string<char> IPv6Full;
		static const std::basic_string<char> IPv6Short1;    //1::
		static const std::basic_string<char> IPv6Short2;
		static const std::basic_string<char> IPv6Short3;
		static const std::basic_string<char> IPv6Short4;
		static const std::basic_string<char> IPv6Short5;
		static const std::basic_string<char> IPv6Short6;
		static const std::basic_string<char> IPv6Short7;
		static const std::basic_string<char> IPv6Short8; //::2:3:4:5:6:7:8
		static const std::basic_string<char> IPv6_IPv4Mapped; //::255.255.255.255 отображённый IPv4
		static const std::basic_string<char> IPv6_IPv4Embedded; //2001:db8:3:4::192.0.2.33 встроенный IPv4

		static const std::basic_string<char> IPv6Address;

		static const std::basic_string<char> IPv6RegEx;

		static const std::basic_string<char> SchemeRegEx;
		static const std::basic_string<char> HostRegEx;//ihost = IP-literal | IPv4address | ireg-name
		static const std::basic_string<char> PortRegEx;
		static const std::basic_string<char> PathRegEx;
		static const std::basic_string<char> ArgRegEx;
		static const std::basic_string<char> QueryRegEx;
		static const std::basic_string<char> FragmentRegEx;

		static const std::basic_string<char> AuthorityRegEx;//iauthority = [ iuserinfo "@" ] ihost [ ":" port ]

		static const std::basic_string<char> HierPart;

		static const std::basic_string<char> URIRegEx;

	};



	/*
	Date functions;
	*/

	static long long  GmtAccuracy;

	static const size_t MaxTimeLen = sizeof("Mon, 28 Sep 1970 06:00:00 GMT");

	static bool ReadGMTTime(const char * TimeStr, tm& OutTm);
	static bool WriteGMTTime(char * TimeStr, tm& InTm);
	static void GetCurGMT(tm& OutTm);
	static void TimeToGMT(time_t t, tm& OutTm);
	inline static time_t GMTToTime(tm& InTm);
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
			SOCKET_HAS_DISCONNECTED,
			INVALID_START_LINE,
			USER_INTERRUPT,
			INVALID_HEADER
		};
	};

	static const char* GetMethodName(int Number);

	static const char* GetMsgByStatus(int Status);

	static int GetMethodIndex(const char* MethodName);

	static int Recive
	(
		QUERY_URL* QueryUrl,
		void* UsrData,
		bool (*ResponseFunc)(void*  UsrData, int Status, const char* Msg, const char* ProtoVer),
		bool (*QueryFunc)(void*  UsrData, const char* Method, const char* Path, const char* ProtoVer),
		bool (*HeadersFunc)(void*  UsrData, const char* Key, const char* Val),
		size_t* Readed = std::make_default_pointer(),
		bool IsPeek = false,
		size_t MaxLenBuf = 4096	
	);


	static int SendQuery
	(
		QUERY_URL* QueryUrl,
		const char* MethodStr, 
		const char* Path, 
		void* UsrData, 
		bool (*HeadersEnumFunc)(void* UsrData, char ** Key, char ** Val),
		const char* ProtoVersion = "1.1"
	);


	/*
	Without headers
	*/
	static inline int SendQuery
	(
		QUERY_URL* QueryUrl,
		const char* MethodStr, 
		const char* Path = "/",
		const char* ProtoVersion = "1.1"
	) {return SendQuery(QueryUrl, MethodStr, Path, 0, [](void*, char**, char**){ return false;}, ProtoVersion); }

	static int SendQuery
	(
		QUERY_URL* QueryUrl,
		int TypeMethod, 
		const char* Path, 
		void* UsrData, 
		bool (*HeadersEnumFunc)(void* UsrData, char ** Key, char ** Val),
		const char* ProtoVersion = "1.1"
	);

	/*
	Without headers
	*/
	static inline int SendQuery
	(
		QUERY_URL* QueryUrl,
		int TypeMethod = METHODS::GET, 
		const char* Path = "/",
		const char* ProtoVersion = "1.1"
	) {return SendQuery(QueryUrl, TypeMethod, Path, 0, [](void*, char**, char**){ return false;}, ProtoVersion); }

	static int SendResponse
	(
		QUERY_URL* QueryUrl,
		int Stat, 		
		void* UsrData, 
		bool (*HeadersEnumFunc)(void* UsrData, char ** Key, char ** Val),
		const char* StatMsg = nullptr, 
		const char* ProtoVersion = "1.1"
	);

	
	/*
	Without headers
	*/
	static inline int SendResponse
	(
		QUERY_URL* QueryUrl,
		int Stat = 200,
		const char* StatMsg = nullptr, 
		const char* ProtoVersion = "1.1"
	) {return SendResponse(QueryUrl, Stat, 0, [](void*, char**,char**){ return false;}, StatMsg, ProtoVersion);}
};


#endif
