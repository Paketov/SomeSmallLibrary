#ifndef __QUERYURL_H_HAS_INCLUDED__
#define __QUERYURL_H_HAS_INCLUDED__

/*
     ExQueryUrl
	 Paketov
	 2015.

	 Library for optimize work with socket functions.
	 For Windows, Linux and another unix like systems.
*/


#include "ExString.h"
#include <limits>
#ifdef _WIN32

#	include <winsock.h>
#	include <io.h>
#	include <fcntl.h>


#	ifndef WSA_VERSION
#		define WSA_VERSION MAKEWORD(2, 2)
#	endif

#	undef IPPROTO_ICMP
#	undef IPPROTO_IGMP
#	undef IPPROTO_GGP
#	undef IPPROTO_TCP
#	undef IPPROTO_PUP
#	undef IPPROTO_UDP
#	undef IPPROTO_IDP
#	undef IPPROTO_ND 
#	undef IPPROTO_RAW
#	undef IPPROTO_MAX

namespace winsock
{
#	include <ws2tcpip.h>
#	include <Ws2ipdef.h>
#	include <winsock2.h>

	template<bool>
	static int GetLastErrSocket()
	{
		switch (WSAGetLastError()) 
		{
		case WSAEADDRINUSE:
#ifdef EADDRINUSE
			return EADDRINUSE;
#else
			break;
#endif
		case WSAEADDRNOTAVAIL:
#ifdef EADDRNOTAVAIL
			return EADDRNOTAVAIL;
#else
			break;
#endif
		case WSAEAFNOSUPPORT:
#ifdef EAFNOSUPPORT
			return EAFNOSUPPORT;
#else
			break;
#endif
		case WSAEALREADY:
#ifdef EALREADY
			return EALREADY;
#else
			break;
#endif
		case WSAEBADF:			return EBADF;
		case WSAECONNABORTED:
#ifdef ECONNABORTED
			return ECONNABORTED;
#else
			break;
#endif
		case WSAECONNREFUSED:
#ifdef ECONNREFUSED
			return ECONNREFUSED;
#else
			break;
#endif
		case WSAECONNRESET:
#ifdef ECONNRESET
			return ECONNRESET;
#else
			break;
#endif
		case WSAEDESTADDRREQ:
#ifdef EDESTADDRREQ
			return EDESTADDRREQ;
#else
			break;
#endif
		case WSAEFAULT:			return EFAULT;
		case WSAEHOSTDOWN:
#ifdef EHOSTDOWN
			return EHOSTDOWN;
#else
			break;
#endif
		case WSAEHOSTUNREACH:
#ifdef EHOSTUNREACH
			return EHOSTUNREACH;
#else
			break;
#endif
		case WSAEINPROGRESS:
#ifdef EINPROGRESS
			return EINPROGRESS;
#else
			break;
#endif
		case WSAEINTR:			return EINTR;
		case WSAEINVAL:			return EINVAL;
		case WSAEISCONN:
#ifdef EISCONN
			return EISCONN;
#else
			break;
#endif
		case WSAELOOP:
#ifdef ELOOP
			return ELOOP;
#else
			break;
#endif
		case WSAEMFILE:			return EMFILE;
		case WSAEMSGSIZE:
#ifdef EMSGSIZE
			return EMSGSIZE;
#else
			break;
#endif
		case WSAENAMETOOLONG:	return ENAMETOOLONG;
		case WSAENETDOWN:
#ifdef ENETDOWN
			return ENETDOWN;
#else
			break;
#endif
		case WSAENETRESET:
#ifdef ENETRESET
			return ENETRESET;
#else
			break;
#endif
		case WSAENETUNREACH:
#ifdef ENETUNREACH
			return ENETUNREACH;
#else
			break;
#endif
		case WSAENOBUFS:
#ifdef ENOBUFS
			return ENOBUFS;
#else
			break;
#endif
		case WSAENOPROTOOPT:
#ifdef ENOPROTOOPT
			return ENOPROTOOPT;
#else
			break;
#endif
		case WSAENOTCONN:
#ifdef ENOTCONN
			return ENOTCONN;
#else
			break;
#endif
		case WSANOTINITIALISED:	return EAGAIN;
		case WSAENOTSOCK:
#ifdef ENOTSOCK
			return ENOTSOCK;
#else
			break;
#endif
		case WSAEOPNOTSUPP:		return EOPNOTSUPP;
		case WSAEPFNOSUPPORT:
#ifdef EPFNOSUPPORT 
			return EPFNOSUPPORT;
#else 
			break;
#endif
		case WSAEPROTONOSUPPORT:
#ifdef EPROTONOSUPPORT
			return EPROTONOSUPPORT;
#else
			break;
#endif
		case WSAEPROTOTYPE:
#ifdef EPROTOTYPE
			return EPROTOTYPE;
#else
			break;
#endif
		case WSAESHUTDOWN:
#ifdef ESHUTDOWN
			return ESHUTDOWN;
#else
			break;
#endif
		case WSAESOCKTNOSUPPORT:
#ifdef ESOCKTNOSUPPORT
			return ESOCKTNOSUPPORT;
#else
			break;
#endif
		case WSAETIMEDOUT:
#ifdef ETIMEDOUT
			return ETIMEDOUT;
#else
			break;
#endif
		case WSAETOOMANYREFS:
#ifdef ETOOMANYREFS
			return ETOOMANYREFS;
#else
			break;
#endif
		case ERROR_IO_PENDING:
		case WSAEWOULDBLOCK:
#ifdef EWOULDBLOCK
			return EWOULDBLOCK;
#else
			return EAGAIN;
#endif
		case WSAHOST_NOT_FOUND:
#ifdef EHOSTUNREACH
			return EHOSTUNREACH;
#else
			break;
#endif
		case WSASYSNOTREADY:
		case WSATRY_AGAIN:		return EAGAIN;
		case WSAVERNOTSUPPORTED:
#ifdef DB_OPNOTSUP
			return DB_OPNOTSUP;
#else
			break;
#endif
		case WSAEACCES:			return EACCES;
		case 0:
			return 0;
		}

		return EFAULT;
	}

	template<bool>
	::netent * readnetnetworklist()
	{
		char Name[BUFSIZ+1], c, NetworksPath[MAX_PATH];
		::netent * NewNet = (::netent*)malloc(sizeof(::netent));
		unsigned CurNnet = 0;
		char * WinDirPath = getenv("windir");
		if(WinDirPath == NULL)
			goto lblOut;
		StringCopy(NetworksPath, WinDirPath);
		StringAppend(NetworksPath, "\\System32\\drivers\\etc\\networks");
		FILE * FNetworks = fopen(NetworksPath, "rt");
		if(FNetworks != NULL)
		{
			while(!feof(FNetworks))
			{
				SkipSpace(FNetworks);
				if((c = fgetc(FNetworks)) == '#')
				{
					fscanf(FNetworks, "%*[^\n]");
					continue;
				}
				ungetc(c, FNetworks);
				unsigned short IpAddr[4] = {0};
				int CountReaded = fscanf(FNetworks, "%[^ #\n\t\v\f\r] %hu.%hu.%hu.%hu", Name, IpAddr,IpAddr+1,IpAddr+2, IpAddr+3);
				if(CountReaded < 2)
					continue;
				NewNet[CurNnet].n_name = StringDuplicate(Name);
				unsigned NumberAliases = 0;
				NewNet[CurNnet].n_aliases = (char**)malloc(sizeof(char*));
				while(true)
				{
					CountReaded = fscanf(FNetworks, "%*[ \t\v\f\r]%[^ #\n\t\v\f\r]", Name);
					if(CountReaded <= 0)
						break;
					NewNet[CurNnet].n_aliases[NumberAliases] = StringDuplicate(Name);
					NumberAliases++;
					NewNet[CurNnet].n_aliases = (char**)realloc(NewNet[CurNnet].n_aliases, (NumberAliases + 1) * sizeof(char*));
				}
				NewNet[CurNnet].n_aliases[NumberAliases] = NULL;
				NewNet[CurNnet].n_addrtype = AF_INET;
				NewNet[CurNnet].n_net = ((unsigned char)IpAddr[3] << 24)|((unsigned char)IpAddr[2] << 16)|((unsigned char)IpAddr[1]<<8)|((unsigned char)IpAddr[0]<<0);
				CurNnet++;
				NewNet = (decltype(NewNet))realloc(NewNet, (CurNnet + 1) * sizeof(::netent));
			}
			fclose(FNetworks);
		}
lblOut:
		memset(NewNet + CurNnet, 0, sizeof(NewNet[CurNnet]));
		return NewNet;
	}

	template<bool>
	::netent * GetNetworksInfo(){ static struct ::netent * gn = readnetnetworklist(); return gn;}

	template<bool>
	::netent * getnetbyname__(const char *name)
	{
		for(::netent * gn = GetNetworksInfo<true>();gn->n_name;gn++)
			if(StringCompare(name, gn->n_name) == 0)
				return gn;
		return NULL;
	}

	template<bool>
	::netent * getnetbyaddr__(long net, int type)
	{
		for(::netent * gn = GetNetworksInfo<true>();gn->n_name;gn++)
			if((gn->n_net == net) && (gn->n_addrtype == type))
				return gn;
		return NULL;
	} 

	template<bool>
	::servent * readservicelist()
	{
		char Name[BUFSIZ+1], ProtocolName[BUFSIZ+1], c, NetworksPath[MAX_PATH];
		::servent * NewServEnt = (::servent*)malloc(sizeof(::servent));
		unsigned CurNnet = 0;
		char * WinDirPath = getenv("windir");
		if(WinDirPath == NULL)
			goto lblOut;
		StringCopy(NetworksPath, WinDirPath);
		StringAppend(NetworksPath, "\\System32\\drivers\\etc\\services");
		FILE * FServices = fopen(NetworksPath, "rt");
		if(FServices != NULL)
		{
			while(!feof(FServices))
			{
				SkipSpace(FServices);
				if((c = fgetc(FServices)) == '#')
				{
					fscanf(FServices, "%*[^\n]");
					continue;
				}
				ungetc(c, FServices);
				unsigned short Port;
				int CountReaded = fscanf(FServices, "%[^ #\n\t\v\f\r] %hu/%s", Name, &Port, ProtocolName);
				NewServEnt[CurNnet].s_port = htons(Port);
				if(CountReaded < 3)
					continue;
				NewServEnt[CurNnet].s_name = NULL;
				NewServEnt[CurNnet].s_proto = NULL;
				for(unsigned i = 0;(i < CurNnet) && (!NewServEnt[CurNnet].s_proto || !NewServEnt[CurNnet].s_name);i++)
				{
					if(StringICompare(Name, NewServEnt[i].s_name) == 0)
						NewServEnt[CurNnet].s_name = NewServEnt[i].s_name;//For economy memory :)
					if(StringICompare(ProtocolName, NewServEnt[i].s_proto) == 0)
						NewServEnt[CurNnet].s_proto = NewServEnt[i].s_proto;//For economy memory :)
				}
				if(NewServEnt[CurNnet].s_name == NULL) 
					NewServEnt[CurNnet].s_name = StringDuplicate(Name);
				if(NewServEnt[CurNnet].s_proto == NULL) 
					NewServEnt[CurNnet].s_proto = StringDuplicate(ProtocolName);
				unsigned NumberAliases = 0;
				NewServEnt[CurNnet].s_aliases = (char**)malloc(sizeof(char*));
				while(true)
				{
					CountReaded = fscanf(FServices, "%*[ \t\v\f\r]%[^ #\n\t\v\f\r]", Name);
					if(CountReaded <= 0)
						break;
					NewServEnt[CurNnet].s_aliases[NumberAliases] = StringDuplicate(Name);
					NumberAliases++;
					NewServEnt[CurNnet].s_aliases = (char**)realloc(NewServEnt[CurNnet].s_aliases, (NumberAliases + 1) * sizeof(char*));
				}
				NewServEnt[CurNnet].s_aliases[NumberAliases] = NULL;
				CurNnet++;
				NewServEnt = (decltype(NewServEnt))realloc(NewServEnt, (CurNnet + 1) * sizeof(::servent));
			}
			fclose(FServices);
		}
lblOut:
		memset(NewServEnt + CurNnet, 0, sizeof(NewServEnt[CurNnet]));
		return NewServEnt;
	}

	template<bool>
	::servent * GetServiceInfo(){ static struct ::servent * gn = readservicelist<true>(); return gn;}

	template<bool>
	::servent * getservbyport__(int port, const char * proto)
	{

		for(::servent * gn = GetServiceInfo<true>(); gn->s_name; gn++)
			if(gn->s_port == port)
			{
				if(proto == NULL)
					return gn;
				if(StringICompare(proto, gn->s_proto) == 0)
					return gn;
			}
			return NULL;
	}

	template<bool>
	::servent * getservbyname__(const char * Name, const char * proto)
	{
		for(::servent * gn = GetServiceInfo<true>();gn->s_name;gn++)
			if(StringICompare(Name, gn->s_name) == 0)
			{
				if(proto == NULL)
					return gn;
				if(StringICompare(proto, gn->s_proto) == 0)
					return gn;
			}
			return NULL;
	}

	template<bool>
	static void * GetWsa()
	{
		static LPWSADATA wd = nullptr;
		if(wd == nullptr)
		{
			wd = (decltype(wd))malloc(sizeof(WSADATA));
			if(WSAStartup(WSA_VERSION, wd) == 0)
				return wd;
			return nullptr;
		}
		return wd;
		return (void*)1;
	}

	template<bool>
	static void EndWsa()
	{
		if(GetWsa() != nullptr)
			WSACleanup();
	}
};

using winsock::addrinfo;
using winsock::getaddrinfo;
using winsock::inet_ntop;
using winsock::ip_mreq_source;

typedef UINT32 socklen_t;

#	define gai_strerror gai_strerrorA
#	define sockaddr  winsock::sockaddr
#	define sockaddr_in  winsock::sockaddr_in
#	define sockaddr_in6  winsock::sockaddr_in6
#	define sockaddr_storage winsock::sockaddr_storage
#	define sockaddr_dl winsock::sockaddr_dl
#	define inet_pton winsock::inet_pton
//Should disable overlapped, when you want use OpenAsFile
//#	define socket(ProtFamily, SockType, Protocol) winsock::WSASocketA((ProtFamily), (SockType), (Protocol), NULL, 0, 0)
#	define poll winsock::WSAPoll
#	define pollfd winsock::pollfd
#	define getnetbyname winsock::getnetbyname__<true>
#	define getnetbyaddr winsock::getnetbyaddr__<true>
#	define getservbyport winsock::getservbyport__<true>
#	define getservbyname winsock::getservbyname__<true>
#	define ipv6_mreq winsock::ipv6_mreq

#	define LAST_ERR_SOCKET winsock::GetLastErrSocket<true>()

#	define IPPROTO_ICMP ((int)winsock::IPPROTO_ICMP)
#	define IPPROTO_IGMP ((int)winsock::IPPROTO_IGMP)
#	define IPPROTO_GGP  ((int)winsock::IPPROTO_GGP)
#	define IPPROTO_TCP  ((int)winsock::IPPROTO_TCP)
#	define IPPROTO_PUP  ((int)winsock::IPPROTO_PUP)
#	define IPPROTO_UDP  ((int)winsock::IPPROTO_UDP)
#	define IPPROTO_IDP  ((int)winsock::IPPROTO_IDP)
#	define IPPROTO_ND   ((int)winsock::IPPROTO_ND)
#	define IPPROTO_RAW  ((int)winsock::IPPROTO_RAW)
#	define IPPROTO_MAX  ((int)winsock::IPPROTO_MAX)
#	define IPPROTO_IPV6 ((int)winsock::IPPROTO_IPV6)

#	define SHUT_RDWR    SD_BOTH
#	define SHUT_RD		SD_RECEIVE
#	define SHUT_WR      SD_SEND

#	define ADDITIONAL_FIELDS  bool			IsNonBlocked;

#	pragma comment(lib, "Ws2_32.lib")

#undef max
#	define INIT_WSA							\
	if(winsock::GetWsa<true>() == nullptr)	\
	{										\
		URL_SET_LAST_ERR					\
		return false;						\
	}


#else

//For unix like os
#	include <stdio.h>
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <netdb.h>
#	include <unistd.h>
#	include <poll.h>
#	include <fcntl.h>
#	include <sys/ioctl.h>
#	include <errno.h>
#	if defined(__linux__)
#		include <sys/sendfile.h>
#   elif defined(__FreeBSD__)
#		include <sys/uio.h>
#	endif
#	define closesocket(socket)  close(socket)
#	define LAST_ERR_SOCKET errno
#	define INIT_WSA   
#	define ADDITIONAL_FIELDS  
#endif


#ifndef INVALID_PORT
#define INVALID_PORT 0
#endif

#ifndef INVALID_SOCKET
#	define INVALID_SOCKET (-1)
#endif

#ifndef SOCKET_ERROR
#	define SOCKET_ERROR (-1)
#endif

#define __QUERY_URL_PROPERTY_THIS ((__QUERY_URL*)((char*)this - ((unsigned)&((__QUERY_URL*)0)->LastError)))

