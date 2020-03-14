#ifndef TASK_HPP
#define TASK_HPP

#include "cbsd.hpp"

//#define STAGE_MAP std::map<uint8_t, cbsdTaskStage>

class cbsdTask {
 public:
	cbsdTask(const uint32_t id, cbsdUser *owner, const std::string &name);
	~cbsdTask();


	/* Functions/Methods */
	inline std::string	&getName(){ return(m_name); }
	inline cbsdUser 	*getOwner(){ return(m_owner); }
	inline uint32_t		 getID(){ return(m_id); }
//	bool			 isRunning(){ return(m_is_running); }

	enum { CREATED=0, IDLE, SCHEDULED, WAITING, RUNNING, FINISHED, FAILED, DELETED };



 private:
	/* Private functions/Methods */
	void			 Log(const uint8_t level, const std::string &data);
        void			 Log(const uint8_t level, std::map<std::string,std::string> data);


	/* Private Variables */
	uint32_t		 m_id;				// ID number in the database
	std::string		 m_name;			// Loginname of the user
	cbsdUser		*m_owner;			// Who made this task


//	STAGE_MAP		 m_stages;

	union {
		uint32_t	 m_config;			// Config [more static] flags
		struct {
		 uint32_t	 m_has_runned_before:1;		// Not the first time we did this dance!

		 uint32_t	 m_unused_config:27;		// Not used (yet)

		 uint32_t	 m_task_repeats:1;		// Do not delete task when done
		 uint32_t	 m_allow_emulated:1;		// Allowed to run on emulated arch
		 uint32_t	 m_runs_on_jails:1;		// Needs to run on jail[s]
		 uint32_t	 m_runs_on_nodes:1;		// Needs to run on node[s]
		};
	};
	union {
		uint32_t	 m_flags;			// General flags
		struct {
		 uint32_t	 m_has_error:1;			// We had an error in the last run

		 uint32_t	 m_unused_flags:18;		// Not used (yet)

		 uint32_t	 m_current_stage:8;		// Task can have max 255 subtasks

		 uint32_t	 m_is_runnning:1;		// Currently running
		 uint32_t	 m_is_enabled:1;		// Currently runnable
		 uint32_t	 m_state:3;			// IDLE, RUNNING, etc
		};
	};
};

#endif
