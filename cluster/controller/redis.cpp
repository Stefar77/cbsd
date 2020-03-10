/* redis.cpp - Redis class for CBSD Cluster Daemon
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

#include "redis.hpp"

cbsdRedis::cbsdRedis(const std::string &host, uint16_t port, const std::string &password, uint32_t database): cbsdConnector("Redis") {
	m_host=host;				// Name of the node
	m_port=port;
	m_password=password;
	m_database=database;
	m_flags=0;				// Clear all flags.

	LOG(cbsdLog::DEBUG) << "Redis connection loaded";
}

cbsdRedis::~cbsdRedis() {
	m_response="";
	m_cv.notify_all();			// Wakie wakie..
	LOG(cbsdLog::DEBUG) << "Redis connection unloaded";
}

/* Private functions */
bool cbsdRedis::_doConnect(){
	if(!Connect(m_host, m_port)){
		LOG(cbsdLog::WARNING) << "Redis failed to connect";
		return(false);
	}
	if(m_password != ""){
		std::vector<std::string> auth = {"AUTH", m_password};
		std::string res=_doRequest(auth);
		if(res != "+OK\r\n"){
			LOG(cbsdLog::WARNING) << "Redis failed to authenticate";
			Disconnect();					// We failed, disconnect so we don't try queries.
			return(false);
		}
	}
	if(m_database != 0){
		std::vector<std::string> dbsel = {"SELECT", std::to_string(m_database)};
		std::string res=_doRequest(dbsel);
		if(res != "+OK\r\n"){
			LOG(cbsdLog::WARNING) << "Redis failed to select database";
			Disconnect();					// We failed, disconnect so we don't try queries.
			return(false);
		}
	}
	return(true);		// We are happy and connected! [for now]
}

std::string cbsdRedis::_doRequest(std::vector<std::string> oplist){
	std::string query="*"+std::to_string(oplist.size())+"\r\n";
	for(int i=0; i<oplist.size(); i++) query.append("$"+std::to_string(oplist[i].size())+"\r\n"+oplist[i]+"\r\n");

//	LOG(cbsdLog::DEBUG) << "Redis query: [" << query << "]";

	if(!isConnected() && !_doConnect()) return("");	// throw an exception?!

	std::unique_lock<std::mutex> lk(m_mutex);

	m_response = "";	// TODO: do this the right way..
	if(!TransmitRaw(query))	return(std::string("")); // Same?!

	m_cv.wait(lk);					// This needs some sort of timeout!

	return(m_response);
}

uint32_t cbsdRedis::_intResult(const std::string &data){
	if(data.substr(0,1) != ":") return(255);		// TODO: Invalid response. throw exception here?!
	return(std::stoi(data.substr(1)));
}

/* Queue functions */
uint32_t cbsdRedis::Publish(const std::string &queue, const std::string &event){
	std::vector<std::string> oplist = {"PUBLISH", queue, event};
	std::string data=_doRequest(oplist);

	LOG(cbsdLog::DEBUG) << "Redis result: [" << data << "]";
	return(_intResult(data));
}

/* HASH functions */
std::string cbsdRedis::hGet(const std::string &hash, const std::string &key){
	std::vector<std::string> oplist = {"HGET", hash, key};
	std::string data=_doRequest(oplist);

	if(data.substr(0,1) != "$") return("");			// Invalid response.
	std::size_t pos=data.find("\n");
	if(pos == std::string::npos) return("");		// Invalid or incomplete response

	return(data.substr(pos+1,data.size()-(pos+3)));

}

uint32_t cbsdRedis::hSet(const std::string &hash, const std::string &key, const std::string &val){
	std::vector<std::string> oplist = {"HSET", hash, key, val};
	return(_intResult(_doRequest(oplist)));
}

bool	cbsdRedis::_handleData(const std::string &data){

//	LOG(cbsdLog::DEBUG) << "Redis received: [" << data << "]";

	m_response=data;
	m_cv.notify_all();			// Wakie wakie..

	return(true);
}