#define URL_SET_LAST_ERR {((__QUERY_URL*)this)->RemoteIp.iError = LAST_ERR_SOCKET;}
#define URL_SET_LAST_ERR_VAL(Val) {((__QUERY_URL*)this)->RemoteIp.iError = (Val);}
#define URL_SET_LAST_ERR_IN_PROPERTY {iError = LAST_ERR_SOCKET;}
#define URL_SET_LAST_ERR_IN_PROPERTY_VAL(Val) {iError = Val;}

#define _QUERY_URL_FIELDS1_															\
	struct{																			\
	TDESCR			hSocket;														\
	int				ProtocolType;													\
	mutable int		iError;															\
	ADDITIONAL_FIELDS																\
	}

#define DEF_GET_OPTION_PROPERTY(RetType, SoketNum, Level, Option)					\
	operator RetType() const														\
	{																				\
		RetType v; if(GetOption(SoketNum, Level, Option, v) != 0)					\
					URL_SET_LAST_ERR_IN_PROPERTY;									\
		return v;																	\
	}

#define DEF_SET_OPTION_PROPERTY(SetType, SoketNum, Level, Option)					\
	SetType operator=(SetType New){													\
	if(SetOption(SoketNum, Level, Option, New) != 0) URL_SET_LAST_ERR_IN_PROPERTY;	\
	return New;}

#define DEF_SOCKET_OPTION_PROPERTY(Name, SetType, GetType, Level, Option)			\
	class{_QUERY_URL_FIELDS1_;public:												\
	static const bool IsHave = true;												\
	static const bool IsSet = true;													\
	static const bool IsGet = true;													\
	DEF_SET_OPTION_PROPERTY(SetType, hSocket, Level, Option)						\
	DEF_GET_OPTION_PROPERTY(GetType, hSocket, Level, Option)						\
	} Name

#define DEF_SOCKET_OPTION_PROPERTY_GET(Name, GetType, Level, Option)				\
	class{_QUERY_URL_FIELDS1_; public:												\
	static const bool IsHave = true;												\
	static const bool IsSet = false;												\
	static const bool IsGet = true;													\
	DEF_GET_OPTION_PROPERTY(GetType, hSocket, Level, Option)} Name

#define DEF_SOCKET_OPTION_PROPERTY_SET(Name, SetType, Level, Option)				\
	class{ _QUERY_URL_FIELDS1_; public:												\
	static const bool IsHave = true;												\
	static const bool IsSet = true;													\
	static const bool IsGet = false;												\
	DEF_SET_OPTION_PROPERTY(SetType, hSocket, Level, Option)} Name

#define DEF_SOCKET_EMPTY_OPTION(Name, SetType, GetType)								\
	class{_QUERY_URL_FIELDS1_;public:												\
	static const bool IsHave = false;												\
	static const bool IsSet = false;												\
	static const bool IsGet = false;												\
	SetType operator=(SetType v)const {URL_SET_LAST_ERR_IN_PROPERTY_VAL(EOPNOTSUPP); return v;} \
	operator GetType() const {URL_SET_LAST_ERR_IN_PROPERTY_VAL(EOPNOTSUPP); return GetType();}	\
	} Name

#define DEF_SOCKET_EMPTY_OPTION_GET(Name, GetType)									\
	class{_QUERY_URL_FIELDS1_; public:												\
	static const bool IsHave = false;												\
	static const bool IsSet = false;												\
	static const bool IsGet = false;												\
	operator GetType() const {URL_SET_LAST_ERR_IN_PROPERTY_VAL(EOPNOTSUPP); return GetType();}	\
	} Name

#define DEF_SOCKET_EMPTY_OPTION_SET(Name, SetType)									\
	class{ _QUERY_URL_FIELDS1_; public:												\
	static const bool IsHave = false;												\
	static const bool IsSet = false;												\
	static const bool IsGet = false;												\
	SetType operator=(SetType v) const {URL_SET_LAST_ERR_IN_PROPERTY_VAL(EOPNOTSUPP); return v;}\
	} Name


template<bool = true>
class __QUERY_URL
{

public:
	typedef decltype(std::declval<sockaddr_in>().sin_port) TPORT;
	typedef decltype(socket(std::variant_arg(), std::variant_arg(), std::variant_arg())) TDESCR;

	struct SOCKET_ADDR
	{
#define SOCKET_ADDR_FIELDS \
		union\
		{\
		sockaddr			Addr;\
		sockaddr_in			AddrInet;\
		sockaddr_in6		AddrInet6;\
		sockaddr_storage	AddrStorage;\
	}

		template<typename RetType>
		inline operator RetType*()
		{
			return (RetType*)this;
		}

		inline sockaddr& operator =(const sockaddr& New)
		{
			Len.Addr = New;
			return Len.Addr;
		}

		inline sockaddr_in& operator =(const sockaddr_in& New)
		{
			Len.AddrInet = New;
			return Len.AddrInet;
		}

		inline sockaddr_in6& operator =(const sockaddr_in6& New)
		{
			Len.AddrInet6 = New;
			return Len.AddrInet6;
		}


		union
		{

			class 
			{
				SOCKET_ADDR_FIELDS;
			public:
				inline operator int() const
				{ 
					return Addr.sa_family; 
				}

				inline int operator =(int Fam)
				{ 
					return Addr.sa_family = Fam; 
				}
			} ProtocolFamily;

			class ___PORT
			{

			public:

				class
				{
					friend ___PORT;
					SOCKET_ADDR_FIELDS;
				public:
					inline operator TPORT()
					{
						switch(Addr.sa_family)
						{
						case AF_INET:
							return  htons(AddrInet.sin_port);
						case AF_INET6:
							return  htons(AddrInet6.sin6_port);
						}
						return 0;
					}
				} Readeble;

				inline operator TPORT() const
				{ 
					switch(Readeble.Addr.sa_family)
					{
					case AF_INET:
						return Readeble.AddrInet.sin_port;
					case AF_INET6:
						return Readeble.AddrInet6.sin6_port;
					}
					return 0;
				}

				inline TPORT operator =(TPORT Prt)
				{ 
					switch(Readeble.Addr.sa_family)
					{
					case AF_INET:
						return Readeble.AddrInet.sin_port = Prt;
					case AF_INET6:
						return Readeble.AddrInet6.sin6_port = Prt;
					}
					return 0; 
				}
			} Port;

			class
			{

				friend SOCKET_ADDR;
				SOCKET_ADDR_FIELDS;
			public:	
				inline operator socklen_t() const
				{
					switch(Addr.sa_family)
					{
					case AF_INET:
						return sizeof(AddrInet);
					case AF_INET6:
						return sizeof(AddrInet6);
					}
					return sizeof(Addr);
				}
			} Len;

			class
			{
				friend __QUERY_URL;
				SOCKET_ADDR_FIELDS;
			public:
				const char * operator=(const char* AddrStr)
				{
					inet_pton(Addr.sa_family, AddrStr, GetData());
					return AddrStr;
				}

				void* GetData() const
				{
					switch(Addr.sa_family)
					{
					case AF_INET:
						return (void*)&AddrInet.sin_addr;
					case AF_INET6:
						return (void*)&AddrInet6.sin6_addr;
					}
					return nullptr;
				}

				char* operator()(char * Dest, size_t Len = 0xffff) const
				{
					inet_ntop(Addr.sa_family, GetData(), Dest, Len);
					return Dest;
				}

				operator std::basic_string<char>() const
				{
					std::basic_string<char> Buf("", INET6_ADDRSTRLEN + 1);
					operator()((char*)Buf.c_str(), INET6_ADDRSTRLEN + 1);
					return Buf;
				}
			} Ip;
		};

	};

	class ADDRESS_INFO
	{
#define ADDRESS_INFO_FIELDS							\
		struct										\
		{											\
		addrinfo * ai;								\
		std::def_var_in_union_with_constructor		\
		<std::basic_string<char>> PortName;			\
		std::def_var_in_union_with_constructor		\
		<std::basic_string<char>> HostName;			\
		int iError;									\
		}


#define ADDRESS_INFO_SET_LAST_ERR(Str, Num)			\
		{											\
		static const ERROR_INFO Uei = {Num, Str};	\
		((ADDRESS_INFO*)this)->HostName.LastErr =	\
		(ERROR_INFO*)&Uei;							\
		}
		friend __QUERY_URL;

		operator addrinfo*() const
		{
			return HostName.ai;
		}

		void InitFields()
		{
			HostName.ai = nullptr;
			LastError.Clear();
			new(&HostName.PortName)  std::basic_string<char>("");
			new(&HostName.HostName)  std::basic_string<char>("");
		}
	public:
		class ADDRESS_INTERATOR
		{
			friend __QUERY_URL;

			inline operator addrinfo*() const
			{
				return Ip.ca;
			}

			SOCKET_ADDR& GetSocketAddr()
			{
				return (SOCKET_ADDR&)Ip.ca;
			}

		public:
			ADDRESS_INTERATOR(addrinfo * CurAddr)
			{
				Ip.ca = CurAddr;
			}
			union
			{
				class
				{
					addrinfo * ca;
				public:
					operator TPORT()
					{
						if(ca == nullptr)
							return 0;
						return ((SOCKET_ADDR*)ca->ai_addr)->Port;
					}
				} Port;

				class
				{
					friend ADDRESS_INTERATOR;
					addrinfo * ca;
				public:
					char* operator()(char * Dest, size_t Len = 0xffff)
					{
						if(ca == nullptr)
							return nullptr;
						inet_ntop(((SOCKET_ADDR*)ca->ai_addr)->ProtocolFamily, ((SOCKET_ADDR*)ca->ai_addr)->Ip.GetData(), Dest, Len);
						return Dest;
					}

					void* GetData()
					{
						if(ca == nullptr)
							return nullptr;
						return ((SOCKET_ADDR*)ca->ai_addr)->Ip.GetData();
					}

					operator std::basic_string<char>()
					{
						std::basic_string<char> Buf("", INET6_ADDRSTRLEN + 1);
						operator()((char*)Buf.c_str(), INET6_ADDRSTRLEN + 1);
						return Buf;
					}

				} Ip;

				class
				{
					addrinfo * ca;
				public:
					inline operator int() const
					{
						return ((SOCKET_ADDR*)ca->ai_addr)->ProtocolFamily;
					}
				} ProtocolFamily;

				class
				{
					addrinfo * ca;
				public:
					inline operator int() const
					{
						return ca->ai_socktype;
					}
				} TypeSocket;

				class
				{
					addrinfo * ca;
				public:
					inline operator int() const
					{
						return ca->ai_protocol;
					}
				} Protocol;

				class
				{
					addrinfo * ca;
				public:
					inline operator int() const
					{
						return ca->ai_flags;
					}
				} Flags;

				class
				{
					addrinfo * ca;
				public:
					inline operator char*() const
					{
						return ca->ai_canonname;
					}
				} CanonicalName;
			};
		};

	private:
		class ADDRESSES
		{
		public:
			class
			{
				friend ADDRESSES;
				ADDRESS_INFO_FIELDS;
			public:
				inline operator int() 
				{ 
					int Count = 0;
					for(decltype(ai) i = ai; i; Count++, i = i->ai_next);
					return Count;
				}
			} Count;

			ADDRESS_INTERATOR operator[](unsigned Index)
			{
				int n = 0;
				for(addrinfo * i = Count.ai; i; n++, i = i->ai_next)
					if(n == Index)
						return ADDRESS_INTERATOR(i);
				return ADDRESS_INTERATOR(nullptr);
			}
		};

	public:

		union
		{
			class
			{		
				ADDRESS_INFO_FIELDS;
				friend ADDRESS_INFO;
				int operator =(int nErr)
				{
					return iError = nErr;
				}
			public:
				operator const char *()
				{
					return strerror(iError);
				}

				int GetNumber()
				{
					return iError;
				}

				void Clear()
				{
					iError = 0;
				}
			} LastError;

			class
			{
				friend ADDRESS_INFO;
				ADDRESS_INFO_FIELDS;
			public:
				operator const char*()
				{
					return HostName->c_str();
				}
			} HostName;

			class 
			{
				ADDRESS_INFO_FIELDS;

			public:
				operator const char*()
				{
					return PortName->c_str();
				}
			} PortName;

			ADDRESSES		Addresses;
		};

		ADDRESS_INFO(const char * FullAddress)
		{
			const char * _Pos;
			int Pos;
			TPORT Port;
			InitFields();

			if((_Pos = strstr(FullAddress, "://")) != nullptr)
			{
				HostName.PortName->append(FullAddress, (unsigned)_Pos - (unsigned)FullAddress);
				_Pos += 3;
			}else 
			{
				HostName.PortName = "http";
				_Pos = FullAddress;
			}
			int CountReaded = sscanf(_Pos,"%*[^/:]%n%*c%hu", &Pos, &Port);
			HostName.HostName->append(_Pos, Pos);
			if(CountReaded > 1)
				NumberToString(Port, *&HostName.PortName);
			Update();
		}

		ADDRESS_INFO(const char * Host, const char * Port)
		{
			InitFields();
			HostName.PortName = Port;
			HostName.HostName = Host;
			Update();
		}
				
		ADDRESS_INFO()
		{
			InitFields();
			Update();
		}

		~ADDRESS_INFO()
		{
			if(HostName.ai != nullptr)
				freeaddrinfo(HostName.ai);
			HostName.PortName->~basic_string();
			HostName.HostName->~basic_string();
		}


		bool Update(int iSocktype = SOCK_STREAM, int iProtocol = IPPROTO_TCP, int iFamily = AF_UNSPEC, int iFlags = 0)
		{
			INIT_WSA;
			addrinfo host_info = {0}, *ah = nullptr;
			host_info.ai_socktype = iSocktype;
			host_info.ai_family = iFamily;
			host_info.ai_protocol = iProtocol;
			host_info.ai_flags = iFlags;                   //AI_PASSIVE
			int ErrNb;
			if((ErrNb = getaddrinfo(HostName, PortName, &host_info, &ah)) != 0)
			{
				URL_SET_LAST_ERR;
				return false;
			}
			if(HostName.ai != nullptr)
				freeaddrinfo(HostName.ai);
			HostName.ai = ah;
			return true;
		}
	};

	template<bool IsUseQuerUrl = true>
	class CHECK_EVENTS_SEL
	{
		struct ELEM_DESCR
		{
			unsigned char f;
			TDESCR        d;
			inline TDESCR GetDescriptor()
			{
			   return d;
			}
			inline void SetDescriptor(TDESCR nd)
			{
				d = nd;
			}
						
			inline __QUERY_URL* GetSock()
			{
			   return nullptr;
			}
			inline void SetSock(__QUERY_URL* ns)
			{
				d = ns->RemoteIp.hSocket;
			}
		};
				
		struct ELEM_SOCK
		{
			unsigned char f;
			__QUERY_URL*   s;
			inline TDESCR GetDescriptor()
			{
			   return s->RemoteIp.hSocket;
			}
			inline void SetDescriptor(TDESCR nd)
			{
				s->RemoteIp.hSocket = nd;
			}
						
			inline __QUERY_URL* GetSock()
			{
			   return s;
			}
			inline void SetSock(__QUERY_URL* ns)
			{
				s = ns;
			}
		};

		typedef typename std::conditional<IsUseQuerUrl, ELEM_SOCK, ELEM_DESCR>::type ELEM;
#	define __WAIT_CHANGES_FIELDS__	struct{ unsigned CountSockets; fd_set rdf, wdf, edf; ELEM * e;};
		
		class INTERATOR
		{		
#	define __INTERATOR_FIELDS__ struct{CHECK_EVENTS_SEL* This; unsigned Index;}

		public:
			inline INTERATOR(CHECK_EVENTS_SEL* w, unsigned i)
			{
				IsFollowWrite.This = w;
				IsFollowWrite.Index = i;
			}

			inline INTERATOR()
			{
				IsFollowWrite.This = nullptr;
				IsFollowWrite.Index = 0;
			}

			union
			{

#define __INTERATOR_PROPERTY__e(Name, FdSet, BitTest)									\
			   class{friend INTERATOR;__INTERATOR_FIELDS__;								\
			   public:																	\
				   inline operator bool() const {return This->Count.e[Index].f & BitTest;}\
				   inline bool operator=(bool v){										\
				   if(v){																\
						FD_SET(This->Count.e[Index].GetDescriptor(), &This->Count.FdSet);\
						This->Count.e[Index].f |= BitTest;								\
				   }else{																\
						FD_CLR(This->Count.e[Index].GetDescriptor(), &This->Count.FdSet);\
						This->Count.e[Index].f &= (~((unsigned char)BitTest));		    \
				   }return v;}															\
			   } Name

#define __INTERATOR_PROPERTY__r(Name, FdSet)											\
			   class{friend INTERATOR;__INTERATOR_FIELDS__;								\
			   public:																	\
				   inline operator bool() const{return FD_ISSET(This->Count.e[Index].GetDescriptor(), &This->Count.FdSet);}\
			   } Name

			   __INTERATOR_PROPERTY__e(IsFollowWrite, wdf, 0x1);
			   __INTERATOR_PROPERTY__e(IsFollowRead,  rdf, 0x2);
			   __INTERATOR_PROPERTY__e(IsFollowError, edf, 0x4);


			   __INTERATOR_PROPERTY__r(IsAdoptedWrite, wdf);
			   __INTERATOR_PROPERTY__r(IsAdoptedRead,  rdf);
			   __INTERATOR_PROPERTY__r(IsAdoptedError, edf);

