#ifndef MASTER_HPP
#define MASTER_HPP

#include <map>
#include "../common/log.hpp"
#include "../common/connector.hpp"
#include "utils.hpp"
#include "module.hpp"				// Jey looped dependency.. :-)
#include "../shared/jail.hpp"			// More loops..

extern cbsdLog *Log;

class cbsdJail;
class cbsdMaster: public cbsdConnector {
 public:
	cbsdMaster();
	~cbsdMaster();

	/* Functions/Methods */
	bool	doSetup(const std::string &hostname, uint16_t port);
	bool	loadModule(cbsdModule *mod);

 protected:


 private:
	bool					_handleData(const std::string &data) override;
	std::map<uint16_t, cbsdModule *>	 m_modules;			// Modules
	std::map<std::string, cbsdJail *>	 m_jails;			// All jails on this host, indexed on global ID number
	std::map<uint32_t, cbsdJail *>		 m_jids;			// All running jails on this host, indexed on JID


};


#endif
