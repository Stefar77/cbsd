#ifndef CBSD_HPP
#define CBSD_HPP

#include <map>
#include <vector>
#include "../common/socket.hpp"
#include "../common/version.hpp"
#include "users.hpp"
#include "nodes.hpp"
#include "module.hpp"
#include "redis.hpp"
#include "../config.hpp"
//#include "jails.hpp"
//#include "tasks.hpp"

#ifdef WITH_REDIS 
#define IFREDIS(...) __VA_ARGS__
#else
#define IFREDIS(...) 
#endif

class cbsdCBSD {
 public:
	cbsdCBSD();
	~cbsdCBSD();

	/* Functions/Methods */

	cbsdNodes			*Nodes(){ return(m_nodes); }
	cbsdUsers			*Users(){ return(m_users); }

	void				 Log(const uint8_t level, const std::string &data);
	void				 Log(const uint8_t level, std::map<std::string,std::string> data);



 private:
	IFREDIS(cbsdRedis	*m_redis;)    		// Global Redis instance
	cbsdNodes		*m_nodes;		// Nodes handler
	cbsdUsers		*m_users;		// Users cache
//	cbsdJails		*m_jails;		// Jails handler
//	cbsdTasks		*m_tasks;		// Tasks handler/cache

};

extern cbsdCBSD *CBSD;

#endif
