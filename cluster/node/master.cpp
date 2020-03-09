/* master.cpp - Master class for CBSD Cluster Node
 *
 * Copyright (c) 2020, Stefan Rink <stefanrink at yahoo dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * -------------------------------------------------------------------------------
 * 
 *  Connects to the controller and has modules that do stuff..
 *
 */

#include <iostream>		// std::cout 
#include "master.hpp"

cbsdMaster::cbsdMaster() {
	m_server=NULL;
	m_jails.clear();
	m_jids.clear();
	m_modules.clear();


	std::map<uint32_t, std::string> jails=cbsdUtils::getRunningJails();
	for(std::map<uint32_t, std::string>::iterator it = jails.begin(); it != jails.end(); it++){
		std::cout << "jail " << it->first << " - " << it->second << "\n";
	}



	LOG(cbsdLog::DEBUG) << "Master loaded";

}

#define UNLOAD(mapkey, type, name) \
	for(std::map<mapkey, type *>::iterator it = name.begin(); it != name.end(); it++){			\
		type *item=it->second;										\
		item->doUnload();										\
		delete item;											\
	}													\
	name.clear() 


cbsdMaster::~cbsdMaster() {
	LOG(cbsdLog::DEBUG) << "Stopping all modules";
	UNLOAD(uint16_t, cbsdModule, m_modules);
	LOG(cbsdLog::DEBUG) << "All modules stopped";


	LOG(cbsdLog::DEBUG) << "Unloading all jail items";
	m_jids.clear();
	UNLOAD(std::string, cbsdJail, m_jails);
	LOG(cbsdLog::DEBUG) << "Unloading jails done";

	if(m_server) delete m_server;

	LOG(cbsdLog::DEBUG) << "Master unloaded";
}


bool cbsdMaster::loadModule(cbsdModule *mod){
	// Only load module we didn't already load...
	std::map<uint16_t, cbsdModule *>::iterator it=m_modules.find(mod->getID());
	if(it == m_modules.end()){
		if(mod->doLoad(this)){
			m_modules[mod->getID()]=mod;
			return(true);
		}
	}
	LOG(cbsdLog::WARNING) << "Module '" << mod->getName() << "' failed to load!";

	delete mod;
	return(false);
}

bool cbsdMaster::addController(const std::string &hostname, uint16_t port){
	if(m_server) return(false);
	m_server = new cbsdConnector("Controller");
	if(!m_server->setupSSL("/etc/ssl/clusterca.crt", "/etc/ssl/GUI.crt", "/etc/ssl/GUI.key", "geheim")){
		delete m_server;
		return(false);
	}

	if(!m_server->Connect(hostname, port)){
		delete m_server;
		return(false);
	}


	std::string mod_list = std::string();
	struct {
		uint16_t	chan;
		uint8_t		cmd;		// Channel 0 needs command
		uint8_t		params;		// # of parameters 
		uint16_t	param0len;	// Leghth of the first parameter
						// .. data comes here and may repeat 'params' times
	} cmd={0,0,1,static_cast<uint16_t>(m_modules.size()*2)}; // CBSDCMD_INIT;

	mod_list.append((char *)&cmd, sizeof(cmd));

	LOG(cbsdLog::DEBUG) << "Init " << std::to_string(cmd.param0len) << "!";

	// Negotiate modules..
	for(std::map<uint16_t, cbsdModule *>::iterator it = m_modules.begin(); it != m_modules.end(); it++) mod_list.append((char *)&it->first, 2);
	Transmit(mod_list);

	return(true);
}

bool cbsdMaster::Transmit(const std::string &data){
	if(!m_server) return(false);

	return(m_server->Transmit(data));
}
