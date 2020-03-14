#ifndef NODES_HPP
#define NODES_HPP

#include <map>
#include "node.hpp"
#include "../common/listener.hpp"
#include "../common/sqlite.hpp"

#define NODES_MAP std::map<uint32_t, cbsdNode *>
#define FOREACH_NODES for (NODES_MAP::iterator it = m_nodes.begin(); it != m_nodes.end(); it++)

class cbsdModule;
class cbsdNode;
class cbsdNodes {
 friend class cbsdNode;
 friend class cbsdCBSD;
 public:
  cbsdNodes();
  ~cbsdNodes();

  /* Statics */
  static cbsdSocket			*accept_cb(int fd, SSL *ssl, const std::string &n, void *m){ cbsdNodes *self=static_cast<cbsdNodes *>(m); return(self->acceptConnection(fd, ssl, n)); }
  bool					 transmitRaw(const std::string &data);




  CBSDDBCLASSI(cbsdNode)		// Switches to protected
  bool					Init(const std::string &CA, const std::string &CRT, const std::string &KEY, const std::string &PW);

  cbsdModule				*getModule(uint16_t id){ std::map<uint16_t, cbsdModule *>::iterator it = m_modules.find(id); if(it != m_modules.end()) return(it->second); return(NULL);  }


 private:
  cbsdListener				*m_listener;					// The listener for nodes
  NODES_MAP				 m_nodes;					// The nodes in our cluster
  MODULES_MAP				 m_modules;					// Nodes have modules we need the counterpart
//OBJ_STORAGE				 m_storage;					// Where do we store/get these items

  IFREDIS(cbsdNode 			*_fetchFromRedis(const std::string &name);)

  cbsdSocket				*acceptConnection(int fd, SSL *ssl, const std::string &name);	// Event when we have a new connection.

  std::thread				 m_threadID;                     // ThreadID for the modules thread..
  void					 threadHandler(void);

  union {
   uint32_t				 m_flags;
   struct {
    uint32_t				 m_flags_reserved:29;
    uint32_t				 m_do_redis_updates:1;		// Update Redis objects perf-data in somewhat real-time
    uint32_t				 m_is_thread_stopping:1;	// Are we stopping the thread?
    uint32_t				 m_is_thread_running:1;		// Are we running our thread?
   };
  };
  std::mutex				 m_mutex;					// Lock for threading
};

#endif
