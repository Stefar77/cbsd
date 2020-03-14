#ifndef USERS_HPP
#define USERS_HPP

#include "user.hpp"

#define USERS_MAP std::map<uint32_t, cbsdUser *>
#define FOREACH_USERS for(USERS_MAP::iterator it = m_users.begin(); it != m_users.end(); it++)

class cbsdUser;
class cbsdUsers {
 friend class cbsdUser;
 public:
	cbsdUsers();
	~cbsdUsers();

	/* Functions/Methods */



	CBSDDBCLASSI(cbsdUser)


 private:
	USERS_MAP			m_users;			// Cache and currently active user accounts.
//	OBJ_STORAGE			m_storage;			// Where do we store/get these items

	std::mutex			m_mutex;

};

#endif
