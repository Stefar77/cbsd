#ifndef TASK_HPP
#define TASK_HPP

#include "cbsd.hpp"

// extern cbsdLog *Log;

class cbsdTask {
 public:
	cbsdTask(const uint32_t id, cbsdUser *owner, const std::string &name);
	~cbsdTask();


	/* Functions/Methods */
	inline std::string	&getName(){ return(m_name); }
	inline cbsdUser 	*getOwner(){ return(m_owner); }
	inline uint32_t		 getID(){ return(m_id); }
//	bool			 isRunning(){ return(m_is_running); }


 private:
	/* Private functions/Methods */
	void			 Log(const uint8_t level, const std::string &data);
        void			 Log(const uint8_t level, std::map<std::string,std::string> data);


	/* Private Variables */
	uint32_t		 m_id;				// ID number in the database
	std::string		 m_name;			// Loginname of the user
	cbsdUser		*m_owner;			// Who made this task
//	cbsdNode		*m_node;			// Node this task is running on; probably needs to be a vector/map?!

};

#endif
