/* nodes.cpp - Nodes class for CBSD Cluster Daemon
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
#include "modules/racct.hpp"

cbsdNodes::cbsdNodes(const std::string &CA, const std::string &CRT, const std::string &KEY, const std::string &PW) {
	m_listener = new cbsdListener(&cbsdNodes::accept_cb, this, 0, ControllerPORT, "Node-Listener");
	if(!m_listener) return; 					// Should never happen..

	// Modules... (for now)
	m_modules[1]=new cbsdRACCT();					// For testing...

	PublishRaw("{\"cmd\":\"event\",\"node\":\"Controller\",\"state\":\"up\"}");	// TODO: Change this!

	if(!m_listener->setupSSL(CA, CRT, KEY, PW)){
		Log(cbsdLog::FATAL, "Nodes failed to load, I should stop now!");
		return;
	}
                                                     
	m_nodes[1]=new cbsdNode(this, 1, "SuperBSD");		// For testing...

	Log(cbsdLog::DEBUG, "Nodes loaded");
}

cbsdNodes::~cbsdNodes() {

	PublishRaw("{\"cmd\":\"event\",\"node\":\"Controller\",\"state\":\"terminated\"}");	// TODO: Change this!

	// First we stop the listener so it doesn't try to access non-existing objects while closing...
	if(NULL != m_listener){
		delete m_listener;
		usleep(300);
		m_listener=NULL; 		// Why not..
	}

	Log(cbsdLog::DEBUG, "Unloading nodes");

	m_mutex.lock();
	for (std::map<uint32_t, cbsdNode*>::iterator it = m_nodes.begin(); it != m_nodes.end(); it++){
//		it->second->nodeEvent(cbsdNode::UNLOAD);
		delete it->second;
	}
	m_nodes.clear();
	m_mutex.unlock();

}

cbsdSocket *cbsdNodes::acceptConnection(int fd, SSL *ssl){
	Log(cbsdLog::DEBUG, "Got new connection!");
	return(m_nodes[1]);
}

bool cbsdNodes::transmitRaw(const std::string &data){
	m_mutex.lock();
	for (std::map<uint32_t, cbsdNode*>::iterator it = m_nodes.begin(); it != m_nodes.end(); it++){
		if(it->second->isOnline()) it->second->transmitRaw(data);
	}
        m_mutex.unlock();
	return(true);	
}


void cbsdNodes::PublishRaw(const std::string &data){
//	CBSD.PublishRaw(data);	
//	m_redis->Publish(RedisEQ, data);
}

CBSDDBCLASS(cbsdNodes, cbsdNode, m_nodes)