			   class
			   {
				    __INTERATOR_FIELDS__;
			   public:
				   inline operator TDESCR() const
				   {
					   return This->Count.e[Index].GetDescriptor();
				   }
			   } Descriptor;

			   class
			   {
				   __INTERATOR_FIELDS__;
			   public:
				   inline operator __QUERY_URL*()
				   {
					   if(This->Count.e == nullptr)
						   return nullptr;
					   return This->Count.e[Index].GetSock();
				   }

				   inline __QUERY_URL* operator->()
				   {
					   if(This->Count.e == nullptr)
						   return nullptr;
					   return This->Count.e[Index].GetSock();
				   }
			   } Connection;
			};

			void Remove()
			{
			    if(IsFollowWrite.Index >= IsFollowWrite.This->Count)
					return;	
				unsigned From = --IsFollowWrite.This->Count.CountSockets;
				IsFollowWrite = false;
				IsFollowRead = false;
				IsFollowError = false;
				if(From != IsFollowWrite.Index)
				{
					IsFollowWrite.This->Count.e[IsFollowWrite.Index] = IsFollowWrite.This->Count.e[From];
				}
				void * New = realloc(IsFollowWrite.This->Count.e, From * sizeof(IsFollowWrite.This->Count.e[0]));
				IsFollowWrite.This->Count.e = (ELEM*)New;

			}
		};
	public:

		CHECK_EVENTS_SEL()
		{
			Count.CountSockets = 0;
			Count.e = nullptr;
			FD_ZERO(&Count.rdf);
			FD_ZERO(&Count.wdf);
			FD_ZERO(&Count.edf);
		}

		~CHECK_EVENTS_SEL()
		{
			if(Count.e != nullptr)
				free(Count.e);
		}
	
		union
		{
			class
			{
				friend CHECK_EVENTS_SEL;
				__WAIT_CHANGES_FIELDS__
			public:
				inline operator unsigned()
				{
					return CountSockets;
				}
			} Count;
		};
				
		int AddConnection(__QUERY_URL& Sock)
		{
			unsigned i = Count.CountSockets, j = i + 1;
			void * New = realloc(Count.e, j * sizeof(Count.e[0]));
			if(New == nullptr)
				return -1;
			Count.e = (ELEM*)New;
			Count.e[i].f = 0;
			Count.e[i].SetSock(&Sock);
			Count.CountSockets = j;
			return i;
		}

		int AddConnection(TDESCR SocketDescriptor)
		{
			unsigned i = Count.CountSockets, j = i + 1;
			void * New = realloc(Count.e, j * sizeof(Count.e[0]));
			if(New == nullptr)
				return -1;
			Count.e = (ELEM*)New;
			Count.e[i].f = 0;
			Count.e[i].SetDescriptor(SocketDescriptor);
			Count.CountSockets = j;
			return i;
		}

		inline INTERATOR operator[](unsigned Index)
		{
			if(Index >= Count)
				throw "CHECK_EVENTS_SEL::operator[] : Out of bound";	
		    return INTERATOR(this, Index);
		}

		int Check(long WaitTimeSec = 0, long WaitTimeMiliSec = 0)
		{
			if((Count == 0) || (Count.e == nullptr))
				return 0;
			timeval tv;
			tv.tv_sec = WaitTimeSec;
			tv.tv_usec = WaitTimeMiliSec;
			return select(Count, &Count.rdf, &Count.wdf, &Count.edf, &tv);
		}
	};

	template<bool IsUseQuerUrl = true>
	class CHECK_EVENTS_POL
	{
#	define __WAIT_CHANGES_FIELDS__ 	struct{unsigned CountSockets; pollfd * pfd; __QUERY_URL** Elements;};

		class INTERATOR
		{
			
#	define __INTERATOR_FIELDS__ struct{CHECK_EVENTS_POL* This; unsigned Index;}		
			
		public:
			inline INTERATOR(CHECK_EVENTS_POL* w, unsigned i)
			{
				IsFollowWrite.This = w;
				IsFollowWrite.Index = i;
			}

			inline INTERATOR()
			{
				IsFollowWrite.This = nullptr;
				IsFollowWrite.Index = 0;
			}

			union
			{

#define __INTERATOR_PROPERTY__e(Name, Opt)												\
			   class{friend INTERATOR;__INTERATOR_FIELDS__;								\
			   public:																	\
				   inline operator bool() const {return This->Count.pfd[Index].events & Opt;}	\
				   inline bool operator=(bool v){										\
				   This->Count.pfd[Index].events |= ((v)?Opt:0);return v;}				\
			   } Name
#define __INTERATOR_PROPERTY__r(Name, Opt)												\
			   class{friend INTERATOR;__INTERATOR_FIELDS__;								\
			   public:																	\
				   inline operator bool() const{return This->Count.pfd[Index].revents & Opt;}\
			   } Name

			   __INTERATOR_PROPERTY__e(IsFollowWrite, POLLIN);
			   __INTERATOR_PROPERTY__e(IsFollowWriteNorm, POLLRDNORM);
			   __INTERATOR_PROPERTY__e(IsFollowWriteBand, POLLRDBAND);
			   __INTERATOR_PROPERTY__e(IsFollowWritePrior,POLLPRI);

			   __INTERATOR_PROPERTY__e(IsFollowRead, POLLOUT);
			   __INTERATOR_PROPERTY__e(IsFollowReadNorm, POLLWRNORM);
			   __INTERATOR_PROPERTY__e(IsFollowReadBand, POLLWRBAND);

			   __INTERATOR_PROPERTY__e(IsFollowError, POLLERR);
			   __INTERATOR_PROPERTY__e(IsFollowDisconnected, POLLHUP);
			   __INTERATOR_PROPERTY__e(IsFollowNotFile, POLLNVAL);


			   __INTERATOR_PROPERTY__r(IsAdoptedWrite, POLLIN);
			   __INTERATOR_PROPERTY__r(IsAdoptedWriteNorm, POLLRDNORM);
			   __INTERATOR_PROPERTY__r(IsAdoptedWriteBand, POLLRDBAND);
			   __INTERATOR_PROPERTY__r(IsAdoptedWritePrior, POLLPRI);

			   __INTERATOR_PROPERTY__r(IsAdoptedRead, POLLOUT);
			   __INTERATOR_PROPERTY__r(IsAdoptedReadNorm, POLLWRNORM);
			   __INTERATOR_PROPERTY__r(IsAdoptedReadBand, POLLWRBAND);
			   
			   __INTERATOR_PROPERTY__r(IsAdoptedError, POLLERR);
			   __INTERATOR_PROPERTY__r(IsAdoptedDisconnected, POLLHUP);
			   __INTERATOR_PROPERTY__r(IsAdoptedNotFile, POLLNVAL);

			   class
			   {
				   __INTERATOR_FIELDS__;
			   public:
				   inline operator decltype(std::declval<pollfd>().revents)() const
				   {
					   return This->Count.pfd[Index].revents;
				   }
			   } ReturnedEvents;

			   class
			   {
				   __INTERATOR_FIELDS__;
			   public:
				   inline operator decltype(std::declval<pollfd>().events)() const
				   {
					   return This->Count.pfd[Index].events;
				   }

				   inline decltype(std::declval<pollfd>().events) operator=(decltype(std::declval<pollfd>().events) v)
				   {
					   This->Count.pfd[Index].events = v;
					   return v;
				   }
			   } RequestedEvents;

			   class
			   {
				    __INTERATOR_FIELDS__;
			   public:
				   inline operator decltype(std::declval<pollfd>().fd)() const
				   {
					   return This->Count.pfd[Index].fd;
				   }	   
			   } Descriptor;

			   class
			   {
				   __INTERATOR_FIELDS__;
			   public:
				   inline operator __QUERY_URL*()
				   {
					   if(!IsUseQuerUrl || (This->Count.Elements == nullptr))
						   return nullptr;
					   return This->Count.Elements[Index];
				   }

				   inline __QUERY_URL* operator->()
				   {
					   if(!IsUseQuerUrl || (This->Count.Elements == nullptr))
						   return nullptr;
					   return This->Count.Elements[Index];
				   }
			   } Connection;
			};

			void Remove()
			{
				if(IsFollowWrite.Index >= IsFollowWrite.This->Count)
					return;	
				unsigned From = --IsFollowWrite.This->Count.CountSockets;
				if(From != IsFollowWrite.Index)
				{
					IsFollowWrite.This->Count.pfd[IsFollowWrite.Index] = IsFollowWrite.This->Count.pfd[From];
					if(IsUseQuerUrl)
						IsFollowWrite.This->Count.Elements[IsFollowWrite.Index] = IsFollowWrite.This->Count.Elements[From];
				}
				void * New = realloc(IsFollowWrite.This->Count.pfd, From * sizeof(IsFollowWrite.This->Count.pfd[0]));
				IsFollowWrite.This->Count.pfd = (pollfd*)New;
				if(IsUseQuerUrl)
				{
					New = realloc(IsFollowWrite.This->Count.Elements, From * sizeof(IsFollowWrite.This->Count.Elements[0]));
					IsFollowWrite.This->Count.Elements = (__QUERY_URL**)New;
				}
			}

			int Check(unsigned WaitTime = 0)
			{
				if(IsFollowWrite.This == nullptr)
					return 0;
				return poll(IsFollowWrite.This->Count.pfd + IsFollowWrite.Index, 1, WaitTime);
			}
		};

		friend INTERATOR;
	public:

		CHECK_EVENTS_POL()
		{
			Count.CountSockets = 0;
			Count.pfd = nullptr;
			Count.Elements = nullptr;
		}

		~CHECK_EVENTS_POL()
		{
			if(Count.pfd != nullptr)
				free(Count.pfd);
			if(IsUseQuerUrl && (Count.Elements != nullptr))
				free(Count.Elements);
		}

		union
		{
			class
			{
				friend CHECK_EVENTS_POL;
				__WAIT_CHANGES_FIELDS__
			public:
				inline operator unsigned()
				{
				    return CountSockets;
				}
			} Count;
		};

		int AddConnection(__QUERY_URL& Sock)
		{
			unsigned i = Count.CountSockets, j = i + 1;
			void * New = realloc(Count.pfd, j * sizeof(Count.pfd[0]));
			if(New == nullptr)
				return -1;
			Count.pfd = (pollfd*)New;
			Count.pfd[i].fd = Sock.RemoteIp.hSocket;
			Count.pfd[i].events = 0;
			Count.pfd[i].revents = 0;
			if(IsUseQuerUrl)
			{
				New = realloc(Count.Elements, j * sizeof(Count.Elements[0]));
				if(New == nullptr)
					return -1;
				Count.Elements = (__QUERY_URL**)New;
				Count.Elements[i] = &Sock;
			}
			Count.CountSockets = j;
			return i;
		}

		int AddConnection(decltype(std::declval<pollfd>().fd) SocketDescriptor)
		{
			if(IsUseQuerUrl)
				return -1;
			unsigned i = Count.CountSockets, j = i + 1;
			void * New = realloc(Count.pfd, j * sizeof(Count.pfd[0]));
			if(New == nullptr)
				return -1;
			Count.pfd = (pollfd*)New;
			Count.pfd[i].fd = SocketDescriptor;
			Count.pfd[i].events = 0;
			Count.pfd[i].revents = 0;
			Count.CountSockets = j;
			return i;
		}

		void UpdateDescriptor()
		{
			if(IsUseQuerUrl)
				for(unsigned i = 0, m = Count;i < m;i++)
					Count.pfd[i].fd = Count.Elements[i].RemoteIp.hSocket;
		}

		void UpdateDescriptor(unsigned Index)
		{
			if(!IsUseQuerUrl || (Index >= Count))
				return;
			Count.pfd[Index].fd = Count.Elements[Index].RemoteIp.hSocket;
		}

		inline INTERATOR operator[](unsigned Index)
		{
			if(Index >= Count)
				throw "CHECK_EVENTS_POL::operator[] : Out of bound";
		    return INTERATOR(this, Index);
		}

		int Check(unsigned WaitTime = 0)
		{
			if((Count == 0) || (Count.pfd == nullptr))
				return 0;
			return poll(Count.pfd, Count, WaitTime);
		}
	};

protected:

	struct PROTOCOL_INTERATOR
	{
#define PROTOCOL_INTERATOR_FIELDS struct protoent * Cur;

		inline PROTOCOL_INTERATOR(struct protoent * New)
		{
			Name.Cur = New;
		}

		union
		{
			class
			{
				friend PROTOCOL_INTERATOR;
				PROTOCOL_INTERATOR_FIELDS;
			public:
				operator char*()
				{
					if(Cur == nullptr)
						return "";
					return Cur->p_name;
				}
			} Name;

			/*
			Property represent protocol index.
			PPROTO_
			*/
			class
			{
				PROTOCOL_INTERATOR_FIELDS;
			public:
				operator short()
				{
					if(Cur == nullptr)
						return -1;
					return Cur->p_proto;
				}
			} Index;

			class
			{
				PROTOCOL_INTERATOR_FIELDS;
			public:
				operator bool() { return Cur == nullptr;}
			} NotHave;

			struct P_NAME
			{
				class
				{
					friend P_NAME;
					PROTOCOL_INTERATOR_FIELDS;
				public:
					inline operator int() 
					{ 
						int Count = 0;
						for(;Cur->p_aliases[Count]; Count++);
						return Count;
					}
				} Count;

				char * operator[](unsigned Index)
				{
					if(Index < Count)
						return Count.Cur->p_aliases[Index];
					return "";
				}
			} Aliases;
		};
	};

	struct PORT_SERVICE_INTERATOR
	{
#define PORT_SERVICE_INTERATOR_FIELDS struct servent * Cur;
		inline PORT_SERVICE_INTERATOR(struct servent * New)
		{
			Name.Cur = New;
		}

		union
		{
			class
			{
				friend PORT_SERVICE_INTERATOR;
				PORT_SERVICE_INTERATOR_FIELDS;
			public:
				operator char*()
				{
					if(Cur == nullptr)
						return "";
					return Cur->s_name;
				}
			} Name;

			class
			{
				PORT_SERVICE_INTERATOR_FIELDS;
			public:
				operator TPORT()
				{
					if(Cur == nullptr)
						return 0;
					return htons(Cur->s_port);
				}
			} Port;

			class
			{
				PORT_SERVICE_INTERATOR_FIELDS;
			public:
				operator TPORT()
				{
					if(Cur == nullptr)
						return 0;
					return Cur->s_port;
				}
			} PortInPacketForm;

			class 
			{
				PORT_SERVICE_INTERATOR_FIELDS;
			public:
				operator char*()
				{
					if(Cur == nullptr)
						return "";
					return Cur->s_proto;
				}

				PROTOCOL_INTERATOR GetInfo()
				{
					if(Cur == nullptr)
						return PROTOCOL_INTERATOR(nullptr);
					return GetSystemProtocol(Cur->s_proto);
				}
			} UsedProtocol;

			class
			{
				PORT_SERVICE_INTERATOR_FIELDS;
			public:
				operator bool()
				{ 
					return Cur == nullptr;
				}
			} NotHave;

			struct P_NAME
			{
				class
				{
					friend P_NAME;
					PORT_SERVICE_INTERATOR_FIELDS;
				public:
					inline operator int() 
					{ 
						int Count = 0;
						if(Cur == nullptr)
							return Count;
						for(;Cur->s_aliases[Count]; Count++);
						return Count;
					}
				} Count;

				char * operator[](unsigned Index)
				{
					if(Index < Count)
						return Count.Cur->s_aliases[Index];
					return "";
				}
			} Aliases;
		};
	};

	struct NET_INTERATOR
	{
#define PROTOCOL_INTERATOR_FIELDS struct netent * Cur;
		inline NET_INTERATOR(struct netent * New)
		{
			Name.Cur = New;
		}

		union
		{
			class
			{
				friend NET_INTERATOR;
				PROTOCOL_INTERATOR_FIELDS;
			public:
				inline operator char*()
				{
					if(Cur == nullptr)
						return "";
					return Cur->n_name;
				}
			} Name;

			/*
			Type net address.
			Usually AF_INET
			*/
			class
			{
				PROTOCOL_INTERATOR_FIELDS;
			public:
				inline operator short()
				{
					if(Cur == nullptr)
						return -1;
					return Cur->n_addrtype;
				}
			} AddrType;

			class
			{
				PROTOCOL_INTERATOR_FIELDS;
			public:
				operator short()
				{
					if(Cur == nullptr)
						return -1;
					return Cur->n_net;
				}
			} NumberNet;

			class
			{
				PROTOCOL_INTERATOR_FIELDS;
			public:
				inline operator bool()
				{ 
					return Cur == nullptr;
				}
			} NotHave;

			struct P_NAME
			{
				class
				{
					friend P_NAME;
					PROTOCOL_INTERATOR_FIELDS;
				public:
					inline operator int() 
					{ 
						int Count = 0;
						for(;Cur->n_aliases[Count]; Count++);
						return Count;
					}
				} Count;

				inline char * operator[](unsigned Index)
				{
					if(Index < Count)
						return Count.Cur->n_aliases[Index];
					return "";
				}
			} Aliases;
		};
	};

	struct INFO_HOST_INTERATOR
	{
#define INFO_HOST_INTERATOR_FIELDS struct hostent * Cur;
		inline INFO_HOST_INTERATOR(struct hostent *New)
		{
			Name.Cur = New;
		}

