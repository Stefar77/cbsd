#ifndef NODES_HPP
#define NODES_HPP

#include <map>
#include "node.hpp"
#include "../common/listener.hpp"
#include "../common/sqlite.hpp"


class cbsdModule;
class cbsdNode;
class cbsdNodes {
 friend class cbsdNode;
 friend class cbsdCBSD;
 public:
  cbsdNodes();
  ~cbsdNodes();

  cbsdNode				*Find(const std::string name);

  /* Statics */
  static cbsdSocket			*accept_cb(int fd, SSL *ssl, void *m){ cbsdNodes *self=static_cast<cbsdNodes *>(m); return(self->acceptConnection(fd, ssl)); }
  bool					 transmitRaw(const std::string &data);

 protected:
  cbsdModule				*getModule(uint16_t id){ return(m_modules[id]); }
  void					 PublishRaw(const std::string &data);
  void					 Log(const uint8_t level, const std::string &data);
  void					 Log(const uint8_t level, std::map<std::string,std::string> data);



 private:
  cbsdListener				*m_listener;					// Listener for nodes
  std::map<uint32_t, cbsdNode *>	 m_nodes;					// Nodes.

  cbsdSocket				*acceptConnection(int fd, SSL *ssl);

  std::map<uint16_t, cbsdModule *>	 m_modules;
  std::mutex				 m_mutex;
};

#endif
