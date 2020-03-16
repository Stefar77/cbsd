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
	m_configuration[0]=0;			// Unknown arch, mem, etc
	m_configuration[1]=0;			// Unknown arch, mem, etc
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
	Log(cbsdLog::DEBUG, getJSON(1));


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
	CNINIT_t *pkt=(CNINIT_t *)data.data();

	if(data.size() < sizeof(CNINIT_t)){ Log(cbsdLog::WARNING, "Invalid size, dropping client"); return(false); }
	if(pkt->head != 1){ Log(cbsdLog::WARNING, "Invalid negotiation, dropping client [" + std::to_string(pkt->head) + "]"); return(false); }
	if(pkt->version != 1){ Log(cbsdLog::WARNING, "Invalid version, dropping client"); return(false); }

	m_arch=pkt->arch;			// x86, x64, a32, a64, ...
	m_cores=pkt->cores;			// >255 cores please, gimme! :-)
	m_memory=pkt->memory;
	m_uptime=pkt->uptime;			// Todo check if rebooted here	
//	m_jail_count=pkt->jails;		//
//	m_bhyve_count=pkt->bhyves;		//

	uint16_t items=pkt->modules;		// Amount of modules

	Log(cbsdLog::DEBUG, "Checking " + std::to_string(items) + " modules!");

	uint16_t *tmp=(uint16_t *)((uint8_t *)data.data()+sizeof(CNINIT_t));		// Pointer to size of first parameter
	for(uint16_t i=1; i<=items; tmp++,i++){

		cbsdModule *module=CBSD->Nodes()->getModule(*tmp);
		if(NULL == module){
			Log(cbsdLog::DEBUG, "Node has module " + std::to_string(*tmp) + " and we don't");
			continue;			// Not found/unknown, we skip it/do not enable it.
		}
		m_modules[*tmp]=module;			// Add module to the node's enabled modules list
		Log(cbsdLog::DEBUG,  "Node has module " + module->getName() + ", enabling it!");
	}
	data=data.substr(sizeof(CNINIT_t)+(items*2));

	// Temporary and ugly way to send back the list of modules we support...
	std::string response;
	uint16_t tval=m_modules.size();		// Deviate from the protocol a bit here for now.. :-/
	response.append((char *)&tval,2);
	for (std::map<uint16_t, cbsdModule *>::iterator it = m_modules.begin(); it != m_modules.end(); it++){ tval=it->first; response.append((char *)&tval,2); }
	transmitRaw(response);			// Temporary to make this work for now.
	m_has_negotiated=true;		
	m_is_authenticated=true;		// TODO: for now it's OK we have certs and firewall..
	return(true);
}


void cbsdNode::_hasData(const std::string &in_data){			// Todo: make this better
	std::string data=in_data;					// ^^^^: Just use A pointer here.

	m_last_seen=std::time(0);

	if(data.size() < 6) return;					// Minimal packetsize is 6 bytes
	uint16_t *tmp=(uint16_t *)data.data();
	uint16_t channel=*tmp;

	if(!m_is_authenticated){ if(!_doAuth(data, channel)){ doDisconnect(); return; }  }

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
		case 0: m_performance=val; break;	// pcpu, pmem, temperature, openfiles TODO: Seperate changed items and push them to redis

//		{
//			std::map<std::string,std::string> stats;	// 
//			stats["cpu"]=std::to_string(0|m_pcpu);
//			stats["mem"]=std::to_string(0|m_pmem);
//			stats["openfiles"]=std::to_string(m_openfiles);
//			stats["temperature"]=std::to_string(m_temperature);
//			stats["module"]="1";
//			Log(cbsdLog::DEBUG, stats); 
//			break;
//		}
		case 1: Log(cbsdLog::WARNING, "Invalid perfdata type!"); break;
	}
}

std::string cbsdNode::getJSON(uint32_t flags){
	std::string output="{\"type\":\"node\",\"name\":\"" + m_name +"\"";

	if(m_id != 0) output.append(",\"gid\":" + std::to_string(m_id));			// Global ID
	output.append(",\"seen\":" + std::to_string(m_last_seen));				// Timestamp
	output.append(",\"state\":\"" + std::string(m_is_authenticated?"connected":"offline")+"\"");
	output.append(",\"maintenance\":" + std::string(m_is_maintenance?"1":"0"));

	output.append(",\"arch\":\"amd64\"");
	output.append(",\"cores\":" + std::to_string(12)); //m_bhyvecount));			// 
	output.append(",\"memory\":" + std::to_string(128*1024)); //m_bhyvecount));			// 
	
	output.append(",\"stats\":{\"pcpu\":" + std::to_string(m_pcpu));			// CPU Usage
	output.append(",\"pmem\":" + std::to_string(m_pmem));					// RAM Usage
	output.append(",\"files\":" + std::to_string(m_openfiles));				// Open files
	output.append(",\"temp\":" + std::to_string(m_temperature));				// Temperature

	output.append(",\"jails\":" + std::to_string(5)); //m_jailcount));			// 
	output.append(",\"bhyves\":" + std::to_string(16)); //m_bhyvecount));			// 

	output.append("}");

	if(1 & flags && m_tasks.size() > 0){
		output.append(",\"tasks\":{");
		// Add them
		output.append("}");
	}
	if(1 & flags && m_jails.size() > 0){
		output.append(",\"jails\":{");
		// Add them
		output.append("}");
	}
	if(1 & flags && m_modules.size() > 0){
		std::map<uint16_t, cbsdModule *>::iterator it = m_modules.begin();
		output.append(",\"modules\":["+std::to_string(it->first));
		for (it++; it != m_modules.end(); it++) output.append((char *)&it->first,2); 
		output.append("]");
	}

	return(output + "}");
}

