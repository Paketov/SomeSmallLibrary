#ifndef __QUERYURL_H_HAS_INCLUDED__
#define __QUERYURL_H_HAS_INCLUDED__

/*
	 ExQueryUrl
	 Paketov
	 2015.

	 Library for optimize work with socket functions.
	 For Windows, Linux and another unix like systems.
*/


#include "ExDynamicBuf.h"
#include "ExHashTable.h"
#include <atomic>
#include <errno.h>

//#define HAVE_KEVENT___


#ifdef _MSC_VER

//#define WIN_PLATFORM
#	include <winsock.h>

#	define WIN_PLATFORM
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

	int GetLastErrSocket();
};

using winsock::addrinfo;
using winsock::ip_mreq_source;
typedef UINT32 socklen_t;

#	define gai_strerror gai_strerrorA
#	define sockaddr  winsock::sockaddr
#	define sockaddr_in  winsock::sockaddr_in
#	define sockaddr_in6  winsock::sockaddr_in6
#	define sockaddr_storage winsock::sockaddr_storage
#	define sockaddr_dl winsock::sockaddr_dl
//Should disable overlapped, when you want use OpenAsFile
//#	define socket(ProtFamily, SockType, Protocol) winsock::WSASocketA((ProtFamily), (SockType), (Protocol), NULL, 0, 0)

#	define pollfd winsock::pollfd
#	define ipv6_mreq winsock::ipv6_mreq

#	define LAST_ERR_SOCKET winsock::GetLastErrSocket()

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

#	define ADDITIONAL_FIELDS  bool IsNonBlocked;

#	undef max

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
#	if defined(__linux__)
#		include <sys/sendfile.h>
#   elif defined(__FreeBSD__)
#		include <sys/uio.h>
#	endif
#	define LAST_ERR_SOCKET errno 
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

#define __QUERY_URL_PROPERTY_THIS ((QUERY_URL*)((char*)this - ((unsigned)&((QUERY_URL*)0)->LastError)))

#define URL_SET_LAST_ERR {((QUERY_URL*)this)->SetLastErr(LAST_ERR_SOCKET);}
#define URL_SET_LAST_ERR_VAL(Val) {((QUERY_URL*)this)->RemoteIp.iError = (Val);}
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
	inline operator RetType() const													\
	{																				\
		RetType v; if(GetOption(SoketNum, Level, Option, v) != 0)					\
					URL_SET_LAST_ERR_IN_PROPERTY;									\
		return v;																	\
	}

#define DEF_SET_OPTION_PROPERTY(SetType, SoketNum, Level, Option)					\
	inline SetType operator=(SetType New){											\
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
	inline SetType operator=(SetType v)const {URL_SET_LAST_ERR_IN_PROPERTY_VAL(EOPNOTSUPP); return v;} \
	inline operator GetType() const {URL_SET_LAST_ERR_IN_PROPERTY_VAL(EOPNOTSUPP); return GetType();}	\
	} Name

#define DEF_SOCKET_EMPTY_OPTION_GET(Name, GetType)									\
	class{_QUERY_URL_FIELDS1_; public:												\
	static const bool IsHave = false;												\
	static const bool IsSet = false;												\
	static const bool IsGet = false;												\
	inline operator GetType() const {URL_SET_LAST_ERR_IN_PROPERTY_VAL(EOPNOTSUPP); return GetType();}	\
	} Name

#define DEF_SOCKET_EMPTY_OPTION_SET(Name, SetType)									\
	class{ _QUERY_URL_FIELDS1_; public:												\
	static const bool IsHave = false;												\
	static const bool IsSet = false;												\
	static const bool IsGet = false;												\
	inline SetType operator=(SetType v) const {URL_SET_LAST_ERR_IN_PROPERTY_VAL(EOPNOTSUPP); return v;}\
	} Name



class QUERY_URL
{
public:
	typedef decltype(std::declval<sockaddr_in>().sin_port) TPORT;
	/*Type of descriptor*/
	typedef decltype(socket(std::variant_arg(), std::variant_arg(), std::variant_arg())) TDESCR;

	class SOCKET_ADDR
	{
#define SOCKET_ADDR_FIELDS				\
		union{							\
		sockaddr			Addr;		\
		sockaddr_in			AddrInet;	\
		sockaddr_in6		AddrInet6;	\
		sockaddr_storage	AddrStorage;\
		}
		class _IP
		{
			friend QUERY_URL;
			SOCKET_ADDR_FIELDS;
		public:
			template<typename Type>
			operator Type*() const
			{
				switch(Addr.sa_family)
				{
				case AF_INET: return (Type*)&AddrInet.sin_addr;
				case AF_INET6: return (Type*)&AddrInet6.sin6_addr;
				}
				return nullptr;
			}
			char* ToString(char * Dest, size_t Len = 0xffff) const;
			std::basic_string<char> ToString() const;
			const char* FromString(const char* AddrStr);
			const std::basic_string<char>& FromString(const std::basic_string<char>& AddrStr);
		};

		class ___PORT
		{ 
			class _READABLE
			{
				friend ___PORT;
				SOCKET_ADDR_FIELDS;
			public:
				operator TPORT() const;
			};
		public:
			_READABLE Readable;
			operator TPORT() const;
			TPORT operator=(TPORT Prt);
			char* ToString(char* DestBuf, size_t LenBuf) const;
			std::basic_string<char> ToString() const;
		};

	public:
		template<typename RetType>
		inline operator RetType*() { return (RetType*)this; }
		inline sockaddr& operator =(const sockaddr& New){
			Len.Addr = New;
			return Len.Addr;
		}
		inline sockaddr_in& operator =(const sockaddr_in& New){
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
			public:// Address family.
				inline operator int() const { return Addr.sa_family; }
				inline int operator =(int Fam) { return Addr.sa_family = Fam; }
			} ProtocolFamily;
			class
			{
				friend SOCKET_ADDR;
				SOCKET_ADDR_FIELDS;
			public:	
				inline operator socklen_t() const
				{
					switch(Addr.sa_family)
					{
					case AF_INET: return sizeof(AddrInet);
					case AF_INET6: return sizeof(AddrInet6);
					}
					return sizeof(Addr);
				}
			} Len;
			_IP Ip;
			___PORT  Port;
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
		}

		friend QUERY_URL;
		inline operator addrinfo*() const { return HostName.ai; }
		void InitFields();
	public:
		class ADDRESS_INTERATOR
		{
			friend QUERY_URL;
			inline operator addrinfo*() const { return Ip.ca; }
			inline SOCKET_ADDR& GetSocketAddr() { return (SOCKET_ADDR&)Ip.ca; }

