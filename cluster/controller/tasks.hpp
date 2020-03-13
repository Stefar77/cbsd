#ifndef TASKS_HPP
#define TASKS_HPP

#include "task.hpp"

class cbsdTask;
class cbsdTasks {
 public:
	cbsdTasks();
	~cbsdTasks();

	/* Functions/Methods */




	CBSDDBCLASSI(cbsdTask)

 private:
	std::map<uint32_t, cbsdTask *>	m_tasks;			// Cache and currently active tasks



	std::mutex			m_mutex;
};

#endif
