/* jail.cpp - Jail class for CBSD Cluster Daemon
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
#include "jail.hpp"

#ifdef ISNODE
cbsdJail::cbsdJail(const uint32_t id, const std::string &name, const std::string &hostname, const std::string &path, const std::string &ethernet){
	m_jid=id;							// This one uses the local jid.
#else
cbsdJail::cbsdJail(cbsdNode *node, const uint32_t id, const std::string &name, const std::string &hostname, const std::string &path, const std::string &ethernet){
	m_node=node;							// Running on that node?
	m_id=id;							// This one sets database ID
#endif
	m_name=name;
	m_hostname=hostname;
	m_path=path;
	m_ethernet=ethernet;

#ifdef ISNODE
	LOG(cbsdLog::DEBUG) << "Jail '" << m_name << "' loaded";
#else
	if(NULL != m_node){
		LOG(cbsdLog::DEBUG) << "Jail '" << m_name << "' on node '" << m_node->getName() << "' loaded";
	}else{
		LOG(cbsdLog::DEBUG) << "Jail '" << m_name << "' on standby";
	}
#endif

}

cbsdJail::~cbsdJail() {
	LOG(cbsdLog::DEBUG) << "Jail '" << m_name << "' unloaded";
}

#ifdef ISNODE
void	cbsdJail::gatherStats(){
	if(0 == m_jid){
		LOG(cbsdLog::WARNING) << "Trying to gathering stats for non-running jail '" << m_name << "'";
		// TODO: Reset stats?
		return;
	}

	LOG(cbsdLog::DEBUG) << "Gathering stats for jail '" << m_name << "'";

//	std::map<std::string item, std::string val> stats=cbsdUtils::queryRACCT("jail:"+m_jid);
//	
//
//
//

}
#endif

void cbsdJail::doUnload(){

}
