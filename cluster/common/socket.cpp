/* socket.cpp - Socket class for CBSD Cluster Daemon
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
#include "socket.hpp"

int ssl_callback(char *buf, int size, int rwflag, void *u){
	char    *pass=(char *)u;
	size_t   len;

	if(NULL == pass || 0 == (len=strlen(pass))) return(0); // No password supplied
  
	strncpy(buf, pass, len+1);
 
	return(len);
}


cbsdSocket::cbsdSocket() {
	m_ssl=NULL;
	m_fd=-1;
//	LOG(cbsdLog::DEBUG) << "Socket created";
}

cbsdSocket::~cbsdSocket() {
//	LOG(cbsdLog::DEBUG) << "Socket destroyed";
}

bool cbsdSocket::isPersistent(){ return(false); };
void cbsdSocket::_hasConnected(){ 
	LOG(cbsdLog::WARNING) << "Unhandled connection event";
	return; 
};

void cbsdSocket::_hasDisconnected(){ 
	LOG(cbsdLog::WARNING) << "Unhandled disconnect event";
	return; 
};

void cbsdSocket::_hasData(const std::string &data){ 
	LOG(cbsdLog::WARNING) << "Unhandled data event";
	return; 
};

void cbsdSocket::_readyForData(){ 
	LOG(cbsdLog::WARNING) << "Unhandled ready event";
	return; 
};

 
void cbsdSocket::doDisconnect(){
	socketEvent(cbsdSocket::CLOSE);
	if(m_ssl){ SSL_shutdown(m_ssl); SSL_free(m_ssl); m_ssl=NULL; }
	if(m_fd != -1){ close(m_fd); m_fd=-1; }
}

void cbsdSocket::socketEvent(const uint8_t ev, void *opt){
	char    buffer[4096];		// Buffer for receiving stuff..

        switch(ev){
		case SSSL:  //LOG(cbsdLog::DEBUG) << "SSL Connection established!"; 
			m_ssl=(SSL *)opt;
			return;

		case OPEN:  
			{ 
				int *fd=(int *)opt; 
				m_fd=*fd; 
			} 
			_hasConnected();
			return;

		case READ:
			{
				std::string data;
				if(m_ssl){
					int bytes_read=SSL_read(m_ssl, buffer, sizeof(buffer));
					if(bytes_read == 0){ LOG(cbsdLog::WARNING) << "Empty read!"; return; }
					if(bytes_read < 0){ LOG(cbsdLog::DEBUG) << "NULL response!"; return; }

					data.append(buffer, bytes_read);
		
				}
				_hasData(data);
			}
			return;




		case WRITE: _readyForData();	return;
		case CLOSE: if(m_fd != -1 || m_ssl != NULL) _hasDisconnected(); return;

	}

	LOG(cbsdLog::WARNING) << "Invalid socket event";
}

bool cbsdSocket::transmitRaw(const std::string &data){
	bool	ret;
        m_transmit_mutex.lock();

	if(NULL == m_ssl){
	        ret=(write(m_fd, data.c_str(), data.size()) == data.size() );
        }else{
	        ret=(SSL_write(m_ssl, data.c_str(), data.size()) == data.size() );
        }                
        m_transmit_mutex.unlock();

        return(ret);
}

