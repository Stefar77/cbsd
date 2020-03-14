/* node.cpp - Node class for CBSD Cluster Daemon
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
 */

#include "cbsd.hpp"
#include "../common/structs.hpp"

cbsdNode::cbsdNode(const uint32_t id, const std::string name) {
	m_id=id;				// ID number in the database
	m_name=name;				// Name of the node
	m_flags=0;				// Clear all flags.
	m_configuration=0;			// Unknown arch, mem, etc
	m_performance=0;			// We know nothing yet...
	m_jails.clear();			// We start clean
	m_modules.clear();			// ...

	IFDEBUG(Log(cbsdLog::DEBUG, "Node loaded");)
}

cbsdNode::~cbsdNode() {
	IFDEBUG(Log(cbsdLog::DEBUG, "Node unloaded");)
}

CBSDDBITEM(cbsdNode, "node", CBSD->Nodes())


/*********** EVENTS ************/

void cbsdNode::_hasConnected(){
	Log(cbsdLog::INFO, "Node has connected");

	m_last_seen=std::time(0);

	Publish("state","up");
	m_is_connected=true;
}

void cbsdNode::_hasDisconnected(){
	Log(cbsdLog::INFO, "Node has disconnected");

	m_modules.clear();				//      We renegotiate this when it reconnects!

	Publish("state","disconnected");		//	TODO: Change this!

	m_is_connected=false;				//	state&=~NODE_IS_CONNECTED;
	m_is_authenticated=false;			//	state&=~NODE_IS_AUTHENTICATED;
	m_has_negotiated=false;				//	Renegotiate when it logs in again
	if(!m_is_maintenance){				//	
		m_has_warning=true;			//	If not in maintenance warn somebody..
//		m_last_warning=WARN_DISCONNECTED;	//	Maybe do this?!
	}

}

bool cbsdNode::_doSysOp(std::string &data){
	Log(cbsdLog::DEBUG, "Input sys_op: [" + data + "]");
	return(false);
}

bool cbsdNode::_doAuth(std::string &data, const uint16_t channel){
	uint8_t *cmd=(uint8_t *)data.data()+2;				// Command
	uint8_t *params=(uint8_t *)data.data()+3;			// # parameters to expect
	uint16_t *tmp=(uint16_t *)((uint8_t *)data.data()+4);		// Pointer to size of first parameter

	if(!m_has_negotiated){
		if(*cmd != 0 || channel != 0){
			Log(cbsdLog::DEBUG, "Invalid negotiation");
			return(false);	// TODO!!
		}
	}


	switch(*cmd){					// Warning; Below is where fairies get murdered!
							// Temporary stuff / needs cleanup
		case 0:{
				if(*params != 1){	// Currently we only have 1 parameter and that is an array of uint16's (mod_id's)
					Log(cbsdLog::WARNING, "Node is signing on with invalid parameters");
					break;
				}
				if(data.size()-6 < *tmp){
					Log(cbsdLog::WARNING, "Node is sending incomplete packages");
					return(false);
				} else {	// Just acts as a container; Because I want to use a local var in this switch statement...

					uint16_t items=*tmp/2;
					for(uint16_t i=1; i<=items; i++){
						tmp++; 
						cbsdModule *module=CBSD->Nodes()->getModule(*tmp);
						if(NULL == module){
							Log(cbsdLog::DEBUG, "Node has module " + std::to_string(*tmp) + " and we don't");
							continue;			// Not found/unknown, we skip it/do not enable it.
						}
						m_modules[*tmp]=module;			// Add module to the node's enabled modules list
						Log(cbsdLog::DEBUG,  "Node has module " + module->getName() + ", enabling it!");
					}
					data=data.substr(6+(items*2));
				}


				// Temporary and ugly way to send back the list of modules we support...
				std::string response;
				uint16_t tval=m_modules.size();		// Deviate from the protocol a bit here for now.. :-/
				response.append((char *)&tval,2);
				for (std::map<uint16_t, cbsdModule *>::iterator it = m_modules.begin(); it != m_modules.end(); it++){ tval=it->first; response.append((char *)&tval,2); }
				transmitRaw(response);			// Temporary to make this work for now.
				m_has_negotiated=true;		
				m_is_authenticated=true;		// TODO: for now it's OK we have certs and firewall..
			}
			break;

		default:
			Log(cbsdLog::WARNING, "Node said something unknown/invalid!");
			break;

	}

	return(false);
}


void cbsdNode::_hasData(const std::string &in_data){			// Todo: make this better
	std::string data=in_data;					// ^^^^: Just use A pointer here.

	m_last_seen=std::time(0);

	if(data.size() < 6) return;					// Minimal packetsize is 6 bytes
	uint16_t *tmp=(uint16_t *)data.data();
	uint16_t channel=*tmp;

	if(!m_is_authenticated){ if(!_doAuth(data, channel)) return; } 

	while(data.size() > 2){
		tmp=(uint16_t *)data.data(); channel=*tmp;		// TODO: Fix this

		if(channel == 0){
			if(!_doSysOp(data)) return;
		}else{
			if(data.size() < 8) return;			// To small!
			cbsdModule *mod=m_modules[channel];
			tmp=(uint16_t *)data.data()+1;			// Type
			uint32_t *len=(uint32_t *)data.data()+1;	// Payload length

			if(*len > data.size()){
				if(mod){
					Log(cbsdLog::DEBUG, "Module " + mod->getName() + " is sending invalid packages!");
				}else{
					Log(cbsdLog::DEBUG, "Node is sending invalid packages!");
				}
				return;
			}

			if(!mod){
				Log(cbsdLog::DEBUG, "Mod " + std::to_string(channel) + " should not be talking to me!");
				data=data.substr(*len+8);
			}else{
				mod->moduleReceive(this, *tmp, data.substr(8,*len));
				data=data.substr(*len+8);

			}

		}
	}


}

/*
 * Ready to write data to the client [not used yet]
 */

void cbsdNode::_readyForData(){
	Log(cbsdLog::DEBUG, "Node is ready");
}

/*  
 * Some performance stuff we store in the node class
 * - Probably should make this do some averaging.
 */
void cbsdNode::setPerfdata(const uint16_t what, uint64_t val){
	switch(what){
		case 0: 
		{
			std::map<std::string,std::string> stats;
			m_performance=val; 	// pcpu, pmem, temperature, openfiles
			stats["cpu"]=std::to_string(0|m_pcpu);
			stats["mem"]=std::to_string(0|m_pmem);
			stats["openfiles"]=std::to_string(m_openfiles);
			stats["temperature"]=std::to_string(m_temperature);
			stats["module"]="1";

			Log(cbsdLog::DEBUG, stats); 

			break;
		}
		case 1: Log(cbsdLog::WARNING, "Invalid perfdata type!"); break;
	}
}



