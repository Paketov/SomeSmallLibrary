#ifndef __QUERYURLOPENSSL_H_HAS_INCLUDED__
#define __QUERYURLOPENSSL_H_HAS_INCLUDED__

#	include "ExQueryUrl.h"

#	include <openssl/crypto.h>
#	include <openssl/x509.h>
#	include <openssl/pem.h>
#	include <openssl/ssl.h>
#	include <openssl/err.h>


#define	__QUERY_URL_OPEN_SSL_FIELDS \
struct{\
	SSL_CTX*		ctx;\
	SSL*			ssl;\
	decltype(ERR_get_error()) iError;\
}


class QUERY_URL_OPEN_SSL : public virtual QUERY_URL
{
	bool EvntConnect();
	virtual bool EvntBeforeClose();
	virtual bool EvntBeforeShutdown(int);
	void InitFields();

	class REMOTE_CERT
	{
		class _SUBJECT_NAME
		{
			friend REMOTE_CERT;
			__QUERY_URL_OPEN_SSL_FIELDS;
		public:
			char* operator()(char * Buf, size_t Len);
			operator std::basic_string<char>();
		};

		class _ISSUER_NAME
		{
			__QUERY_URL_OPEN_SSL_FIELDS;
		public:
			char* operator()(char * Buf, size_t Len);
			operator std::basic_string<char>();
		};

		class _IS_HAVE
		{
			__QUERY_URL_OPEN_SSL_FIELDS; public:
		operator bool();
		};
	public:
		union
		{
			_SUBJECT_NAME SubjectName;
			_ISSUER_NAME IssuerName;
			_IS_HAVE IsHave;
		};
		inline operator X509*() { return SSL_get_peer_certificate(SubjectName.ssl); }
		static inline void Free(X509* Cert) { X509_free(Cert); }
	};

	class LOCAL_CERT
	{
		class _SUBJECT_NAME
		{
			friend LOCAL_CERT;
			__QUERY_URL_OPEN_SSL_FIELDS;
		public:
			char * operator()(char * Buf, size_t Len);
			operator std::basic_string<char>();
		};

		class _ISSUER_NAME
		{
			__QUERY_URL_OPEN_SSL_FIELDS;
		public:
			char * operator()(char * Buf, size_t Len);
			operator std::basic_string<char>();
		};

		class _IS_HAVE
		{
			__QUERY_URL_OPEN_SSL_FIELDS;
		public:
			operator bool();
		};
	public:
		union
		{
			_SUBJECT_NAME SubjectName;
			_ISSUER_NAME IssuerName;
			_IS_HAVE IsHave;
		};

		inline operator X509*() { return SSL_get_certificate(SubjectName.ssl); }
		static inline void Free(X509* Cert) { X509_free(Cert); }
	};

	class _LIFE_TIMEOUT
	{
		__QUERY_URL_OPEN_SSL_FIELDS;
	public:
		operator long();
		long operator= (long New);
	};

	class _IS_VER
	{
		__QUERY_URL_OPEN_SSL_FIELDS;
	public:
		operator bool();
	};

	class _VER_MODE
	{
		__QUERY_URL_OPEN_SSL_FIELDS;
	public:
		operator int();
		int operator=(int New);
	};
public:
	QUERY_URL_OPEN_SSL();
	virtual ~QUERY_URL_OPEN_SSL();

	union
	{
		class
		{
			friend QUERY_URL_OPEN_SSL;
			__QUERY_URL_OPEN_SSL_FIELDS;
			inline decltype(ERR_get_error()) operator=(decltype(ERR_get_error()) NewErr) { return iError = NewErr; }
			inline void Set() { iError = ERR_get_error(); }
		public:
			inline int GetNumber() const { return iError; }
			inline operator const char*() { return ERR_error_string(iError, nullptr); }
			inline void Clear() { iError = 0; }
		} SSLLastError;


		/*
		Info about remote certificate.
		*/
		REMOTE_CERT RemoteHostCertificate;

		/*
		Info about local certificate.
		*/
		LOCAL_CERT LocalHostCertificate;


		class
		{
			__QUERY_URL_OPEN_SSL_FIELDS;
		public:
			inline operator const char*() { return SSL_get_cipher(ssl); }
		} Cipher;

		_LIFE_TIMEOUT  LifeTimeout;

		/*
		Test is verify.
		*/
		_IS_VER IsVerify;

		/*
		Get and set verify mode.
		*/
		_VER_MODE VerifyMode;

		/*
		Get SSL_CTX structure for manualy using.
		*/
		class 
		{
			__QUERY_URL_OPEN_SSL_FIELDS;
		public:
			inline operator SSL_CTX*() const { return ctx; }
		} CTX;

		/*
		Get ssl structure for manualy using.
		*/
		class
		{
			__QUERY_URL_OPEN_SSL_FIELDS;
		public:
			inline operator SSL*() const { return ssl; }		
		} SSL;
	};

	/*
	Set special version of ssl.
	*/
	bool InitCTXVersion(const SSL_METHOD* MethodSSL = SSLv23_server_method());


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
		);


	/*
	Accepting client over ssl.
	*/
	virtual bool AcceptClient(QUERY_URL_OPEN_SSL & DestCoonection);
	virtual int Send(const void * QueryBuf, size_t SizeBuf, int Flags = 0);
	virtual int Recive(void * Buf, size_t SizeBuf, int Flags = 0);
	virtual int Recive
		(
		std::basic_string<char>& StrBuf, 
		std::basic_string<char>::size_type MaxLen = std::numeric_limits<std::basic_string<char>::size_type>::max(), 
		int Flags = 0
		);
	virtual long long SendFile(QUERY_URL& InSocket, size_t Count);
	virtual long long SendFile(TDESCR InFileDescriptor, size_t Count, off_t Offset = 0);
};



#endif
