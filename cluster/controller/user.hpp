#ifndef USER_HPP
#define USER_HPP

#include "../common/log.hpp"
#include "../common/socket.hpp"
extern cbsdLog *Log;
#include <vector>

class cbsdUser {
 public:
	cbsdUser(const uint32_t id, const std::string &name);
	~cbsdUser();



	/* Functions/Methods */
	inline std::string		&getName(){ return(m_name); }
	bool				isConnected(){ return(m_connections.size() > 0); }



 private:
	uint32_t		m_id;				// ID number in the database
	std::string		m_name;				// Loginname of the user

	std::vector<cbsdSocket *> m_connections;		//


};

#endif
