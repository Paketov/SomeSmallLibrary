#ifndef __QUERYURLOPENSSL_H_HAS_INCLUDED__
#define __QUERYURLOPENSSL_H_HAS_INCLUDED__

#	include "ExQueryUrl.h"

#	include <openssl/crypto.h>
#	include <openssl/x509.h>
#	include <openssl/pem.h>
#	include <openssl/ssl.h>
#	include <openssl/err.h>
#	define IS_HAVE_OPEN_SSL


#define	__QUERY_URL_OPEN_SSL_FIELDS \
struct{\
	SSL_CTX*		ctx;\
	SSL*			ssl;\
	decltype(ERR_get_error()) iError;\
}


template<bool = true>
class __QUERY_URL_OPEN_SSL : public QUERY_URL
{

	bool EvntConnect()
	{	
		EvntBeforeClose();
		if(SSLLastError.ctx == nullptr)
		{
			SSLLastError.ctx = SSL_CTX_new(SSLv23_client_method());
			if(SSLLastError.ctx == nullptr)
				goto SSLErrOut;
		}
		SSLLastError.ssl = SSL_new(SSLLastError.ctx);
		if(SSLLastError.ssl == nullptr)
			goto SSLErrOut;

		if(SSL_set_fd(SSLLastError.ssl, Descriptor) == 0)
			goto SSLErrFree;
		if(SSL_connect(SSLLastError.ssl) < 0)
		{
SSLErrFree:
			SSL_free(SSLLastError.ssl);
SSLErrOut:
			SSLLastError.Set();
			QUERY_URL::SetLastErr(EFAULT);
			QUERY_URL::Close();
			return false;
		}
		return true;
	}

	virtual bool EvntBeforeClose()
	{
		if(SSLLastError.ssl != nullptr)
		{
			SSL_shutdown(SSLLastError.ssl);
			SSL_free(SSLLastError.ssl);
			SSLLastError.ssl = nullptr;
		}
		return true;
	}

	virtual bool EvntBeforeShutdown(int)
	{
		if(SSLLastError.ssl != nullptr)
			SSL_shutdown(SSLLastError.ssl);

		return true;
	}

	virtual void EvntUninitFields()
	{
		if(SSLLastError.ssl != nullptr)
		{
			SSL_shutdown(SSLLastError.ssl);
			SSL_free(SSLLastError.ssl);
			SSLLastError.ssl = nullptr;
		}
		if(SSLLastError.ctx != nullptr)
		{
			SSL_CTX_free(SSLLastError.ctx);
			SSLLastError.ctx = nullptr;
		}
	}

	virtual int EvntGetCountPandingData()
	{
		return SSL_pending(SSLLastError.ssl);
	}


	void InitFields()
	{
		SSLLastError.ctx = nullptr;
		SSLLastError.ssl = nullptr;
		SSLLastError.Clear();
	}


public:
	__QUERY_URL_OPEN_SSL()
	{
		SSL_load_error_strings();
		SSL_library_init();
		SSLeay_add_ssl_algorithms();
		InitFields();
	}

	~__QUERY_URL_OPEN_SSL()
	{
		EvntUninitFields();
	}

	union
	{
		class
		{
			friend __QUERY_URL_OPEN_SSL;
			__QUERY_URL_OPEN_SSL_FIELDS;

			decltype(ERR_get_error()) operator=(decltype(ERR_get_error()) NewErr)
			{		
				return iError = NewErr;
			}

			void Set()
			{
				iError = ERR_get_error();
			}
		public:

			int GetNumber()
			{
				return iError;
			}

			operator const char*()
			{
				return ERR_error_string(iError, nullptr);
			}

			void Clear()
			{
				iError = 0;
			}
		} SSLLastError;


		/*
		Info about remote certificate.
		*/
		class REMOTE_CERT
		{
		public:
			union
			{
				class
				{
					friend REMOTE_CERT;
					__QUERY_URL_OPEN_SSL_FIELDS;
				public:
					char * operator()(char * Buf, size_t Len)
					{
						if(ssl == nullptr)
							return nullptr;
						X509* Cert = SSL_get_peer_certificate (ssl);
						if(Cert == nullptr)
							return nullptr;
						char * str = X509_NAME_oneline(X509_get_subject_name(Cert), 0, 0);
						if(str == nullptr)
						{
							X509_free(Cert);
							return nullptr;
						}
						strncpy(Buf, str, Len);
						OPENSSL_free(str);
						X509_free(Cert);
						return Buf;
					}

