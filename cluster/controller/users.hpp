#ifndef USERS_HPP
#define USERS_HPP

#include "user.hpp"

class cbsdUser;
class cbsdUsers {
 public:
	cbsdUsers();
	~cbsdUsers();

	/* Functions/Methods */



	CBSDDBCLASSI(cbsdUser)


 private:
	std::map<uint32_t, cbsdUser *>	m_users;			// Cache and currently active user accounts.


	std::mutex			m_mutex;

};

#endif