		union
		{

			class
			{
				friend INFO_HOST_INTERATOR;
				INFO_HOST_INTERATOR_FIELDS;
			public:
				inline operator char*()
				{
					if(Cur == nullptr)
						return "";
					return Cur->h_name;
				}
			} Name;

			/*
			Type net address.
			AF_INET or another AF_ - like
			*/
			class
			{
				INFO_HOST_INTERATOR_FIELDS;
			public:
				inline operator short()
				{
					if(Cur == nullptr)
						return -1;
					return Cur->h_addrtype;
				}
			} AddrType;

			class
			{
				INFO_HOST_INTERATOR_FIELDS;
			public:
				inline operator short()
				{
					if(Cur == nullptr)
						return -1;
					return Cur->h_length;
				}
			} LengthAddress;

			class
			{
				INFO_HOST_INTERATOR_FIELDS;
			public:
				inline operator bool()
				{ 
					return Cur == nullptr;
				}
			} NotHave;

			struct P_NAME
			{
				class
				{
					friend P_NAME;
					INFO_HOST_INTERATOR_FIELDS;
				public:
					inline operator int() 
					{ 
						int Count = 0;
						for(;Cur->h_aliases[Count]; Count++);
						return Count;
					}
				} Count;

				char * operator[](unsigned Index)
				{
					if(Index < Count)
						return Count.Cur->h_aliases[Index];
					return "";
				}
			} Aliases;

			class P_ADDRESES
			{
				class ADDRESS_INTERATOR
				{
					void * Cur;
					short AddrType;
				public:
					ADDRESS_INTERATOR(void * nCur, short nAddrType)
					{
						Cur	  = nCur;
						AddrType = nAddrType;
					}

					inline operator void*()
					{
						return Cur;
					}

					char* operator()(char * Dest, size_t Len = 0xffff)
					{
						if(Cur == nullptr)
							return nullptr;
						inet_ntop(AddrType, Cur, Dest, Len);
						return Dest;
					}

					operator std::basic_string<char>()
					{
						std::basic_string<char> Buf("", INET6_ADDRSTRLEN + 1);
						operator()((char*)Buf.c_str(), INET6_ADDRSTRLEN + 1);
						return Buf;
					}
				};

			public:
				class
				{
					friend P_ADDRESES;
					INFO_HOST_INTERATOR_FIELDS;
				public:
					inline operator int()
					{
						int Count = 0;
						for(;Cur->h_addr_list[Count]; Count++);
						return Count;
					}
				} Count;

				inline ADDRESS_INTERATOR operator[](int Index)
				{
					if(Index >= Count)
						return ADDRESS_INTERATOR(nullptr, 0);
					return ADDRESS_INTERATOR(Count.Cur->h_addr_list[Index], Count.Cur->h_addrtype);
				}
			} Addresses;
		};
	};

	virtual void EvntUninitFields(){}

	virtual bool EvntBind()
	{
		return true;
	}

	virtual bool EvntConnect()
	{
		return true;
	}

	virtual bool EvntAcceptClient(TDESCR ClientDescr)
	{
		return true;
	}
	
	virtual int EvntGetCountPandingData()
	{
#ifdef _WIN32
		u_long res = -1;
		if(ioctlsocket(RemoteIp.hSocket, FIONREAD, &res) == SOCKET_ERROR)
		{
			URL_SET_LAST_ERR;
			return -1;
		}
		return res;
#else
		int res;
		if(ioctl(RemoteIp.hSocket, FIONREAD, &res) < 0)
			URL_SET_LAST_ERR;
		return res;
#endif
	}

	virtual bool EvntIsNotHaveRecvData()
	{
	   return (CountPandingData <= 0) && IsRecivedFin;
	}

	virtual bool EvntBeforeShutdown(int How)
	{
		return true;
	}

	virtual bool EvntBeforeClose()
	{
		return true;
	}

	void SetLastErr(int Num)
	{
		LastError = Num;
	}

	void InitFields()
	{
#ifdef _WIN32
		RemoteIp.IsNonBlocked = false;
#endif
		RemoteIp.hSocket = INVALID_SOCKET;
		LastError.Clear();
	}

	static inline int SetOption(int hSocket, int Level, int Option, bool& New)
	{										
		int v = New;
		return setsockopt(hSocket, Level, Option, (char*)&v, sizeof(int));
	}

	template<typename SetType>
	static inline int SetOption(int hSocket, int Level, int Option, SetType& New)
	{
		return setsockopt(hSocket, Level, Option, (char*)&New, sizeof(SetType));
	}

	static inline int GetOption(int hSocket, int Level, int Option, bool& New)
	{										
		int v = 0;
		int l = sizeof(v);
		auto r = getsockopt(hSocket, Level, Option, (char*)&v, &l);
		New = v;
		return r;
	}

	template<typename GetType>
	static inline int GetOption(int hSocket, int Level, int Option, GetType& New)
	{
		int l = sizeof(GetType);
		return getsockopt(hSocket, Level, Option, (char*)&New, &l);
	}



public:

	union
	{
		/*
		Get last error on class level.
		*/
		class
		{		
			friend __QUERY_URL;
			_QUERY_URL_FIELDS1_;

			int operator =(int nErr)
			{
				return iError = nErr;
			}

		public:
			//Get string description error
			inline operator const char *()
			{
				return strerror(iError);
			}

			//Get number error
			inline int GetNumber()
			{
				return iError;
			}

			//Set err to OK state
			inline void Clear()
			{
				iError = 0;
			}
		} LastError;

		/*
		Remote host name.
		*/
		class 
		{
			_QUERY_URL_FIELDS1_;
		public:
			operator char*() const
			{
				SOCKET_ADDR sa;
				if(!__QUERY_URL_PROPERTY_THIS->RemoteIp.GetRemoteAddress(sa))
					return "";
				return GetInfoAboutHost(sa.Ip.GetData(), sa.Len, sa.ProtocolFamily).Name;
			}
		} RemoteHostName;

		/*
		Get remote port.
		*/
		class
		{
			friend __QUERY_URL;
			_QUERY_URL_FIELDS1_;
		public:

			//As number
			inline operator TPORT() const
			{
				SOCKET_ADDR sa;
				if(!__QUERY_URL_PROPERTY_THIS->RemoteIp.GetRemoteAddress(sa))
					return INVALID_PORT;
				return sa.Port.Readeble;
			}
			//As c string
			char* operator()(char * Dest, size_t Len = 0xffff) const
			{
				NumberToString(operator TPORT(), Dest, Len);
				return Dest;
			}
			//As stl string
			std::basic_string<char> operator()() const
			{
				std::basic_string<char> Buf("", 6);
				NumberToString(operator TPORT(), (char*)Buf.c_str(), 6);
				return Buf;
			}

			//Get more info about remote port
			inline PORT_SERVICE_INTERATOR GetInfo() const
			{
				return GetSystemService(operator TPORT());
			}
		} RemotePort;

		/*
		Get dest ip.
		*/
		class
		{
			friend __QUERY_URL;
			_QUERY_URL_FIELDS1_;
			inline bool GetRemoteAddress(SOCKET_ADDR & Address) const
			{
				int Len = sizeof(SOCKET_ADDR);
				if((getpeername(hSocket, Address, &Len) != 0) || (Len != Address.Len))
				{
					URL_SET_LAST_ERR_IN_PROPERTY;
					return false;
				}
				return true;
			}
		public:
			void* GetData() const
			{
				SOCKET_ADDR sa;
				GetRemoteAddress(sa);
				return sa.Ip.GetData();
			}

			//As c string
			char* operator()(char * Dest, size_t Len = 0xffff) const
			{
				SOCKET_ADDR sa;
				if(GetRemoteAddress(sa))
				{
					sa.Ip(Dest, Len);
					return Dest;
				}	
				return nullptr;
			}

			//As stl string
			operator std::basic_string<char>() const
			{
				std::basic_string<char> Buf("", INET6_ADDRSTRLEN + 1);
				operator()((char*)Buf.c_str(), INET6_ADDRSTRLEN + 1);
				return Buf;
			}

			//Get info about this IP
			inline INFO_HOST_INTERATOR GetInfo() const
			{
				SOCKET_ADDR sa;
				if(GetRemoteAddress(sa))
					return GetInfoAboutHost(sa.Ip.GetData(), sa.Len, sa.ProtocolFamily);
				return INFO_HOST_INTERATOR(nullptr);
			}
		} RemoteIp;

		/*
		Get local host name.
		*/
		class
		{
			friend __QUERY_URL;
			_QUERY_URL_FIELDS1_;
		public:

			operator char*() const
			{
				SOCKET_ADDR sa;
				if(__QUERY_URL_PROPERTY_THIS->LocalIp.GetLocalAddress(sa))
					return GetInfoAboutHost(sa.Ip.GetData(), sa.Len, sa.ProtocolFamily).Name;
				return "";
			}
		} LocalHostName;

		/*
		Get local port.
		*/
		class
		{
			_QUERY_URL_FIELDS1_;
		public:

			inline operator TPORT() const
			{
				SOCKET_ADDR sa;
				if(!__QUERY_URL_PROPERTY_THIS->LocalIp.GetLocalAddress(sa))
					return INVALID_PORT;
				return sa.Port.Readeble;
			}
			
			char* operator()(char* Dest, size_t Len = 0xffff) const
			{
				NumberToString(operator TPORT(), Dest, Len);
				return Dest;
			}

			std::basic_string<char> operator()() const
			{
				std::basic_string<char> Buf("", 6);
				NumberToString(operator TPORT(), (char*)Buf.c_str(), 6);
				return Buf;
			}

			inline PORT_SERVICE_INTERATOR GetInfo() const
			{
				return GetSystemService(operator TPORT());
			}
		} LocalPort;

		/*
		Get local ip addreess.
		*/
		class 
		{
			friend __QUERY_URL;
			_QUERY_URL_FIELDS1_;
			inline bool GetLocalAddress(SOCKET_ADDR & Address) const
			{
				int Len = sizeof(SOCKET_ADDR);
				if((getsockname(hSocket, Address, &Len) != 0) || (Len != Address.Len))
				{
					URL_SET_LAST_ERR_IN_PROPERTY
					return false;
				}
				return true;
			}
		public:
			void* GetData() const
			{
				SOCKET_ADDR sa;
				GetLocalAddress(sa);
				return sa.Ip.GetData();
			}

			char* operator()(char* Dest, size_t Len = 0xffff) const
			{
				SOCKET_ADDR sa;
				if(GetLocalAddress(sa))
				{
					sa.Ip(Dest, Len);
					return Dest;
				}	
				return nullptr;
			}

			operator std::basic_string<char>() const
			{
				std::basic_string<char> Buf("", INET6_ADDRSTRLEN + 1);
				operator()((char*)Buf.c_str(), INET6_ADDRSTRLEN + 1);
				return Buf;
			}

			inline INFO_HOST_INTERATOR GetInfo() const
			{
				SOCKET_ADDR sa;
				if(GetLocalAddress(sa))
					return GetInfoAboutHost(sa.Ip.GetData(), sa.Len, sa.ProtocolFamily);
				return INFO_HOST_INTERATOR(nullptr);
			}
		} LocalIp;

		/*
		AF_ ...
		This property specifies the desired address family for the returned addresses. 
		Valid values for this field include AF_INET and AF_INET6.  
		*/
		class
		{
			_QUERY_URL_FIELDS1_;
		public:
			operator decltype(std::declval<addrinfo>().ai_protocol)()
			{
				SOCKET_ADDR SockAddr;
				if(!__QUERY_URL_PROPERTY_THIS->RemoteIp.GetRemoteAddress(SockAddr))
					return -1;
				return SockAddr.ProtocolFamily;
			}

			operator char*() const
			{
				SOCKET_ADDR SockAddr;
				if(!__QUERY_URL_PROPERTY_THIS->RemoteIp.GetRemoteAddress(SockAddr))
					return "";
				switch(SockAddr.ProtocolFamily) 
				{ 
				case AF_UNSPEC: 
					return "UNSPEC"; 
				case AF_UNIX:
					return "UNIX";
				case AF_INET: 
					return "IPv4"; 
				case AF_INET6: 
					return "IPv6"; 
				case AF_NETBIOS: 
					return "NETBIOS";
				} 
				return ""; 
			}

		} ProtocolFamily;

		/*
		IPPROTO_ ...
		This property specifies the protocol for the returned socket addresses.
		*/
		class
		{
			_QUERY_URL_FIELDS1_;
			inline int operator=(int NewVal)
			{
				return ProtocolType = NewVal;
			}
		public:
			operator int() const
			{
				return ProtocolType;
			}
			operator char*() const
			{
				return GetSystemProtocol(ProtocolType).Name;
			}

			inline PROTOCOL_INTERATOR GetInfo() const
			{
				return GetSystemProtocol(ProtocolType);
			}
		} Protocol;

		/*
		Set socket to non blocket mode.
		*/
		class 
		{
			_QUERY_URL_FIELDS1_;
		public:
			bool operator=(bool NewVal)
			{
#ifdef _WIN32
				u_long nonBlocking = NewVal;
				if (ioctlsocket(hSocket, FIONBIO, &nonBlocking) == SOCKET_ERROR)
					URL_SET_LAST_ERR_IN_PROPERTY
				else
					IsNonBlocked = NewVal;
#else
				int nonBlocking = NewVal;
				if (fcntl(hSocket, F_SETFL, O_NONBLOCK, nonBlocking) == -1)
					URL_SET_LAST_ERR;
#endif
				return NewVal;
			}

			operator bool() const
			{
#ifdef _WIN32
				return IsNonBlocked;
#else
				return fcntl(hSocket, F_GETFL, 0) & O_NONBLOCK;
#endif
			}
		} IsNonBlocked;

		/*
		    Get count data in recive buffer.
		*/
		class 
		{
			_QUERY_URL_FIELDS1_;
		public:
			inline operator int() const
			{
				return __QUERY_URL_PROPERTY_THIS->EvntGetCountPandingData();
			}
		} CountPandingData;

		/*
		     Get open state.
		*/
		class
		{			
			_QUERY_URL_FIELDS1_;
		public:
			inline operator bool()
			{
				return hSocket != INVALID_SOCKET;
			}
		} IsOpen;

		/*
		   Get file descriptor.
		*/
		class
		{
		_QUERY_URL_FIELDS1_;
		public:
			inline operator TDESCR()
			{
			   return hSocket;
			}
		} Descriptor;

		/*
			Is remote host send fin flag in packet.
		*/
		class
		{			
			_QUERY_URL_FIELDS1_;
		public:
			operator bool() const
			{
				pollfd pfd;
				pfd.events = POLLIN;
				pfd.revents = 0;
				pfd.fd = hSocket;
				if(poll(&pfd, 1, 0) == -1)
				{
					URL_SET_LAST_ERR;
					return false;
				}
				return pfd.revents & POLLHUP;
			}
		} IsRecivedFin;


		class
		{			
			_QUERY_URL_FIELDS1_;
		public:
			inline operator bool() const
			{
				return __QUERY_URL_PROPERTY_THIS->EvntIsNotHaveRecvData();
			}
		} IsNotHaveRecvData;

		/*
		This field specifies the preferred socket type, for
		example SOCK_STREAM or SOCK_DGRAM. 
		*/

		DEF_SOCKET_OPTION_PROPERTY_GET(TypeSocket, int, SOL_SOCKET, SO_TYPE);

		/*
		Get or set option on various system.
		Example:
		Url.Options[SO_SNDTIMEO] = 123;
		int GetI = Url.Options[SO_SNDTIMEO]; //Eq. 123
		*/
		class
		{
			_QUERY_URL_FIELDS1_;
			class OPTION_INTERATOR
			{
				int Level;
				int hSocket;
				int OptIndex;
			public:
				OPTION_INTERATOR(int nLevel, int nSocket, int nOptIndex)
				{
					Level = nLevel;
					hSocket = nSocket;
					OptIndex = nOptIndex;
				}

				template<typename RetType>
				operator RetType()
				{
					RetType v;
					if(GetOption(hSocket, Level, OptIndex, v) != 0)
						URL_SET_LAST_ERR_IN_PROPERTY;
					return v;
				}

				template<typename SetType>
				SetType & operator=(SetType & New)
				{
					if(SetOption(hSocket, Level, OptIndex, New) != 0)
						URL_SET_LAST_ERR_IN_PROPERTY;
					return New;
				}
			};
		public:
			/*
			Index as SO_ ...
			*/
			OPTION_INTERATOR operator[](int OptIndex)
			{
				return OPTION_INTERATOR(SOL_SOCKET, hSocket, OptIndex);
			}

			OPTION_INTERATOR operator()(int OptIndex, int nLevel)
			{
				return OPTION_INTERATOR(nLevel, hSocket, OptIndex);
			}

		} Options;

