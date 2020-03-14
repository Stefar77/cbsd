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

cbsdNodes::cbsdNodes(){
	m_flags=0;
	m_do_redis_updates=true;

	m_modules.clear();
	m_nodes.clear();

	m_listener = new cbsdListener(&cbsdNodes::accept_cb, this, 0, ControllerPORT, "Node-Listener");
	if(!m_listener) return; 					// Should never happen..

	// Modules... (for now)
	#define ADDMODULE(num, name) m_modules[num]=new name(); m_modules[num]->Init()
	ADDMODULE(1, cbsdRACCT);			// For testing...

	IFDEBUG(Log(cbsdLog::DEBUG, "Nodes loaded");)
}

bool    cbsdNodes::Init(const std::string &CA, const std::string &CRT, const std::string &KEY, const std::string &PW){
	if(m_is_thread_running) return(false);

	if(!m_listener || !m_listener->setupSSL(CA, CRT, KEY, PW)){
		Log(cbsdLog::FATAL, "Nodes failed to initialize!");
		return(false);
	}
               
	// Start the thread..
	m_threadID=std::thread([=] { threadHandler(); });
	return(true);	
}

cbsdNodes::~cbsdNodes() {
	// First we stop the listener so it doesn't try to access non-existing objects while closing...
	if(NULL != m_listener){
		delete m_listener;
		usleep(300);
		m_listener=NULL; 		// Why not..
	}

	IFDEBUG(Log(cbsdLog::DEBUG, "Stopping nodes thread");)
	if(m_is_thread_running){
		m_is_thread_stopping=true;
//		while(m_is_thread_running) usleep(100);
		IFDEBUG(Log(cbsdLog::DEBUG, "Joining the thread");)
		m_threadID.join();
	}

	IFDEBUG(Log(cbsdLog::DEBUG, "Unloading nodes");)

	m_mutex.lock();
	FOREACH_NODES {
//		it->second->nodeEvent(cbsdNode::UNLOAD);
		delete it->second;
	}
	m_nodes.clear();
	m_mutex.unlock();

}

IFREDIS(cbsdNode *cbsdNodes::_fetchFromRedis(const std::string &name){
	std::map<std::string,std::string> test=CBSD->Redis()->hGetAll("node:"+name);
	if(test.size() == 0) return(NULL);

	SMAPIT it = test.find("id");
	if(it == test.end()) return(NULL);

	uint32_t id=stoi(test["id"]);
	m_nodes[id]=new cbsdNode(id, name);	// TODO also set rest of items..
	Log(cbsdLog::DEBUG, "Loaded node from Redis");
	return(m_nodes[id]);
})

cbsdSocket *cbsdNodes::acceptConnection(int fd, SSL *ssl, const std::string &name){
	if(name == "") return(NULL);		// For now..

	cbsdNode *node=find(name); if(node) return(node);
	IFREDIS(if((node=_fetchFromRedis(name))) return(node);)

	// TODO: SQL

			
	Log(cbsdLog::WARNING, "Invalid node '"+name+"' tried to connect");
	SSL_write(ssl, "--quit", 6); // Yikes.

	return(NULL);
	
}

bool cbsdNodes::transmitRaw(const std::string &data){
	m_mutex.lock();
	FOREACH_NODES {
		if(it->second->isOnline()) it->second->transmitRaw(data);
	}
        m_mutex.unlock();
	return(true);	
}

CBSDDBCLASS(cbsdNodes, cbsdNode, m_nodes)

void    cbsdNodes::threadHandler(void){
        IFDEBUG(Log(cbsdLog::DEBUG, "Starting nodes thread");)

        m_is_thread_running=true;
        while(!m_is_thread_stopping){
		sleep(2);

		/* Check if nodes are still alive */
		m_mutex.lock();
		FOREACH_NODES {
			if(!it->second->isOnline()) continue;		// Is it offline, skip it!
			if(it->second->isFresh()) continue;		// Did we receive recently, skip also!

			if(it->second->isStale()){ 
				IFDEBUG(Log(cbsdLog::DEBUG, "Node is stale! " + it->second->getName());)
				it->second->doDisconnect(); continue; 
			}

			IFDEBUG(Log(cbsdLog::DEBUG, "Node check " + it->second->getName());)
			// Send a NULL packet and check if the socket is still alive
			if(!it->second->transmitRaw({0x00,0x00,0x00,0x00})){
				IFDEBUG(Log(cbsdLog::DEBUG, "Node check failed");)
			}
		}
        	m_mutex.unlock();


		//if(m_do_redis_updates){
		//	IFDEBUG(Log(cbsdLog::DEBUG, "Updating stats in redis");)
		//}

	}
        m_is_thread_running=false;
	m_is_thread_stopping=false;

        IFDEBUG(Log(cbsdLog::DEBUG, "Stopped nodes thread");)
}

