#ifndef __QUERYURL_H_HAS_INCLUDED__
#define __QUERYURL_H_HAS_INCLUDED__

#include "ExString.h"

#ifdef _WIN32

#	include <winsock.h>


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

	struct ::netent * readnetnetworklist()
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

	struct ::netent * GetNetworksInfo(){ static struct ::netent * gn = readnetnetworklist(); return gn;}

	struct ::netent * getnetbyname__(const char *name)
	{
		for(::netent * gn = GetNetworksInfo();gn->n_name;gn++)
			if(StringCompare(name, gn->n_name) == 0)
				return gn;
		return NULL;
	}

#define getnetbyname winsock::getnetbyname__

	struct ::netent * getnetbyaddr__(long net, int type)
	{
		for(::netent * gn = GetNetworksInfo();gn->n_name;gn++)
			if((gn->n_net == net) && (gn->n_addrtype == type))
				return gn;
		return NULL;
	} 
#define getnetbyaddr winsock::getnetbyaddr__

	struct ::servent * readservicelist()
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

	struct ::servent * GetServiceInfo(){ static struct ::servent * gn = readservicelist(); return gn;}

	struct ::servent * getservbyport__(int port, const char * proto)
	{

		for(::servent * gn = GetServiceInfo();gn->s_name;gn++)
			if(gn->s_port == port)
			{
				if(proto == NULL)
					return gn;
				if(StringICompare(proto, gn->s_proto) == 0)
					return gn;
			}
		return NULL;
	}

#define getservbyport winsock::getservbyport__
	struct ::servent * getservbyname__(const char * Name, const char * proto)
	{
		for(::servent * gn = GetServiceInfo();gn->s_name;gn++)
			if(StringICompare(Name, gn->s_name) == 0)
			{
				if(proto == NULL)
					return gn;
				if(StringICompare(proto, gn->s_proto) == 0)
					return gn;
			}
		return NULL;
	} 

#	define getservbyname winsock::getservbyname__
};

using winsock::addrinfo;
using winsock::getaddrinfo;
using winsock::inet_ntop;
using winsock::ip_mreq_source;

#	define gai_strerror gai_strerrorA
#	define sockaddr  winsock::sockaddr
#	define sockaddr_in  winsock::sockaddr_in
#	define sockaddr_in6  winsock::sockaddr_in6

#	define IPPROTO_ICMP winsock::IPPROTO_ICMP
#	define IPPROTO_IGMP winsock::IPPROTO_IGMP 
#	define IPPROTO_GGP  winsock::IPPROTO_GGP
#	define IPPROTO_TCP  winsock::IPPROTO_TCP
#	define IPPROTO_PUP  winsock::IPPROTO_PUP
#	define IPPROTO_UDP  winsock::IPPROTO_UDP
#	define IPPROTO_IDP  winsock::IPPROTO_IDP
#	define IPPROTO_ND   winsock::IPPROTO_ND
#	define IPPROTO_RAW  winsock::IPPROTO_RAW
#	define IPPROTO_MAX  winsock::IPPROTO_MAX
#	ifndef WSA_VERSION
#		define WSA_VERSION MAKEWORD(2, 2)
#	endif
#	pragma comment(lib, "Ws2_32.lib")

#else

//For unix like operation sistem
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <netdb.h>
#   include <unistd.h>
#   include <fcntl.h>
#	define closesocket(socket)  close(socket)
#endif


#ifdef USE_OPEN_SSL
#	include <openssl/crypto.h>
#	include <openssl/x509.h>
#	include <openssl/pem.h>
#	include <openssl/ssl.h>
#	include <openssl/err.h>
#	define IS_HAVE_OPEN_SSL
#endif


#if defined(IS_HAVE_OPEN_SSL)
#define IS_HAVE_SSL 1
#else
#define IS_HAVE_SSL 0
#endif


#define ERR_ARG(Str) Str, L ## Str





class QUERY_URL
{
public:
	typedef decltype(std::declval<sockaddr_in>().sin_port) TPORT;

	struct SOCKET_ADDR
	{
		union
		{
			struct sockaddr		Addr;
			struct sockaddr_in  AddrInet;
			struct sockaddr_in6 AddrInet6;
		};

		template<typename RetType>
		inline operator RetType*()
		{
			return (RetType*)this;
		}

		inline sockaddr& operator =(const sockaddr& New)
		{
			Addr = New;
			return Addr;
		}

		inline sockaddr_in& operator =(const sockaddr_in& New)
		{
			AddrInet = New;
			return AddrInet;
		}

		inline sockaddr_in6& operator =(const sockaddr_in6& New)
		{
			AddrInet6 = New;
			return AddrInet6;
		}

		inline int ProtocolFamily() const
		{ 
			return Addr.sa_family; 
		}

		TPORT & Port()
		{
			switch(ProtocolFamily())
			{
			case AF_INET:
				return AddrInet.sin_port;
			case AF_INET6:
				return AddrInet6.sin6_port;
			}
			static TPORT i = 0;
			return i;
		}

		unsigned Len() const
		{
		   	switch(ProtocolFamily())
			{
			case AF_INET:
				return sizeof(AddrInet);
			case AF_INET6:
				return sizeof(AddrInet6);
			}
			return sizeof(Addr);
		}

		inline TPORT ReadeblePort()
		{ 
			return htons(Port());
		}