		union
		{
			/*
			Have on windows.
			Returns the number of seconds a socket has been connected. This option is only valid for connection-oriented protocols.
			*/
#ifdef SO_CONNECT_TIME
			DEF_SOCKET_OPTION_PROPERTY_GET(ConnectTime, int, SOL_SOCKET, SO_CONNECT_TIME);
#else
			DEF_SOCKET_EMPTY_OPTION_GET(ConnectTime, int);
#endif
			/*
			Have on windows.
			Returns the maximum outbound message size for message-oriented sockets supported by the protocol. 
			Has no meaning for stream-oriented sockets.
			*/
#ifdef SO_MAX_MSG_SIZE
			DEF_SOCKET_OPTION_PROPERTY_GET(MaxMessageSize, int, SOL_SOCKET, SO_MAX_MSG_SIZE);
#else
			DEF_SOCKET_EMPTY_OPTION_GET(MaxMessageSize, int);
#endif
			/*
			Have on windows.
			Returns the maximum size, in bytes, for outbound datagrams supported by the protocol. 
			This socket option has no meaning for stream-oriented sockets.
			*/
#ifdef SO_MAXDG
			DEF_SOCKET_OPTION_PROPERTY_GET(MaxDatagramSize, int, SOL_SOCKET, SO_MAXDG);
#else
			DEF_SOCKET_EMPTY_OPTION_GET(MaxDatagramSize, int);
#endif

			/*
			Have on some unix.
			Enable or disable the receiving of the SCM_CREDENTIALS control message.
			*/
#ifdef SO_PASSCRED
			DEF_SOCKET_OPTION_PROPERTY(IsEnableCredentionalsContrMsg, bool, bool, SOL_SOCKET, SO_PASSCRED);
#else
			DEF_SOCKET_EMPTY_OPTION(IsEnableCredentionalsContrMsg, bool, bool);
#endif
			/*
			Have on some unix.
			Enable or disable the receiving of the SCM_CREDENTIALS control message.
			*/
#ifdef SO_TIMESTAMP
			DEF_SOCKET_OPTION_PROPERTY(IsEnableTimestampMsg, bool, bool, SOL_SOCKET, SO_TIMESTAMP);
#else
			DEF_SOCKET_EMPTY_OPTION(IsEnableTimestampMsg, bool, bool);
#endif
			/*
			Have on some unix.
			Set the mark for each packet sent through this socket (similar to the netfilter MARK target but socket-based). 
			Changing the mark can be used for mark-based routing without netfilter or for packet filtering. 
			Setting this option requires the CAP_NET_ADMIN capability.
			*/
#ifdef SO_MARK
			DEF_SOCKET_OPTION_PROPERTY(IsSetMarkForEachPacket, bool, bool, SOL_SOCKET, SO_MARK);
#else
			DEF_SOCKET_EMPTY_OPTION(IsSetMarkForEachPacket, bool, bool);
#endif


#ifdef SO_ACCEPTFILTER
			DEF_SOCKET_OPTION_PROPERTY(AcceptFilter, accept_filter_arg&, accept_filter_arg, SOL_SOCKET, SO_ACCEPTFILTER);
#else
			DEF_SOCKET_EMPTY_OPTION(AcceptFilter, std::empty_type&, std::empty_type);
#endif

			/*
			Have on some unix.
			Set the protocol-defined priority for  all  packets
            to  be  sent on this socket.  Linux uses this value
            to order the  networking  queues:  packets  with  a
            higher priority may be processed first depending on
            the selected device queueing discipline. For ip(4),
            this  also  sets the IP type-of-service (TOS) field
            for outgoing packets.
			*/
#ifdef SO_PRIORITY
			DEF_SOCKET_OPTION_PROPERTY(Priority, int, int, SOL_SOCKET, SO_PRIORITY);
#else
			DEF_SOCKET_EMPTY_OPTION(Priority, int, int);
#endif

			/*
			Have on windows.
			Once set, affects whether subsequent sockets that are created will be non-overlapped.
			The possible values for this option are SO_SYNCHRONOUS_ALERT and SO_SYNCHRONOUS_NONALERT. 
			This option should not be used.
			Instead use the WSASocket function and leave the WSA_FLAG_OVERLAPPED bit in the dwFlags parameter turned off.
			*/
#ifdef SO_OPENTYPE
			DEF_SOCKET_OPTION_PROPERTY(OpenType, int, int, SOL_SOCKET, SO_OPENTYPE);
#else
			DEF_SOCKET_EMPTY_OPTION(OpenType, int, int);
#endif

			/*
			Have on windows.
			Use this option for listening sockets. When the option is set, 
			the socket responds to all incoming connections with an RST rather than accepting them.
			*/
#ifdef SO_PAUSE_ACCEPT
			DEF_SOCKET_OPTION_PROPERTY(IsPauseAccept, bool, bool, SOL_SOCKET, SO_PAUSE_ACCEPT);
#else
			DEF_SOCKET_EMPTY_OPTION(IsPauseAccept, bool, bool);
#endif

			/*
			Have on windows.
			Enables local port scalability for a socket by allowing port allocation to be 
			maximized by allocating wildcard ports multiple times for different local address port pairs on a 
			local machine. On platforms where both options are available, prefer 
			SO_REUSE_UNICASTPORT instead of this option. See the SO_PORT_SCALABILITY reference for more information.
			*/
#ifdef SO_PORT_SCALABILITY 
			DEF_SOCKET_OPTION_PROPERTY(IsPortScalability, bool, bool, SOL_SOCKET, SO_PORT_SCALABILITY);
#else
			DEF_SOCKET_EMPTY_OPTION(IsPortScalability, bool, bool);
#endif

			/*
			Have on windows.
			When set, allow ephemeral port reuse for Winsock API connection functions which require an 
			explicit bind, such as ConnectEx. Note that connection functions with an implicit bind (such as connect without an explicit bind) 
			have this option set by default. Use this option instead of SO_PORT_SCALABILITY on platforms where both are available.
			*/
#ifdef SO_REUSE_UNICASTPORT  
			DEF_SOCKET_OPTION_PROPERTY(IsReuseUnicastPort, bool, bool, SOL_SOCKET, SO_REUSE_UNICASTPORT);
#else
			DEF_SOCKET_EMPTY_OPTION(IsReuseUnicastPort, bool, bool);
#endif

			/*
			Have on windows.
			When set on a socket, this option indicates that the socket will never be used to receive unicast packets, 
			and consequently that its port can be shared with other multicast-only applications. Setting the value to 1 
			enables always sharing multicast traffic on the port. Setting the value to 0 (default) disables this behavior.
			*/
#ifdef SO_REUSE_MULTICASTPORT 
			DEF_SOCKET_OPTION_PROPERTY(IsReuseMulticastPort, bool, bool, SOL_SOCKET, SO_REUSE_MULTICASTPORT);
#else
			DEF_SOCKET_EMPTY_OPTION(IsReuseMulticastPort, bool, bool);
#endif
			/*
			The timeout, in milliseconds, for blocking send calls. 
			The default for this option is zero, which indicates that a send operation will not time out. 
			If a blocking send call times out, the connection is in an indeterminate state and should be closed.
			*/
#ifdef SO_SNDTIMEO
			DEF_SOCKET_OPTION_PROPERTY(SendTimeout, int, int, SOL_SOCKET, SO_SNDTIMEO);
#else
			DEF_SOCKET_EMPTY_OPTION(SendTimeout, int, int);
#endif

#ifdef SO_RCVTIMEO
			DEF_SOCKET_OPTION_PROPERTY(ReceiveTimeout, int, int, SOL_SOCKET, SO_RCVTIMEO);
#else
			DEF_SOCKET_EMPTY_OPTION(ReceiveTimeout, int, int);
#endif

			/*
			The total per-socket buffer space reserved for sends. 
			This is unrelated to SO_MAX_MSG_SIZE and does not necessarily correspond to the size of a TCP send window.
			*/
#ifdef SO_SNDBUF
			DEF_SOCKET_OPTION_PROPERTY(SendSizeBuffer, int, int, SOL_SOCKET, SO_SNDBUF);
#else
			DEF_SOCKET_EMPTY_OPTION(SendSizeBuffer, int, int);
#endif
			/*
			The total per-socket buffer space reserved for receives. 
			This is unrelated to SO_MAX_MSG_SIZE and does not necessarily correspond to the size of the TCP receive window.
			*/
#ifdef SO_RCVBUF
			DEF_SOCKET_OPTION_PROPERTY(ReceiveSizeBuffer, int, int, SOL_SOCKET, SO_RCVBUF);
#else
			DEF_SOCKET_EMPTY_OPTION(ReceiveSizeBuffer, int, int);
#endif
			/*
			Enables keep-alive for a socket connection. 
			Valid only for protocols that support the notion of keep-alive (connection-oriented protocols). 
			For TCP, the default keep-alive timeout is 2 hours and the keep-alive interval is 1 second. 
			The default number of keep-alive probes varies based on the version of Windows.
			*/
#ifdef SO_KEEPALIVE
			DEF_SOCKET_OPTION_PROPERTY(IsKeepAlive, bool, bool, SOL_SOCKET, SO_KEEPALIVE);
#else
			DEF_SOCKET_EMPTY_OPTION(IsKeepAlive, bool, bool);
#endif
			/*
			Configure a socket for sending broadcast data. 
			This option is only Valid for protocols that support broadcasting (IPX and UDP, for example).
			*/
#ifdef SO_BROADCAST
			DEF_SOCKET_OPTION_PROPERTY(IsBroadcast, bool, bool, SOL_SOCKET, SO_BROADCAST);
#else
			DEF_SOCKET_EMPTY_OPTION(IsBroadcast, bool, bool);
#endif
			/*
			Enable socket debugging.  Only allowed for processes with the
			CAP_NET_ADMIN capability or an effective user ID of 0.
			*/
#ifdef SO_DEBUG
			DEF_SOCKET_OPTION_PROPERTY(IsDebug, bool, bool, SOL_SOCKET, SO_DEBUG);
#else
			DEF_SOCKET_EMPTY_OPTION(IsDebug, bool, bool);
#endif

			/*
			Don't send via a gateway, send only to directly connected
			hosts.  The same effect can be achieved by setting the
			MSG_DONTROUTE flag on a socket send(2) operation.  Expects an
			integer boolean flag.
			*/
#ifdef SO_DONTROUTE
			DEF_SOCKET_OPTION_PROPERTY(IsDontRoute, bool, bool, SOL_SOCKET, SO_DONTROUTE);
#else
			DEF_SOCKET_EMPTY_OPTION(IsDontRoute, bool, bool);
#endif
			/*
			Returns whether a socket is in listening mode. 
			This option is only Valid for connection-oriented protocols.
			*/
#ifdef SO_ACCEPTCONN
			DEF_SOCKET_OPTION_PROPERTY_GET(IsAcceptConnection, bool, SOL_SOCKET, SO_ACCEPTCONN);
#else
			DEF_SOCKET_EMPTY_OPTION_GET(IsAcceptConnection, bool, bool);
#endif
			/*
			Use the local loopback address when sending data from this socket. 
			This option should only be used when all data sent will also be received locally. 
			This option is not supported by the Windows TCP/IP provider. 
			If this option is used on Windows Vista and later, the getsockopt and setsockopt functions fail with WSAEINVAL. 
			On earlier versions of Windows, these functions fail with WSAENOPROTOOPT.
			*/
#ifdef SO_USELOOPBACK
			DEF_SOCKET_OPTION_PROPERTY(IsUsedLoopBack, bool, bool, SOL_SOCKET, SO_USELOOPBACK);
#else
			DEF_SOCKET_EMPTY_OPTION(IsUsedLoopBack, bool, bool);
#endif
			/*
			The socket can be bound to an address which is already in use. 
			Not applicable for ATM sockets.
			*/
#ifdef SO_REUSEADDR
			DEF_SOCKET_OPTION_PROPERTY(IsReuseAddr, bool, bool, SOL_SOCKET, SO_REUSEADDR);
#else
			DEF_SOCKET_EMPTY_OPTION(IsReuseAddr, bool, bool);
#endif

			/*
			If true, the Linger option is disabled.
			*/
#ifdef SO_DONTLINGER
			DEF_SOCKET_OPTION_PROPERTY(IsDontLinger, bool, bool, SOL_SOCKET, SO_DONTLINGER);
#else
			DEF_SOCKET_EMPTY_OPTION(IsDontLinger, bool, bool);
#endif
			/*
			When enabled, a close(2) or shutdown(2) will not return until
			all queued messages for the socket have been successfully sent
			or the linger timeout has been reached.  Otherwise, the call
			returns immediately and the closing is done in the background.
			When the socket is closed as part of exit(2), it always
			lingers in the background.
			*/
#ifdef SO_LINGER
			DEF_SOCKET_OPTION_PROPERTY(Linger, linger&, linger, SOL_SOCKET, SO_LINGER);
#else
			DEF_SOCKET_EMPTY_OPTION(Linger, std::empty_type&, std::empty_type);
#endif
			/*
			Specify the minimum number of bytes in the buffer until the
			socket layer will pass the data to the protocol (SO_SNDLOWAT)
			or the user on receiving (SO_RCVLOWAT).
			
			*/
#ifdef SO_SNDLOWAT
			DEF_SOCKET_OPTION_PROPERTY(SendLowWaterMark, int, int, SOL_SOCKET, SO_SNDLOWAT);
#else
			DEF_SOCKET_EMPTY_OPTION(SendLowWaterMark, int, int);
#endif

			/*
			Задаёт минимальное количество данных на приём(сокет вернёт управление из Recive только при достижении 
			этого количества данных).
			*/
#ifdef SO_RCVLOWAT
			DEF_SOCKET_OPTION_PROPERTY(ReceiveLowWaterMark, int, int, SOL_SOCKET, SO_RCVLOWAT);
#else
			DEF_SOCKET_EMPTY_OPTION(ReceiveLowWaterMark, int, int);
#endif
			/*
			Returns the last error code on this socket. 
			This per-socket error code is not always immediately set.
			*/
#ifdef SO_ERROR
			DEF_SOCKET_OPTION_PROPERTY_GET(LastSocketError, int, SOL_SOCKET, SO_ERROR);
#else
			DEF_SOCKET_EMPTY_OPTION_GET(IsDebug, int);
#endif
			/*
			Indicates that out-of-bound data should be returned in-line with regular data. 
			This option is only valid for connection-oriented protocols that support out-of-band data.
			*/
#ifdef SO_OOBINLINE
			DEF_SOCKET_OPTION_PROPERTY(IsReceivedOOBDataInLine, bool, bool, SOL_SOCKET, SO_OOBINLINE);
#else
			DEF_SOCKET_EMPTY_OPTION(IsReceivedOOBDataInLine, bool, bool);
#endif

			/*
			 Return the credentials of the foreign process  connected  to  this  socket.   Only useful for PF_UNIX
             sockets.  Argument is a  ucred  structure. Only valid as a getsockopt.
			*/
#ifdef SO_PEERCRED
			DEF_SOCKET_OPTION_PROPERTY(PeerCredentials, ucred&, ucred, SOL_SOCKET, SO_PEERCRED);
#else
			DEF_SOCKET_EMPTY_OPTION(PeerCredentials, std::empty_type&, std::empty_type);
#endif

			/*
			 Have on some unix.
			 Set the associated FIB (routing table) for the socket (set only).
			*/
#ifdef SO_SETFIB
			DEF_SOCKET_OPTION_PROPERTY_SET(FIBRoutingTable, int, SOL_SOCKET, SO_SETFIB);
#else
			DEF_SOCKET_EMPTY_OPTION_SET(FIBRoutingTable, int);
#endif


		} SockOptions;

		union
		{
			/*
			Join the socket to the supplied multicast group on the specified interface.
			*/
#ifdef IP_ADD_MEMBERSHIP
			DEF_SOCKET_OPTION_PROPERTY_SET(AddMembership, ip_mreq&, IPPROTO_IP, IP_ADD_MEMBERSHIP);
#else
			DEF_SOCKET_EMPTY_OPTION_SET(AddMembership, ip_mreq&);
#endif
#ifdef IPV6_ADD_MEMBERSHIP
			DEF_SOCKET_OPTION_PROPERTY_SET(Ip6AddMembership, ipv6_mreq&, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP);
#else
			DEF_SOCKET_EMPTY_OPTION_SET(Ip6AddMembership, ipv6_mreq&);
#endif

			/*
			Join the supplied multicast group on the given interface and accept data sourced from the supplied source address.
			*/
#ifdef IP_ADD_SOURCE_MEMBERSHIP
			DEF_SOCKET_OPTION_PROPERTY_SET(AddSourceMembership, ip_mreq_source&, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP);
#else
			DEF_SOCKET_EMPTY_OPTION_SET(AddSourceMembership, ip_mreq_source&);
#endif
			/*
			Removes the given source as a sender to the supplied multicast group and interface.
			*/
#ifdef IP_BLOCK_SOURCE
			DEF_SOCKET_OPTION_PROPERTY_SET(AddBlockSourceMembership, ip_mreq_source&, IPPROTO_IP, IP_BLOCK_SOURCE);
#else
			DEF_SOCKET_EMPTY_OPTION_SET(AddBlockSourceMembership, ip_mreq_source&);
#endif

			/*
			Leaves the specified multicast group from the specified interface. 
			Service providers must support this option when multicast is supported. 
			*/
#ifdef IP_DROP_MEMBERSHIP
			DEF_SOCKET_OPTION_PROPERTY_SET(DropMembership, ip_mreq&, IPPROTO_IP, IP_DROP_MEMBERSHIP);
#else
			DEF_SOCKET_EMPTY_OPTION_SET(DropMembership, ip_mreq&);
#endif

#ifdef IPV6_DROP_MEMBERSHIP
			DEF_SOCKET_OPTION_PROPERTY_SET(Ip6DropMembership, ipv6_mreq&, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP);
#else
			DEF_SOCKET_EMPTY_OPTION_SET(Ip6DropMembership, ipv6_mreq&);
#endif

