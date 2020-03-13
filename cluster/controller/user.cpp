/* user.cpp - User class for CBSD Cluster Daemon
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

#include "user.hpp"

cbsdUser::cbsdUser(const uint32_t id, const std::string &name){
	m_id=id;
	m_name=name;

	Log(cbsdLog::DEBUG, "User loaded");
}

cbsdUser::~cbsdUser() {
	Log(cbsdLog::DEBUG, "User unloaded");
}

void cbsdUser::Log(const uint8_t level, const std::string &data){
	std::map<std::string,std::string> item;
	item["msg"]=data;
	Log(level, item);
}
        
void cbsdUser::Log(const uint8_t level, std::map<std::string,std::string> data){
	data["user"]=m_name;
	CBSD->Log(level, data);
}

bool	cbsdUser::linkItem(cbsdTask *task){ 
	std::map<uint32_t, cbsdTask *>::iterator it = m_tasks.find(task->getID()); 
	if(it != m_tasks.end()){ m_tasks[task->getID()]=task; return(true); } 
	return(false);
}

bool    cbsdUser::unlinkItem(cbsdTask *task){ 
	std::map<uint32_t, cbsdTask *>::iterator it = m_tasks.find(task->getID()); 
	if(it != m_tasks.end()){ m_tasks.erase(it); return(true); } 
	return(false); 
}