					operator std::basic_string<char>()
					{
						if(ssl == nullptr)
							return "";
						X509* Cert = SSL_get_peer_certificate (ssl);
						if(Cert == nullptr)
							return "";
						char * str = X509_NAME_oneline(X509_get_subject_name(Cert), 0, 0);
						if(str == nullptr)
						{
							X509_free(Cert);
							return "";
						}
						std::basic_string<char> Ret = str;
						OPENSSL_free(str);
						X509_free(Cert);
						return Ret;
					}
				} SubjectName;

				class
				{
					__QUERY_URL_OPEN_SSL_FIELDS;
				public:

					char * operator()(char * Buf, size_t Len)
					{
						if(ssl == nullptr)
							return nullptr;
						X509* Cert = SSL_get_peer_certificate (ssl);
						if(Cert == nullptr)
							return nullptr;
						char * str = X509_NAME_oneline(X509_get_issuer_name(Cert), 0, 0);
						if(str == nullptr)
						{
							X509_free(Cert);
							return nullptr;
						}
						strncpy(Buf, str, Len);
						OPENSSL_free(str);
						X509_free(Cert);
						return Buf;
					}

					operator std::basic_string<char>()
					{
						if(ssl == nullptr)
							return "";
						X509* Cert = SSL_get_peer_certificate (ssl);
						if(Cert == nullptr)
							return "";
						char * str = X509_NAME_oneline(X509_get_issuer_name(Cert), 0, 0);
						if(str == nullptr)
						{
							X509_free(Cert);
							return "";
						}
						std::basic_string<char> Ret = str;
						OPENSSL_free(str);
						X509_free(Cert);
						return Ret;
					}
				} IssuerName;

				class
				{
					__QUERY_URL_OPEN_SSL_FIELDS;
				public:
					inline operator bool()
					{
						if(ssl == nullptr)
							return false;
						X509* Cert = SSL_get_peer_certificate (ssl);
						if(Cert != nullptr)
						{
							X509_free(Cert);
							return true;
						}
						return false;
					}
				} IsHave;
			};

			inline operator X509*()
			{
				return SSL_get_peer_certificate(SubjectName.ssl);
			}

			static void Free(X509* Cert)
			{
				X509_free(Cert);
			}
		} RemoteHostCertificate;


		/*
		Info about local certificate.
		*/
		class LOCAL_CERT
		{
		public:
			union
			{
				class
				{
					friend LOCAL_CERT;
					__QUERY_URL_OPEN_SSL_FIELDS;
				public:
					char * operator()(char * Buf, size_t Len)
					{
						if(ssl == nullptr)
							return nullptr;
						X509* Cert = SSL_get_certificate(ssl);
						if(Cert == nullptr)
							return nullptr;
						char * str = X509_NAME_oneline(X509_get_subject_name(Cert), 0, 0);
						if(str == nullptr)
						{
							X509_free(Cert);
							return nullptr;
						}
						strncpy(Buf, str, Len);
						OPENSSL_free(str);
						X509_free(Cert);
						return Buf;
					}

					operator std::basic_string<char>()
					{
						if(ssl == nullptr)
							return "";
						X509* Cert = SSL_get_certificate(ssl);
						if(Cert == nullptr)
							return "";
						char * str = X509_NAME_oneline(X509_get_subject_name(Cert), 0, 0);
						if(str == nullptr)
						{
							X509_free(Cert);
							return "";
						}
						std::basic_string<char> Ret = str;
						OPENSSL_free(str);
						X509_free(Cert);
						return Ret;
					}
				} SubjectName;

				class
				{
					__QUERY_URL_OPEN_SSL_FIELDS;
				public:

					char * operator()(char * Buf, size_t Len)
					{
						if(ssl == nullptr)
							return nullptr;
						X509* Cert = SSL_get_certificate(ssl);
						if(Cert == nullptr)
							return nullptr;
						char * str = X509_NAME_oneline(X509_get_issuer_name(Cert), 0, 0);
						if(str == nullptr)
						{
							X509_free(Cert);
							return nullptr;
						}
						strncpy(Buf, str, Len);
						OPENSSL_free(str);
						X509_free(Cert);
						return Buf;
					}