			/*
			Drops membership to the given multicast group, interface, and source address.
			*/
#ifdef IP_DROP_SOURCE_MEMBERSHIP
			DEF_SOCKET_OPTION_PROPERTY_SET(DropSourceMembership, ip_mreq_source&, IPPROTO_IP, IP_DROP_SOURCE_MEMBERSHIP);
#else
			DEF_SOCKET_EMPTY_OPTION_SET(DropSourceMembership, ip_mreq_source&);
#endif
			/*
			Indicates that data should not be fragmented regardless of the local MTU. 
			Valid only for message oriented protocols. 
			Microsoft TCP/IP providers respect this option for UDP and ICMP.
			*/
#ifdef IP_DONTFRAGMENT
			DEF_SOCKET_OPTION_PROPERTY(IsDontFragment, bool, bool, IPPROTO_IP, IP_DONTFRAGMENT);
#else
			DEF_SOCKET_EMPTY_OPTION(IsDontFragment, bool, bool);
#endif
			/*
			When set to true, indicates the application provides the IP header. 
			Applies only to SOCK_RAW sockets. 
			The TCP/IP service provider may set the ID field, 
			if the value supplied by the application is zero. 
			*/
#ifdef IP_HDRINCL
			DEF_SOCKET_OPTION_PROPERTY(IsIncludeHeader, bool, bool, IPPROTO_IP, IP_HDRINCL);
#else
			DEF_SOCKET_EMPTY_OPTION(IsIncludeHeader, bool, bool);
#endif
#ifdef IPV6_HDRINCL
			DEF_SOCKET_OPTION_PROPERTY(IsIp6IncludeHeader, bool, bool, IPPROTO_IPV6, IPV6_HDRINCL);
#else
			DEF_SOCKET_EMPTY_OPTION(IsIp6IncludeHeader, bool, bool);
#endif
			/*
			Set the multicast hop limit for the socket.  Argument is a
			pointer to an integer.  -1 in the value means use the route
			default, otherwise it should be between 0 and 255.
			*/
#ifdef IPV6_MULTICAST_HOPS
			DEF_SOCKET_OPTION_PROPERTY(Ip6MulticastHopLimit, int, int, IPPROTO_IPV6, IPV6_MULTICAST_HOPS);
#else
			DEF_SOCKET_EMPTY_OPTION(IsIp6IncludeHeader, int, int);
#endif
			/*
			When set to true, indicates the application provides the IP header. 
			Applies only to SOCK_RAW sockets. 
			The TCP/IP service provider may set the ID field, 
			if the value supplied by the application is zero. 
			*/
#ifdef IP_MULTICAST_IF
			DEF_SOCKET_OPTION_PROPERTY(Multicast, int, int, IPPROTO_IP, IP_MULTICAST_IF);
#else
			DEF_SOCKET_EMPTY_OPTION(Multicast, int, int);
#endif
#ifdef IPV6_MULTICAST_IF
			DEF_SOCKET_OPTION_PROPERTY(Ip6Multicast, int, int, IPPROTO_IPV6, IPV6_MULTICAST_IF);
#else
			DEF_SOCKET_EMPTY_OPTION(Ip6Multicast, int, int);
#endif
			/*
			Controls whether data sent by an application on the local computer
			(not necessarily by the same socket) in a multicast session will be received by a
			socket joined to the multicast destination group on the loopback interface.
			*/
#ifdef IP_MULTICAST_LOOP
			DEF_SOCKET_OPTION_PROPERTY(IsMulticastLoop, bool, bool, IPPROTO_IP, IP_MULTICAST_LOOP);
#else
			DEF_SOCKET_EMPTY_OPTION(IsMulticastLoop, bool, bool);
#endif
#ifdef IPV6_MULTICAST_LOOP
			DEF_SOCKET_OPTION_PROPERTY(IsIp6MulticastLoop, bool, bool, IPPROTO_IPV6, IPV6_MULTICAST_LOOP);
#else
			DEF_SOCKET_EMPTY_OPTION(IsMulticastLoop, bool, bool);
#endif
			/*
			Set or read the time-to-live value of outgoing multicast
			packets for this socket.  It is very important for multicast
			packets to set the smallest TTL possible.  The default is 1
			which means that multicast packets don't leave the local
			network unless the user program explicitly requests it.
			Argument is an integer.
			*/
#ifdef IP_MULTICAST_LOOP
			DEF_SOCKET_OPTION_PROPERTY(MulticastTimeToLive, int, int, IPPROTO_IP, IP_MULTICAST_TTL);
#else
			DEF_SOCKET_EMPTY_OPTION(MulticastTimeToLive, int, int);
#endif
			/*
			Pass an IP_PKTINFO ancillary message that contains a pktinfo
			structure that supplies some information about the incoming
			packet.  This only works for datagram oriented sockets.
			*/
#ifdef IP_PKTINFO
			DEF_SOCKET_OPTION_PROPERTY(PacketInfo, int, int, IPPROTO_IP, IP_PKTINFO);
#else
			DEF_SOCKET_EMPTY_OPTION(PacketInfo, int, int);
#endif
			/*
			Indicates that packet information should be returned by the recvmsg.
			*/
#ifdef IPV6_PKTINFO
			DEF_SOCKET_OPTION_PROPERTY(IsIp6PacketInfo, bool, bool, IPPROTO_IPV6, IPV6_PKTINFO);
#else
			DEF_SOCKET_EMPTY_OPTION(IsIp6PacketInfo, bool, bool);
#endif

			/*

			*/
#ifdef IP_RECVIF
			DEF_SOCKET_OPTION_PROPERTY(IsRecive, bool, bool, IPPROTO_IP, IP_RECVIF);
#else
			DEF_SOCKET_EMPTY_OPTION(IsRecive, bool, bool);
#endif
#ifdef IPV6_RECVIF
			DEF_SOCKET_OPTION_PROPERTY(IsIp6Recive, bool, bool, IPPROTO_IPV6, IPV6_RECVIF);
#else
			DEF_SOCKET_EMPTY_OPTION(IsIp6Recive, bool, bool);
#endif

			/*
			Indicates if a socket created for the AF_INET6 address family is restricted to IPv6 communications only.
			*/
#ifdef IPV6_V6ONLY
			DEF_SOCKET_OPTION_PROPERTY(IsIp6Only, bool, bool, IPPROTO_IPV6, IPV6_V6ONLY);
#else
			DEF_SOCKET_EMPTY_OPTION(IsIp6Only, bool, bool);
#endif
			/*
			Set or receive the Type-Of-Service (TOS) field that is sent
			with every IP packet originating from this socket.  It is used
			to prioritize packets on the network.
			*/
#ifdef IP_TOS
			DEF_SOCKET_OPTION_PROPERTY(IsTypeOfService, bool, bool, IPPROTO_IP, IP_TOS);
#else
			DEF_SOCKET_EMPTY_OPTION(IsTypeOfService, bool, bool);
#endif
			/* 
			Changes the default value set by the TCP/IP service provider 
			in the TTL field of the IP header in outgoing datagrams.
			*/
#ifdef IP_TTL
			DEF_SOCKET_OPTION_PROPERTY(IsSetTimeToLive, bool, bool, IPPROTO_IP, IP_TTL);
#else
			DEF_SOCKET_EMPTY_OPTION(IsSetTimeToLive, bool, bool);
#endif

			/* 
			Changes the default value set by the TCP/IP service provider 
			in the TTL field of the IP header in outgoing datagrams.
			*/
#ifdef IP_UNBLOCK_SOURCE
			DEF_SOCKET_OPTION_PROPERTY_SET(UnblockSource, ip_mreq_source&, IPPROTO_IP, IP_UNBLOCK_SOURCE);
#else
			DEF_SOCKET_EMPTY_OPTION_SET(UnblockSource, ip_mreq_source&);
#endif

			/* 
			Have on unix.
			Setting this boolean option enables transparent proxying on
			this socket.  This socket option allows the calling
			application to bind to a nonlocal IP address and operate both
			as a client and a server with the foreign address as the local
			endpoint.  NOTE: this requires that routing be set up in a way
			that packets going to the foreign address are routed through
			the TProxy box (i.e., the system hosting the application that
			employs the IP_TRANSPARENT socket option).  Enabling this
			socket option requires superuser privileges (the CAP_NET_ADMIN
			capability).
			*/
#ifdef IP_TRANSPARENT
			DEF_SOCKET_OPTION_PROPERTY(IsEnablesTransparentProxying, bool, bool, IPPROTO_IP, IP_TRANSPARENT);
#else
			DEF_SOCKET_EMPTY_OPTION(IsEnablesTransparentProxying, bool, bool);
#endif

			/* 
			Have on unix.
			If enabled (argument is nonzero), the reassembly of outgoing
			packets is disabled in the netfilter layer.  The argument is
			an integer.

			This option is valid only for SOCK_RAW sockets.
			*/
#ifdef IP_NODEFRAG
			DEF_SOCKET_OPTION_PROPERTY(IsNoDefrag, bool, bool, IPPROTO_IP, IP_NODEFRAG);
#else
			DEF_SOCKET_EMPTY_OPTION(IsNoDefrag, bool, bool);
#endif

			/* 
			Have on unix.
			This option provides access to the advanced full-state
			filtering API.
			There are two macros, MCAST_INCLUDE and MCAST_EXCLUDE, which
			can be used to specify the filtering mode.  Additionally, the
			IP_MSFILTER_SIZE(n) macro exists to determine how much memory
			is needed to store ip_msfilter structure with n sources in the
			source list.

			*/
#ifdef IP_MSFILTER
			DEF_SOCKET_OPTION_PROPERTY(MsFilter, ip_msfilter&, ip_msfilter, IPPROTO_IP, IP_MSFILTER);
#else
			DEF_SOCKET_EMPTY_OPTION(MsFilter, std::empty_type, std::empty_type);
#endif

			/* 
			Have on unix.
			If enabled, this boolean option allows binding to an IP
			address that is nonlocal or does not (yet) exist.  This
			permits listening on a socket, without requiring the
			underlying network interface or the specified dynamic IP
			address to be up at the time that the application is trying to
			bind to it.  This option is the per-socket equivalent of the
			ip_nonlocal_bind /proc interface described below.
			*/
#ifdef IP_FREEBIND
			DEF_SOCKET_OPTION_PROPERTY(IsFreeBind, bool, bool, IPPROTO_IP, IP_FREEBIND);
#else
			DEF_SOCKET_EMPTY_OPTION(IsFreeBind, bool, bool);
#endif
		} IpOptions;

		union
		{
			/*
			Have on windows.
			If TRUE, the service provider implements the Berkeley Software Distribution (BSD) style (default) 
			for handling expedited data. This option is the inverse of the TCP_EXPEDITED_1122 option. 
			*/
#ifdef TCP_BSDURGENT
			DEF_SOCKET_OPTION_PROPERTY(IsBsdUrgent, bool, bool, IPPROTO_TCP, TCP_BSDURGENT);
#else
			DEF_SOCKET_EMPTY_OPTION(IsBsdUrgent, bool, bool);
#endif
			/*
			Have on windows.
			If TRUE, the service provider implements the expedited data as specified in RFC-1222. 
			Otherwise, the Berkeley Software Distribution (BSD) style (default) is used.
			*/
#ifdef TCP_EXPEDITED_1122
			DEF_SOCKET_OPTION_PROPERTY(IsExpeditedRFC1222, bool, bool, IPPROTO_TCP, TCP_EXPEDITED_1122);
#else
			DEF_SOCKET_EMPTY_OPTION(IsExpeditedRFC1222, bool, bool);
#endif

			/*
			Have on windows.
			If this value is non-negative, it represents the desired connection timeout in seconds. 
			If it is -1, it represents a request to disable connection timeout (i.e. the connection will retransmit forever). 
			If the connection timeout is disabled, the retransmit timeout increases exponentially 
			for each retransmission up to its maximum value of 60sec and then stays there.
			*/
#ifdef TCP_MAXRT
			DEF_SOCKET_OPTION_PROPERTY(RequestDisableConnectTimeout, int, int, IPPROTO_TCP, TCP_MAXRT);
#else
			DEF_SOCKET_EMPTY_OPTION(RequestDisableConnectTimeout, int, int);
#endif

			/*
			Have on windows, some unix.
			Enables or disables the Nagle algorithm for TCP sockets. 
			This option is disabled (set to FALSE) by default.
			*/
#ifdef TCP_NODELAY
			DEF_SOCKET_OPTION_PROPERTY(IsEnableNagleAlgorithm, bool, bool, IPPROTO_TCP, TCP_NODELAY);
#else
			DEF_SOCKET_EMPTY_OPTION(IsEnableNagleAlgorithm, bool, bool);
#endif

			/*
			Have on windows, some unix.
			Enables or disables RFC 1323 time stamps. 
			Note that there is also a global configuration for timestamps (default is off), 
			"Timestamps" in (set/get)-nettcpsetting. Setting this socket option overrides that global configuration setting.
			*/
#ifdef TCP_TIMESTAMPS
			DEF_SOCKET_OPTION_PROPERTY(IsEnableRFC1323TimeStamps, bool, bool, IPPROTO_TCP, TCP_TIMESTAMPS);
#else
			DEF_SOCKET_EMPTY_OPTION(IsEnableRFC1323TimeStamps, bool, bool);
#endif

			/*
			Have on unix.
			The argument for this option is a string.  This option allows
			the caller to set the TCP congestion control algorithm to be
			used, on a per-socket basis.  Unprivileged processes are
			restricted to choosing one of the algorithms in
			tcp_allowed_congestion_control (described above).  Privileged
			processes (CAP_NET_ADMIN) can choose from any of the available
			congestion-control algorithms (see the description of
			tcp_available_congestion_control above).
			*/
#ifdef TCP_CONGESTION

			DEF_SOCKET_OPTION_PROPERTY_SET(CongestionControlAlgorithm, const char*, IPPROTO_TCP, TCP_CONGESTION);
#else
			DEF_SOCKET_EMPTY_OPTION_SET(CongestionControlAlgorithm, const char*);
#endif

			/*
			Have on unix.
			If set, don't send out partial frames.  All queued partial
			frames are sent when the option is cleared again.  This is
			useful for prepending headers before calling sendfile, or
			for throughput optimization.  As currently implemented, there
			is a 200 millisecond ceiling on the time for which output is
			corked by TCP_CORK.  If this ceiling is reached, then queued
			data is automatically transmitted.  This option can be
			combined with TCP_NODELAY only since Linux 2.5.71.  This
			option should not be used in code intended to be portable.
			*/
#ifdef TCP_CORK
			DEF_SOCKET_OPTION_PROPERTY(IsDontSendPartialFrames, bool, bool, IPPROTO_TCP, TCP_CORK);
#else
			DEF_SOCKET_EMPTY_OPTION(IsDontSendPartialFrames, bool, bool);
#endif

			/*
			Have on unix.
			Allow a listener to be awakened only when data arrives on the
			socket.  Takes an integer value (seconds), this can bound the
			maximum number of attempts TCP will make to complete the
			connection.  This option should not be used in code intended
			to be portable.
			*/
#ifdef TCP_DEFER_ACCEPT
			DEF_SOCKET_OPTION_PROPERTY(DeferAccept, int, int, IPPROTO_TCP, TCP_DEFER_ACCEPT);
#else
			DEF_SOCKET_EMPTY_OPTION(DeferAccept, int, int);
#endif

			/*
			Have on unix.
			In computer networking, TCP Fast Open (TFO) is an extension to 
			speed up the opening of successive TCP connections between two endpoints.
			*/
#ifdef TCP_FASTOPEN
			DEF_SOCKET_OPTION_PROPERTY(IsFastOpen, bool, bool, IPPROTO_TCP, TCP_FASTOPEN);
#else
			DEF_SOCKET_EMPTY_OPTION(IsFastOpen, bool, bool);
#endif
			/*
			Have on unix.
			Used to collect information about this socket.  The kernel
			returns a struct tcp_info as defined in the file
			/usr/include/linux/tcp.h.  This option should not be used in
			code intended to be portable.
			*/
#ifdef TCP_INFO
			DEF_SOCKET_OPTION_PROPERTY(TcpInfo, tcp_info&, tcp_info, IPPROTO_TCP, TCP_INFO);
#else
			DEF_SOCKET_EMPTY_OPTION(TcpInfo, std::empty_type&, std::empty_type);
#endif

			/*
			Have on unix.
			The maximum number of keepalive probes TCP should send before
			dropping the connection.  This option should not be used in
			code intended to be portable.
			*/
#ifdef TCP_KEEPCNT
			DEF_SOCKET_OPTION_PROPERTY(KeepCnt, int, int, IPPROTO_TCP, TCP_KEEPCNT);
#else
			DEF_SOCKET_EMPTY_OPTION(KeepCnt, int, int);
#endif