		void* AddressData()
		{
			switch(ProtocolFamily())
			{
			case AF_INET:
				return &AddrInet.sin_addr;
			case AF_INET6:
				return &AddrInet6.sin6_addr;
			}
			return NULL;
		}
	};
private:
#define URL_SET_LAST_ERR \
	{\
		((QUERY_URL*)this)->RemoteIp.iError = GetLastErr();\
	}

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
					if(Cur == NULL)
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
					if(Cur == NULL)
						return -1;
					return Cur->p_proto;
				}
			} Index;

			class
			{
				PROTOCOL_INTERATOR_FIELDS;
			public:
				operator bool() { return Cur == NULL;}
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
					if(Cur == NULL)
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
					if(Cur == NULL)
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
					if(Cur == NULL)
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
					if(Cur == NULL)
						return "";
					return Cur->s_proto;
				}

				PROTOCOL_INTERATOR GetInfo()
				{
					if(Cur == NULL)
						return PROTOCOL_INTERATOR(NULL);
					return GetSystemProtocol(Cur->s_proto);
				}
			} UsedProtocol;

			class
			{
				PORT_SERVICE_INTERATOR_FIELDS;
			public:
				operator bool()
				{ 
					return Cur == NULL;
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
						if(Cur == NULL)
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
					if(Cur == NULL)
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
					if(Cur == NULL)
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
					if(Cur == NULL)
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
					return Cur == NULL;
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
					if(Cur == NULL)
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
					if(Cur == NULL)
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
					if(Cur == NULL)
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
					return Cur == NULL;
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
						if(Cur == NULL)
							return NULL;
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
						return ADDRESS_INTERATOR(NULL, 0);
					return ADDRESS_INTERATOR(Count.Cur->h_addr_list[Index], Count.Cur->h_addrtype);
				}
			} Addresses;
		};
	};

	static void * GetWsa()
	{
#ifdef _WIN32
		static LPWSADATA wd = nullptr;
		if(wd == nullptr)
		{
			wd = (decltype(wd))malloc(sizeof(WSADATA));
			if(WSAStartup(WSA_VERSION, wd) == 0)
				return wd;
			return nullptr;
		}
		return wd;
#endif
		return (void*)1;
	}

	static void EndWsa()
	{
#ifdef _WIN32

		if(GetWsa() != nullptr)
			WSACleanup();
#endif
	}

	void InitFields()
	{

#ifdef IS_HAVE_OPEN_SSL
		RemoteIp.ctx = NULL;
		RemoteIp.ssl = NULL;
#endif
#ifdef _WIN32
		RemoteIp.IsNonBlocket = false;
#endif
		RemoteIp.hSocket = -1;
		RemoteIp.IsEnableSSLLayer = false;
		RemoteIp.PortionSize = 500;
		LastError.Clear();
	}

	bool InitSSL()
	{
#ifdef IS_HAVE_OPEN_SSL			
		ctx = NULL;
		ssl = NULL;
		SSL_load_error_strings();
		SSL_library_init();

		ctx = SSL_CTX_new(SSLv23_client_method());
		ssl = SSL_new(ctx);

		if(SSL_set_fd(ssl, Ip.hSocket) == 0)
		{
			LastError.SetError(ERR_ARG("SSL not connect with socket"), 14);
			goto SSLErrOut;
		}
		if(SSL_connect(ssl) < 0)
		{
			LastError.SetError(ERR_ARG("SSL not connect with socket"), 5);
SSLErrOut:
			SSL_shutdown(ssl);
			SSL_free(ssl);
			ssl = NULL;
			SSL_CTX_free(ctx);
			ctx = NULL;
			closesocket(hSocket);
			hSocket = -1;
			return false;
		}
#endif

		return true;
	}

	void UninitSSL()
	{
#ifdef IS_HAVE_OPEN_SSL	
		if(ssl)
		{
			SSL_shutdown(ssl);
			SSL_free(ssl);
		}
		if(ctx)
			SSL_CTX_free(ctx);
#endif
	}

	static int GetLastErr()
	{
#ifdef _WIN32
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
#endif
		return errno;
	}

	static inline int SetOption(int hSocket, int Level, int Option, bool & New)
	{										
		int v = New;
		return setsockopt(hSocket, Level, Option, (char*)&v, sizeof(v));
	}

	template<typename SetType>
	static inline int SetOption(int hSocket, int Level, int Option, SetType & New)
	{
		return setsockopt(hSocket, Level, Option, (char*)&New, sizeof(SetType));
	}


#ifdef _WIN32
#	define ADDITIONAL_FIELDS			\
		   bool			IsNonBlocket;
#else
#	define NON_BLOCKET_FIELD
#endif

#ifdef IS_HAVE_OPEN_SSL
#	define ADDITIONAL_FIELDS			\
		   ADDITIONAL_FIELDS			\
		   SSL_CTX*		ctx;			\
		   SSL*			ssl;
#endif

#define _QUERY_URL_FIELDS1_																\
	struct{																				\
		int				hSocket;														\
		unsigned		PortionSize;													\
		int				ProtocolType;													\
		bool			IsEnableSSLLayer;												\
		int				iError;															\
		ADDITIONAL_FIELDS																\
	}

#define DEF_GET_OPTION_PROPERTY(RetType, SoketNum, Level, Option)						\
	operator RetType()																	\
	{																					\
	    std::conditional<sizeof(RetType) < sizeof(int), int, RetType>::type v;	        \
		int l = sizeof(v);																\
		if(getsockopt(SoketNum, Level, Option, (char*)&v, &l) != 0)						\
			URL_SET_LAST_ERR															\
		return v;																		\
	}

#define DEF_SET_OPTION_PROPERTY(SetType, SoketNum, Level, Option)						\
	SetType operator=(SetType New){if(SetOption(SoketNum, Level, Option, New) != 0) URL_SET_LAST_ERR return New;}

#define DEF_SOCKET_OPTION_PROPERTY(Name, SetType, GetType, Level, Option)				\
		class{_QUERY_URL_FIELDS1_;public:												\
			DEF_SET_OPTION_PROPERTY(SetType, hSocket, Level, Option)					\
			DEF_GET_OPTION_PROPERTY(GetType, hSocket, Level, Option)					\
		} Name

