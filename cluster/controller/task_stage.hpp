#ifndef TASK_STAGE_HPP
#define TASK_STAGE_HPP

#include "cbsd.hpp"

// extern cbsdLog *Log;

class cbsdTaskStage {
 public:
	cbsdTaskStage(cbsdTask *task, const std::string &name, const std::string &command);
	~cbsdTaskStage();


	/* Functions/Methods */
	inline std::string	&getName(){ return(m_name); }
	inline cbsdTask 	*getTask(){ return(m_task); }
	bool		 	addNode(cbsdNode *node);			// Add a node we run this on
	bool		 	addJail(cbsdJail *jail);			// Add a jail we run this on
	bool		 	setJump(int code, uint8_t rule_nr);		// Jump to rule # on return-code

	enum { CREATED=0, IDLE, SCHEDULED, WAITING, RUNNING, FINISHED, FAILED, DELETED };


 private:
	/* Private functions/Methods */
	void			 Log(const uint8_t level, const std::string &data);
        void			 Log(const uint8_t level, std::map<std::string,std::string> data);


	/* Private Variables */
	std::string		 m_name;			// Name of this stage
	std::string		 m_command;			// Command we need to run
	cbsdTask		*m_task;			// Where do we belong?
	std::vector<cbsdNode *>  m_nodes;			// Nodes we need to run on
	std::vector<cbsdJail *>  m_jails;			// Jails we need to run on

	int			 m_retcode;			// Last return code

	union {
		uint32_t	 m_flags;			// General flags
		struct {
		 uint32_t	 m_has_error:1;			// We had an error in the last run

		 uint32_t	 m_unused_flags:18;		// Not used (yet)

		 uint32_t	 m_stage_id:8;			// My ID number
		 uint32_t	 m_is_runnning:1;		// Currently running
		 uint32_t	 m_is_enabled:1;		// Currently runnable
		 uint32_t	 m_state:3;			// IDLE, RUNNING, etc
		};
	};
};

#endif
