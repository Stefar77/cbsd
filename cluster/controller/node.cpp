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

#include "module.hpp"

cbsdNode::cbsdNode(cbsdNodes *nodes, const uint32_t id, const std::string name) {
	m_nodes=nodes;
	m_id=id;				// ID number in the database
	m_name=name;				// Name of the node
	m_flags=0;				// Clear all flags.
	m_configuration=0;			// Unknown arch, mem, etc
	m_performance=0;			// We know nothing yet...
	m_jails.clear();			// We start clean

	LOG(cbsdLog::DEBUG) << "Node " << m_name << " loaded";
}

cbsdNode::~cbsdNode() {
	LOG(cbsdLog::DEBUG) << "Node " << m_name << " unloaded";
}

//bool cbsdNode::isPersistent(){ return(true); };

int cbsdNode::_handleCommand(uint16_t command, uint8_t parameters, char *packet) {
	switch(command){
		case NCMD_PING:	transmitRaw({0x01,0x00,0x01,0x00}); return(0); 	 // Send PONG back
		case NCMD_PONG:	return(0);					 // Ignore PONGs
	}
	return(1);
}


void cbsdNode::_handlePacket(char *packet, size_t len) {
	int		rc = 0;
	uint32_t	index=0;

	// Parse the packet..
	while(rc == 0 && index < len){
		uint8_t	array_size = packet[index++];
		if(array_size != 0 && index < len && len-index > 3){
			for(uint8_t array_index=0; array_index<array_size; array_index++){
				uint16_t command=(packet[index]<<8)|packet[index+1];
				uint8_t param_count=packet[index+2];
				index+=3;					// Skip Command and 'Param array length'
				
				uint32_t tmp_index=index;
				// Check the parameters first..
				for(int param=0; param<param_count; param++){
					if(tmp_index > len || len-tmp_index < 2){ rc=-2; break; }	// inclomplete packets probably
					
					uint16_t plen=(packet[tmp_index]<<8)|packet[tmp_index+1];
					tmp_index+=plen+2;
				}
				if(rc != 0) break;							// We failed, bail out!
				if((rc=_handleCommand(command, param_count, &packet[index]))!=0) break;	// Make it so or break it so!
			}
		}else rc=-2;
	}
	LOG(cbsdLog::DEBUG) << "Node has sent me a packet";
}


/*********** EVENTS ************/

void cbsdNode::_hasConnected(){
	LOG(cbsdLog::INFO)  << "Node " << m_name << " has connected";

	m_nodes->PublishRaw("{\"cmd\":\"event\",\"node\":\""+m_name+"\",\"state\":\"up\"}"); // TODO: Change this!

	m_is_connected=true;			//	state|=NODE_IS_CONNECTED;		

	transmitRaw("testing");
}

void cbsdNode::_hasDisconnected(){
	LOG(cbsdLog::INFO) << "Node " << m_name << " has disconnected";

	m_nodes->PublishRaw("{\"cmd\":\"event\",\"node\":\""+m_name+"\",\"state\":\"disconnected\"}"); // TODO: Change this!

	m_modules.clear();				//      We renegotiate this when it reconnects!

	m_is_connected=false;				//	state&=~NODE_IS_CONNECTED;
	m_is_authenticated=false;			//	state&=~NODE_IS_AUTHENTICATED;
	m_has_negotiated=false;				//	Renegotiate when it logs in again
	if(!m_is_maintenance){				//	
		m_has_warning=true;			//	If not in maintenance warn somebody..
//		m_last_warning=WARN_DISCONNECTED;	//	Maybe do this?!
	}

}

bool cbsdNode::_doSysOp(std::string &data){
	LOG(cbsdLog::DEBUG) << "Input sys_op: [" << data << "]";
	return(false);
}

bool cbsdNode::_doAuth(std::string &data, const uint16_t channel){
	uint8_t *cmd=(uint8_t *)data.data()+2;				// Command
	uint8_t *params=(uint8_t *)data.data()+3;			// # parameters to expect
	uint16_t *tmp=(uint16_t *)((uint8_t *)data.data()+4);		// Size of first parameter

	if(!m_has_negotiated){
		if(*cmd != 0 || channel != 0){
			LOG(cbsdLog::DEBUG) << "Node " << m_name << " invalid negotiation";
			return(false);	// TODO!!
		}
	}


	switch(*cmd){
		case 0:
			if(*params != 1){	// Currently we only have 1 parameter and that is an array of uint16's (mod_id's)
				LOG(cbsdLog::WARNING) << "Node " << m_name << " is signing on with invalid parameters";
				break;
			}
			if(data.size()-6 < *tmp){
				LOG(cbsdLog::WARNING) << "Node " << m_name << " is sending incomplete packages";
				return(false);
			}
			// Did we already do this?
			if(m_has_negotiated){
				LOG(cbsdLog::WARNING) << "Node " << m_name << " is trying to renegotiate";
				return(false);
			}

			{	// Container because I want to use a local var...
				uint16_t items=*tmp/2;
				for(uint16_t i=1; i<=items; i++){
					tmp=(uint16_t *)((uint8_t *)data.data()+4+(i*2));
					cbsdModule *module=m_nodes->getModule(*tmp);
					if(NULL == module){
						LOG(cbsdLog::DEBUG) << "Node has module " << std::to_string(*tmp) << " and we don't";
						continue;	// Not found/unknown, we skip it/do not enable it.
					}
					m_modules[*tmp]=module;		// Add module to the node's enabled modules list
					LOG(cbsdLog::DEBUG) << "Node has module " << std::to_string(*tmp);
				}
				data=data.substr(6+(items*2));
			}
			m_has_negotiated=true;		
			m_is_authenticated=true;		// TODO: for now it's OK we have certs and firewall..
			break;

		default:
			LOG(cbsdLog::WARNING) << "Node " << m_name << " said something unknown/invalid!";
			break;

	}

	return(false);
}


void cbsdNode::_hasData(const std::string &in_data){
	std::string data=in_data;
	if(data.size() < 2) return;			// To small?
	uint16_t *tmp=(uint16_t *)data.data();
	uint16_t channel=*tmp;

	if(!m_is_authenticated){ if(!_doAuth(data, channel)) return; } 

	while(data.size() > 2){
		tmp=(uint16_t *)data.data(); channel=*tmp;

		if(channel == 0){
			if(!_doSysOp(data)) return;
		}else{
			if(data.size() < 8) return;			// To small!
			cbsdModule *mod=m_modules[channel];
			tmp=(uint16_t *)data.data()+1;			// Type
			uint32_t *len=(uint32_t *)data.data()+1;	// Payload length

			if(*len > data.size()){
				if(mod){
					LOG(cbsdLog::DEBUG) << "Module " << mod->getName() << " on node " << m_name << " is sending invalid packages!";
				}else{
					LOG(cbsdLog::DEBUG) << "Node " << m_name << " is sending invalid packages!";
				}
				return;
			}

			if(!mod){
				LOG(cbsdLog::DEBUG) << "Node " << m_name << " mod " << std::to_string(channel) <<  " should not be talking to me!";
				data=data.substr(*len+8);
			}else{
				mod->moduleReceive(this, *tmp, data.substr(8,*len));
				data=data.substr(*len+8);

			}

		}
	}


}

void cbsdNode::_readyForData(){
	LOG(cbsdLog::DEBUG) << "Node " << m_name << " is ready";
}