		public:
			inline ADDRESS_INTERATOR(addrinfo * CurAddr) { Ip.ca = CurAddr; }
			union
			{
				class { addrinfo* ca; public:
					inline operator TPORT() const {
						if(ca == nullptr) return 0; return ((SOCKET_ADDR*)ca->ai_addr)->Port;
					}
					inline std::basic_string<char> ToString() const{
						if(ca == nullptr) return "";
						return ((SOCKET_ADDR*)ca->ai_addr)->Port.ToString();
					}
					inline char* ToString(char * Dest, size_t Len = 0xffff) const{
						if(ca == nullptr) return nullptr;
						return ((SOCKET_ADDR*)ca->ai_addr)->Port.ToString(Dest, Len);
					}
				} Port;
				class
				{
					friend ADDRESS_INTERATOR;
					addrinfo* ca;
				public:
					template<typename Type>
					inline operator Type*() const {
						if(ca == nullptr) return nullptr; return ((SOCKET_ADDR*)ca->ai_addr)->Ip;
					}
					inline char* ToString(char * Dest, size_t Len = 0xffff) const {
						if(ca == nullptr) return nullptr; return ((SOCKET_ADDR*)ca->ai_addr)->Ip.ToString(Dest, Len); 
					}
					inline std::basic_string<char> ToString() const { if(ca == nullptr) return "";  return ((SOCKET_ADDR*)ca->ai_addr)->Ip.ToString(); }
				} Ip;

				class{ addrinfo* ca; public:
					inline operator int() const { if(ca == nullptr) return -1; return ((SOCKET_ADDR*)ca->ai_addr)->ProtocolFamily; }
				} ProtocolFamily;

				class{ addrinfo* ca; public: //SOCK_xx
					inline operator int() const { if(ca == nullptr) return -1; return ca->ai_socktype; }
				} TypeSocket;

				class{ addrinfo* ca; public: //IPPROTO_xx
					inline operator int() const { if(ca == nullptr) return -1; return ca->ai_protocol; }
				} Protocol;

				class{ addrinfo* ca; public:// AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST
					inline operator int() const { if(ca == nullptr) return -1; return ca->ai_flags; }
				} Flags;

				class{ addrinfo* ca; public:// Canonical name for nodename
					inline operator const char*() const { if(ca == nullptr) return nullptr; return ca->ai_canonname; }
				} CanonicalName;
			};
		};

	private:
		class ADDRESSES{
			class _COUNT{
				friend ADDRESSES;
				ADDRESS_INFO_FIELDS;
			public:
				operator int();
			};
		public:
			_COUNT Count;
			ADDRESS_INTERATOR operator[](unsigned Index);
		};

		class _HOST_NAME
		{
			friend ADDRESS_INFO;
			ADDRESS_INFO_FIELDS;
		public:
			inline operator const char*() { return HostName->c_str(); }
			const char* operator= (const char* New);
			std::basic_string<char>& operator= (std::basic_string<char>& New);
		};

