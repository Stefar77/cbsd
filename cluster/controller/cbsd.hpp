#ifndef CBSD_HPP
#define CBSD_HPP

#include<ctime>
#include <map>
#include <vector>
#include "../common/socket.hpp"
#include "../common/version.hpp"
#include "../common/macros.hpp"
#ifdef WITH_REDIS
#include "connectors/redis.hpp"
#define IFREDIS(...) __VA_ARGS__
#else
#define IFREDIS(...)
#endif
#include "users.hpp"
#include "tasks.hpp"
#include "module.hpp"
#include "nodes.hpp"
#include "../config.hpp"
//#include "jails.hpp"



class cbsdCBSD {
 public:
	cbsdCBSD(const std::string &name);
	~cbsdCBSD();

	/* Functions/Methods */
	bool 				 Init();


	IFREDIS(inline cbsdRedis	*Redis(){ return(m_redis); })
	inline cbsdNodes		*Nodes(){ return(m_nodes); }
	inline cbsdUsers		*Users(){ return(m_users); }
	inline cbsdTasks		*Tasks(){ return(m_tasks); }

	void				 Log(const uint8_t level, const std::string &data);
	void				 Log(const uint8_t level, std::map<std::string,std::string> data);
	void				 Publish(const std::string &key, const std::string &val);
	void				 Publish(std::map<std::string,std::string> data);



 private:
	std::string 		m_name;			// Controller name
	IFREDIS(cbsdRedis	*m_redis;)    		// Global Redis instance
	cbsdNodes		*m_nodes;		// Nodes handler
	cbsdUsers		*m_users;		// Users cache
//	cbsdJails		*m_jails;		// Jails handler
	cbsdTasks		*m_tasks;		// Tasks handler/cache


	std::mutex		 m_mutex;		// My lock
};

extern cbsdCBSD *CBSD;

#endif
