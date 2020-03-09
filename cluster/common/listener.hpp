#ifndef LISTENER_HPP
#define LISTENER_HPP
#include <map>
#include "socket.hpp"


// Threading is needed.
#define THREADING(cmd) cmd 
#include <thread>

class cbsdListener {
 public:
	cbsdListener(cbsdSocket *(*cb)(int, SSL*, void*), void *clss, const uint32_t ip, const uint16_t port, const std::string name);
	~cbsdListener();

	/* Functions/Methods */
	bool				Start();
	bool				setupSSL(const std::string ca, const std::string crt, const std::string key, const std::string pass);

	inline std::string		&getName(){ return(m_name); }

 protected: 


 private:
	/* Functions/Methods */
	void				_handleAccept(int fd);
	void 				_handleDisconnect(int fd);

	inline void			_connectFailed(int fd, SSL *ssl, const std::string &msg){ std::cout << "Connection error: " << msg << "\n"; close(fd); if(m_flag_ssl_ready){ SSL_shutdown(ssl); SSL_free(ssl);  }}
	bool 				_initialize(void);





	/* Private variables */
	cbsdSocket			*(*m_connect_cb_fn)(int, SSL*, void *);		//
	void				*m_connect_cb_cl;		// The class..
	SSL_CTX				*m_ssl_ctx;			// SSL Contex
	int				m_fd;				// Listen socket
	int				m_sync_fd[2];			// Sync sockets
	int				m_kq;				// Kernel Queue
	struct kevent			m_evSet;			//
	struct kevent			m_evList[32];			// Event list

	std::thread			m_listenThread;			// Listen thread
	void				listenThreadHandler(void);
	inline std::thread		listenThreadProc(void){ return std::thread([=] { listenThreadHandler(); }); }

	std::map<int, cbsdSocket *>	m_connections;			// The connections



	std::mutex			m_mutex;			// My queue lock
	std::string			m_name;				// Name of the listener
	char				*m_cert_pass;			//

	uint32_t			m_accept_size;			// Size of accept queue
	uint32_t			m_ip;				// My IP
	uint16_t			m_port;				// My port number
	union {
	 uint16_t			m_flags;			// flags
	 struct{
	  uint16_t			m_flag_reserved:14;		// ...
	  uint16_t			m_flag_ssl_ready:1;		// SSL Initialized
	  uint16_t			m_flag_thread_running:1;	// Thread is running
	 };
	};

};

#endif