			/*
			Have on unix.
			The time (in seconds) the connection needs to remain idle
			before TCP starts sending keepalive probes, if the socket
			option SO_KEEPALIVE has been set on this socket.  This option
			should not be used in code intended to be portable.
			*/
#ifdef TCP_KEEPIDLE
			DEF_SOCKET_OPTION_PROPERTY(KeepIdle, int, int, IPPROTO_TCP, TCP_KEEPIDLE);
#else
			DEF_SOCKET_EMPTY_OPTION(KeepIdle, int, int);
#endif

			/*
			Have on unix.
			The time (in seconds) between individual keepalive probes.
			This option should not be used in code intended to be
			portable.
			*/
#ifdef TCP_KEEPINTVL
			DEF_SOCKET_OPTION_PROPERTY(KeepIntvl, int, int, IPPROTO_TCP, TCP_KEEPINTVL);
#else
			DEF_SOCKET_EMPTY_OPTION(KeepIntvl, int, int);
#endif
			/*
			Have on unix, windows.
			The maximum segment size for outgoing TCP packets.  In Linux
			2.2 and earlier, and in Linux 2.6.28 and later, if this option
			is set before connection establishment, it also changes the
			MSS value announced to the other end in the initial packet.
			Values greater than the (eventual) interface MTU have no
			effect.  TCP will also impose its minimum and maximum bounds
			over the value provided.
			*/
#ifdef TCP_MAXSEG
			DEF_SOCKET_OPTION_PROPERTY(MaxSegmentSize, int, int, IPPROTO_TCP, TCP_MAXSEG);
#else
			DEF_SOCKET_EMPTY_OPTION(MaxSegmentSize, int, int);
#endif

			/*
			Have on unix.
			Enable quickack mode if set or disable quickack mode if
			cleared.  In quickack mode, acks are sent immediately, rather
			than delayed if needed in accordance to normal TCP operation.
			This flag is not permanent, it only enables a switch to or
			from quickack mode.  Subsequent operation of the TCP protocol
			will once again enter/leave quickack mode depending on
			internal protocol processing and factors such as delayed ack
			timeouts occurring and data transfer.  This option should not
			be used in code intended to be portable.
			*/
#ifdef TCP_QUICKACK
			DEF_SOCKET_OPTION_PROPERTY(IsQuickackEnable, bool, bool, IPPROTO_TCP, TCP_QUICKACK);
#else
			DEF_SOCKET_EMPTY_OPTION(IsQuickackEnable, bool, bool);
#endif

			/*
			Have on unix.
			This option takes an unsigned int as an argument.  When the
			value is greater than 0, it specifies the maximum amount of
			time in milliseconds that transmitted data may remain
			unacknowledged before TCP will forcibly close the
			corresponding connection and return ETIMEDOUT to the
			application.  If the option value is specified as 0, TCP will
			to use the system default.
			*/
#ifdef TCP_USER_TIMEOUT
			DEF_SOCKET_OPTION_PROPERTY(UserTimeout, unsigned, unsigned, IPPROTO_TCP, TCP_USER_TIMEOUT);
#else
			DEF_SOCKET_EMPTY_OPTION(UserTimeout, unsigned, unsigned);
#endif

			/*
			Have on unix.
			Set the number of SYN retransmits that TCP should send before
			aborting the attempt to connect.  It cannot exceed 255.  This
			option should not be used in code intended to be portable.
			*/
#ifdef TCP_SYNCNT
			DEF_SOCKET_OPTION_PROPERTY(SynRetransmitCount, int, int, IPPROTO_TCP, TCP_SYNCNT);
#else
			DEF_SOCKET_EMPTY_OPTION(SynRetransmitCount, int, int);
#endif

			/*
			Have on unix.
			Bound the size of the advertised window to this value.  The
			kernel imposes a minimum size of SOCK_MIN_RCVBUF/2.  This
			option should not be used in code intended to be portable.
			*/
#ifdef TCP_WINDOW_CLAMP
			DEF_SOCKET_OPTION_PROPERTY(WindowClamp, int, int, IPPROTO_TCP, TCP_WINDOW_CLAMP);
#else
			DEF_SOCKET_EMPTY_OPTION(WindowClamp, int, int);
#endif
		} TcpOptions;

		union
		{
			/*
			Have on windows, unix.
			When TRUE, UDP datagrams are sent with a checksum.
			*/
#ifdef UDP_CHECKSUM_COVERAGE
			DEF_SOCKET_OPTION_PROPERTY(IsEnableChecksum, bool, bool, IPPROTO_UDP, UDP_CHECKSUM_COVERAGE);
#else
			DEF_SOCKET_EMPTY_OPTION(IsEnableChecksum, bool, bool);
#endif
			/*
			Have on windows, unix.
			When TRUE, UDP datagrams are sent with the checksum of zero. 
			Required for service providers. If a service provider does not have a mechanism to disable UDP checksum calculation,
			it may simply store this option without taking any action. This option is not supported for IPv6.
			*/
#ifdef UDP_NOCHECKSUM
			DEF_SOCKET_OPTION_PROPERTY(IsZeroChecksum, bool, bool, IPPROTO_UDP, UDP_NOCHECKSUM);
#else
			DEF_SOCKET_EMPTY_OPTION(IsZeroChecksum, bool, bool);
#endif
			/*
			Have on unix.
			If this option is enabled, then all data output on this socket
			is accumulated into a single datagram that is transmitted when
			the option is disabled.  This option should not be used in
			code intended to be portable.
			*/
#ifdef UDP_CORK
			DEF_SOCKET_OPTION_PROPERTY(IsCork, bool, bool, IPPROTO_UDP, UDP_CORK);
#else
			DEF_SOCKET_EMPTY_OPTION(IsCork, bool, bool);
#endif
		} UdpOptions;
	};

	/*
	Get info about protocol by index number
	IPPROTO_
	example:
	char * ProtocolName = GetSystemProtocol(IPPROTO_TCP).Name; //ProtocolName eq. "tcp"
	*/
	static PROTOCOL_INTERATOR GetSystemProtocol(int Index)
	{
		return PROTOCOL_INTERATOR(getprotobynumber(Index));
	}

	/*
	Get info about protocol by name string
	example:
	int ProtocolIndex = GetSystemProtocol("ip").Index; //ProtocolIndex eq. 0
	*/
	static PROTOCOL_INTERATOR GetSystemProtocol(const char * Name)
	{
		return PROTOCOL_INTERATOR(getprotobyname(Name));
	}

	/*
	Get info about service by port number
	example:
	char * PortServiceName = GetSystemService(80).Name; //ProtocolName eq. "http"
	*/
	static PORT_SERVICE_INTERATOR GetSystemService(int PortNumber, const char * Prot = nullptr)
	{
		return PORT_SERVICE_INTERATOR(getservbyport(htons(PortNumber), Prot));
	}

	/*
	Get info about service by name
	example:
	int Port = GetSystemProtocol("http").Port; //ProtocolIndex eq. 80
	*/
	static PORT_SERVICE_INTERATOR GetSystemService(const char * Name, const char * Prot = nullptr)
	{
		return PORT_SERVICE_INTERATOR(getservbyname(Name, Prot));
	}

	/*
	Get info about network by number
	example:
	int NumNet = GetSystemNetwork("loop").NumberNet; //ProtocolIndex eq. 127.0.0.0
	*/
	static NET_INTERATOR GetSystemNetwork(long net, int type)
	{
		return NET_INTERATOR(getnetbyaddr(net, type));
	}

	/*
	Get info about network by Name
	example:
	int NumNet = GetSystemNetwork("loop").NumberNet; //ProtocolIndex eq. 127.0.0.0
	*/
	static NET_INTERATOR GetSystemNetwork(const char * Name)
	{
		return NET_INTERATOR(getnetbyname(Name));
	}

	/*
	Get info about host by address
	Addr - Pointer on address
	Len  - Length of address
	Type - Type as AF_INET or AF_INET6 and another AF_

	example:
	std::string strAddr = Url.GetInfoAboutHost("google.com").Addresses[0]; //strAddr eq. "173.194.122.197"
	*/
	static INFO_HOST_INTERATOR GetInfoAboutHost(const void * Addr, int Len, int Type)
	{
		return INFO_HOST_INTERATOR(gethostbyaddr((const char*)Addr, Len, Type));
	}

	/*
	Get info about host by name or address in string
	example:
	std::string strAddr = Url.GetInfoAboutHost("google.com").Addresses[0]; //strAddr eq. "173.194.122.197"
	*/
	static INFO_HOST_INTERATOR GetInfoAboutHost(const char * NameOrTextAddress)
	{
		return INFO_HOST_INTERATOR(gethostbyname(NameOrTextAddress));
	}