#define DEF_SOCKET_OPTION_PROPERTY_GET(Name, GetType, Level, Option)					\
		class{_QUERY_URL_FIELDS1_; public: DEF_GET_OPTION_PROPERTY(GetType, hSocket, Level, Option)} Name

#define DEF_SOCKET_OPTION_PROPERTY_SET(Name, SetType, Level, Option)					\
		class{ _QUERY_URL_FIELDS1_; public:	DEF_SET_OPTION_PROPERTY(SetType, hSocket, Level, Option)} Name
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
				if(!((QUERY_URL*)this)->RemoteIp.GetRemoteAddress(sa))
					return "";
				return GetInfoAboutHost(sa.AddressData(), sa.Len(), sa.ProtocolFamily()).Name;
			}
		} RemoteHostName;

		/*
		Get remote port.
		*/
		class
		{
			friend QUERY_URL;
			_QUERY_URL_FIELDS1_;
		public:
			inline operator TPORT() const
			{
				SOCKET_ADDR sa;
				if(!((QUERY_URL*)this)->RemoteIp.GetRemoteAddress(sa))
					return 0;
				return sa.ReadeblePort();
			}

			char* operator()(char * Dest, size_t Len = 0xffff) const
			{
				NumberToString(operator unsigned short(), Dest, Len);
				return Dest;
			}

			std::basic_string<char> operator()() const
			{
				std::basic_string<char> Buf("", 6);
				NumberToString(operator unsigned short(), (char*)Buf.c_str(), 6);
				return Buf;
			}

			inline PORT_SERVICE_INTERATOR GetInfo() const
			{
				return GetSystemService(operator unsigned short());
			}
		} RemotePort;

		/*
		Get dest ip.
		*/
		class
		{
			friend QUERY_URL;
			_QUERY_URL_FIELDS1_;
		    inline bool GetRemoteAddress(SOCKET_ADDR & Address) const
			{
				int Len = sizeof(SOCKET_ADDR);
				if((getpeername(hSocket, Address, &Len) != 0) || (Len != Address.Len()))
				{
					URL_SET_LAST_ERR
					return false;
				}
				return true;
			}
		public:
			void* GetData() const
			{
				SOCKET_ADDR sa = {0};
				GetRemoteAddress(sa);
				return sa.AddressData();
			}

			char* operator()(char * Dest, size_t Len = 0xffff) const
			{
				SOCKET_ADDR sa;
				if(GetRemoteAddress(sa))
				{
					inet_ntop(sa.ProtocolFamily(), sa.AddressData(), Dest, Len);
					return Dest;
				}	
				return NULL;
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
				if(GetRemoteAddress(sa))
					return GetInfoAboutHost(sa.AddressData(), sa.Len(), sa.ProtocolFamily());
				return INFO_HOST_INTERATOR(NULL);
			}
		} RemoteIp;

		/*
		Get local host name.
		*/
		class
		{
			friend QUERY_URL;
			_QUERY_URL_FIELDS1_;
		public:

			operator char*() const
			{
				SOCKET_ADDR sa;
				if(((QUERY_URL*)this)->LocalIp.GetLocalAddress(sa))
					return GetInfoAboutHost(sa.AddressData(), sa.Len(), sa.ProtocolFamily()).Name;
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
				if(!((QUERY_URL*)this)->LocalIp.GetLocalAddress(sa))
					return 0;
				return sa.ReadeblePort();
			}

			char* operator()(char * Dest, size_t Len = 0xffff) const
			{
				NumberToString(operator unsigned short(), Dest, Len);
				return Dest;
			}

			std::basic_string<char> operator()() const
			{
				std::basic_string<char> Buf("", 6);
				NumberToString(operator unsigned short(), (char*)Buf.c_str(), 6);
				return Buf;
			}

			inline PORT_SERVICE_INTERATOR GetInfo() const
			{
				return GetSystemService(operator unsigned short());
			}
		} LocalPort;

		/*
		Get local ip addreess.
		*/
		class 
		{
			friend QUERY_URL;
			_QUERY_URL_FIELDS1_;
			inline bool GetLocalAddress(SOCKET_ADDR & Address) const
			{
				int Len = sizeof(SOCKET_ADDR);
				if((getsockname(hSocket, Address, &Len) != 0) || (Len != Address.Len()))
				{
					URL_SET_LAST_ERR
					return false;
				}
				return true;
			}
		public:
			void* GetData() const
			{
				SOCKET_ADDR sa = {0};
				GetLocalAddress(sa);
				return sa.AddressData();
			}

			char* operator()(char * Dest, size_t Len = 0xffff) const
			{
				SOCKET_ADDR sa;
				if(GetLocalAddress(sa))
				{
					inet_ntop(sa.ProtocolFamily(), sa.AddressData(), Dest, Len);
					return Dest;
				}	
				return NULL;
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
					return GetInfoAboutHost(sa.AddressData(), sa.Len(), sa.ProtocolFamily());
				return INFO_HOST_INTERATOR(NULL);
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
				if(!((QUERY_URL*)this)->RemoteIp.GetRemoteAddress(SockAddr))
					return -1;
				return SockAddr.ProtocolFamily();
			}

			operator char*()
			{
				SOCKET_ADDR SockAddr;
				if(!((QUERY_URL*)this)->RemoteIp.GetRemoteAddress(SockAddr))
					return "";
				switch(SockAddr.ProtocolFamily()) 
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
		   Is secure socket layer enabled.
		*/
		class
		{
			friend QUERY_URL;
			_QUERY_URL_FIELDS1_;
			bool operator =(bool NewValue)
			{
				return IsEnableSSLLayer = NewValue;
			}
		public:
			operator bool() const
			{
				return IsEnableSSLLayer;
			}
		} IsEnableSSL;

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
				DWORD nonBlocking = NewVal;
				if (ioctlsocket(hSocket, FIONBIO, &nonBlocking) != NO_ERROR)
					URL_SET_LAST_ERR
				else
					IsNonBlocket = NewVal;
#else
				int nonBlocking = NewVal;
				if (fcntl(hSocket, F_SETFL, O_NONBLOCK, nonBlocking) == -1)
					URL_SET_LAST_ERR
#endif
				return NewVal;
			}

			operator bool() const
			{
#ifdef _WIN32
				return IsNonBlocket;
#else
				return fcntl(hSocket, F_GETFL, 0) & O_NONBLOCK;
#endif
			}
		} IsNonBlocket;

		/*
		This field specifies the preferred socket type, for
		example SOCK_STREAM or SOCK_DGRAM. 
		*/
		DEF_SOCKET_OPTION_PROPERTY_GET(TypeSocket, int, SOL_SOCKET, SO_TYPE);

		/*
		The timeout, in milliseconds, for blocking send calls. 
		The default for this option is zero, 
		which indicates that a send operation will not time out. 
		If a blocking send call times out, the connection is in an indeterminate state and should be closed.
		*/
		DEF_SOCKET_OPTION_PROPERTY(ReceiveTimeout, timeval&, timeval, SOL_SOCKET, SO_RCVTIMEO);

		/*
		The timeout, in milliseconds, for blocking send calls. 
		The default for this option is zero, which indicates that a send operation will not time out. 
		If a blocking send call times out, the connection is in an indeterminate state and should be closed.
		*/
		DEF_SOCKET_OPTION_PROPERTY(SendTimeout, timeval&, timeval, SOL_SOCKET, SO_SNDTIMEO);

		/*
		The total per-socket buffer space reserved for sends. 
		This is unrelated to SO_MAX_MSG_SIZE and does not necessarily correspond to the size of a TCP send window.
		*/
		DEF_SOCKET_OPTION_PROPERTY(SendSizeBuffer, int, int, SOL_SOCKET, SO_SNDBUF);

		/*
		The total per-socket buffer space reserved for receives. 
		This is unrelated to SO_MAX_MSG_SIZE and does not necessarily correspond to the size of the TCP receive window.
		*/
		DEF_SOCKET_OPTION_PROPERTY(ReceiveSizeBuffer, int, int, SOL_SOCKET, SO_RCVBUF);

		/*
		Enables keep-alive for a socket connection. 
		Valid only for protocols that support the notion of keep-alive (connection-oriented protocols). 
		For TCP, the default keep-alive timeout is 2 hours and the keep-alive interval is 1 second. 
		The default number of keep-alive probes varies based on the version of Windows.
		*/
		DEF_SOCKET_OPTION_PROPERTY(IsKeepAlive, bool, bool, SOL_SOCKET, SO_KEEPALIVE);

		/*
		Configure a socket for sending broadcast data. 
		This option is only Valid for protocols that support broadcasting (IPX and UDP, for example).
		*/
		DEF_SOCKET_OPTION_PROPERTY(IsBroadcast, bool, bool, SOL_SOCKET, SO_BROADCAST);

		/*
		Enable socket debugging.  Only allowed for processes with the
		CAP_NET_ADMIN capability or an effective user ID of 0.
		*/
		DEF_SOCKET_OPTION_PROPERTY(IsDebug, bool, bool, SOL_SOCKET, SO_DEBUG);

		/*
		Don't send via a gateway, send only to directly connected
		hosts.  The same effect can be achieved by setting the
		MSG_DONTROUTE flag on a socket send(2) operation.  Expects an
		integer boolean flag.
		*/
		DEF_SOCKET_OPTION_PROPERTY(IsDontRoute, bool, bool, SOL_SOCKET, SO_DONTROUTE);

		/*
		Returns whether a socket is in listening mode. 
		This option is only Valid for connection-oriented protocols.
		*/
		DEF_SOCKET_OPTION_PROPERTY_GET(IsAcceptConnection, bool, SOL_SOCKET, SO_ACCEPTCONN);

		/*
		Use the local loopback address when sending data from this socket. 
		This option should only be used when all data sent will also be received locally. 
		This option is not supported by the Windows TCP/IP provider. 
		If this option is used on Windows Vista and later, the getsockopt and setsockopt functions fail with WSAEINVAL. 
		On earlier versions of Windows, these functions fail with WSAENOPROTOOPT.
		*/
		DEF_SOCKET_OPTION_PROPERTY(IsUsedLoopBack, bool, bool, SOL_SOCKET, SO_USELOOPBACK);

		/*
		The socket can be bound to an address which is already in use. 
		Not applicable for ATM sockets.
		*/
		DEF_SOCKET_OPTION_PROPERTY(IsReuseAddr, bool, bool, SOL_SOCKET, SO_REUSEADDR);


		/*
		If true, the Linger option is disabled.
		*/
		DEF_SOCKET_OPTION_PROPERTY(IsDontLinger, bool, bool, SOL_SOCKET, SO_DONTLINGER);

		/*
		When enabled, a close(2) or shutdown(2) will not return until
		all queued messages for the socket have been successfully sent
		or the linger timeout has been reached.  Otherwise, the call
		returns immediately and the closing is done in the background.
		When the socket is closed as part of exit(2), it always
		lingers in the background.
		*/
		DEF_SOCKET_OPTION_PROPERTY(Linger, linger&, linger, SOL_SOCKET, SO_LINGER);

		/*
		Specify the minimum number of bytes in the buffer until the
		socket layer will pass the data to the protocol (SO_SNDLOWAT)
		or the user on receiving (SO_RCVLOWAT).
		*/
		DEF_SOCKET_OPTION_PROPERTY(SendLowWaterMark, int, int, SOL_SOCKET, SO_SNDLOWAT);

		DEF_SOCKET_OPTION_PROPERTY(ReceiveLowWaterMark, int, int, SOL_SOCKET, SO_RCVLOWAT);

		/*
		Returns the last error code on this socket. 
		This per-socket error code is not always immediately set.
		*/
		DEF_SOCKET_OPTION_PROPERTY_GET(LastSocketError, int, SOL_SOCKET, SO_ERROR);

		/*
		Indicates that out-of-bound data should be returned in-line with regular data. 
		This option is only valid for connection-oriented protocols that support out-of-band data.
		*/
		DEF_SOCKET_OPTION_PROPERTY(IsReceivedOOBDataInLine, bool, bool, SOL_SOCKET, SO_OOBINLINE);

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
					union
					{
						RetType Val; int i;
					} v;
					v.i = 0;
					int l = sizeof(v);
					if(getsockopt(hSocket, Level, OptIndex, (char*)&v, &l) != 0)
						URL_SET_LAST_ERR
					return v.Val;
				}

				template<typename SetType>
				SetType operator=(SetType New)
				{
					union
					{
						SetType Val; int i;
					} v;
					v.i = 0, v.Val = New;
					if(setsockopt(hSocket, Level, OptIndex, (char*)&v, sizeof(v)) != 0)
						URL_SET_LAST_ERR
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


		class
		{
		public:
			union
			{
				/*
				Join the socket to the supplied multicast group on the specified interface.
				*/
				DEF_SOCKET_OPTION_PROPERTY_SET(AddMembership, ip_mreq&, IPPROTO_IP, IP_ADD_MEMBERSHIP);

				/*
				Join the supplied multicast group on the given interface and accept data sourced from the supplied source address.
				*/
				DEF_SOCKET_OPTION_PROPERTY_SET(AddSourceMembership, ip_mreq_source&, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP);

				/*
				Removes the given source as a sender to the supplied multicast group and interface.
				*/
				DEF_SOCKET_OPTION_PROPERTY_SET(AddBlockSourceMembership, ip_mreq_source&, IPPROTO_IP, IP_BLOCK_SOURCE);

				/*
				Leaves the specified multicast group from the specified interface. 
				Service providers must support this option when multicast is supported. 
				*/
				DEF_SOCKET_OPTION_PROPERTY_SET(DropMembership, ip_mreq&, IPPROTO_IP, IP_DROP_MEMBERSHIP);

				/*
				Drops membership to the given multicast group, interface, and source address.
				*/
				DEF_SOCKET_OPTION_PROPERTY_SET(DropSourceMembership, ip_mreq_source&, IPPROTO_IP, IP_DROP_SOURCE_MEMBERSHIP);

				/*
				Indicates that data should not be fragmented regardless of the local MTU. 
				Valid only for message oriented protocols. 
				Microsoft TCP/IP providers respect this option for UDP and ICMP.
				*/
				DEF_SOCKET_OPTION_PROPERTY(IsDontFragment, bool, bool, IPPROTO_IP, IP_DONTFRAGMENT);

				/*
				When set to true, indicates the application provides the IP header. 
				Applies only to SOCK_RAW sockets. 
				The TCP/IP service provider may set the ID field, 
				if the value supplied by the application is zero. 
				*/
				DEF_SOCKET_OPTION_PROPERTY(IsIncludeHeader, bool, bool, IPPROTO_IP, IP_HDRINCL);
								
				/*
				When set to true, indicates the application provides the IP header. 
				Applies only to SOCK_RAW sockets. 
				The TCP/IP service provider may set the ID field, 
				if the value supplied by the application is zero. 
				*/
				DEF_SOCKET_OPTION_PROPERTY(Multicast, int, int, IPPROTO_IP, IP_MULTICAST_IF);

				/*
				Controls whether data sent by an application on the local computer
				(not necessarily by the same socket) in a multicast session will be received by a
				socket joined to the multicast destination group on the loopback interface.
				*/
				DEF_SOCKET_OPTION_PROPERTY(IsMulticastLoop, bool, bool, IPPROTO_IP, IP_MULTICAST_LOOP);

				/*
				Set or read the time-to-live value of outgoing multicast
				packets for this socket.  It is very important for multicast
				packets to set the smallest TTL possible.  The default is 1
				which means that multicast packets don't leave the local
				network unless the user program explicitly requests it.
				Argument is an integer.
				*/
				DEF_SOCKET_OPTION_PROPERTY(MulticastTimeToLive, int, int, IPPROTO_IP, IP_MULTICAST_TTL);

				/*
				Pass an IP_PKTINFO ancillary message that contains a pktinfo
				structure that supplies some information about the incoming
				packet.  This only works for datagram oriented sockets.
				*/
				DEF_SOCKET_OPTION_PROPERTY(PacketInfo, int, int, IPPROTO_IP, IP_PKTINFO);

				/*
				Allows or blocks broadcast reception.
				*/
				DEF_SOCKET_OPTION_PROPERTY(IsReceiveBroadcast, bool, bool, IPPROTO_IP, IP_RECEIVE_BROADCAST);


			};
		} IpOptions;
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
	static PORT_SERVICE_INTERATOR GetSystemService(int PortNumber, const char * Prot = NULL)
	{
		return PORT_SERVICE_INTERATOR(getservbyport(htons(PortNumber), Prot));
	}

	/*
	Get info about service by name
	example:
	int Port = GetSystemProtocol("http").Port; //ProtocolIndex eq. 80
	*/
	static PORT_SERVICE_INTERATOR GetSystemService(const char * Name, const char * Prot = NULL)
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

	class ADDRESS_INFO
	{
#define ADDRESS_INFO_FIELDS							\
		struct										\
		{											\
		addrinfo * ai;							\
		std::def_var_in_union_with_constructor	\
		<std::basic_string<char>> PortName;	\
		std::def_var_in_union_with_constructor	\
		<std::basic_string<char>> HostName;	\
		int iError;								\
	}


#define ADDRESS_INFO_SET_LAST_ERR(Str, Num)						\
		{														\
		static const ERROR_INFO Uei = {Num, Str};			\
		((ADDRESS_INFO*)this)->HostName.LastErr =			\
		(ERROR_INFO*)&Uei;									\
	}
		friend QUERY_URL;

		operator addrinfo*() const
		{
			return HostName.ai;
		}

		void InitFields()
		{
			HostName.ai = NULL;
			LastError.Clear();
			new(&HostName.PortName)  std::basic_string<char>("");
			new(&HostName.HostName)  std::basic_string<char>("");
		}


		class ADDRESSES
		{
			friend QUERY_URL;
			class ADDRESS_INTERATOR
			{
				friend QUERY_URL;

				operator addrinfo*() const
				{
					return Ip.ca;
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
							return ((SOCKET_ADDR*)ca->ai_addr)->Port();
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
							inet_ntop(((SOCKET_ADDR*)ca->ai_addr)->ProtocolFamily(), ((SOCKET_ADDR*)ca->ai_addr)->AddressData(), Dest, Len);
							return Dest;
						}

						void* GetData()
						{
							if(ca == nullptr)
								return nullptr;
							return ((SOCKET_ADDR*)ca->ai_addr)->AddressData();
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
							return ((SOCKET_ADDR*)ca->ai_addr)->ProtocolFamily();
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
				return ADDRESS_INTERATOR(NULL);
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

			if((_Pos = strstr(FullAddress, "://")) != NULL)
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


		~ADDRESS_INFO()
		{
			if(HostName.ai != NULL)
				freeaddrinfo(HostName.ai);
		}


		bool Update(int iSocktype = SOCK_STREAM, int iProtocol = IPPROTO_TCP, int iFamily = AF_UNSPEC, int iFlags = 0)
		{
			if(GetWsa() == nullptr)
			{
				LastError = GetLastErr();
				return false;
			}
			addrinfo host_info = {0}, *ah = NULL;
			host_info.ai_socktype = iSocktype;
			host_info.ai_family = iFamily;
			host_info.ai_protocol = iProtocol;
			host_info.ai_flags = iFlags;                   //AI_PASSIVE
			int ErrNb;
			if((ErrNb = getaddrinfo(HostName, PortName, &host_info, &ah)) != 0)
			{
				LastError = GetLastErr();
				return false;
			}
			if(HostName.ai != NULL)
				freeaddrinfo(HostName.ai);
			HostName.ai = ah;
			return true;
		}
	};

	/*
		Connect with server at a specific address.
	*/
	bool Connect(ADDRESS_INFO::ADDRESSES::ADDRESS_INTERATOR& Address)
	{
		if(GetWsa() == nullptr)
		{
			URL_SET_LAST_ERR
			return false;
		}
		{
			if(RemoteIp.hSocket != -1)
			{
				shutdown(RemoteIp.hSocket, SD_BOTH);
				closesocket(RemoteIp.hSocket);
			}
			addrinfo *i = Address;
			if((RemoteIp.hSocket = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == -1)
			{
				URL_SET_LAST_ERR
				return false;
			}else if (connect(RemoteIp.hSocket, i->ai_addr, i->ai_addrlen) == -1)
			{
				URL_SET_LAST_ERR
				closesocket(RemoteIp.hSocket);
				RemoteIp.hSocket = -1;
				return false;
			}		
			RemoteIp.ProtocolType = i->ai_protocol;
		}
		if(IsEnableSSL)
			if(!InitSSL())
				return false;
		return true;
	}
	
	/*
		Connect with server by address.
	*/
	bool Connect(ADDRESS_INFO& AddrInfo)
	{
		if(GetWsa() == nullptr)
		{
			URL_SET_LAST_ERR
				return false;
		}
		{
			if(RemoteIp.hSocket != -1)
			{
				shutdown(RemoteIp.hSocket, SD_BOTH);
				closesocket(RemoteIp.hSocket);
			}
			addrinfo *i = AddrInfo;
			for (; i != NULL; i = i->ai_next) 
			{
				if((RemoteIp.hSocket = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == -1)
					continue;
				if (connect(RemoteIp.hSocket, i->ai_addr, i->ai_addrlen) != -1)
					break;
				closesocket(RemoteIp.hSocket);
			}
			if(i == NULL)
			{
				URL_SET_LAST_ERR
				RemoteIp.hSocket = -1;
				return false;
			}
			RemoteIp.ProtocolType = i->ai_protocol;
		}
		if(IsEnableSSL)
			if(!InitSSL())
				return false;
		return true;
	}

	/*
		Connect with server.
	*/
	bool Connect
	(
		const char * Port, 
		const char* HostAddr = NULL, 
		int iSocktype = SOCK_STREAM, 
		int iProtocol = IPPROTO_TCP, 
		int iFamily = AF_UNSPEC, 
		int Flags = 0
	)
	{	
		if(GetWsa() == nullptr)
		{
			URL_SET_LAST_ERR
			return false;
		}
		{
			if(RemoteIp.hSocket != -1)
			{
				shutdown(RemoteIp.hSocket, SD_BOTH);
				closesocket(RemoteIp.hSocket);
			}
			addrinfo host_info = {0}, *ah = NULL, *i;
			host_info.ai_socktype = iSocktype;
			host_info.ai_family = iFamily;
			host_info.ai_protocol = iProtocol;
			host_info.ai_flags = Flags;                   //AI_PASSIVE

			if(getaddrinfo(HostAddr, Port, &host_info, &ah) != 0)
			{
				URL_SET_LAST_ERR
				return false;
			}

			for (i = ah; i != NULL; i = i->ai_next) 
			{
				if((RemoteIp.hSocket = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == -1)
					continue;
				if (connect(RemoteIp.hSocket, i->ai_addr, i->ai_addrlen) != -1)
					break;
				closesocket(RemoteIp.hSocket);
			}

			if(i == NULL)
			{
				URL_SET_LAST_ERR
				RemoteIp.hSocket = -1;
				if(ah != NULL)
					freeaddrinfo(ah);
				return false;
			}
			RemoteIp.ProtocolType = i->ai_protocol;
			if(ah != NULL)
				freeaddrinfo(ah);
		}
		if(IsEnableSSL)
			if(!InitSSL())
				return false;
		return true; 
	}



	/*
	  Create waiting client on port at a specific address.
	*/
	bool Bind(ADDRESS_INFO::ADDRESSES::ADDRESS_INTERATOR& Address, int MaxConnection = SOMAXCONN)
	{	
		if(GetWsa() == nullptr)
		{
			URL_SET_LAST_ERR
			return false;
		}
		if(RemoteIp.hSocket != -1)
		{
			shutdown(RemoteIp.hSocket, SD_BOTH);
			closesocket(RemoteIp.hSocket);
		}
		addrinfo *i = Address;
		if((RemoteIp.hSocket = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == -1)
		{
			URL_SET_LAST_ERR
			return false;
		}else if(bind(RemoteIp.hSocket, i->ai_addr, i->ai_addrlen) == -1)
		{
			closesocket(RemoteIp.hSocket);
			RemoteIp.hSocket = -1;
			URL_SET_LAST_ERR
			return false;
		}else if(listen(RemoteIp.hSocket, MaxConnection) == -1)
		{
			closesocket(RemoteIp.hSocket);
			RemoteIp.hSocket = -1;
			URL_SET_LAST_ERR
			return false;
		}
		RemoteIp.ProtocolType = i->ai_protocol;
		if(IsEnableSSL)
			if(!InitSSL())
				return false;
		return true;
	}

	/*
	  Create waiting client on port by address info.
	*/
	bool Bind(ADDRESS_INFO& AddrInfo, int MaxConnection = SOMAXCONN)
	{	
		if(GetWsa() == nullptr)
		{
			URL_SET_LAST_ERR
			return false;
		}
		if(RemoteIp.hSocket != -1)
		{
			shutdown(RemoteIp.hSocket, SD_BOTH);
			closesocket(RemoteIp.hSocket);
		}
		addrinfo *i = AddrInfo;
		for (;i != NULL; i = i->ai_next) 
		{
			if((RemoteIp.hSocket = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == -1)
				continue;
			if(bind(RemoteIp.hSocket, i->ai_addr, i->ai_addrlen) == -1)
			{
				closesocket(RemoteIp.hSocket);
				RemoteIp.hSocket = -1;
				continue;
			}
			if(listen(RemoteIp.hSocket, MaxConnection) == -1)
			{
				closesocket(RemoteIp.hSocket);
				RemoteIp.hSocket = -1;
				continue;
			}
			break;
		}
		RemoteIp.ProtocolType = i->ai_protocol;
		if(IsEnableSSL)
			if(!InitSSL())
				return false;
		return true;
	}

	/*
	  Create waiting client on port.
	*/
	bool Bind
	(
		const char * Port, 
		int MaxConnection = SOMAXCONN, 
		int iSocktype = SOCK_STREAM, 
		int iProtocol = IPPROTO_TCP, 
		int iFamily = AF_INET, 
		int Flags = AI_PASSIVE
	)
	{	
		if(GetWsa() == nullptr)
		{
			URL_SET_LAST_ERR
			return false;
		}
		if(RemoteIp.hSocket != -1)
		{
			shutdown(RemoteIp.hSocket, SD_BOTH);
			closesocket(RemoteIp.hSocket);
		}
		addrinfo host_info = {0},*ah = NULL, *i;
		host_info.ai_socktype = iSocktype;
		host_info.ai_family = iFamily;
		host_info.ai_protocol = iProtocol;
		host_info.ai_flags = Flags;                   //AI_PASSIVE
		if(getaddrinfo(NULL, Port, &host_info, &ah) != 0)
		{
			URL_SET_LAST_ERR
			return false;
		}

		for (i = ah; i != NULL; i = i->ai_next) 
		{
			if((RemoteIp.hSocket = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) == -1)
				continue;
			if(bind(RemoteIp.hSocket, i->ai_addr, i->ai_addrlen) == -1)
			{
				closesocket(RemoteIp.hSocket);
				RemoteIp.hSocket = -1;
				continue;
			}
			if(listen(RemoteIp.hSocket, MaxConnection) == -1)
			{
				closesocket(RemoteIp.hSocket);
				RemoteIp.hSocket = -1;
				continue;
			}
			break;
		}
		RemoteIp.ProtocolType = i->ai_protocol;
		if(ah != NULL)
				freeaddrinfo(ah);
		if(IsEnableSSL)
			if(!InitSSL())
				return false;
		return true; 
	}


	bool AcceptClient(QUERY_URL & DestCoonection)
	{
		SOCKET_ADDR SockAddr;
		int ClientAddressSize = sizeof(SockAddr);
		int ConnectedSocket = accept(RemoteIp.hSocket, SockAddr, &ClientAddressSize);
		if((ConnectedSocket == -1) || (SockAddr.Len() != ClientAddressSize))
		{
			URL_SET_LAST_ERR
			return false;
		}
		DestCoonection.RemoteIp.hSocket = ConnectedSocket;
		DestCoonection.RemoteIp.ProtocolType = RemoteIp.ProtocolType;
		DestCoonection.RemoteIp.IsNonBlocket = false;
		DestCoonection.RemoteIp.IsEnableSSLLayer = RemoteIp.IsEnableSSLLayer;
		return true;
	}


	QUERY_URL()
	{
		InitFields();
	}

	QUERY_URL(bool nIsEnableSSL)
	{
		InitFields();
		IsEnableSSL = nIsEnableSSL;
	}

	~QUERY_URL()
	{
		if(IsEnableSSL)
			UninitSSL();
		if(RemoteIp.hSocket != -1)
		{
			shutdown(RemoteIp.hSocket, SD_BOTH);
			closesocket(RemoteIp.hSocket);
		}
	}

	inline bool SendQuery(const std::basic_string<char> & InQuery)
	{
		return SendQuery((void*)InQuery.c_str(), InQuery.length());
	}

	bool SendQuery(const void * QueryBuf, unsigned SizeBuf)
	{
		if(IsEnableSSL)
		{ 
#ifdef	IS_HAVE_OPEN_SSL
			if(SSL_write(ssl, QueryBuf, SizeBuf) < 0)
			{
				URL_SET_LAST_ERR
				return false;
			}
#endif
		}else if(send(RemoteIp.hSocket, (char*)QueryBuf, SizeBuf, 0) < 0)
		{
			URL_SET_LAST_ERR
			return false;
		}
		return true;
	}


	bool TakeResponse(void * Buf, unsigned SizeBuf, unsigned * SizeReaded = NULL)
	{
		int ReadedSize;
		if(IsEnableSSL)
		{
#ifdef	IS_HAVE_OPEN_SSL
			if((ReadedSize = SSL_read(ssl, Buf, SizeBuf)) < 0)
			{
				URL_SET_LAST_ERR
				return false;
			}
#endif
		}else if((ReadedSize = recv(RemoteIp.hSocket, (char*)Buf, SizeBuf, 0)) == -1)
		{
			URL_SET_LAST_ERR
			return false;
		}
		if(SizeReaded != NULL)
			*SizeReaded = ReadedSize;
		return true;
	}

	bool TakeResponse(std::basic_string<char> & StrBuf)
	{
		if(StrBuf.capacity() < RemoteIp.PortionSize)
			StrBuf.resize(RemoteIp.PortionSize);

		char * Buf = (char*)StrBuf.c_str();
		unsigned CurSize = 0;

		if(IsEnableSSL)
		{
#ifdef	IS_HAVE_OPEN_SSL
			while(true)
			{
				int ReadedSize = SSL_read(ssl, Buf, PortionSize);
				if(ReadedSize < 0)
				{
					URL_SET_LAST_ERR
					return false;
				}else if(ReadedSize == 0)
					break;
				else
				{
					CurSize += ReadedSize;
					StrBuf.resize(CurSize + PortionSize);
					Buf = (char*)StrBuf.c_str() + CurSize;  
				}
			}
#endif
		}else
		{
			while(true)
			{
				int ReadedSize = recv(RemoteIp.hSocket, Buf, RemoteIp.PortionSize, 0);
				if(ReadedSize == -1)
				{
					URL_SET_LAST_ERR
					return false;
				}else if(ReadedSize == 0)
					break;
				else
				{
					CurSize += ReadedSize;
					StrBuf.resize(CurSize + RemoteIp.PortionSize);
					Buf = (char*)StrBuf.c_str() + CurSize;  
				}
			}
		}
		*Buf = '\0';
		return true;
	}


	inline bool SendAndTakeQuery(void * SendBuf, unsigned SizeSendBuf, void * Buf, unsigned SizeBuf, unsigned * SizeReaded = NULL)
	{
		if(!SendQuery(SendBuf,SizeSendBuf))
			return false;
		return TakeResponse(Buf,SizeBuf,SizeReaded);
	}

	inline bool SendAndTakeQuery(std::basic_string<char> & strQuery, void * Buf, unsigned SizeBuf, unsigned * SizeReaded = NULL)
	{
		if(!SendQuery((void*)strQuery.c_str(), strQuery.length()))
			return false;
		return TakeResponse(Buf, SizeBuf, SizeReaded);
	}

	inline bool SendAndTakeQuery(void * SendBuf, unsigned SizeSendBuf, std::basic_string<char> & Result)
	{
		if(!SendQuery(SendBuf,SizeSendBuf))
			return false;
		return TakeResponse(Result);
	}

	inline bool SendAndTakeQuery(char * SendStr, std::basic_string<char> & Result)
	{
		if(!SendQuery(SendStr,strlen(SendStr)))
			return false;
		return TakeResponse(Result);
	}

	inline bool SendAndTakeQuery(char * SendStr, void * Buf, unsigned SizeBuf, unsigned * SizeReaded = NULL)
	{
		if(!SendQuery(SendStr,strlen(SendStr)))
			return false;
		return TakeResponse(Buf,SizeBuf,SizeReaded);
	}


	inline bool SendAndTakeQuery(std::basic_string<char> & strQuery, std::basic_string<char> & Result)
	{
		if(!SendQuery((void*)strQuery.c_str(), strQuery.length()))
			return false;
		return TakeResponse(Result);
	}

};



#endif // QUERYURL_H_INCLUDED
