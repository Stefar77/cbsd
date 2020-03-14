#ifndef TASKS_HPP
#define TASKS_HPP

#include "task.hpp"

#define TASKS_MAP std::map<uint32_t, cbsdTask *>
#define FOREACH_TASKS for (TASKS_MAP::iterator it = m_tasks.begin(); it != m_tasks.end(); it++)

class cbsdTask;
class cbsdTasks {
 public:
	cbsdTasks();
	~cbsdTasks();

	/* Functions/Methods */




	CBSDDBCLASSI(cbsdTask)

 private:
	TASKS_MAP			m_tasks;			// Cache and currently active tasks
//	OBJ_STORAGE			m_storage;			// Where do we store/get these items


	std::mutex			m_mutex;			// Our general lock
};

#endif