	/*
	Connect with server at a specific address.
	*/
	bool Connect(typename ADDRESS_INFO::ADDRESS_INTERATOR& Address)
	{
		INIT_WSA;
		if(IsOpen)
		{
			ShutdownSendRecive();
			Close();
		}
		addrinfo *i = Address;
		if((RemoteIp.hSocket = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == INVALID_SOCKET)
		{
			URL_SET_LAST_ERR;
			return false;
		}else if (connect(RemoteIp.hSocket, i->ai_addr, i->ai_addrlen) == SOCKET_ERROR)
		{
			URL_SET_LAST_ERR;
			Close();
			return false;
		}		
		RemoteIp.ProtocolType = i->ai_protocol;
		return EvntConnect();
	}

	/*
	Connect with server by address.
	*/
	bool Connect(ADDRESS_INFO& AddrInfo)
	{
		INIT_WSA;
		if(IsOpen)
		{
			ShutdownSendRecive();
			Close();
		}
		addrinfo *i = AddrInfo;
		for (; i != nullptr; i = i->ai_next) 
		{
			if((RemoteIp.hSocket = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == INVALID_SOCKET)
				continue;
			if (connect(RemoteIp.hSocket, i->ai_addr, i->ai_addrlen) != SOCKET_ERROR)
				break;
			Close();
		}
		if(i == nullptr)
		{
			URL_SET_LAST_ERR;
			return false;
		}
		RemoteIp.ProtocolType = i->ai_protocol;
		return EvntConnect(); 
	}

	/*
	Connect with server.
		@Port - number or type protocol. Example: "http" or "80".
		@HostAddr - Address of host. Example "117.134.75.32" or "example.com".
		@Protocol - Type of protocol.
		@Family - Famaly of using protocol.
		@Flags - Flags used in "hints" argument to getaddrinfo().

		@return - true is success and false at error.
	*/
	bool Connect
	(
		const char* Port, 
		const char* HostAddr = nullptr, 
		int Socktype = SOCK_STREAM, 
		int Protocol = IPPROTO_TCP, 
		int Family = AF_UNSPEC, 
		int Flags = 0
	)
	{	
		INIT_WSA;
		if(IsOpen)
		{
			ShutdownSendRecive();
			Close();
		}
		addrinfo hi = {0}, *ah = nullptr, *i;
		hi.ai_socktype = Socktype;
		hi.ai_family = Family;
		hi.ai_protocol = Protocol;
		hi.ai_flags = Flags;                   //AI_PASSIVE

		if(getaddrinfo(HostAddr, Port, &hi, &ah) != 0)
		{
			URL_SET_LAST_ERR;
			return false;
		}

		for (i = ah; i != nullptr; i = i->ai_next) 
		{
			if((RemoteIp.hSocket = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == INVALID_SOCKET)
				continue;
			if (connect(RemoteIp.hSocket, i->ai_addr, i->ai_addrlen) != SOCKET_ERROR)
				break;
			Close();
		}
		if(i == nullptr)
		{
			URL_SET_LAST_ERR;
			if(ah != nullptr)
				freeaddrinfo(ah);
			return false;
		}
		RemoteIp.ProtocolType = i->ai_protocol;
		if(ah != nullptr)
			freeaddrinfo(ah);
		return EvntConnect(); 
	}

	/*
	Create waiting client on port at a specific address.
	*/
	bool Bind(typename ADDRESS_INFO::ADDRESS_INTERATOR& Address, int MaxConnection = SOMAXCONN)
	{	
		INIT_WSA;
		if(IsOpen)
		{
			ShutdownSendRecive();
			Close();
		}
		addrinfo *i = Address;
		if((RemoteIp.hSocket = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == INVALID_SOCKET)
		{
			URL_SET_LAST_ERR;
			return false;
		}else if(bind(RemoteIp.hSocket, i->ai_addr, i->ai_addrlen) == SOCKET_ERROR)
		{
			Close();
			URL_SET_LAST_ERR;
			return false;
		}else if(listen(RemoteIp.hSocket, MaxConnection) == SOCKET_ERROR)
		{
			Close();
			URL_SET_LAST_ERR;
			return false;
		}
		RemoteIp.ProtocolType = i->ai_protocol;

		return EvntBind();
	}

	/*
	Create waiting connection on port by address info.
	*/
	bool Bind(ADDRESS_INFO& AddrInfo, int MaxConnection = SOMAXCONN)
	{	
		INIT_WSA;
		if(IsOpen)
		{
			ShutdownSendRecive();
			Close();
		}
		addrinfo *i = AddrInfo;
		for (;i != nullptr; i = i->ai_next) 
		{
			if((RemoteIp.hSocket = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == INVALID_SOCKET)
				continue;
			if(bind(RemoteIp.hSocket, i->ai_addr, i->ai_addrlen) == SOCKET_ERROR)
			{
				Close();
				continue;
			}
			if(listen(RemoteIp.hSocket, MaxConnection) == SOCKET_ERROR)
			{
				Close();
				continue;
			}
			break;
		}
		RemoteIp.ProtocolType = i->ai_protocol;
		return EvntBind();
	}

	/*
	Create waiting connection on port.
		@Port - number or type protocol. Example: "http" or "80".
		@MaxConnection - argument defines the maximum length to which the queue of pending connections for sockfd may grow.
		@Protocol - Type of protocol.
		@Family - Famaly of using protocol.
		@Flags - Flags used in "hints" argument to getaddrinfo().

		@return - true is success and false at error.
	*/
	bool Bind
	(
		const char * Port, 
		int MaxConnection = SOMAXCONN, 
		int Socktype = SOCK_STREAM, 
		int Protocol = IPPROTO_TCP,
		int Family = AF_INET, 
		int Flags = AI_PASSIVE
	)
	{	
		INIT_WSA;
		if(IsOpen)
		{
			ShutdownSendRecive();
			Close();
		}
		addrinfo host_info = {0},*ah = nullptr, *i;
		host_info.ai_socktype = Socktype;
		host_info.ai_family = Family;
		host_info.ai_protocol = Protocol;
		host_info.ai_flags = Flags;                   //AI_PASSIVE
		if(getaddrinfo(nullptr, Port, &host_info, &ah) != 0)
		{
			URL_SET_LAST_ERR;
			return false;
		}

		for (i = ah; i != nullptr; i = i->ai_next) 
		{
			if((RemoteIp.hSocket = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == INVALID_SOCKET)
				continue;
			if(bind(RemoteIp.hSocket, i->ai_addr, i->ai_addrlen) == SOCKET_ERROR)
			{
				Close();
				continue;
			}
			if(listen(RemoteIp.hSocket, MaxConnection) == SOCKET_ERROR)
			{
				Close();
				continue;
			}
			break;
		}
		RemoteIp.ProtocolType = i->ai_protocol;
		if(ah != nullptr)
			freeaddrinfo(ah);
		return EvntBind();
	}

	/*
	Accepting incoming client.
	*/
	bool AcceptClient(__QUERY_URL & DestCoonection)
	{
		TDESCR ConnectedSocket;
		if((ConnectedSocket = accept(RemoteIp.hSocket, nullptr, nullptr)) == INVALID_SOCKET)
		{
			URL_SET_LAST_ERR;
			return false;
		}
		DestCoonection.RemoteIp.hSocket = ConnectedSocket;
		DestCoonection.RemoteIp.ProtocolType = RemoteIp.ProtocolType;
#ifdef _WIN32
		DestCoonection.RemoteIp.IsNonBlocked = false;
#endif
		DestCoonection.LastError.Clear();
		return EvntAcceptClient(ConnectedSocket);
	}

	/*
	Duplicate for sharing in new process.
	*/
	bool Duplicate(int TargetProcessHandle)
	{
	
#ifdef _WIN32
		WSAPROTOCOL_INFOA ProtInfo;
		if(winsock::WSADuplicateSocketA(RemoteIp.hSocket, TargetProcessHandle, &ProtInfo) == SOCKET_ERROR)
		{
			URL_SET_LAST_ERR;
			return false;
		}
		return true;
#else
		return true;
#endif
	}


	/*
	Open as standart file.
	NOTE: On windows for correct use, socket descriptor should be opened without OVERLAPED function.
	@mode - Mode for open as file.
	@return - FILE structure.
	*/
	FILE* OpenAsFile(const char * mode = "w+")
	{
		FILE* File;
#ifdef _WIN32
		const char * m = mode; 
		int fileflag = 0;
		while (*mode == ' ') ++mode;
		switch (*mode) 
		{
			case 'r': fileflag = _O_RDONLY; break;
			case 'w': fileflag = _O_CREAT | _O_WRONLY; break;
			case 'a': fileflag = _O_APPEND; break;
			default:  URL_SET_LAST_ERR_VAL(EFAULT); return nullptr;
		}
		while(*++mode)
			switch(*mode) 
			{
				case ' ': break;
				case '+': fileflag |= _O_RDWR; break;
				case 'b': fileflag |= _O_BINARY; break;
				case 't': fileflag |= _O_TEXT; break;
				case 'c': break; case 'n': break;
				default: URL_SET_LAST_ERR_VAL(EFAULT); return nullptr;
			}

		int fd = _open_osfhandle((intptr_t)RemoteIp.hSocket, fileflag);
		if(fd == -1)
		{
			URL_SET_LAST_ERR_VAL(EFAULT);
			return nullptr;
		}
		File = _fdopen(fd, m);
#else
		File = fdopen(RemoteIp.hSocket, mode);
#endif
		if(File == nullptr)
			URL_SET_LAST_ERR;
		return File;
	}

	__QUERY_URL()
	{
		InitFields();
	}

	~__QUERY_URL()
	{
		EvntUninitFields();
		if(IsOpen)
		{
			ShutdownSendRecive();
			Close();
		}
	}

	/*
	Create socket for ReciveFrom and SendTo functions.
	*/
	bool OnlyCreate(int iSocktype = SOCK_DGRAM, int iProtocol = IPPROTO_UDP, int iFamily = AF_INET)
	{
		if(IsOpen)
		{
			ShutdownSendRecive();
			Close();
		}
		if((RemoteIp.hSocket = socket(iFamily, iSocktype, iProtocol)) == INVALID_SOCKET)
		{
			URL_SET_LAST_ERR;
			return false;
		}
		RemoteIp.ProtocolType = iProtocol;
	}



	/*
	Check event on this socket.
	@InEventFlags - POLL.. flags for check. For example POLLIN for check incoming data.
	@Timeout - Time for check. On default set 0 for only check without waiting.
	@return - -1 on error, on another case flag with event POLL.. . For example POLLHUP on disconnect.
	*/
	int CheckEvents(decltype(std::declval<pollfd>().events) InEventFlags, int Timeout = 0)
	{
		pollfd pfd;
		pfd.events = InEventFlags;
		pfd.revents = 0;
		pfd.fd = RemoteIp.hSocket;
		if(poll(&pfd, 1, Timeout) == -1)
		{
			URL_SET_LAST_ERR;
			return -1;
		}
		return pfd.revents;
	}

	/*
	Send file to remote host.
	{
		@InSocket - Soket buffer for send
		@InFileDescriptor - file descriptor for sendsanding data.
	}
	@Count - Size of sending data
	@Offset - Offsen data in file descriptor
	*/
	inline long long SendFile(__QUERY_URL& InSocket, size_t Count)
	{
	  return SendFile(InSocket.RemoteIp.hSocket, Count, 0);
	}

	virtual long long SendFile(TDESCR InFileDescriptor, size_t Count, off_t Offset = 0)
	{
#ifdef _WIN32
#pragma comment(lib, "Mswsock.lib")
		WSAOVERLAPPED Overlap = {0};
		DWORD ResFlag, WritedCount = 0;
		if((Overlap.hEvent = winsock::WSACreateEvent()) == NULL)
		{
			URL_SET_LAST_ERR_VAL(EFAULT);
			return -1;
		}
		Overlap.OffsetHigh = ((Overlap.Offset = Offset) >> 32);
		if(!TransmitFile(RemoteIp.hSocket, (HANDLE)InFileDescriptor, Count, 0, &Overlap, nullptr, 0))
		{
			if(winsock::WSAGetLastError() == ERROR_IO_PENDING)
			{
				if(winsock::WSAGetOverlappedResult(RemoteIp.hSocket, &Overlap, &WritedCount, !IsNonBlocked, &ResFlag))
				{
					winsock::WSACloseEvent(Overlap.hEvent);
					return WritedCount;
				}
			}
		}
		winsock::WSACloseEvent(Overlap.hEvent);
		URL_SET_LAST_ERR;
		return -1;
#elif defined(__FreeBSD__)
		off_t sbytes = 0;
		int r = sendfile(RemoteIp.hSocket, InFileDescriptor, Offset, Count, 0, &sbytes, 0);
		if (r == -1)
		{
			if(errno == EAGAIN)
				return (sbytes?sbytes:-1);
			URL_SET_LAST_ERR;
			return -1;
		}
		return Count;
#elif defined(__linux__)
		off_t o = Offset;
		long long done = 0;
		while (Count)
		{
			off_t todo = (Count > 0x7fffffff)? 0x7fffffff: Count;
			off_t i = sendfile(RemoteIp.hSocket, InFileDescriptor, &o, todo);
			if (i == todo) 
			{
				done += todo;
				Count -= todo;
				if (Count == 0) 
					return done;
				continue;
			} else if (i == -1) 
			{
				URL_SET_LAST_ERR;
				return -1;
			} else
				return done + i;
		}
		return 0;
#else
		if(Count == 0)
			return 0;
		if(Offset != 0)
			if(lseek(RemoteIp.hSocket, Offset, SEEK_SET))
			{
			   URL_SET_LAST_ERR;
			   return -1;
			}
		unsigned long long SizeBuf;
		int r, wr, w = 0;
		void* Buf;
#	ifdef SO_SNDBUF
		SizeBuf = SockOptions.SendSizeBuffer;
#	else
		SizeBuf = 0xffff;
#	endif
		SizeBuf = (SizeBuf < Count)?SizeBuf: Count;
		Buf = malloc(SizeBuf);
		if(Buf == nullptr)
		{
			URL_SET_LAST_ERR;
			return -1;
		}
		while(true)
		{
			if((r = read(InFileDescriptor, Buf, SizeBuf)) == -1)
			{
				if(w > 0)
					return w;
				URL_SET_LAST_ERR;
				free(Buf);
			   return -1;
			}
			if((wr = write(RemoteIp.hSocket, Buf, r)) == -1)
			{
				URL_SET_LAST_ERR;
				free(Buf);
				return -1;
			}
			if(r < SizeBuf)
			{
				free(Buf);
				return wr;
			}
			w += wr;
			SizeBuf = ((Count - w) < SizeBuf)?(Count - w): SizeBuf;
		}
		free(Buf);
		return 0;
#endif
	}
	
	/*
	Close socket descriptor.
	*/
	bool Close()
	{
		if(RemoteIp.hSocket == INVALID_SOCKET)
			return true;
		if(!EvntBeforeClose())
			return false;
		if(closesocket(RemoteIp.hSocket) != SOCKET_ERROR)
		{
			RemoteIp.hSocket = INVALID_SOCKET;
			return true;
		}	
		return false;
	}

	/*
	Shutdown session.
	*/
	inline bool ShutdownSend()
	{
		if(!EvntBeforeShutdown(SHUT_WR))
			return false;
		return shutdown(RemoteIp.hSocket, SHUT_WR) != SOCKET_ERROR; 
	}

	inline bool ShutdownRecive()
	{		
		if(!EvntBeforeShutdown(SHUT_RD))
			return false;
		return shutdown(RemoteIp.hSocket, SHUT_RD) != SOCKET_ERROR; 
	}

	inline bool ShutdownSendRecive()
	{
		if(!EvntBeforeShutdown(SHUT_RDWR))
			return false;
		return shutdown(RemoteIp.hSocket, SHUT_RDWR) != SOCKET_ERROR; 
	}

	/*
	  Send data via system call to descriptor.
	  @Buf - Sending data
	  @SizeBuf  - Count byte in @Buf

	  @return - on succesful, count writed bytes to socket, on error, -1 is returned. 
	*/
	int Write(const void * Buf, size_t SizeBuf)
	{
#ifdef _WIN32
		DWORD Written;
		OVERLAPPED Overlap = {0}, *ovlp = nullptr;
lblTryAgain:
		if(!WriteFile((HANDLE)RemoteIp.hSocket, Buf, SizeBuf, &Written, ovlp))
		{
			DWORD LastErr = GetLastError();
			if((LastErr == ERROR_INVALID_PARAMETER) && (ovlp == nullptr))
			{
				if(!IsNonBlocked)
					Overlap.hEvent = CreateEventW(nullptr, true, false, nullptr); 
				ovlp = &Overlap; 
				goto lblTryAgain;
			}else if(LastErr == ERROR_IO_PENDING)
			{
				if(Overlap.hEvent != NULL)
				{
					WaitForSingleObject(Overlap.hEvent, INFINITE);	
					if(GetOverlappedResult((HANDLE)RemoteIp.hSocket, ovlp, &Written, TRUE))
					{
						CloseHandle(Overlap.hEvent);
						return Written;
					}
				}else
				{
				   URL_SET_LAST_ERR_VAL(EWOULDBLOCK);
				   return -1;
				}
			}
			URL_SET_LAST_ERR_VAL(EFAULT);
			if(Overlap.hEvent != NULL)
				CloseHandle(Overlap.hEvent);
			return -1;
		}
		if(Overlap.hEvent != NULL)
			CloseHandle(Overlap.hEvent);
		return Written;
#else
		ssize_t Written = write(RemoteIp.hSocket, Buf, SizeBuf);
		if(Written == -1)
			URL_SET_LAST_ERR;
		return Written;
#endif
	}

	/*
	  Recive data via system call to descriptor.
	  @Buf - Buf for getted data
	  @SizeBuf  - Count byte in @Buf

	  @return - on succesful, count readed bytes from socket buffer, on error, -1 is returned. 
	*/
	int Read(void * Buf, size_t SizeBuf)
	{
#ifdef _WIN32
		DWORD Readed;
		OVERLAPPED Overlap = {0}, *ovlp = nullptr;
lblTryAgain:
		if(!ReadFile((HANDLE)RemoteIp.hSocket, Buf, SizeBuf, &Readed, &ovlp))
		{
			DWORD LastErr = GetLastError();
			if((LastErr == ERROR_INVALID_PARAMETER) && (ovlp == nullptr))
			{
				if(!IsNonBlocked)
					Overlap.hEvent = CreateEventW(nullptr, true, false, nullptr); 
				ovlp = &Overlap; 
				goto lblTryAgain;
			}else if(LastErr == ERROR_IO_PENDING)
			{
				if(Overlap.hEvent != NULL)
				{
					WaitForSingleObject(Overlap.hEvent, INFINITE);	
					if(GetOverlappedResult((HANDLE)RemoteIp.hSocket, ovlp, &Readed, TRUE))
					{
						CloseHandle(Overlap.hEvent);
						return Readed;
					}
				}else
				{
				   URL_SET_LAST_ERR_VAL(EWOULDBLOCK);
				   return -1;
				}
			}
			URL_SET_LAST_ERR_VAL(EFAULT);
			if(Overlap.hEvent != NULL)
				CloseHandle(Overlap.hEvent);
			return -1;
		}
		if(Overlap.hEvent != NULL)
			CloseHandle(Overlap.hEvent);
		return Readed;
#else
		ssize_t Readed = read(RemoteIp.hSocket, Buf, SizeBuf);
		if(Readed == -1)
			URL_SET_LAST_ERR;
		return Readed;
#endif
	}

	/*
	 Send via send socket function.
	 @QueryBuf - Sending data
	 @SizeBuf  - Count byte in @QueryBuf
	 @Flags   - Additional flags: MSG_DONTROUTE or MSG_OOB

	 @return - on succesful, count writed bytes to socket, on error, -1 is returned.
	*/
	virtual int Send(const void * QueryBuf, size_t SizeBuf, int Flags = 0)
	{
		int WritenSize;
		if((WritenSize = send(RemoteIp.hSocket, (const char*)QueryBuf, SizeBuf, Flags)) == SOCKET_ERROR)
			URL_SET_LAST_ERR;
		return WritenSize;
	}

	inline int Send(const std::basic_string<char> & InQuery)
	{
		return Send((const void*)InQuery.c_str(), InQuery.length());
	}

	/*
	 Recive via recv socket function.
	 @Buf - Buf for getted data
	 @SizeBuf  - Size @Buf in bytes
	 @Flags   - Additional flags: MSG_PEEK or MSG_OOB or MSG_WAITALL

	 @return - on succesful, count readed bytes from socket buffer, on error, -1 is returned.
	*/
	virtual int Recive(void * Buf, size_t SizeBuf, int Flags = 0)
	{
		int ReadedSize;
		if((ReadedSize = recv(RemoteIp.hSocket, (char*)Buf, SizeBuf, Flags)) == SOCKET_ERROR)
			URL_SET_LAST_ERR;
		return ReadedSize;
	}

	virtual int Recive
	(
		std::basic_string<char>& StrBuf, 
		std::basic_string<char>::size_type MaxLen = std::numeric_limits<std::basic_string<char>::size_type>::max(), 
		int Flags = 0
	)
	{
		char* Buf;
		//Ignore MSG_PEEK
		Flags &= ~MSG_PEEK;
		unsigned CurSize = 0, CountBytesInBuff, ReadedSize = 0;
		CountBytesInBuff = CountPandingData + 2;
		if(CountBytesInBuff < 50)
			CountBytesInBuff = 50;
		StrBuf.resize(CountBytesInBuff);
		Buf = (char*)StrBuf.c_str();
		while(true)
		{				
			if(CurSize >= MaxLen)
					break;
			if(CountBytesInBuff > (MaxLen - CurSize))
				CountBytesInBuff = MaxLen - CurSize; 
			ReadedSize = Recive(Buf, CountBytesInBuff, Flags);
			if(ReadedSize == SOCKET_ERROR)
			{
				URL_SET_LAST_ERR;
				return -1;
			}else if(ReadedSize == 0)
				break;
			else
			{
				CurSize += ReadedSize;
				CountBytesInBuff = CountPandingData;
				if(CountBytesInBuff == 0)
					CountBytesInBuff = 50;
				StrBuf.resize(CurSize + CountBytesInBuff + 2);
				Buf = (char*)StrBuf.c_str() + CurSize;
			}
		}
		*Buf = '\0';
		return CurSize;
	}


	/*
	@Flags:
	MSG_PEEK - Peeks at the incoming data. 
	MSG_OOB  - Processes Out Of Band (OOB) data.
	*/
	virtual int ReciveFrom(void * Buffer, size_t LenBuff, SOCKET_ADDR& AddressSender, int Flags = 0)
	{ 
		int Len = sizeof(SOCKET_ADDR);
		int CountRecived;
		if((CountRecived = recvfrom(RemoteIp.hSocket, (char*)Buffer, LenBuff, Flags, AddressSender, &Len)) == SOCKET_ERROR)
			URL_SET_LAST_ERR;
		return CountRecived;
	}

	int ReciveFrom(void * Buffer, size_t LenBuff, typename ADDRESS_INFO::ADDRESS_INTERATOR& AddressSender, int Flags = 0)
	{
		return ReciveFrom(Buffer, LenBuff, AddressSender.GetSocketAddr(), Flags);
	}

	/*
	@Flags:
	MSG_PEEK - Peeks at the incoming data. 
	MSG_OOB  - Processes Out Of Band (OOB) data.
	MSG_DONTROUTE - Not use routing.
	*/
	virtual int SendTo(const void * Buffer, size_t LenBuff, SOCKET_ADDR& AddressReciver, int Flags = 0)
	{ 
		int CountSending;
		if((CountSending = sendto(RemoteIp.hSocket, (const char*)Buffer, LenBuff, Flags, AddressReciver, sizeof(SOCKET_ADDR))) == SOCKET_ERROR)
			URL_SET_LAST_ERR;
		return CountSending;
	}

	inline int SendTo(const void * Buffer, size_t LenBuff, typename ADDRESS_INFO::ADDRESS_INTERATOR& AddressSender, int Flags = 0)
	{
		return SendTo(Buffer, LenBuff, AddressSender.GetSocketAddr(), Flags);
	}

	/*
	 Send and receive immediately.
	 @SendBuf - Data for send
	 @SizeSendBuf  - Size @SendBuf in bytes
	 @ReciveBuf   - Buffer for recive data
	 @SizeReciveBuf  - Size @ReciveBuf in bytes

	 @return - on succesful, count readed bytes from socket buffer, on error, -1 is returned.
	*/

	inline int SendAndRecive(const void* SendBuf, size_t SizeSendBuf, void* ReciveBuf, unsigned SizeReciveBuf)
	{
		if(!Send(SendBuf, SizeSendBuf))
			return -1;
		return Recive(ReciveBuf, SizeReciveBuf);
	}

	inline int SendAndRecive(std::basic_string<char>& strQuery, void* ReciveBuf, size_t SizeBuf)
	{
		if(!Send((void*)strQuery.c_str(), strQuery.length()))
			return -1;
		return Recive(ReciveBuf, SizeBuf);
	}

	inline int SendAndRecive(const void* SendBuf, unsigned SizeSendBuf, std::basic_string<char>& Result)
	{
		if(!Send(SendBuf, SizeSendBuf))
			return -1;
		return Recive(Result);
	}

	inline int SendAndRecive(const char* SendStr, std::basic_string<char>& Result)
	{
		if(!Send(SendStr, strlen(SendStr)))
			return -1;
		return Recive(Result);
	}

	inline int SendAndRecive(const char* SendStr, void* ReciveBuf, unsigned SizeBuf)
	{
		if(!Send(SendStr, strlen(SendStr)))
			return -1;
		return Recive(ReciveBuf, SizeBuf);
	}

	inline int SendAndRecive(std::basic_string<char>& strQuery, std::basic_string<char>& Result)
	{
		if(!Send((void*)strQuery.c_str(), strQuery.length()))
			return -1;
		return Recive(Result);
	}

};

#undef socket

typedef __QUERY_URL<true> QUERY_URL;

#endif // QUERYURL_H_INCLUDED
