/* task.cpp - Task class for CBSD Cluster Daemon
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

#include "task.hpp"

cbsdTask::cbsdTask(const uint32_t id, cbsdUser *owner, const std::string &name){
	m_id=id;
	m_name=name;
//	if((m_owner=owner) != NULL) owner->linkItem(this);	// Lock it so we do not unload the owner..
	Log(cbsdLog::DEBUG, "Task loaded");
}

cbsdTask::~cbsdTask() {
//	if(NULL != m_owner) owner->unlinkItem(this);		// We are no longer locking this
	Log(cbsdLog::DEBUG, "Task unloaded");
}

void cbsdTask::Log(const uint8_t level, const std::string &data){
	std::map<std::string,std::string> item;
	item["msg"]=data;
	Log(level, item);
}
        
void cbsdTask::Log(const uint8_t level, std::map<std::string,std::string> data){
	data["task"]=m_name;
	CBSD->Log(level, data);
}

