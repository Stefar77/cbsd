/* module.cpp - Module class for CBSD Cluster Node
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
#include "master.hpp"

cbsdModule::cbsdModule(const uint16_t id, const std::string &name) : m_name(name), m_id(id) {
        m_flags = 0;

	LOG(cbsdLog::DEBUG) << "Module '" << m_name << "' loaded";
}

cbsdModule::~cbsdModule() {
	LOG(cbsdLog::DEBUG) << "Module '" << m_name << "' unloaded";
}


//void	cbsdModule::moduleUnloading(){
//	LOG(cbsdLog::WARNING) << "Module '" << m_name << "' unhandled event: moduleUnloading()";
//}

//void	cbsdModule::moduleLoaded(){
//	LOG(cbsdLog::WARNING) << "Module '" << m_name << "' unhandled event: moduleUnloading()";
//}

void    cbsdModule::moduleThread(void){
	LOG(cbsdLog::WARNING) << "Module '" << m_name << "' unimplemented callback: moduleThread()";
	sleep(1);
}

void    cbsdModule::threadHandler(void){
        LOG(cbsdLog::DEBUG) << "Starting '" << m_name << "' thread";

        m_flag_thread_running=true;
        while(!m_flag_thread_stopping) 
		if(m_flag_thread_disabled) sleep(1); else moduleThread();

        m_flag_thread_running=false;

        LOG(cbsdLog::DEBUG) << "Stopped '" << m_name << "' thread";
}


bool	cbsdModule::doLoad(cbsdMaster *master){
	m_master=master;

	if(!moduleLoaded()) return(false);

	m_threadID=threadHandlerProc();
	return(true);
}

void	cbsdModule::doUnload(){
	moduleUnloading();
	if(m_flag_thread_running){
		m_flag_thread_stopping=true;
         
		// Wait for the thread to stop..
		while(m_flag_thread_running) usleep(100);
        
		LOG(cbsdLog::DEBUG) << "Joining '" << m_name << "' thread";
		m_threadID.join();
	}

}

void	cbsdModule::TransmitBuffered(const std::string &data){
	if(!m_master){ // 'Should never happen!' -- famous last words..
		LOG(cbsdLog::WARNING) << "Module '" << m_name << "' has no master!";
		return; 
	} 

	std::string tmp=std::string();
	tmp.append((char *)&m_id,2);
	tmp.append(data);
	m_master->Transmit(tmp);
	
}


