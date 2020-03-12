#ifndef USERS_HPP
#define USERS_HPP

#include "user.hpp"

class cbsdUser;
class cbsdUsers {
 public:
	cbsdUsers();
	~cbsdUsers();

	/* Functions/Methods */
	cbsdUser	*find(const std::string &name);
	cbsdUser	*find(const uint32_t id);

 protected:
	void		 Log(const uint8_t level, const std::string &data);
	void		 Log(const uint8_t level, std::map<std::string,std::string> data);


 private:
	std::map<uint32_t, cbsdUser *>	m_users;			// Cache and currently active user accounts.


	std::mutex			m_mutex;

};

#endif