		class _PORT_NAME
		{
			ADDRESS_INFO_FIELDS;
		public:
			inline operator const char*() { return PortName->c_str(); }
			const char* operator= (const char* New);
			std::basic_string<char>& operator= (std::basic_string<char>& New);
		};
	public:
		union
		{
			_HOST_NAME HostName;
			_PORT_NAME PortName;
			ADDRESSES  Addresses;
		};
		ADDRESS_INFO(const char* FullAddress);
		ADDRESS_INFO(const char* Host, const char* Port);
		ADDRESS_INFO();
		~ADDRESS_INFO();
		bool Update(int iSocktype = SOCK_STREAM, int iProtocol = IPPROTO_TCP, int iFamily = AF_UNSPEC, int iFlags = 0);
	};

	template<bool IsUseQuerUrl = true>
	class CHECK_EVENTS_SEL
	{
		struct ELEM_DESCR
		{
			unsigned char f;
			TDESCR        d;
			inline TDESCR GetDescriptor() { return d; }
			inline void SetDescriptor(TDESCR nd) { d = nd; }
			inline QUERY_URL* GetSock() { return nullptr; }
			inline void SetSock(QUERY_URL* ns) { d = ns->RemoteIp.hSocket; }
		};

		struct ELEM_SOCK
		{
			unsigned char f;
			QUERY_URL*   s;
			inline TDESCR GetDescriptor() { return s->RemoteIp.hSocket; }
			inline void SetDescriptor(TDESCR nd) { s->RemoteIp.hSocket = nd; }			
			inline QUERY_URL* GetSock() { return s; }
			inline void SetSock(QUERY_URL* ns) { s = ns; }
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

				class {
					__INTERATOR_FIELDS__;
				public:
					inline operator TDESCR() const { return This->Count.e[Index].GetDescriptor(); }
				} Descriptor;

				class
				{
					__INTERATOR_FIELDS__;
				public:
					inline operator QUERY_URL*()
					{
						if(This->Count.e == nullptr)
							return nullptr;
						return This->Count.e[Index].GetSock();
					}

					inline QUERY_URL* operator->()
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
				inline operator unsigned() { return CountSockets; }
			} Count;
		};

		int AddConnection(QUERY_URL& Sock)
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

	template<bool IsUseAssocData = true, typename AssocDataType = std::empty_type>
	class CHECK_EVENTS_POL
	{

	public:

		class INTERATOR
		{
#	define __INTERATOR_FIELDS__ struct{CHECK_EVENTS_POL* This; unsigned Index;}
			friend CHECK_EVENTS_POL;

			inline INTERATOR(CHECK_EVENTS_POL* w, unsigned i)
			{
				IsFollowWrite.This = w;
				IsFollowWrite.Index = i;
			}
		public:

			inline INTERATOR()
			{
				IsFollowWrite.This = nullptr;
				IsFollowWrite.Index = 0;
			}

			union
			{
#define __INTERATOR_PROPERTY__e(Name, Opt)												\
				class{friend CHECK_EVENTS_POL;friend INTERATOR;__INTERATOR_FIELDS__;		\
				public:																	\
				inline operator bool() const {return This->Count.pfd[Index].events & Opt;}\
				inline bool operator=(bool v){										\
				This->Count.pfd[Index].events |= ((v)?Opt:0);return v;}				\
				} Name
#define __INTERATOR_PROPERTY__r(Name, Opt)												\
				class{friend CHECK_EVENTS_POL;friend INTERATOR;__INTERATOR_FIELDS__;		\
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

				class{
					__INTERATOR_FIELDS__;
				public:
					inline operator short() const { return This->Count.pfd[Index].revents; }
				} ReturnedEvents;

				class {
					__INTERATOR_FIELDS__;
				public:
					inline operator short() const { return This->Count.pfd[Index].events; }

					inline short operator=(short v)
					{
						return This->Count.pfd[Index].events = v;
					}
				} RequestedEvents;

				class{
					__INTERATOR_FIELDS__;
				public:
					inline operator TDESCR() const { return This->Count.pfd[Index].fd; }	   
				} Descriptor;

				class {
					__INTERATOR_FIELDS__;
				public:
					inline operator AssocDataType() { return This->Count.data[Index]; }

					inline AssocDataType operator=(AssocDataType NewVal)
					{
						This->Count.data[Index] = NewVal;
						return This->Count.data[Index];
					}
				} Data;

				class{
					__INTERATOR_FIELDS__;
				public:
					inline operator bool() const { return This == nullptr; }
				} IsEmpty;

				class{
					__INTERATOR_FIELDS__;
				public:
					inline operator unsigned() const { return Index; }
				} Index;
			};

			inline void ClearReturnedEvents() { IsFollowWrite.This->Count.pfd[IsFollowWrite.Index].revents = 0; }

			void Remove()
			{	
				if(IsFollowWrite.This == nullptr)
					return;
				IsFollowWrite.This->Count.pfd.RemoveSubstituting(IsFollowWrite.Index);
				if(IsUseAssocData)
					IsFollowWrite.This->Count.data.RemoveSubstituting(IsFollowWrite.Index);
				if(IsFollowWrite.Index <= 0)
					IsFollowWrite.This == nullptr;
				else
					IsFollowWrite.Index--;
			}

			int Check(unsigned WaitTime = 0)
			{
				if(IsFollowWrite.This == nullptr)
					return 0;
				return poll(&IsFollowWrite.This->Count.pfd[IsFollowWrite.Index], 1, WaitTime);
			}
		};
		friend INTERATOR;
		CHECK_EVENTS_POL(){}

		class{
			friend CHECK_EVENTS_POL;
			friend INTERATOR;
			DYNAMIC_BUF<pollfd>			pfd;
			DYNAMIC_BUF<AssocDataType>	data;
		public:
			inline operator size_t() const { return pfd.Count; }
		} Count;

		int AddConnection(QUERY_URL* Sock, long lNetworkEvents = POLLIN, AssocDataType AssocData = AssocDataType())
		{
			return AddConnection(Sock->RemoteIp.hSocket, lNetworkEvents, AssocData);
		}

		int AddConnection(TDESCR SocketDescriptor, long lNetworkEvents = POLLIN, AssocDataType AssocData = AssocDataType())
		{
			pollfd& pfd = Count.pfd.Append();
			pfd.fd = SocketDescriptor;
			pfd.revents = 0;
			pfd.events = lNetworkEvents;
			if(IsUseAssocData)
				Count.data.Append() = AssocData;
			return Count - 1;
		}

		INTERATOR EnumWhoHasEvents()
		{
			for(size_t i = 0, m = Count; i < m; i++)
				if(Count.pfd[i].revents != 0)
					return INTERATOR(this, i);
			return INTERATOR();
		}

		INTERATOR EnumWhoHasEvents(INTERATOR& Prev)
		{
			for(size_t i = Prev.IsFollowWrite.Index + 1, m = Count; i < m; i++)
				if(Count.pfd[i].revents != 0)
					return INTERATOR(this, i);
			return INTERATOR();
		}

		inline INTERATOR operator[](unsigned Index)
		{
			if(Index >= Count)
				throw "CHECK_EVENTS_POL::operator[] : Out of bound";
			return INTERATOR(this, Index);
		}

		int Check(unsigned WaitTime = 0)
		{
			if(Count == 0)
				return 0;
			return poll(Count.pfd.BeginBuf, Count, WaitTime);
		}
	};

	template<bool IsUseAssocData = true>
	class CHECK_EVENTS
	{
		typedef decltype(socket(std::variant_arg(), std::variant_arg(), std::variant_arg())) TDESCR;

#ifdef WIN_PLATFORM	
	public:
		enum
		{
			WRITE = FD_WRITE | FD_CONNECT,	/*Ready for write*/
			READ = FD_ACCEPT | FD_READ,		/*Ready for read*/
			DISCONNECT = FD_CLOSE			/*Connection disconnected*/
		};
	private:
		struct EVENT_ELEMENT { TDESCR fd; void* Data; };
		DYNAMIC_BUF<WSAEVENT>		Events;
		DYNAMIC_BUF<EVENT_ELEMENT>	Data;
#elif defined(HAVE_KEVENT___)


#define EV_ADD		0x0001		/* add event to kq (implies enable) */
#define EV_DELETE	0x0002		/* delete event from kq */
#define EV_ENABLE	0x0004		/* enable event */
#define EV_DISABLE	0x0008		/* disable event (not reported) */
#define EV_RECEIPT	0x0040		/* force EV_ERROR on success, data == 0 */
		struct kevent {
			uintptr_t	ident;		/* identifier for this event */
			int16_t		filter;		/* filter for event */
			uint16_t	flags;		/* general flags */
			uint32_t	fflags;		/* filter-specific flags */
			intptr_t	data;		/* filter-specific data */
			void		*udata;		/* opaque user data identifier */
		};
		int kqueue(void){ return 0; }
		int kevent(int kq, const struct kevent *changelist, int nchanges, struct kevent *eventlist, int nevents,const struct timespec *timeout){ return 0; }

		struct EVENT_ELEMENT
		{
			TDESCR	 fd;
			void*	 Data;
		};

		int									kq;
		DYNAMIC_BUF<EVENT_ELEMENT>			Data;
		DYNAMIC_BUF<struct kevent>			REvents;
		int									CountREvents;
#elif defined(HAVE_EPOLL___)			

#endif

		std::atomic<bool>				SetEventLocker;
		bool IsHaveSignal;
		bool IsSignalSended;

	public:

		CHECK_EVENTS()
		{
			IsHaveSignal = IsSignalSended = false;
#if defined(HAVE_KEVENT___)
			kq = kqueue();
			CountREvents = 0;
#elif defined(HAVE_EPOLL___)			

#endif
		}

		~CHECK_EVENTS()
		{
#if defined(WIN_PLATFORM)
			for(size_t i = 0, m = Events.Count; i < m; i++)
				winsock::WSACloseEvent(Events[i]);
#elif defined(HAVE_KEVENT___)
			close(kq);
#endif
		}

		inline int AddConnection(QUERY_URL* Sock, long lNetworkEvents = POLLIN, void* AssocData = nullptr)
		{
			return AddConnection(Sock->RemoteIp.hSocket, lNetworkEvents, AssocData);
		}

		int AddConnection(TDESCR SocketDescriptor, long lNetworkEvents = POLLIN, void* AssocData = nullptr)
		{
#if defined(WIN_PLATFORM)
			HANDLE Event = CreateEventW(nullptr, TRUE, FALSE, nullptr);
			if(Event == WSA_INVALID_EVENT)
				return -1;
			if(winsock::WSAEventSelect(SocketDescriptor, Event, lNetworkEvents) ==  SOCKET_ERROR)
			{
				CloseHandle(Event);
				return -1;
			}
			Events.Append() = Event;
			auto& r = Data.Append();
			r.fd = SocketDescriptor;
			r.Data = AssocData;
#elif defined(HAVE_KEVENT___)
			REvents.Append();
			auto& elem = Data.Append();
			elem.Data = AssocData;
			elem.fd = SocketDescriptor;
			struct kevent ke;
			EV_SET(&ke, SocketDescriptor, lNetworkEvents, EV_ADD | EV_ENABLE, 0, 0, AssocData);
			if(kevent(kq, &ke, 1, nullptr, 0, nullptr) == -1)
				return -1;
#elif defined(HAVE_EPOLL___)
#endif
			return Events.Count - 1;
		}

		int EnumWhoHasEvents(long* REvents = std::make_default_pointer())
		{
#if defined(WIN_PLATFORM)
			winsock::WSANETWORKEVENTS e;
			for(size_t i = GetBeginIndex(), m = Data.Count; i < m; i++)
			{
				winsock::WSAEnumNetworkEvents(Data[i].fd, Events[i], &e);
				if(e.lNetworkEvents != 0)
				{
					*REvents = e.lNetworkEvents;
					return i;
				}
			}
#elif defined(HAVE_KEVENT___)
			for(size_t i = 0, m = CountREvents; i < m; i++)
			{
				if(REvents[i].filter != 0)
				{
					*REvents = REvents[i].filter;
					return i;
				}
			}
#elif defined(HAVE_EPOLL___)
#endif
			return -1;
		}

		int EnumWhoHasEvents(int Prev, long* REvents = std::make_default_pointer())
		{
#if defined(WIN_PLATFORM)
			winsock::WSANETWORKEVENTS e;
			for(size_t i = Prev + 1, m = Data.Count; i < m; i++)
			{
				winsock::WSAEnumNetworkEvents(Data[i].fd, Events[i], &e);
				if(e.lNetworkEvents != 0)
				{
					*REvents = e.lNetworkEvents;
					return i;
				}
			}
#elif defined(HAVE_KEVENT___)
			for(size_t i = Prev, m = CountREvents; i < m; i++)
			{
				if(REvents[i].filter != 0)
				{
					*REvents = REvents[i].filter;
					return i;
				}
			}
#elif defined(HAVE_EPOLL___)
#endif
			return -1;
		}

		inline TDESCR DescriptorByEnumIndex(int Index) 
		{ 
#if defined(WIN_PLATFORM)
			return Data[Index].fd;
#elif defined(HAVE_KEVENT___)
			return REvents[Index].ident;
#elif defined(HAVE_EPOLL___)

#endif
		}

		inline void*& DataByEnumIndex(int Index) 
		{ 
#if defined(WIN_PLATFORM)
			return Data[Index].Data;
#elif defined(HAVE_KEVENT___)
			return REvents[Index].udata;
#elif defined(HAVE_EPOLL___)

#endif
		}

		inline int GetBeginIndex() const
		{
#if defined(WIN_PLATFORM)
			return (IsHaveSignal)?1:0;
#elif defined(HAVE_KEVENT___)
			return REvents[Index].ident;
#elif defined(HAVE_EPOLL___)

#endif
		}

		inline int GetEndIndex() const
		{
#if defined(WIN_PLATFORM)
			return Data.Count;
#elif defined(HAVE_KEVENT___)
			return REvents[Index].ident;
#elif defined(HAVE_EPOLL___)

#endif
		}

		int RemoveByEnumIndex(int Index) 
		{ 
#if defined(WIN_PLATFORM)
			CloseHandle(Events[Index]);	
			Events.RemoveSubstituting(Index);
			Data.RemoveSubstituting(Index);
			if((Index <= 1) && IsHaveSignal)
				return -1;
			return Index - 1;
#elif defined(HAVE_KEVENT___)	
			auto& elem = REvents[Index];
			elem.flags = EV_DELETE;
			kevent(kq, &elem, 1, nullptr, 0, nullptr);
			Data.RemoveSubstituting(Index);
			REvents.RemoveSubstituting(Index);
#elif defined(HAVE_EPOLL___)
#endif
		}

		inline TDESCR DescriptorByIndex(int Index) 
		{ 
#if defined(WIN_PLATFORM) || defined(HAVE_KEVENT___)
			return Data[Index].fd;
#elif defined(HAVE_EPOLL___)

#endif
		}

		inline void*& DataByIndex(int Index) { return Data[Index].Data; }

		inline size_t Count() const
		{	
			return GetEndIndex() - GetBeginIndex();
		}

		void Remove(int Index) 
		{ 
#if defined(WIN_PLATFORM)
			CloseHandle(Events[Index]);	
			Events.RemoveSubstituting(Index);
			Data.RemoveSubstituting(Index);
#elif defined(HAVE_KEVENT___)	
			auto& elem = Data[Index];
			struct kevent ke;
			EV_SET(&ke, elem.fd, 0, EV_DELETE, 0, 0, elem.Data);
			kevent(kq, &ke, 1, nullptr, 0, nullptr);
			Data.RemoveSubstituting(Index);
			REvents.Pop();
#elif defined(HAVE_EPOLL___)
#endif
		}

		/*Signals thread save*/
		void EnableSignal()
		{
			for(bool v = false; !SetEventLocker.compare_exchange_strong(v, true); v = false);
			if(!IsHaveSignal)
			{
#if defined(WIN_PLATFORM)
				Events.InsertInPositionSubstituting(0) = CreateEventW(nullptr, TRUE, FALSE, nullptr);
				auto& r = Data.InsertInPositionSubstituting(0);
				r.fd = 0;
#elif defined(HAVE_KEVENT___)	
#elif defined(HAVE_EPOLL___)
#endif
				IsHaveSignal = true;
			}
			SetEventLocker = false;
		}

		void DisableSignal()
		{
			for(bool v = false; !SetEventLocker.compare_exchange_strong(v, true); v = false);
			if(IsHaveSignal)
			{
#if defined(WIN_PLATFORM)
				CloseHandle(Events[0]);
				Events.RemoveSubstituting(0);
				Data.RemoveSubstituting(0);
#elif defined(HAVE_KEVENT___)	
#elif defined(HAVE_EPOLL___)
#endif	
				IsSignalSended = IsHaveSignal = false;
			}
			SetEventLocker = false;
		}

		bool IsSignalEnabled()
		{
			return IsHaveSignal;
		}

		void SetSignal()
		{
			for(bool v = false; !SetEventLocker.compare_exchange_strong(v, true); v = false);
			if(IsHaveSignal)
			{			
				IsSignalSended = true;
#if defined(WIN_PLATFORM)
				SetEvent(Events[0]);	
#elif defined(HAVE_KEVENT___)	
#elif defined(HAVE_EPOLL___)
#endif	
			}
			SetEventLocker = false;
		}

		void ResetSignal()
		{

			for(bool v = false; !SetEventLocker.compare_exchange_strong(v, true); v = false);
			if(IsHaveSignal)
			{	
				IsSignalSended = false;
#if defined(WIN_PLATFORM)
				ResetEvent(Events[0]);
#elif defined(HAVE_KEVENT___)	
#elif defined(HAVE_EPOLL___)
#endif	
			}
			SetEventLocker = false;
		}

		inline bool IsSignalSet() 
		{
			return IsSignalSended; 
		}

		inline bool CheckSignalAndReset() 
		{ 
			for(bool v = false; !SetEventLocker.compare_exchange_strong(v, true); v = false);
			bool r;
			if(r = IsSignalSended)
			{
				IsSignalSended = false;
#if defined(WIN_PLATFORM)
				ResetEvent(Events[0]);
#elif defined(HAVE_KEVENT___)	
#elif defined(HAVE_EPOLL___)
#endif	
			}
			SetEventLocker = false;
			return r; 
		}
		/*
		*/
		int Check(unsigned WaitTime = 0, bool* IsEndTimeOut = std::make_default_pointer())
		{			
			if(Events.Count == 0)
				return true;
#if defined(WIN_PLATFORM)
			switch(WaitForMultipleObjectsEx(Data.Count, Events.BeginBuf, FALSE, WaitTime, FALSE))
			{
			case WSA_WAIT_FAILED:
				return -1;
			case WAIT_TIMEOUT:
				return  0;
			}
#elif defined(HAVE_KEVENT___)
			timespec ts;
			ts.tv_sec = WaitTime / 1000;
			ts.tv_nsec = (WaitTime - ts.tv_sec * 1000) * 1000000;
			if((CountREvents = kevent(kq, nullptr, 0, REvents.BeginBuf, REvents.Count, &ts)) == -1)
				return -1;
#elif defined(HAVE_EPOLL___)
#endif
			return 1;
		}
	};

	struct IPv6ADDR
	{
		union {
			uint16_t Addr[8];
			struct {
				uint64_t		AsNumber1;
				uint64_t		AsNumber2;
			};
		};

		inline bool operator==(const IPv6ADDR& Another) const { return memcmp(Addr, Another.Addr, sizeof(Addr)) == 0; }
		inline bool operator!=(const IPv6ADDR& Another) const { return !operator==(Another); }
		inline IPv6ADDR
		(
			uint16_t Adr1, uint16_t Adr2, uint16_t Adr3 = 0, uint16_t Adr4 = 0, 
			uint16_t Adr5 = 0, uint16_t Adr6 = 0, uint16_t Adr7 = 0, uint16_t Adr8 = 0
		)
		{
			Addr[0] = htons(Adr1); Addr[1] = htons(Adr2);
			Addr[2] = htons(Adr3); Addr[3] = htons(Adr4);
			Addr[4] = htons(Adr5); Addr[5] = htons(Adr6);
			Addr[6] = htons(Adr7); Addr[7] = htons(Adr8);
		}
		inline IPv6ADDR() {}
		inline void SetByIndex(size_t i, uint16_t v) { Addr[i] = htons(v); }
		inline uint16_t GetByIndex(size_t i) const { return ntohs(Addr[i]); }
		inline int FromString(const std::basic_string<char>& Str) { return FromString(Str.c_str()); }
		int FromString(const char* BufSource);
		std::basic_string<char> ToString() const;
		char* ToString(char* Dest, size_t Len) const;
	};

	struct IPv4ADDR
	{
		union
		{
			uint8_t				Addr[4];
			uint32_t			AsNumber;
		};

		inline IPv4ADDR(uint8_t Adr1, uint8_t Adr2, uint8_t Adr3 = 0, uint8_t Adr4 = 0)
		{
			Addr[0] = Adr1; Addr[1] = Adr2; Addr[2] = Adr3; Addr[3] = Adr4;
		}
		inline IPv4ADDR(uint32_t Adr) { AsNumber = Adr; }
		inline IPv4ADDR() {}
		inline void SetByIndex(size_t i, uint8_t v) { Addr[i] = v; }
		inline uint8_t GetByIndex(size_t i) const { return Addr[i]; }
		inline bool operator==(const IPv4ADDR& Another) const { return memcmp(Addr, Another.Addr, sizeof(Addr)) == 0; }
		inline bool operator!=(const IPv4ADDR& Another) const { return !operator==(Another); }

		inline int FromString(const std::basic_string<char>& Str) { return FromString(Str.c_str()); }
		int FromString(const char* BufSource);
		std::basic_string<char> ToString() const;
		char* ToString(char* Dest, size_t Len) const;
	};

protected:

	class PROTOCOL_INTERATOR
	{
#define PROTOCOL_INTERATOR_FIELDS struct protoent * Cur;
		class _NAME
		{
			friend PROTOCOL_INTERATOR;
			PROTOCOL_INTERATOR_FIELDS;
		public:
			operator char*();
		};

		/*
		Property represent protocol index.
		PPROTO_
		*/
		class _INDEX { PROTOCOL_INTERATOR_FIELDS; public: operator short(); };

		class P_NAME
		{
			class _COUNT
			{
				friend P_NAME;
				PROTOCOL_INTERATOR_FIELDS;
			public:
				operator int();
			};
		public:
			_COUNT Count;
			char * operator[](unsigned Index);
		};
	public:
		inline PROTOCOL_INTERATOR(struct protoent * New) { Name.Cur = New; }

		union
		{
			_NAME Name;
			_INDEX Index;/* Property represent protocol index. PPROTO_ */
			class
			{ PROTOCOL_INTERATOR_FIELDS; public:
				operator bool() const { return Cur == nullptr;}
			} NotHave;
			P_NAME Aliases;
		};
	};

	class PORT_SERVICE_INTERATOR
	{
#define PORT_SERVICE_INTERATOR_FIELDS struct servent * Cur;
		class _NAME
		{
			friend PORT_SERVICE_INTERATOR;
			PORT_SERVICE_INTERATOR_FIELDS;
		public:
			operator char*();
		};

		class _PORT { PORT_SERVICE_INTERATOR_FIELDS; public: operator TPORT(); };
		class _PORT_IN_PACKET_FORM { PORT_SERVICE_INTERATOR_FIELDS; public: operator TPORT(); };

		class _USED_PROTOCOL
		{
			PORT_SERVICE_INTERATOR_FIELDS;
		public:
			operator char*();
			PROTOCOL_INTERATOR GetInfo();
		};

		class P_NAME
		{
			class _COUNT
			{
				friend P_NAME;
				PORT_SERVICE_INTERATOR_FIELDS;
			public:
				operator int();
			};
		public:
			_COUNT Count;
			char* operator[](unsigned Index);
		};

	public:
		inline PORT_SERVICE_INTERATOR(struct servent * New) { Name.Cur = New; }
		union
		{
			_NAME Name;
			_PORT Port;
			_PORT_IN_PACKET_FORM PortInPacketForm;
			_USED_PROTOCOL UsedProtocol;
			class { PORT_SERVICE_INTERATOR_FIELDS;
			public: operator bool() const { return Cur == nullptr; }
			} NotHave;
			P_NAME Aliases;
		};
	};

	class NET_INTERATOR
	{
#define PROTOCOL_INTERATOR_FIELDS struct netent * Cur

		class _NAME{
			friend NET_INTERATOR;
			PROTOCOL_INTERATOR_FIELDS;
		public: operator char*(); 
		};
		class _ADDR_TYPE { PROTOCOL_INTERATOR_FIELDS; public: operator short(); };
		class _NUMBER_NET { PROTOCOL_INTERATOR_FIELDS; public: operator short(); };

		class P_NAME
		{
			class _COUNT
			{
				friend P_NAME;
				PROTOCOL_INTERATOR_FIELDS;
			public:
				operator int();
			};
		public:
			_COUNT Count;
			char* operator[](unsigned Index);
		};

	public:
		inline NET_INTERATOR(struct netent * New) { Name.Cur = New; }
		union
		{
			_NAME Name;
			_ADDR_TYPE AddrType;/* Type net address. Usually AF_INET */
			_NUMBER_NET NumberNet;
			class { PROTOCOL_INTERATOR_FIELDS; public: inline operator bool() const { return Cur == nullptr; } } NotHave;
			P_NAME Aliases;
		};
	};

	class INFO_HOST_INTERATOR
	{
#define INFO_HOST_INTERATOR_FIELDS struct hostent* Cur
		class _NAME{
			friend INFO_HOST_INTERATOR;
			INFO_HOST_INTERATOR_FIELDS;
		public: operator char*(); };

		class _ADDR_TYPE{ INFO_HOST_INTERATOR_FIELDS; public: operator short(); };
		class _LENGTH_ADDRESS { INFO_HOST_INTERATOR_FIELDS; public: operator short() const; };
		struct P_NAME
		{
		private:
			class  _COUNT { friend P_NAME;
				INFO_HOST_INTERATOR_FIELDS;
			public:
				operator int();
			};
		public:
			_COUNT Count;
			char * operator[](unsigned Index);
		};

		class P_ADDRESES
		{
			class ADDRESS_INTERATOR{
				void * Cur;
				short AddrType;
			public:
				ADDRESS_INTERATOR(void * nCur, short nAddrType);
				inline operator void*() {return Cur;}
				char* operator()(char * Dest, size_t Len = 0xffff);
				operator std::basic_string<char>();
			};
			class _COUNT{
				friend P_ADDRESES;
				INFO_HOST_INTERATOR_FIELDS;
			public:
				operator int();
			};
		public:
			_COUNT Count;
			ADDRESS_INTERATOR operator[](int Index);
		};

	public:
		inline INFO_HOST_INTERATOR(struct hostent *New) { Name.Cur = New; }
		union
		{
			_NAME Name;
			_ADDR_TYPE AddrType;/* Type net address. AF_INET or another AF_ - like */
			_LENGTH_ADDRESS LengthAddress;
			class { INFO_HOST_INTERATOR_FIELDS; public: inline operator bool() const { return Cur == nullptr; } } NotHave;
			P_NAME  Aliases;
			P_ADDRESES  Addresses;
		};
	};

	class __OPTIONS
	{
		_QUERY_URL_FIELDS1_;
		class OPTION_INTERATOR
		{
			int Level;
			int hSocket;
			int OptIndex;
		public:
			OPTION_INTERATOR(int nLevel, int nSocket, int nOptIndex);
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
		/*Index as SO_ ...*/
		OPTION_INTERATOR operator[](int OptIndex);
		OPTION_INTERATOR operator()(int OptIndex, int nLevel);
	}; 

	class __IS_RECIVED_FIN{
		_QUERY_URL_FIELDS1_;
	public:
		operator bool() const;
	};

	class __IS_NON_BLOCKED{
		_QUERY_URL_FIELDS1_;
	public:
		bool operator=(bool NewVal);
		operator bool() const;
	};

	class __PROTOCOL_FAMILY
	{
		_QUERY_URL_FIELDS1_;
	public:
		operator decltype(std::declval<addrinfo>().ai_protocol)();
		const char* ToString() const;
	};

	class __LOCAL_IP
	{
		friend QUERY_URL;
		_QUERY_URL_FIELDS1_;
		bool GetLocalAddress(SOCKET_ADDR& Address) const;
	public:
		template<typename Type>
		operator Type*() const
		{
			SOCKET_ADDR sa;
			GetLocalAddress(sa);
			return sa.Ip;
		}
		char* ToString(char * Dest, size_t Len = 0xffff) const;//As c string
		std::basic_string<char> ToString() const;//As stl string
		INFO_HOST_INTERATOR GetInfo() const;
	};

	class __LOCAL_PORT
	{
		_QUERY_URL_FIELDS1_;
	public:
		operator TPORT() const;
		char* ToString(char * Dest, size_t Len = 0xffff) const;//As c string
		std::basic_string<char> ToString() const;//As stl string
		PORT_SERVICE_INTERATOR GetInfo() const;
	};

	class __LOCAL_HOST_NAME
	{
		friend QUERY_URL;
		_QUERY_URL_FIELDS1_;
	public:
		operator char*() const;
	};

	class __REMOTE_IP
	{
		friend QUERY_URL;
		_QUERY_URL_FIELDS1_;
		inline bool GetRemoteAddress(SOCKET_ADDR & Address) const;
	public:
		template<typename Type>
		operator Type*() const
		{
			SOCKET_ADDR sa;
			GetRemoteAddress(sa);
			return sa.Ip;
		}
		char* ToString(char * Dest, size_t Len = 0xffff) const;//As c string
		std::basic_string<char> ToString() const;//As stl string
		inline INFO_HOST_INTERATOR GetInfo() const;//Get info about this IP
	};

	class __REMOTE_PORT
	{
		friend QUERY_URL;
		_QUERY_URL_FIELDS1_;
	public:
		inline operator TPORT() const;	//As number
		char* ToString(char * Dest, size_t Len = 0xffff) const;	//As c string
		std::basic_string<char> ToString() const; //As stl string
		inline PORT_SERVICE_INTERATOR GetInfo() const;//Get more info about remote port
	};

	class __REMOTE_HOST_NAME
	{
		_QUERY_URL_FIELDS1_;
	public:
		operator char*() const;
	};

	virtual bool EvntBind();
	virtual bool EvntConnect();
	virtual bool EvntAcceptClient(TDESCR ClientDescr);
	virtual int EvntGetCountPandingData();
	virtual bool EvntIsNotHaveRecvData();
	virtual bool EvntBeforeShutdown(int How);
	virtual bool EvntBeforeClose();

	inline void SetLastErr(int Num) { LastError = Num; }

	void InitFields();

	static int SetOption(int hSocket, int Level, int Option, bool& New);

	template<typename SetType>
	static inline int SetOption(int hSocket, int Level, int Option, SetType& New) { return setsockopt(hSocket, Level, Option, (char*)&New, sizeof(SetType)); }

	static int GetOption(int hSocket, int Level, int Option, bool& New);

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
			friend QUERY_URL;
			_QUERY_URL_FIELDS1_;
			inline int operator =(int nErr) { return iError = nErr; }
		public:
			
			inline const char* ToString() { return strerror(iError); }//Get string description error
			inline operator int() { return iError; }//Get number error
			inline void Clear() { iError = 0; }//Set err to OK state
		} LastError;

		/*
		Remote host name.
		*/
		__REMOTE_HOST_NAME RemoteHostName;

		/*
		Get remote port.
		*/
		__REMOTE_PORT RemotePort;

		/*
		Get dest ip.
		*/
		__REMOTE_IP RemoteIp;

		/*
		Get local host name.
		*/
		__LOCAL_HOST_NAME LocalHostName;

		/*
		Get local port.
		*/
		__LOCAL_PORT LocalPort;

		/*
		Get local ip addreess.
		*/
		__LOCAL_IP LocalIp;

		/*
		AF_ ...
		This property specifies the desired address family for the returned addresses. 
		Valid values for this field include AF_INET and AF_INET6.  
		*/
		__PROTOCOL_FAMILY ProtocolFamily;

		/*
		IPPROTO_ ...
		This property specifies the protocol for the returned socket addresses.
		*/
		class{
			_QUERY_URL_FIELDS1_;
			inline int operator=(int NewVal) { return ProtocolType = NewVal; }
		public:
			inline operator int() const { return ProtocolType; }
			inline char* ToString() const { return GetSystemProtocol(ProtocolType).Name; }
			inline PROTOCOL_INTERATOR GetInfo() const { return GetSystemProtocol(ProtocolType); }
		} Protocol;

		/*
		Set socket to non blocket mode.
		*/
		__IS_NON_BLOCKED IsNonBlocked;

		/*
			Get count data in recive buffer.
		*/
		class  {
			_QUERY_URL_FIELDS1_;
		public:
			inline operator int() const { return __QUERY_URL_PROPERTY_THIS->EvntGetCountPandingData(); }
		} CountPandingData;

		/*
			 Get open state.
		*/
		class{			
			_QUERY_URL_FIELDS1_;
		public:
			inline operator bool() const { return hSocket != INVALID_SOCKET; }
		} IsOpen;

		/*
		   Get file descriptor.
		*/
		class
		{
		_QUERY_URL_FIELDS1_;
		public:
			inline operator TDESCR() const { return hSocket; }
			inline TDESCR operator=(TDESCR NewDescriptor) { return hSocket = NewDescriptor; }
		} Descriptor;

		/*
			Is remote host send fin flag in packet.
		*/
		__IS_RECIVED_FIN IsRecivedFin;


		class
		{			
			_QUERY_URL_FIELDS1_;
		public:
			inline operator bool() const { return __QUERY_URL_PROPERTY_THIS->EvntIsNotHaveRecvData(); }
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
		__OPTIONS Options;

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
			DEF_SOCKET_EMPTY_OPTION_GET(IsAcceptConnection, bool);
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

#ifdef SO_UPDATE_ACCEPT_CONTEXT
			DEF_SOCKET_OPTION_PROPERTY(UpdateAcceptContext, TDESCR&, TDESCR, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT);
#else
			DEF_SOCKET_EMPTY_OPTION(UpdateAcceptContext, TDESCR&, TDESCR, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT);
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
	static PROTOCOL_INTERATOR GetSystemProtocol(int Index);

	/*
	Get info about protocol by name string
	example:
	int ProtocolIndex = GetSystemProtocol("ip").Index; //ProtocolIndex eq. 0
	*/
	static PROTOCOL_INTERATOR GetSystemProtocol(const char * Name);

	/*
	Get info about service by port number
	example:
	char * PortServiceName = GetSystemService(80).Name; //ProtocolName eq. "http"
	*/
	static PORT_SERVICE_INTERATOR GetSystemService(int PortNumber, const char * Prot = nullptr);

	/*
	Get info about service by name
	example:
	int Port = GetSystemProtocol("http").Port; //ProtocolIndex eq. 80
	*/
	static PORT_SERVICE_INTERATOR GetSystemService(const char * Name, const char * Prot = nullptr);

	/*
	Get info about network by number
	example:
	int NumNet = GetSystemNetwork("loop").NumberNet; //ProtocolIndex eq. 127.0.0.0
	*/
	static NET_INTERATOR GetSystemNetwork(long net, int type);

	/*
	Get info about network by Name
	example:
	int NumNet = GetSystemNetwork("loop").NumberNet; //ProtocolIndex eq. 127.0.0.0
	*/
	static NET_INTERATOR GetSystemNetwork(const char * Name);

	/*
	Get info about host by address
	Addr - Pointer on address
	Len  - Length of address
	Type - Type as AF_INET or AF_INET6 and another AF_

	example:
	std::string strAddr = Url.GetInfoAboutHost("google.com").Addresses[0]; //strAddr eq. "173.194.122.197"
	*/
	static INFO_HOST_INTERATOR GetInfoAboutHost(const void * Addr, int Len, int Type);

	/*
	Get info about host by name or address in string
	example:
	std::string strAddr = Url.GetInfoAboutHost("google.com").Addresses[0]; //strAddr eq. "173.194.122.197"
	*/
	static INFO_HOST_INTERATOR GetInfoAboutHost(const char * NameOrTextAddress);


	static bool StringToAddr(const char* Str, void* Dest, int Family = AF_INET);
	static bool AddrToString(const void* Src, char* Dest, size_t BufSize,  int Family = AF_INET);
	/*
	Connect with server at a specific address.
	*/
	bool Connect(ADDRESS_INFO::ADDRESS_INTERATOR& Address);

	/*
	Connect with server by address.
	*/
	bool Connect(ADDRESS_INFO& AddrInfo);

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
	);

	/*
	Create waiting client on port at a specific address.
	*/
	bool Bind(ADDRESS_INFO::ADDRESS_INTERATOR& Address, int MaxConnection = SOMAXCONN);

	/*
	Create waiting connection on port by address info.
	*/
	bool Bind(ADDRESS_INFO& AddrInfo, int MaxConnection = SOMAXCONN);

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
	);

	/*
	Accepting incoming client.
	*/
	bool AcceptClient(QUERY_URL& DestConnection);

	bool AcceptClient(QUERY_URL& DestConnection, SOCKET_ADDR& AddressClient);

	bool SkipClient();
		
	bool SkipClient(SOCKET_ADDR& AddressSkipedClient);
	/*
	Duplicate for sharing in new process.
	*/
	bool Duplicate(int TargetProcessHandle);


	/*
	Open as standart file.
	NOTE: On windows for correct use, socket descriptor should be opened without OVERLAPED function.
	@mode - Mode for open as file.
	@return - FILE structure.
	*/
	FILE* OpenAsFile(const char * mode = "w+");

	QUERY_URL();

	virtual ~QUERY_URL();

	/*
	Create socket for ReciveFrom and SendTo functions.
	*/
	bool OnlyCreate(int iSocktype = SOCK_DGRAM, int iProtocol = IPPROTO_UDP, int iFamily = AF_INET);

	/*
	Check event on this socket.
	@InEventFlags - POLL.. flags for check. For example POLLIN for check incoming data.
	@Timeout - Time for check. On default set 0 for only check without waiting.
	@return - -1 on error, on another case flag with event POLL.. . For example POLLHUP on disconnect.
	*/
	int CheckEvents(decltype(std::declval<pollfd>().events) InEventFlags, int Timeout = 0);

	/*
	Send file to remote host.
	{
		@InSocket - Soket buffer for send
		@InFileDescriptor - file descriptor for sendsanding data.
	}
	@Count - Size of sending data
	@Offset - Offsen data in file descriptor
	*/
	virtual long long SendFile(QUERY_URL& InSocket, size_t Count);

	virtual long long SendFile(TDESCR InFileDescriptor, size_t Count, off_t Offset = 0);
	
	/*
	Close socket descriptor.
	*/
	bool Close();

	/*
	Shutdown session.
	*/
	bool ShutdownSend();
	bool ShutdownRecive();
	bool ShutdownSendRecive();

	/*
	  Send data via system call to descriptor.
	  @Buf - Sending data
	  @SizeBuf  - Count byte in @Buf

	  @return - on succesful, count writed bytes to socket, on error, -1 is returned. 
	*/
	int Write(const void* Buf, size_t SizeBuf);

	/*
	  Recive data via system call to descriptor.
	  @Buf - Buf for getted data
	  @SizeBuf  - Count byte in @Buf

	  @return - on succesful, count readed bytes from socket buffer, on error, -1 is returned. 
	*/
	int Read(void * Buf, size_t SizeBuf);

	/*
	 Send via send socket function.
	 @QueryBuf - Sending data
	 @SizeBuf  - Count byte in @QueryBuf
	 @Flags   - Additional flags: MSG_DONTROUTE or MSG_OOB

	 @return - on succesful, count writed bytes to socket, on error, -1 is returned.
	*/
	virtual int Send(const void * QueryBuf, size_t SizeBuf, int Flags = 0);

	inline int Send(const std::basic_string<char> & InQuery) { return Send((const void*)InQuery.c_str(), InQuery.length()); }

	/*
	 Recive via recv socket function.
	 @Buf - Buf for getted data
	 @SizeBuf  - Size @Buf in bytes
	 @Flags   - Additional flags: MSG_PEEK or MSG_OOB or MSG_WAITALL

	 @return - on succesful, count readed bytes from socket buffer, on error, -1 is returned.
	*/
	virtual int Recive(void * Buf, size_t SizeBuf, int Flags = 0);

	virtual int Recive
	(
		std::basic_string<char>& StrBuf, 
		std::basic_string<char>::size_type MaxLen = std::numeric_limits<std::basic_string<char>::size_type>::max(), 
		int Flags = 0
	);

	/*
	@Flags:
	MSG_PEEK - Peeks at the incoming data. 
	MSG_OOB  - Processes Out Of Band (OOB) data.
	*/
	virtual int ReciveFrom(void * Buffer, size_t LenBuff, SOCKET_ADDR& AddressSender, int Flags = 0);
	int ReciveFrom(void * Buffer, size_t LenBuff, ADDRESS_INFO::ADDRESS_INTERATOR& AddressSender, int Flags = 0);

	/*
	@Flags:
	MSG_PEEK - Peeks at the incoming data. 
	MSG_OOB  - Processes Out Of Band (OOB) data.
	MSG_DONTROUTE - Not use routing.
	*/
	virtual int SendTo(const void * Buffer, size_t LenBuff, SOCKET_ADDR& AddressReciver, int Flags = 0);
	int SendTo(const void * Buffer, size_t LenBuff, ADDRESS_INFO::ADDRESS_INTERATOR& AddressSender, int Flags = 0);

	/*
	 Send and receive immediately.
	 @SendBuf - Data for send
	 @SizeSendBuf  - Size @SendBuf in bytes
	 @ReciveBuf   - Buffer for recive data
	 @SizeReciveBuf  - Size @ReciveBuf in bytes

	 @return - on succesful, count readed bytes from socket buffer, on error, -1 is returned.
	*/

	int SendAndRecive(const void* SendBuf, size_t SizeSendBuf, void* ReciveBuf, unsigned SizeReciveBuf);
	int SendAndRecive(std::basic_string<char>& strQuery, void* ReciveBuf, size_t SizeBuf);
	int SendAndRecive(const void* SendBuf, unsigned SizeSendBuf, std::basic_string<char>& Result);
	int SendAndRecive(const char* SendStr, std::basic_string<char>& Result);
	int SendAndRecive(const char* SendStr, void* ReciveBuf, unsigned SizeBuf);
	int SendAndRecive(std::basic_string<char>& strQuery, std::basic_string<char>& Result);

};

#endif // QUERYURL_H_INCLUDED
