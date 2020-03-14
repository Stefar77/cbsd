/* module.cpp - Module class for CBSD Cluster Daemon
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

#include <iostream>		// std::cout 
#include "node.hpp"

cbsdModule::cbsdModule(const uint16_t id, const std::string &name) : m_name(name), m_id(id) {
	m_flags = 0;

	IFDEBUG(Log(cbsdLog::DEBUG, "Module loaded");)
}

cbsdModule::~cbsdModule() {
	IFDEBUG(Log(cbsdLog::DEBUG, "Module unloaded");)
}

void    cbsdModule::moduleThread(void){
	Log(cbsdLog::WARNING, "unimplemented callback: moduleThread()");
	sleep(1);
}

void    cbsdModule::threadHandler(void){
	IFDEBUG(Log(cbsdLog::DEBUG, "Starting thread");)

	m_flag_thread_running=true;
	while(!m_flag_thread_stopping) 
		if(m_flag_thread_disabled) sleep(1); else moduleThread();

	m_flag_thread_running=false;

	IFDEBUG(Log(cbsdLog::DEBUG, "Stopped thread");)
}

bool	cbsdModule::Init(){
	if(!moduleLoaded()) return(false);

	m_threadID=std::thread([=] { threadHandler(); });
	return(true);
}

void	cbsdModule::doUnload(){
	moduleUnloading();
	if(m_flag_thread_running){
		m_flag_thread_stopping=true;
		IFDEBUG(Log(cbsdLog::DEBUG, "Joining the thread");)
		m_threadID.join();
	}

}

bool	cbsdModule::transmitRaw(cbsdNode *node, const std::string &data){
	if(!node){ // 'Should never happen!' -- famous last words..
		Log(cbsdLog::WARNING, "Module tried to transmit to or with NULL!");
		return(false); 
	} 

	std::string tmp=std::string();
	tmp.append((char *)&m_id,2);
	tmp.append(data);
	if(node) return(node->transmitRaw(tmp)); else return(CBSD->Nodes()->transmitRaw(tmp));
	
}

bool	cbsdModule::Transmit(cbsdNode *node, const uint16_t channel, const std::string &data){
	if(!node){ // 'Should never happen!' -- famous last words..
		Log(cbsdLog::WARNING, "Module tried to transmit to or with NULL!");
		return(false); 
	} 

	std::string tmp=std::string();
	tmp.append((char *)&m_id,2);
	tmp.append((char *)&channel,2);
	uint32_t len=tmp.size();
	tmp.append((char *)&len,4);
	tmp.append(data);
	if(node) return(node->transmitRaw(tmp)); else return(CBSD->Nodes()->transmitRaw(tmp));	
}


void cbsdModule::Log(const uint8_t level, const std::string &data){
	std::map<std::string,std::string> item;
	item["msg"]=data;
	Log(level, item);
}
                                        
void cbsdModule::Log(const uint8_t level, std::map<std::string,std::string> data){
	data["module"]=m_name;
	CBSD->Log(level, data);
}

