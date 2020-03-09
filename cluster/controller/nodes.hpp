#ifndef NODES_HPP
#define NODES_HPP

#include <map>
#include "node.hpp"
#include "../common/listener.hpp"

class cbsdNodes {
 public:
  cbsdNodes();
  ~cbsdNodes();

  cbsdNode				*Find(const std::string name);

  /* Statics */
  static cbsdSocket			*accept_cb(int fd, SSL *ssl, void *m){ cbsdNodes *self=static_cast<cbsdNodes *>(m); return(self->acceptConnection(fd, ssl)); }

 protected:

 private:
  cbsdListener				*m_listener;
  std::map<uint32_t, cbsdNode *>	m_nodes;

  cbsdSocket				*acceptConnection(int fd, SSL *ssl);


  std::mutex				m_mutex;
};

#endif