					operator std::basic_string<char>()
					{
						if(ssl == nullptr)
							return "";
						X509* Cert = SSL_get_certificate(ssl);
						if(Cert == nullptr)
							return "";
						char * str = X509_NAME_oneline(X509_get_issuer_name(Cert), 0, 0);
						if(str == nullptr)
						{
							X509_free(Cert);
							return "";
						}
						std::basic_string<char> Ret = str;
						OPENSSL_free(str);
						X509_free(Cert);
						return Ret;
					}
				} IssuerName;

				class
				{
					__QUERY_URL_OPEN_SSL_FIELDS;
				public:
					inline operator bool()
					{
						if(ssl == nullptr)
							return false;
						X509* Cert = SSL_get_certificate(ssl);
						if(Cert != nullptr)
						{
							X509_free(Cert);
							return true;
						}
						return false;
					}
				} IsHave;
			};

			inline operator X509*()
			{
				return SSL_get_certificate(SubjectName.ssl);
			}

			static void Free(X509* Cert)
			{
				X509_free(Cert);
			}
		} LocalHostCertificate;


		class
		{
			__QUERY_URL_OPEN_SSL_FIELDS;
		public:

			operator const char*()
			{
				return SSL_get_cipher(ssl);
			}
		} Cipher;

		class 
		{
			__QUERY_URL_OPEN_SSL_FIELDS;
		public:
			operator long()
			{
				if(ctx == nullptr)
					return -1;
				return SSL_CTX_get_timeout(ctx);
			}
			long operator= (long New)
			{
				if(ctx == nullptr)
					return -1;
				SSL_CTX_set_timeout(ctx, New);
				return New;
			}
		} LifeTimeout;

		/*
		Test is verify.
		*/
		class
		{
			__QUERY_URL_OPEN_SSL_FIELDS;
		public:
			operator bool()
			{
				if(ctx == nullptr)
					return false;

				return SSL_CTX_get_verify_mode(ctx) != SSL_VERIFY_NONE;
			}
		} IsVerify;

		/*
		Get and set verify mode.
		*/
		class
		{
			__QUERY_URL_OPEN_SSL_FIELDS;
		public:
			operator int()
			{
				if(ctx == nullptr)
					return SSL_VERIFY_NONE;
				return SSL_CTX_get_verify_mode(ctx);
			}

			int operator=(int New)
			{
				if(ctx == nullptr)
					return SSL_VERIFY_NONE;
				SSL_CTX_set_verify(ctx, New, nullptr);
				return New;
			}
		} VerifyMode;

		/*
		Get SSL_CTX structure for manualy using.
		*/
		class 
		{
			__QUERY_URL_OPEN_SSL_FIELDS;
		public:

			operator SSL_CTX* ()
			{
				return ctx;
			}
		} CTX;

		/*
		Get ssl structure for manualy using.
		*/
		class
		{
			__QUERY_URL_OPEN_SSL_FIELDS;
		public:
			operator SSL* ()
			{
				return ssl;
			}		
		} SSL;
	};

	/*
	Set special version of ssl.
	*/
	bool InitCTXVersion(const SSL_METHOD* MethodSSL = SSLv23_server_method())
	{
		EvntBeforeClose();
		if(SSLLastError.ctx != nullptr)
			SSL_CTX_free(SSLLastError.ctx);
		SSLLastError.ctx = SSL_CTX_new(MethodSSL);
		if(SSLLastError.ctx == nullptr)
		{
			SSLLastError.Set();
			QUERY_URL::SetLastErr(EFAULT);
			return false;
		}
		return true;
	}


	/*
	Set certificate info. Use for creating server.
	*/
	bool SetLocalCertificate
	(	
		const char * CertFile, 
		const char * PrivateKeyFile = nullptr, 
		int TypeCertFile = SSL_FILETYPE_PEM, 
		int TypeKeyFile = SSL_FILETYPE_PEM,
		const SSL_METHOD* MethodSSL = SSLv23_server_method(),
		bool IsVerifyClient = false,
		const char * CAFile = nullptr,
		const char * CAPath = nullptr,
		int ModeVerify = SSL_VERIFY_PEER,
		int VerifyDepth = 1
	)
	{
		if(!InitCTXVersion(MethodSSL))
			return false;
		if(SSL_CTX_use_certificate_file(SSLLastError.ctx, CertFile, TypeCertFile) <= 0)
		{
lblErrOut:
			SSLLastError.Set();
			SSL_CTX_free(SSLLastError.ctx);
			SSLLastError.ctx = nullptr;
			QUERY_URL::SetLastErr(EFAULT);
			return false;
		}
		if(PrivateKeyFile == nullptr)
			PrivateKeyFile = CertFile;
		if(SSL_CTX_use_PrivateKey_file(SSLLastError.ctx, PrivateKeyFile, TypeKeyFile) <= 0)
			goto lblErrOut;

		if(!SSL_CTX_check_private_key(SSLLastError.ctx))
			goto lblErrOut;
		if(IsVerifyClient)
		{
			if (!SSL_CTX_load_verify_locations(SSLLastError.ctx, CAFile, CAPath)) 
				goto lblErrOut;

			SSL_CTX_set_verify(SSLLastError.ctx, ModeVerify, nullptr);
			SSL_CTX_set_verify_depth(SSLLastError.ctx, VerifyDepth);
		}
	}


