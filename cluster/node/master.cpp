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
#include "../config.hpp"
extern bool keepRunning;


cbsdMaster::cbsdMaster(): cbsdConnector("Master") {
	m_has_negotiated=false;
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
	_doUnload();							// Stop the thread..
	LOG(cbsdLog::DEBUG) << "Stopping all modules";
	UNLOAD(uint16_t, cbsdModule, m_modules);
	LOG(cbsdLog::DEBUG) << "All modules stopped";


	LOG(cbsdLog::DEBUG) << "Unloading all jail items";
	m_jids.clear();
	UNLOAD(std::string, cbsdJail, m_jails);
	LOG(cbsdLog::DEBUG) << "Unloading jails done";

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

bool cbsdMaster::_doNetInit(){
	std::string ipacket = std::string();
//	std::string mod_list = std::string();

	CNINIT_t	ipkt;
	ipkt.head=1;				// Special header [chan=0, cmd=0, params=1]
	ipkt.version=1;				// Packet version
	ipkt.uptime=cbsdUtils::getUptime(); 	//
	ipkt.memory=cbsdUtils::getMemory(); 	//
	ipkt.cores=cbsdUtils::cpuCores();	//
	ipkt.arch=cbsdUtils::getArch();		//
	ipkt.modules=m_modules.size();		//
	ipkt.jails=0;				//
	ipkt.bhyves=0;				//
	ipacket.append((char *)&ipkt, sizeof(CNINIT_t));

	// Add all module ID's
	for(std::map<uint16_t, cbsdModule *>::iterator it = m_modules.begin(); it != m_modules.end(); it++) 
		ipacket.append((char *)&it->first, 2); 

	/*
		// Add all jails
		//...

		// Add all bhyves
		//...

	*/

//	struct {
//		uint16_t	chan;
//		uint8_t		cmd;		// Channel 0 needs command
//		uint8_t		params;		// # of parameters 
//		uint16_t	param0len;	// Leghth of the first parameter
//						// .. data comes here and may repeat 'params' times
//	} cmd={0,0,1,static_cast<uint16_t>(m_modules.size()*2)}; // CBSDCMD_INIT;
//
//	mod_list.append((char *)&cmd, sizeof(cmd));

//	LOG(cbsdLog::DEBUG) << "Init " << std::to_string(ipkt.modules) << "!";

	// Negotiate modules..
//	for(std::map<uint16_t, cbsdModule *>::iterator it = m_modules.begin(); it != m_modules.end(); it++) mod_list.append((char *)&it->first, 2);



	return(TransmitRaw(ipacket));
}

bool cbsdMaster::doSetup(const std::string &hostname, uint16_t port){
	if(!setupSSL(ClusterCA, NodeCRT, NodeKEY, NodePassword)){ LOG(cbsdLog::FATAL) << "Failed to initialize SSL!"; return(false); }
	if(Connect(hostname, port) && _doNetInit()) return(true); 

	LOG(cbsdLog::FATAL) << "Master failed to connect!"; 
//	if(Connect() && _doNetInit()) return(true);
//	}
	return(false);
}

//bool cbsdMaster::Transmit(const std::string &data){
//	m_server->Transmit(data));
//}

bool cbsdMaster::_handleData(const std::string &data){
	if(data == "--quit"){ keepRunning=false; return(false); }
	if(!m_has_negotiated){
		uint16_t *tmp=(uint16_t *)data.data();
		uint16_t cnt=*tmp;
		if(cnt > 0){		// TODO: FIX ME
			for(int i=0; i<cnt; i++){
				tmp++;
				LOG(cbsdLog::DEBUG) << "Enable module [" << *tmp << "]";
				m_modules[*tmp]->setEnabled(true);
			}
		}
		m_has_negotiated=true;
		return(true);
	}
	uint32_t *ping=(uint32_t *)data.data();
	if(*ping == 0){ TransmitRaw(data); return(true); }	// For now


	LOG(cbsdLog::DEBUG) << "Got data from server [" << data << "]";
	return(true);
}

void cbsdMaster::_Disconnected(){
	m_has_negotiated=false;
	LOG(cbsdLog::DEBUG) << "Got disconnected from server";
}

bool cbsdMaster::_Reconnect(){
	LOG(cbsdLog::DEBUG) << "Trying to reconnect to server";
	if(!Connect()) return(false);
	return(_doNetInit());
}

