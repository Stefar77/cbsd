#ifndef LISTENER_HPP
#define LISTENER_HPP
#include <map>
#include "socket.hpp"

#define SOCKET_MAP	std::map<int, cbsdSocket*>
#define FOREACH_SOCKET  for (SOCKET_MAP::iterator it = m_connections.begin(); it != m_connections.end(); it++)
#define SOCKET_CB(name)	cbsdSocket *(*m_connect_cb##name)(int, SSL*, const std::string &, void*)

// Threading is needed.
#define THREADING(cmd) cmd 
#include <thread>
#include "macros.hpp"

class cbsdListener {
 public:
	cbsdListener(SOCKET_CB(), void *clss, const uint32_t ip, const uint16_t port, const std::string &name);
	~cbsdListener();

	/* Functions/Methods */
	bool				setupSSL(const std::string &ca, const std::string &crt, const std::string &key, const std::string &pass);

	inline std::string		&getName(){ return(m_name); }

 protected: 

 private:
	/* Private functions/Methods */
	void				_handleAccept(int fd);
	void 				_handleDisconnect(int fd);
	bool 				_initialize(void);
	inline void			_connectFailed(int fd, SSL *ssl, const std::string &msg);


	/* Private variables */
	SOCKET_CB(_fn);
	//cbsdSocket			*(*m_connect_cb_fn)(int, SSL*, const std::string &, void *);		//
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

	SOCKET_MAP			m_connections;			// The connections



	std::mutex			m_mutex;			// My queue lock
	std::string			m_name;				// Name of the listener
	char				*m_cert_pass;			//

	uint32_t			m_accept_size;			// Size of accept queue
	uint32_t			m_ip;				// My IP
	uint16_t			m_port;				// My port number
	union {
	 uint16_t			m_flags;			// flags
	 struct{
	  uint16_t			m_flag_reserved:13;		// ...
	  uint16_t			m_is_accepting:1;		// We are not accepting any new connections
	  uint16_t			m_is_ssl_ready:1;		// SSL Initialized
	  uint16_t			m_is_thread_running:1;		// Thread is running
	 };
	};

};

#endif
