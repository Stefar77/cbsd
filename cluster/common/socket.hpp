#ifndef SOCKET_HPP
#define SOCKET_HPP
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/event.h>                  // kqueue
#include <netinet/in.h>
#include <unistd.h>                     // close etc
#include <openssl/ssl.h>                // SSL
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <fcntl.h>                      // f_set...
#include "log.hpp"


#define THREADING(cmd) cmd 
#include <thread>

class cbsdSocket {
 friend class cbsdListener;
 friend class cbsdModule;
 public:
	cbsdSocket();
	virtual ~cbsdSocket();

	enum { 
		OPEN=0,			// Opened socket
		SSSL,			// Start SSL
		READ,			// Read event
		WRITE,			// Ready to write more
		CLOSE			// Close event
	};

	/* Functions/Methods */

 protected:
        virtual bool 	isPersistent();
	void		socketEvent(const uint8_t ev, void *opt);
	void		socketEvent(const uint8_t ev){ socketEvent(ev, NULL); }
	bool		transmitRaw(const std::string &data);
	void 		doDisconnect();



 private:
	/* Functions/Methods */

        virtual void	_hasConnected();
        virtual void	_hasDisconnected();
        virtual void	_hasData(const std::string &data);
        virtual void	_readyForData();

	/* Private variables */
	int				m_fd;			// Unsecure socket
	SSL				*m_ssl;			// SSL Socket
        std::mutex			m_transmit_mutex;       // My transmission lock


//	std::mutex			m_mutex;		// My queue lock




};

#endif