	/*
	Accepting client over ssl.
	*/
	bool AcceptClient(__QUERY_URL_OPEN_SSL & DestCoonection)
	{
		DestCoonection.Close();
		if(!QUERY_URL::AcceptClient(DestCoonection))
			return false;	
		DestCoonection.SSLLastError.ssl = SSL_new(SSLLastError.ctx);
		if(DestCoonection.SSLLastError.ssl == nullptr)
		{
			SSLLastError.Set();
			return false;
		}
		if(SSL_set_fd(DestCoonection.SSLLastError.ssl, DestCoonection.Descriptor) == 0)
			goto lblErrOut;
		//https://www.google.ru/?gws_rd=ssl#q=SSL_accept+returned+0
		//http://stackoverflow.com/questions/13855789/ssl-accept-error-on-openssl-examples
		int r = SSL_accept(DestCoonection.SSLLastError.ssl);
		if(r == 0)
		{
			SSLLastError = SSL_get_error(DestCoonection.SSLLastError.ssl, 0);
			goto lblErrOut2;
		}else if(r < 0)
		{
lblErrOut:
			SSLLastError.Set();
lblErrOut2:
			DestCoonection.QUERY_URL::ShutdownSendRecive();
			DestCoonection.QUERY_URL::Close();
			return false;		
		}
		return true;
	}

	virtual int Send(const void * QueryBuf, size_t SizeBuf, int Flags = 0)
	{
		if(SSLLastError.ssl == nullptr)
			goto lblErr;
		int WritenSize;
		if((WritenSize = SSL_write(SSLLastError.ssl, QueryBuf, SizeBuf)) < 0)
		{
			SSLLastError.Set();
lblErr:
			QUERY_URL::SetLastErr(EFAULT);
			return -1;
		}
		return WritenSize;
	}

	virtual int Recive(void * Buf, size_t SizeBuf, int Flags = 0)
	{
		if(SSLLastError.ssl == nullptr)
			goto lblErr;
		int ReadedSize;
		
		if((ReadedSize = ((Flags == MSG_PEEK)? SSL_peek: SSL_read)(SSLLastError.ssl, Buf, SizeBuf)) < 0)
		{
			SSLLastError.Set();
lblErr:
			QUERY_URL::SetLastErr(EFAULT);
			return -1;
		}
		return ReadedSize;
	}

	virtual int Recive(std::basic_string<char> & StrBuf, int Flags = 0)
	{
		if(SSLLastError.ssl == nullptr)
			goto lblErr;
		char * Buf;
		unsigned CurSize = 0, CountBytesInBuff = 0;
		CountBytesInBuff = SSL_pending(SSLLastError.ssl);
		if(CountBytesInBuff < 50)
			CountBytesInBuff = 50;
		StrBuf.resize(CountBytesInBuff + 2);
		Buf = (char*)StrBuf.c_str();
		while(true)
		{
			int ReadedSize = SSL_read(SSLLastError.ssl, Buf, CountBytesInBuff);
			if(ReadedSize < 0)
			{
				SSLLastError.Set();
lblErr:
				QUERY_URL::SetLastErr(EFAULT);
				return -1;
			}else if(ReadedSize == 0)
				break;
			else
			{
				CurSize += ReadedSize;
				CountBytesInBuff = SSL_pending(SSLLastError.ssl);
				if(CountBytesInBuff == 0)
					CountBytesInBuff = 50;
				StrBuf.append("", CountBytesInBuff);
				Buf = (char*)StrBuf.c_str() + CurSize;
			}
		}
		*Buf = '\0';
		return CurSize;
	}


};

typedef __QUERY_URL_OPEN_SSL<true>  QUERY_URL_OPEN_SSL;

#endif
