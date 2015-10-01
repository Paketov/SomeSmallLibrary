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

#define getservbyname winsock::getservbyname__
};

using winsock::addrinfo; 
using winsock::sockaddr_in6;
using winsock::getaddrinfo;
using winsock::inet_ntop;


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
#endif


#ifdef OPENSSL_GLOBAL
#define IS_HAVE_OPEN_SSL
#endif

#if defined(IS_HAVE_OPEN_SSL)
#define IS_HAVE_SSL 1
#else
#define IS_HAVE_SSL 0
#endif


#define ERR_ARG(Str) Str, L ## Str




class QUERY_URL_CLIENT
{

	typedef std::basic_string<char> String;

	unsigned				PortionSize;

	struct ADDR_INFO : addrinfo
	{
		inline int ProtocolFamily()
		{ 
			return ai_family; 
		}


		unsigned short Port()
		{
			switch(ProtocolFamily())
			{
			case AF_INET:
				if(ai_addrlen == sizeof(sockaddr_in))
					return ((sockaddr_in*)ai_addr)->sin_port;
				return 0;
			case AF_INET6:
				if(ai_addrlen == sizeof(sockaddr_in6))
					return ((sockaddr_in6*)ai_addr)->sin6_port;   
			}
			return 0;
		}

		unsigned short ReadeblePort() 
		{ 
			return htons(Port());
		}

		void* AddressData()
		{
			switch(ProtocolFamily())
			{
			case AF_INET:
				if(ai_addrlen == sizeof(sockaddr_in))
					return &((sockaddr_in*)ai_addr)->sin_addr;
				else if(ai_addrlen == sizeof(sockaddr))
					return ((sockaddr*)ai_addr)->sa_data;
				break;
			case AF_INET6:
				if(ai_addrlen == sizeof(sockaddr_in6))
					return &((sockaddr_in6*)ai_addr)->sin6_addr;
				else if(ai_addrlen == sizeof(sockaddr))
					return ((sockaddr*)ai_addr)->sa_data;
				break;
			}
			return NULL;
		}

		inline ADDR_INFO * Next() 
		{ 
			return (ADDR_INFO*)ai_next;
		}

	};

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
				operator unsigned short()
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
				operator unsigned short()
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

#ifdef IS_HAVE_OPEN_SSL
	SSL_CTX				*ctx; //For ssl connection
	SSL					*ssl; //For ssl connection
#endif

#ifdef _WIN32
	void * lpWSAData;
#endif

	int StartWsa()
	{
#ifdef _WIN32
		if(lpWSAData == NULL)
		{
			lpWSAData = malloc(sizeof(WSADATA));
			return WSAStartup(WSA_VERSION, (LPWSADATA)lpWSAData);
		}
#endif
		return 0;
	}

	void EndWsa()
	{
#ifdef _WIN32
		if(lpWSAData != NULL)
		{
			free(lpWSAData);
			WSACleanup();
		}
#endif
	}

