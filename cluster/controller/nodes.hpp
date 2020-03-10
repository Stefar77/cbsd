#ifndef NODES_HPP
#define NODES_HPP

#include <map>
#include "node.hpp"
#include "../common/listener.hpp"

class cbsdModule;
class cbsdNode;
class cbsdNodes {
 friend class cbsdNode;
 public:
  cbsdNodes();
  ~cbsdNodes();

  cbsdNode				*Find(const std::string name);

  /* Statics */
  static cbsdSocket			*accept_cb(int fd, SSL *ssl, void *m){ cbsdNodes *self=static_cast<cbsdNodes *>(m); return(self->acceptConnection(fd, ssl)); }
  bool					 transmitRaw(const std::string &data);

 protected:
  cbsdModule				*getModule(uint16_t id){ return(m_modules[id]); }

 private:
  cbsdListener				*m_listener;
  std::map<uint32_t, cbsdNode *>	m_nodes;

  cbsdSocket				*acceptConnection(int fd, SSL *ssl);

  std::map<uint16_t, cbsdModule *>	m_modules;
  std::mutex				m_mutex;
};

#endif
