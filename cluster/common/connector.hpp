#ifndef CONNECTOR_HPP
#define CONNECTOR_HPP
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/event.h>                  // kqueue
#include <netinet/in.h>
#include <unistd.h>                     // close etc
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include "socket.hpp"
#include <arpa/inet.h>


class cbsdConnector {
 public:
	cbsdConnector(const std::string &name);
	~cbsdConnector();

	/* Functions/Methods */
	bool				Connect(const std::string &hostname, const uint16_t port);
	bool				setupSSL(const std::string &ca, const std::string &cert, const std::string &key, const std::string &pass);
	bool 				Transmit(const std::string &data);


 protected:


 private:
	/* Functions/Methods */

	/* Private variables */
	std::string			m_name;                         // Name of the listener
        char				*m_cert_pass;                   // Password to open private key

	SSL_CTX				*m_ssl_ctx;		// SSL Context
	SSL				*m_ssl;			// SSL Connection
	int				m_sock;

//	std::thread			threadID;
//	void				threadHandler(void);
//	inline std::thread		threadHandlerProc(void){return std::thread([=] { threadHandler(); });}

	inline bool 			_ConnectFailed();

//	std::mutex			m_mutex;		// My queue lock


	union {
	 uint16_t                       m_flags;                        // flags
	 struct{
	  uint16_t                      m_flag_reserved:15;             // ...
	  uint16_t                      m_flag_ssl_ready:1;             // SSL Initialized
	 };
	};


};

#endif
