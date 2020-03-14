/* cbsd.cpp - CBSD class for CBSD Cluster Daemon
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

cbsdCBSD::cbsdCBSD(const std::string &name){
	m_name=name;
	IFDEBUG(Log(cbsdLog::DEBUG, "Loading");)
}

cbsdCBSD::~cbsdCBSD() {
	Publish("state", "down");

	IFDEBUG(Log(cbsdLog::DEBUG, "Unloading!");)

	delete m_tasks;
	delete m_nodes;
	delete m_users;
	delete m_redis; 

}

bool cbsdCBSD::Init(){
	IFDEBUG(Log(cbsdLog::DEBUG, "Initializing!");)

	m_redis = new cbsdRedis(RedisIP, RedisPORT, RedisPassword, RedisDatabase);
	m_users = new cbsdUsers();
	m_nodes = new cbsdNodes();
	m_tasks = new cbsdTasks();

	if(!m_nodes->Init(ClusterCA, ControllerCRT, ControllerKEY, ControllerPassword)){
		Publish("state", "fail");
		return(false);
	}

	Publish("state", "up");
	return(true);	
}


void cbsdCBSD::Log(const uint8_t level, const std::string &data){
	std::map<std::string,std::string> item;
	item["msg"]=data;
	Log(level, item);
}
                                
void cbsdCBSD::Log(const uint8_t level, std::map<std::string,std::string> data){
	data["controller"]=m_name;

	std::string msg="";
	for (std::map<std::string,std::string>::iterator it = data.begin(); it != data.end(); it++){
		msg.append(it->first+"='"+it->second+"' ");
	}

//	m_mutex.lock();
	LOG(level) << msg;
//	m_mutex.unlock();
}

void cbsdCBSD::Publish(const std::string &key, const std::string &val){
	std::map<std::string,std::string> item;
	item[key]=val;
	Publish(item);
}
                                
void cbsdCBSD::Publish(std::map<std::string,std::string> data){
	data["controller"]=m_name;
	std::string msg="{";
	for (std::map<std::string,std::string>::iterator it = data.begin(); it != data.end(); it++){
		msg.append(std::string((msg.size() == 1?"":","))+"\""+it->first+"\":\""+it->second+"\"");
	}
	msg.append("}");
	m_redis->Publish("events", msg); // Tomporary
}
