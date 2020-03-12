/* users.cpp - Users class for CBSD Cluster Daemon
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

cbsdUsers::cbsdUsers(){

	Log(cbsdLog::DEBUG, "Users database started");
}

cbsdUsers::~cbsdUsers() {
	Log(cbsdLog::DEBUG, "Users database unloaded");
}


cbsdUser        *cbsdUsers::find(const std::string &name){
	cbsdUser *item=NULL;
	// Step one check cache
	m_mutex.lock();
	for (std::map<uint32_t, cbsdUser*>::iterator it = m_users.begin(); it != m_users.end(); it++){
		if(it->second->getName() == name){ item=it->second; break; }
	}
	m_mutex.unlock();
	if(item) return(item); // Success, local hit!

	// TODO: Check external
	// map<std::string, std::string> res=Datastore->fetch("users", "name", name);

	return(NULL);
}

cbsdUser        *cbsdUsers::find(const uint32_t id){
	cbsdUser *item=NULL;
	// Step one check cache
	m_mutex.lock();
	std::map<uint32_t, cbsdUser*>::iterator it = m_users.find(id);
	if(it != m_users.end()) item=it->second;
	m_mutex.unlock();
	if(item) return(item); // Success, local hit!

	// TODO: Check external
	// map<std::string, std::string> res=Datastore->fetch("users", "id", std::to_string(id));

	return(NULL);
}

void cbsdUsers::Log(const uint8_t level, const std::string &data){
	std::map<std::string,std::string> item;
	item["msg"]=data;
	CBSD->Log(level, item);
}
         
void cbsdUsers::Log(const uint8_t level, std::map<std::string,std::string> data){
	CBSD->Log(level, data);
}
 

