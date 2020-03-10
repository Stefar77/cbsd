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
 friend class cbsdMaster;
 public:
	cbsdConnector(const std::string &name);
	virtual ~cbsdConnector();

	/* Functions/Methods */
	bool				Connect(const std::string &hostname, const uint16_t port);
	bool				setupSSL(const std::string &ca, const std::string &cert, const std::string &key, const std::string &pass);
	bool 				TransmitRaw(const std::string &data);
	bool				isConnected(){ return(m_is_connected); }
	void 				Disconnect();

 protected:
	void				_doUnload();

 private:
	/* Functions/Methods */
	void				threadHandler(void);
	inline std::thread		threadHandlerProc(void){return std::thread([=] { threadHandler(); });}
	inline bool 			_ConnectFailed();
	virtual bool			_handleData(const std::string &data)=0;
	

	/* Private variables */
	std::string			m_name;                         // Name of the listener
	
        char				*m_cert_pass;                   // Password to open private key

	SSL_CTX				*m_ssl_ctx;		// SSL Context
	SSL				*m_ssl;			// SSL Connection
	int				m_sock;
	int				m_sync_fd[2];
	int                             m_kq;                           // Kernel Queue
        struct kevent                   m_evSet;                        //
        struct kevent                   m_evList[32];                   // Event list

	std::thread			threadID;

//	std::mutex			m_mutex;			// My queue lock


	union {
	 uint16_t                       m_flags;                        // flags
	 struct{
	  uint16_t                      m_flag_reserved:12;		// ...
	  uint16_t                      m_is_thread_stopping:1;		// SSL Initialized
	  uint16_t                      m_is_thread_running:1;		// SSL Initialized
	  uint16_t                      m_is_connected:1;		// Connected
	  uint16_t                      m_is_ssl_ready:1;		// SSL Initialized
	 };
	};


};

#endif