	void InitFields()
	{

#ifdef IS_HAVE_OPEN_SSL
		ctx = NULL;
		ssl = NULL;
#endif
		Ip.HostName = NULL;
		Ip.AllAddressHost = NULL;
		Ip.CurUseAddress = NULL;
		Ip.Port = 0;
		Ip.hSocket = -1;
		Ip.IsEnableSSLLayer = false;
#ifdef _WIN32
		lpWSAData = NULL;
		Ip.IsNonBlocket = false;
#endif
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

	bool InitInfo(unsigned char * IPAddress, unsigned short Port = 80, bool IsHaveSSL = false)
	{
		IsEnableSSL = IsHaveSSL;
	}

	bool InitInfo(char * lpAddress)
	{
		char * _Pos;
		int Pos;
		unsigned short _Port;
		PortionSize = 500;

		if((_Pos = strstr(lpAddress, "://")) != NULL)
		{
			_Port = (strncmp(lpAddress, "https",5) == 0)?443:80;
			_Pos += 3;
		}else 
		{
			_Port = 80;
			_Pos = lpAddress;
		}

		sscanf(_Pos,"%*[^/:]%n%*c%hu", &Pos, &_Port);

		Host.Append(_Pos, Pos);
		Port = _Port;
		if(_Port == 443)
			IsEnableSSL = IS_HAVE_SSL;
		if(*Host.HostName == '\0')
		{
			LastError.SetError(ERR_ARG("Invalid host name"), 1);
			return false;
		}

		return true;
	}


#ifdef _WIN32
#define NON_BLOCKET_FIELD bool IsNonBlocket;
#else
#define NON_BLOCKET_FIELD
#endif

#define _QUERY_URL_FIELDS1_  \
	struct{\
	int				hSocket;\
	unsigned short  Port;\
	ADDR_INFO		*AllAddressHost;\
	ADDR_INFO		*CurUseAddress;\
	char			*HostName;\
	bool			IsEnableSSLLayer;\
	char			*lpError;\
	wchar_t			*lpwError;\
	unsigned		uError;\
	NON_BLOCKET_FIELD\
	}

public:

	union
	{

		class URL_ERROR
		{		
			friend QUERY_URL_CLIENT;
			_QUERY_URL_FIELDS1_;

			void SetError(char * ErrorMessage, wchar_t * WErrorMessage, unsigned Number)
			{
				lpError = ErrorMessage;
				lpwError = WErrorMessage;
				uError = Number;
			}

		public:
			operator const char *()
			{
				return lpError;
			}

			operator const wchar_t *()
			{
				return lpwError;
			}

			unsigned GetNumber()
			{
				return uError;
			}

			void Clear()
			{
				SetError(ERR_ARG("OK"), 0);
			}
		} LastError;

		class 
		{
			friend QUERY_URL_CLIENT;
			_QUERY_URL_FIELDS1_;

			void SetByData()
			{
				struct hostent * Res = gethostbyaddr((const char*)CurUseAddress->ai_addr, CurUseAddress->ai_addrlen, CurUseAddress->ProtocolFamily());
				Set(Res->h_name);
			}

			void Set(char *NewStr)
			{
				realloc(HostName, StringLength(NewStr) + 1);
				StringCopy(HostName, NewStr);
			}

			void Append(char * StrAppend, size_t Len)
			{
				size_t FirstSize = (HostName == NULL)?0:StringLength(HostName);
				HostName = (char*)realloc(HostName, FirstSize + Len + 1);
				if(FirstSize == 0)
					*HostName = '\0';
				StringAppend(HostName, StrAppend, Len);
			}
			void Clear()
			{
				if(HostName)
				{
					free(HostName);
					HostName = NULL;
				}
			}

		public:

			operator char*()
			{
				return HostName;
			}

			bool IsEmpty()
			{
				return (HostName)?(*HostName == '\0'):true;
			}
		} Host;

		class
		{
			_QUERY_URL_FIELDS1_;
		public:
			inline operator int()
			{
				return CurUseAddress->ai_socktype;
			}
		} TypeSocket;

		class
		{
			friend QUERY_URL_CLIENT;
			_QUERY_URL_FIELDS1_;

			class AS_INTERATOR
			{
				ADDR_INFO * Cur;
			public:

				AS_INTERATOR(ADDR_INFO * New)
				{
					Cur = New;
				}

				operator unsigned short()
				{
					return (Cur != NULL)?Cur->ReadeblePort():0;
				} 

				char* operator()(char * Dest, size_t Len = 6)
				{
					if(Cur != NULL)
						NumberToString((unsigned short)*this, Dest, Len);
					return Dest;
				}

				inline PORT_SERVICE_INTERATOR GetInfo()
				{
					return GetSystemService((unsigned short)*this);
				}

				operator std::basic_string<char>()
				{
					std::basic_string<char> Buf("", 6);
					NumberToString((unsigned short)*this, (char*)Buf.c_str(), 6);
					return Buf;
				}

			};

			unsigned short operator =(unsigned short New)
			{
				return Port = New;
			}

		public:

			inline operator unsigned short()
			{
				return Port;
			}

			char* operator()(char * Dest, size_t Len = 0xffff)
			{
				NumberToString((unsigned short)*this, Dest, Len);
				return Dest;
			}

			operator std::basic_string<char>()
			{
				std::basic_string<char> Buf("", 6);
				NumberToString((unsigned short)*this, (char*)Buf.c_str(), 6);
				return Buf;
			}


			inline PORT_SERVICE_INTERATOR GetInfo()
			{
				return GetSystemService(Port);
			}


			AS_INTERATOR operator[](size_t Index)
			{
				ADDR_INFO * ai = AllAddressHost;
				for(unsigned i = 0;ai != NULL;ai = ai->Next(), i++)
					if(i == Index)
						break;
				return AS_INTERATOR(ai);
			}

		} Port;

		class
		{
			friend QUERY_URL_CLIENT;
			_QUERY_URL_FIELDS1_;
			class AS_INTERATOR
			{
				ADDR_INFO * Cur;
			public:
				AS_INTERATOR(ADDR_INFO * New)
				{
					Cur = New;
				}

				void* GetData()
				{
					if(Cur != NULL)
						return Cur->AddressData();
					return NULL;
				}	


				operator std::basic_string<char>()
				{
					std::basic_string<char> Buf("", INET6_ADDRSTRLEN + 1);
					operator()((char*)Buf.c_str(), INET6_ADDRSTRLEN + 1);
					return Buf;
				}

				char* operator()(char * Dest, size_t Len = 6)
				{
					if(Cur != NULL)
						inet_ntop(Cur->ProtocolFamily(), Cur->AddressData(), Dest, Len);
					return Dest;
				}

				inline INFO_HOST_INTERATOR GetInfo()
				{
					return GetInfoAboutHost(Cur->AddressData(), Cur->ai_addrlen, Cur->ProtocolFamily());
				}
			};
		public:

			void* GetData()
			{
				return CurUseAddress->AddressData();
			}

			char* operator()(char * Dest, size_t Len = 0xffff)
			{
				inet_ntop(CurUseAddress->ProtocolFamily(), CurUseAddress->AddressData(), Dest, Len);
				return Dest;
			}

			operator std::basic_string<char>()
			{
				std::basic_string<char> Buf("", INET6_ADDRSTRLEN + 1);
				operator()((char*)Buf.c_str(), INET6_ADDRSTRLEN + 1);
				return Buf;
			}

			inline INFO_HOST_INTERATOR GetInfo()
			{
				return GetInfoAboutHost(CurUseAddress->AddressData(), CurUseAddress->ai_addrlen, CurUseAddress->ProtocolFamily());
			}

			AS_INTERATOR operator[](size_t Index)
			{
				ADDR_INFO * ai = AllAddressHost;
				for(unsigned i = 0;ai != NULL;ai = ai->Next(), i++)
					if(i == Index)
						break;
				return AS_INTERATOR(ai);
			}
		} Ip;

		class
		{
			_QUERY_URL_FIELDS1_;
		public:
			operator int()
			{
				int Count = 0;
				for(auto Ai = AllAddressHost; Ai;Ai = Ai->Next(), Count++);
				return Count;
			}
		} CountAddresses;

		class
		{
			_QUERY_URL_FIELDS1_;
		public:

			operator unsigned short()
			{
				struct sockaddr_in6 sin;
				int addrlen = sizeof(sin);
				if(getsockname(hSocket, (struct sockaddr*)&sin, &addrlen) == 0)
					return ntohs(sin.sin6_port);
				return 0;
			}

			char* operator()(char * Dest, size_t Len = 6)
			{
				NumberToString((unsigned short)*this, Dest, Len);
				return Dest;
			}

			inline PORT_SERVICE_INTERATOR GetInfo()
			{
				return GetSystemService((unsigned short)*this);
			}

			operator std::basic_string<char>()
			{
				std::basic_string<char> Buf("", 6);
				NumberToString((unsigned short)*this, (char*)Buf.c_str(), 6);
				return Buf;
			}
		} LocalPort;

		class 
		{
			_QUERY_URL_FIELDS1_;
		public:

			char* operator()(char * Dest, size_t Len = 0xffff)
			{
				struct sockaddr_in6 sin;
				int addrlen = sizeof(sin);
				void * p;
				if(getsockname(hSocket, (struct sockaddr*)&sin, &addrlen) == 0)
				{
					switch(sin.sin6_family)
					{
					case AF_INET:
						if(addrlen == sizeof(sockaddr_in))
							p = &((sockaddr_in*)&sin)->sin_addr;
						else if(addrlen == sizeof(sockaddr))
							p = ((sockaddr*)&sin)->sa_data;
						else
							return NULL;
						break;
					case AF_INET6:
						if(addrlen == sizeof(sockaddr_in6))
							p = &((sockaddr_in6*)&sin)->sin6_addr;
						else if(addrlen == sizeof(sockaddr))
							p = ((sockaddr*)&sin)->sa_data;
						else
							return NULL;
						break;
					}
				}
				inet_ntop(sin.sin6_family, p, Dest, Len);
				return Dest;
			}

			inline INFO_HOST_INTERATOR GetInfo()
			{
				struct sockaddr_in6 sin;
				int addrlen = sizeof(sin);
				void * p;
				if(getsockname(hSocket, (struct sockaddr*)&sin, &addrlen) == 0)
				{
					switch(sin.sin6_family)
					{
					case AF_INET:
						if(addrlen == sizeof(sockaddr_in))
							p = &((sockaddr_in*)&sin)->sin_addr;
						else if(addrlen == sizeof(sockaddr))
							p = ((sockaddr*)&sin)->sa_data;
						else
							return NULL;
						break;
					case AF_INET6:
						if(addrlen == sizeof(sockaddr_in6))
							p = &((sockaddr_in6*)&sin)->sin6_addr;
						else if(addrlen == sizeof(sockaddr))
							p = ((sockaddr*)&sin)->sa_data;
						else
							return NULL;
						break;
					}
				}
				return GetInfoAboutHost(p, addrlen, sin.sin6_family);
			}

			operator std::basic_string<char>()
			{
				std::basic_string<char> Buf("", INET6_ADDRSTRLEN + 1);
				operator()((char*)Buf.c_str(), INET6_ADDRSTRLEN + 1);
				return Buf;
			}

		} LocalIp;

		class
		{
			_QUERY_URL_FIELDS1_;
		public:
			operator decltype(std::declval<addrinfo>().ai_protocol)()
			{
				return CurUseAddress->ProtocolFamily();
			}

			operator char*()
			{
				switch(CurUseAddress->ProtocolFamily()) 
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

		class 
		{
			_QUERY_URL_FIELDS1_;
		public:

			operator int()
			{
				return CurUseAddress->ai_protocol;
			}

			operator char*()
			{
				return GetSystemProtocol(CurUseAddress->ai_protocol).Name;
			}

			inline PROTOCOL_INTERATOR GetInfo()
			{
				return GetSystemProtocol(CurUseAddress->ai_protocol);
			}
		} Protocol;

		class
		{
			friend QUERY_URL_CLIENT;
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


		//Params


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
					((URL_ERROR*)this)->SetError(ERR_ARG("Failed to set non-blocking socket"), 34);
				else
					IsNonBlocket = NewVal;
#else
				int nonBlocking = NewVal;
				if (fcntl(hSocket, F_SETFL, O_NONBLOCK, nonBlocking) == -1)
					((URL_ERROR*)this)->SetError(ERR_ARG("Failed to set non-blocking socket"), 34);
#endif
				return NewVal;
			}

			operator bool()
			{
#ifdef _WIN32
				return IsNonBlocket;
#else
				return fcntl(hSocket, F_GETFL, 0) & O_NONBLOCK;
#endif
			}
		} IsNonBlocket;



		/*
		The timeout, in milliseconds, for blocking send calls. 
		The default for this option is zero, 
		which indicates that a send operation will not time out. 
		If a blocking send call times out, the connection is in an indeterminate state and should be closed.
		*/
		class 
		{
			_QUERY_URL_FIELDS1_;
		public:

			timeval & operator=(timeval & New)
			{
				if(setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&New, sizeof(timeval)) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not set time interval"), 38);
				return New;
			}

			operator timeval()
			{
				timeval v;
				int l = sizeof(v);
				if(getsockopt(hSocket,SOL_SOCKET, SO_RCVTIMEO, (char*)&v, &l) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not get time interval"), 39);
				return v;
			}
		} ReceiveTimeout;

		/*
		The timeout, in milliseconds, for blocking send calls. 
		The default for this option is zero, which indicates that a send operation will not time out. 
		If a blocking send call times out, the connection is in an indeterminate state and should be closed.
		*/
		class 
		{
			_QUERY_URL_FIELDS1_;
		public:

			timeval & operator=(timeval & New)
			{
				if(setsockopt(hSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&New, sizeof(timeval)) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not set time interval"), 40);
				return New;
			}

			operator timeval()
			{
				timeval v;
				int l = sizeof(v);
				if(getsockopt(hSocket,SOL_SOCKET, SO_SNDTIMEO, (char*)&v, &l) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not get time interval"), 41);
				return v;
			}
		} SendTimeout;

		/*
		The total per-socket buffer space reserved for sends. 
		This is unrelated to SO_MAX_MSG_SIZE and does not necessarily correspond to the size of a TCP send window.
		*/
		class 
		{
			_QUERY_URL_FIELDS1_;
		public:

			int operator=(int New)
			{ 
				if(setsockopt(hSocket, SOL_SOCKET, SO_SNDBUF, (char*)&New, sizeof(New)) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not set size buffer"), 42);
				return New;
			}

			operator int()
			{
				int v;
				int l = sizeof(v);
				if(getsockopt(hSocket,SOL_SOCKET, SO_SNDBUF, (char*)&v, &l) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not get size buffer"), 43);
				return v;
			}
		} SendSizeBuffer;

		/*
		The total per-socket buffer space reserved for receives. 
		This is unrelated to SO_MAX_MSG_SIZE and does not necessarily correspond to the size of the TCP receive window.
		*/
		class 
		{
			_QUERY_URL_FIELDS1_;
		public:

			int operator=(int New)
			{
				if(setsockopt(hSocket, SOL_SOCKET, SO_RCVBUF, (char*)&New, sizeof(New)) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not set size buffer"), 44);
				return New;
			}

			operator int()
			{
				int v;
				int l = sizeof(v);
				if(getsockopt(hSocket,SOL_SOCKET, SO_RCVBUF, (char*)&v, &l) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not get size buffer"), 45);
				return v;
			}
		} ReceiveSizeBuffer;

		/*
		Enables keep-alive for a socket connection. 
		Valid only for protocols that support the notion of keep-alive (connection-oriented protocols). 
		For TCP, the default keep-alive timeout is 2 hours and the keep-alive interval is 1 second. 
		The default number of keep-alive probes varies based on the version of Windows.
		*/
		class
		{			
			_QUERY_URL_FIELDS1_;
		public:

			bool operator=(bool New)
			{
				int v = New;
				if(setsockopt(hSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&v, sizeof(v)) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not set keep alive state"), 46);
				return New;
			}

			operator bool()
			{
				int v;
				int l = sizeof(v);
				if(getsockopt(hSocket,SOL_SOCKET, SO_KEEPALIVE, (char*)&v, &l) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not get keep alive state"), 47);
				return v;
			}

		} IsKeepAlive;

		/*
		Configure a socket for sending broadcast data. 
		This option is only Valid for protocols that support broadcasting (IPX and UDP, for example).
		*/
		class
		{			
			_QUERY_URL_FIELDS1_;
		public:

			bool operator=(bool New)
			{
				int v = New;
				if(setsockopt(hSocket, SOL_SOCKET, SO_BROADCAST, (char*)&v, sizeof(v)) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not set keep alive state"), 46);
				return New;
			}
			operator bool()
			{
				int v;
				int l = sizeof(v);
				if(getsockopt(hSocket,SOL_SOCKET, SO_BROADCAST, (char*)&v, &l) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not get keep alive state"), 47);
				return v;
			}
		} IsBroadcast;

		/*
		Enable socket debugging.  Only allowed for processes with the
		CAP_NET_ADMIN capability or an effective user ID of 0.

		*/
		class
		{			
			_QUERY_URL_FIELDS1_;
		public:
			bool operator=(bool New)
			{
				int v = New;
				if(setsockopt(hSocket, SOL_SOCKET, SO_DEBUG, (char*)&v, sizeof(v)) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not set keep alive state"), 46);
				return New;
			}
			operator bool()
			{
				int v;
				int l = sizeof(v);
				if(getsockopt(hSocket,SOL_SOCKET, SO_DEBUG, (char*)&v, &l) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not get keep alive state"), 47);
				return v;
			}
		} IsDebug;

		/*
		Don't send via a gateway, send only to directly connected
		hosts.  The same effect can be achieved by setting the
		MSG_DONTROUTE flag on a socket send(2) operation.  Expects an
		integer boolean flag.

		*/
		class
		{			
			_QUERY_URL_FIELDS1_;
		public:
			bool operator=(bool New)
			{
				int v = New;
				if(setsockopt(hSocket, SOL_SOCKET, SO_DONTROUTE , (char*)&v, sizeof(v)) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not set keep alive state"), 46);
				return New;
			}
			operator bool()
			{
				int v;
				int l = sizeof(v);
				if(getsockopt(hSocket,SOL_SOCKET, SO_DONTROUTE , (char*)&v, &l) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not get keep alive state"), 47);
				return v;
			}
		} IsDontRoute;

		/*
		Returns whether a socket is in listening mode. 
		This option is only Valid for connection-oriented protocols.
		*/
		class
		{
			_QUERY_URL_FIELDS1_;
		public:
			operator bool()
			{
				int v;
				int l = sizeof(v);
				if(getsockopt(hSocket,SOL_SOCKET, SO_ACCEPTCONN , (char*)&v, &l) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not get keep alive state"), 47);
				return v;
			}
		} IsAcceptConnection;

		/*
		Use the local loopback address when sending data from this socket. 
		This option should only be used when all data sent will also be received locally. 
		This option is not supported by the Windows TCP/IP provider. 
		If this option is used on Windows Vista and later, the getsockopt and setsockopt functions fail with WSAEINVAL. 
		On earlier versions of Windows, these functions fail with WSAENOPROTOOPT.
		*/
		class
		{
			_QUERY_URL_FIELDS1_;
		public:
			bool operator=(bool New)
			{
				int v = New;
				if(setsockopt(hSocket, SOL_SOCKET, SO_USELOOPBACK, (char*)&v, sizeof(v)) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not set keep alive state"), 46);
				return New;
			}
			operator bool()
			{
				int v;
				int l = sizeof(v);
				if(getsockopt(hSocket,SOL_SOCKET, SO_USELOOPBACK, (char*)&v, &l) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not get keep alive state"), 47);
				return v;
			}
		} IsUsedLoopBack;


		/*
		The socket can be bound to an address which is already in use. 
		Not applicable for ATM sockets.
		*/
		class
		{
			_QUERY_URL_FIELDS1_;
		public:
			bool operator=(bool New)
			{
				int v = New;
				if(setsockopt(hSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&v, sizeof(v)) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not set keep alive state"), 46);
				return New;
			}
			operator bool()
			{
				int v;
				int l = sizeof(v);
				if(getsockopt(hSocket,SOL_SOCKET, SO_REUSEADDR, (char*)&v, &l) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not get keep alive state"), 47);
				return v;
			}
		} IsReuseAddr;

		/*
		If true, the Linger option is disabled.
		*/
		class
		{
			_QUERY_URL_FIELDS1_;
		public:
			bool operator=(bool New)
			{
				int v = New;
				if(setsockopt(hSocket, SOL_SOCKET, SO_DONTLINGER, (char*)&v, sizeof(v)) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not set keep alive state"), 46);
				return New;
			}
			operator bool()
			{
				int v;
				int l = sizeof(v);
				if(getsockopt(hSocket,SOL_SOCKET, SO_DONTLINGER, (char*)&v, &l) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not get keep alive state"), 47);
				return v;
			}
		} IsDontLinger;

		/*
		When enabled, a close(2) or shutdown(2) will not return until
		all queued messages for the socket have been successfully sent
		or the linger timeout has been reached.  Otherwise, the call
		returns immediately and the closing is done in the background.
		When the socket is closed as part of exit(2), it always
		lingers in the background.

		*/
		class
		{
			_QUERY_URL_FIELDS1_;
		public:
			linger & operator=(linger & New)
			{
				if(setsockopt(hSocket, SOL_SOCKET, SO_LINGER, (char*)&New, sizeof(linger)) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not set keep alive state"), 46);
				return New;
			}
			operator linger()
			{
				linger v;
				int l = sizeof(v);
				if(getsockopt(hSocket,SOL_SOCKET, SO_LINGER, (char*)&v, &l) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not get keep alive state"), 47);
				return v;
			}
		} Linger;

		/*
		Specify the minimum number of bytes in the buffer until the
		socket layer will pass the data to the protocol (SO_SNDLOWAT)
		or the user on receiving (SO_RCVLOWAT).
		*/
		class
		{
			_QUERY_URL_FIELDS1_;
		public:
			int operator=(int New)
			{
				if(setsockopt(hSocket, SOL_SOCKET, SO_SNDLOWAT, (char*)&New, sizeof(New)) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not set keep alive state"), 46);
				return New;
			}
			operator int()
			{
				int v;
				int l = sizeof(v);
				if(getsockopt(hSocket,SOL_SOCKET, SO_SNDLOWAT, (char*)&v, &l) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not get keep alive state"), 47);
				return v;
			}
		} SendLowWaterMark;

		class
		{
			_QUERY_URL_FIELDS1_;
		public:
			int operator=(int New)
			{
				if(setsockopt(hSocket, SOL_SOCKET, SO_RCVLOWAT, (char*)&New, sizeof(New)) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not set keep alive state"), 46);
				return New;
			}
			operator int()
			{
				int v;
				int l = sizeof(v);
				if(getsockopt(hSocket,SOL_SOCKET, SO_RCVLOWAT, (char*)&v, &l) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not get keep alive state"), 47);
				return v;
			}
		} ReceiveLowWaterMark;

		/*
		Returns the last error code on this socket. 
		This per-socket error code is not always immediately set.
		*/
		class
		{
			_QUERY_URL_FIELDS1_;
		public:
			operator int()
			{
				int v;
				int l = sizeof(v);
				if(getsockopt(hSocket,SOL_SOCKET, SO_ERROR, (char*)&v, &l) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not get keep alive state"), 47);
				return v;
			}
		} LastSocketError;

		/*
		Indicates that out-of-bound data should be returned in-line with regular data. 
		This option is only valid for connection-oriented protocols that support out-of-band data.
		*/
		class
		{
			_QUERY_URL_FIELDS1_;
		public:
			bool operator=(bool New)
			{
				int v = New;
				if(setsockopt(hSocket, SOL_SOCKET, SO_OOBINLINE, (char*)&v, sizeof(v)) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not set keep alive state"), 46);
				return New;
			}
			operator bool()
			{
				int v;
				int l = sizeof(v);
				if(getsockopt(hSocket,SOL_SOCKET, SO_OOBINLINE, (char*)&v, &l) != 0)
					((URL_ERROR*)this)->SetError(ERR_ARG("Not get keep alive state"), 47);
				return v;
			}
		} IsReceivedOOBDataInLine;

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
				int hSocket;
				int OptIndex;
			public:
				OPTION_INTERATOR(int nSocket, int nOptIndex)
				{
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
					if(getsockopt(hSocket, SOL_SOCKET, OptIndex, (char*)&v, &l) != 0)
						((URL_ERROR*)this)->SetError(ERR_ARG("Not get option"), 70);
					return v.Val;
				}

				template<typename SetType>
				SetType operator=(SetType New)
				{
					union
					{
						SetType Val; int i;
					} v;
					v.i   = 0;
					v.Val = New;
					if(setsockopt(hSocket, SOL_SOCKET, OptIndex, (char*)&v, sizeof(v)) != 0)
						((URL_ERROR*)this)->SetError(ERR_ARG("Not set option"), 71);
					return New;
				}
			};
		public:
			/*
			Index as SO_ ...
			*/
			OPTION_INTERATOR operator[](int OptIndex)
			{
				return OPTION_INTERATOR(hSocket, OptIndex);
			}
		} Options;

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


	bool Connect(int iSocktype = SOCK_STREAM, int iProtocol = IPPROTO_TCP, int iFamily = AF_UNSPEC)
	{	
		if(StartWsa() != 0) 
		{
			LastError.SetError(ERR_ARG("Dont start wsa"), 15);
			return false;
		}

		{	
			if(Ip.AllAddressHost == NULL)
			{
				if(!InitAddressHost(iSocktype, iProtocol, iFamily))
					return false;
			}
			ADDR_INFO *i;
			for (i = Ip.AllAddressHost; i != NULL; i = i->Next()) 
			{
				Ip.hSocket = socket(i->ai_family, i->ai_socktype, i->ai_protocol);
				if(Ip.hSocket == -1)
					continue;
				if (connect(Ip.hSocket, i->ai_addr, i->ai_addrlen) != -1)
					break;
				closesocket(Ip.hSocket);
			}

			if(i == NULL)
			{
				LastError.SetError(ERR_ARG("Could not connect"), 3);
				Ip.hSocket = -1;
				return false;
			}
			Ip.CurUseAddress = i;

			if(Host.IsEmpty())
				Host.SetByData();
		}

		if(IsEnableSSL)
			if(!InitSSL())
				return false;
		return true; 
	}

	bool InitAddressHost(int iSocktype = SOCK_STREAM, int iProtocol = IPPROTO_TCP, int iFamily = AF_UNSPEC)
	{
		if(Host.IsEmpty())
		{
			LastError.SetError(ERR_ARG("Host name is empty"), 23);
			return false;
		}
		if((unsigned short)Port == 0)
		{
			LastError.SetError(ERR_ARG("Port is empty"), 24);
			return false;
		}

		struct addrinfo host_info = {0};
		host_info.ai_socktype = iSocktype;
		host_info.ai_family = iFamily;
		host_info.ai_protocol = iProtocol;

		char PortBuf[6];
		Port(PortBuf);
		if(getaddrinfo(Host, PortBuf, &host_info, (addrinfo**)&Ip.AllAddressHost) != 0)
		{
			LastError.SetError(ERR_ARG("Dont get server info"), 2);
			return false;
		}
		return true;
	}

	QUERY_URL_CLIENT(String & Host)
	{
		InitFields();
		InitInfo((char*)Host.c_str());
	}

	QUERY_URL_CLIENT(char * Host)
	{
		InitFields();
		InitInfo(Host);
	}

	~QUERY_URL_CLIENT()
	{
		if(IsEnableSSL)
			UninitSSL();
		if(Ip.hSocket != -1)
			closesocket(Ip.hSocket);
		if(Ip.AllAddressHost)
			freeaddrinfo(Ip.AllAddressHost);
		Host.Clear();
	}

	inline bool SendQuery(const String & InQuery)
	{
		return SendQuery((void*)InQuery.c_str(), InQuery.length());
	}

	bool SendQuery(const void * QueryBuf, unsigned SizeBuf)
	{
		if(IsEnableSSL)
		{
#ifdef	IS_HAVE_OPEN_SSL
			if(SSL_write(ssl, QueryBuf,SizeBuf) < 0)
			{
				LastError.SetError(ERR_ARG("Not send ssl query"), 7);
				return false;
			}
#endif
		}else if(send(Ip.hSocket, (char*)QueryBuf, SizeBuf, 0) < 0)
		{
			LastError.SetError(ERR_ARG("Not send query"), 8);
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
				LastError.SetError(ERR_ARG("Not get data from sll server"), 9);
				return false;
			}
#endif
		}else if((ReadedSize = recv(Ip.hSocket, (char*)Buf, SizeBuf, 0)) == -1)
		{
			LastError.SetError(ERR_ARG("Not get data from server"), 10);
			return false;
		}
		if(SizeReaded != NULL)
			*SizeReaded = ReadedSize;
		return true;
	}

	bool TakeResponse(std::basic_string<char> & StrBuf)
	{
		if(StrBuf.capacity() < PortionSize)
			StrBuf.resize(PortionSize);

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
					LastError.SetError(ERR_ARG("Not get data from sll server"), 9);
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
				int ReadedSize = recv(Ip.hSocket, Buf, PortionSize, 0);
				if(ReadedSize == -1)
				{
					LastError.SetError(ERR_ARG("Not get data from server"), 10);
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

	inline bool SendAndTakeQuery(String & strQuery, void * Buf, unsigned SizeBuf, unsigned * SizeReaded = NULL)
	{
		if(!SendQuery((void*)strQuery.c_str(), strQuery.length()))
			return false;
		return TakeResponse(Buf, SizeBuf, SizeReaded);
	}

	inline bool SendAndTakeQuery(void * SendBuf, unsigned SizeSendBuf, String & Result)
	{
		if(!SendQuery(SendBuf,SizeSendBuf))
			return false;
		return TakeResponse(Result);
	}

	inline bool SendAndTakeQuery(char * SendStr, String & Result)
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


	inline bool SendAndTakeQuery(String & strQuery, String & Result)
	{
		if(!SendQuery((void*)strQuery.c_str(), strQuery.length()))
			return false;
		return TakeResponse(Result);
	}

};



#endif // QUERYURL_H_INCLUDED
