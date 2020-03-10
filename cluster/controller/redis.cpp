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
	LOG(cbsdLog::DEBUG) << "Redis connection unloaded";
}

/* Private functions */
bool cbsdRedis::_doConnect(){
	LOG(cbsdLog::WARNING) << "Redis failed to connect";
	return(false);
}

std::string cbsdRedis::_doRequest(std::vector<std::string> oplist){
	std::string query="*"+std::to_string(oplist.size())+"\r\n";
	for(int i=0; i<oplist.size(); i++) query.append("$"+std::to_string(oplist[i].size())+"\r\n"+oplist[i]+"\r\n");

	LOG(cbsdLog::DEBUG) << "Redis query: [" << query << "]";

	if(!m_is_connected && !_doConnect()) return("");	// throw an exception?!

	TransmitRaw(query);
	// Wait for response..



	return(std::string("ok"));
}


/* Queue functions */
uint32_t cbsdRedis::Publish(const std::string &queue, const std::string &event){
	std::vector<std::string> oplist = {"PUBLISH", queue, event};
	std::string data=_doRequest(oplist);

	LOG(cbsdLog::DEBUG) << "Redis result: [" << data << "]";
	return(0);
}

/* HASH functions */
std::string cbsdRedis::hGet(const std::string &hash, const std::string &key){
	std::vector<std::string> oplist = {"HGET", hash, key};
	std::string data=_doRequest(oplist);

	LOG(cbsdLog::DEBUG) << "Redis result: [" << data << "]";
	return("");

}

uint32_t cbsdRedis::hSet(const std::string &hash, const std::string &key, const std::string &val){
	std::vector<std::string> oplist = {"HSET", hash, key, val};
	std::string data=_doRequest(oplist);

	LOG(cbsdLog::DEBUG) << "Redis result: [" << data << "]";

	return(0);
}

bool	cbsdRedis::_handleData(const std::string &data){
	LOG(cbsdLog::DEBUG) << "Redis received: [" << data << "]";
	return(true);
}

