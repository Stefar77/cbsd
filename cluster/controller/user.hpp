#ifndef USER_HPP
#define USER_HPP

#include <vector>
#include "cbsd.hpp"

extern cbsdLog *Log;
class cbsdTask;
class cbsdUser {
 public:
	cbsdUser(const uint32_t id, const std::string &name);
	~cbsdUser();



	/* Functions/Methods */
	inline std::string		&getName(){ return(m_name); }
	bool				 isConnected(){ return(m_connections.size() > 0); }

	/* Linking stuff for dynamic loading/unloading etc */
	bool				 linkItem(cbsdTask *task);
	bool				 unlinkItem(cbsdTask *task);

 private:
	CBSDDBITEMI(cbsdUser)						// Generic stuff

	uint32_t			 m_id;				// ID number in the database
	std::string			 m_name;			// Loginname of the user

	std::vector<cbsdSocket *>	 m_connections;			// Current connections
	std::map<uint32_t, cbsdTask *>	 m_tasks;			// Current loaded/running tasks

};

#endif
