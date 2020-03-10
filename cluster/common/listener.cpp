/* listener.cpp - Listener class for CBSD Cluster Daemon
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
#include "listener.hpp"

extern int ssl_callback(char *buf, int size, int rwflag, void *u);

cbsdListener::cbsdListener(cbsdSocket *(*cb)(int, SSL*, void*), void *cls, uint32_t ip, const uint16_t port, const std::string name) {
	m_ssl_ctx=NULL;			// Clear context
	m_ip=ip;			// Setup BIND address
	m_port=port;			// Setup port
	m_name=name;			// My name
	m_flags=0;			// Reset all flags
	m_cert_pass=NULL;		// Cert password
	m_accept_size=10;		// Todo: make customizable
	m_connect_cb_fn=cb;		// Callback Function
	m_connect_cb_cl=cls;		// Callback Class

	LOG(cbsdLog::DEBUG) << "Listener " << m_name << " loaded";
}

cbsdListener::~cbsdListener() {
	// Delete connections
	m_mutex.lock();
	for (std::map<int, cbsdSocket*>::iterator it = m_connections.begin(); it != m_connections.end(); it++){
		if(dynamic_cast<cbsdSocket*>(it->second)) delete it->second;
	}
	m_connections.clear();
	m_mutex.unlock();

	if(m_flag_thread_running){		// Thread [still] running?
		m_flag_thread_running=false;	// Stop it
		close(m_sync_fd[0]);		// Wake up thread 1
		m_listenThread.join();		// Kill the thread..
	}
 

	close(m_fd); 				// Close the socket
	SSL_CTX_free(m_ssl_ctx);		// Free the context
	EVP_cleanup();				// Cleanup SSL
	m_ssl_ctx=NULL;				// ... why not ...

	if(NULL != m_cert_pass) free(m_cert_pass);

	LOG(cbsdLog::DEBUG) << "Listener " << m_name << " unloaded";
}

bool cbsdListener::setupSSL(const std::string ca, const std::string crt, const std::string key, const std::string pass) {
	if(m_flag_ssl_ready) return(true);
	
	SSL_load_error_strings();		// Initialize SSL errros
        SSL_library_init();			// Initialize Library
        OpenSSL_add_ssl_algorithms();		// Do that..

	const SSL_METHOD *method=TLS_server_method();
        RAND_seed("CBSD-ROCKS!",10);

	if((m_ssl_ctx=SSL_CTX_new(method))==NULL){ 
		LOG(cbsdLog::FATAL)  << "Failed to initalize SSL!"; //ERR_print_errors_fp(stderr); 
		return(false); 
	}

	SSL_CTX_set_ecdh_auto(m_ssl_ctx, 1);
        SSL_CTX_set_default_passwd_cb(m_ssl_ctx, ssl_callback);
	if(!pass.empty()){
		m_cert_pass=strdup(pass.c_str());
		SSL_CTX_set_default_passwd_cb_userdata(m_ssl_ctx, m_cert_pass);
	}

	if (SSL_CTX_use_certificate_chain_file(m_ssl_ctx, ca.c_str()) <= 0){ 
		LOG(cbsdLog::FATAL) << "Error; Failed to initalize SSL CA-chain!"; // ERR_print_errors_fp(stderr); 
		return(false); 
	}

	if (SSL_CTX_use_certificate_file(m_ssl_ctx, crt.c_str(), SSL_FILETYPE_PEM) <= 0){ 
		LOG(cbsdLog::FATAL) << "Failed to initalize SSL CRT"; ERR_print_errors_fp(stderr);
		return(false); 
	}

	if (SSL_CTX_use_PrivateKey_file(m_ssl_ctx, key.c_str(), SSL_FILETYPE_PEM) <= 0 ){ 
		LOG(cbsdLog::FATAL) << "Private key does not work on certificate!"; // ERR_print_errors_fp(stderr);
		return(false); 
	}

	if(SSL_CTX_check_private_key(m_ssl_ctx) != 1){ 
		LOG(cbsdLog::FATAL) << "Certificate does not match key!"; 
		return(false); 
	}

	if(!SSL_CTX_load_verify_locations(m_ssl_ctx, ca.c_str(), NULL)){ 
		LOG(cbsdLog::FATAL) << "Failed to initalize SSL CA.";	// ERR_print_errors_fp(stderr);
		return(false); 
	}

//	if(!SSL_CTX_set_min_proto_version(m_ssl_ctx, 3)){ 
//		LOG(cbsdLog::FATAL) << "Failed to initalize SSL version"; ERR_print_errors_fp(stderr);
//		return(false); 
//	}

	STACK_OF(X509_NAME) *list;
	list = SSL_load_client_CA_file( ca.c_str() );
	if( list == NULL ){ LOG(cbsdLog::FATAL) << "SSL Chain error"; return(false); }

	SSL_CTX_set_client_CA_list( m_ssl_ctx, list );
	SSL_CTX_set_verify( m_ssl_ctx, SSL_VERIFY_PEER, NULL );
//	SSL_CTX_set_verify_depth(m_ctx_ctx, 1 );
	

	m_listenThread=listenThreadProc();

	LOG(cbsdLog::DEBUG) << "SSL Initialized";
	
	return((m_flag_ssl_ready=true));
}


bool cbsdListener::_initialize(void){
        struct sockaddr_in addr = {};
        addr.sin_len = sizeof(addr);
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(m_port);

	if((m_fd = socket(addr.sin_family, SOCK_STREAM, 0)) == -1){ LOG(cbsdLog::FATAL) << "socket failed"; return(false); }

	int on = 1;
	setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	// Control / ability to shutdown thread...
	socketpair(AF_UNIX, SOCK_STREAM, 0, m_sync_fd);
	
	if (bind(m_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1){ perror("bind failed"); return(false); }
	if(listen(m_fd, m_accept_size) == -1){ perror("listen failed"); return(false); }

	m_kq = kqueue();
	EV_SET(&m_evSet, m_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
	if(-1 == kevent(m_kq, &m_evSet, 1, NULL, 0, NULL)) return(false);

	EV_SET(&m_evSet, m_sync_fd[1], EVFILT_READ, EV_ADD, 0, 0, NULL);
	if(-1 == kevent(m_kq, &m_evSet, 1, NULL, 0, NULL)) return(false);

	return(true);
}

void cbsdListener::_handleAccept(int fd){
	struct sockaddr_storage addr;
	socklen_t socklen = sizeof(addr);
	SSL *ssl = NULL;
	BIO *sbio = NULL;

	int connfd = accept(fd, (struct sockaddr *)&addr, &socklen);
	if(connfd == -1){ /* connectFails++; localErrors++; */  return; }

	int ret;
	if(m_flag_ssl_ready){
		ssl = SSL_new(m_ssl_ctx);
		if(!ssl){
			LOG(cbsdLog::CRITICAL) << "Failed to create SSL layer"; close(connfd); 
			return;
		}
		SSL_set_fd(ssl, connfd);
		try {
			ret=SSL_accept(ssl);
		} catch (const std::exception&) {
			LOG(cbsdLog::CRITICAL) << "SSL Exception";
			SSL_free(ssl);
			close(connfd);
			return;

		}

		if(ret <= 0){
			LOG(cbsdLog::CRITICAL) << "SSL Handshake Error [" << SSL_get_error(ssl, ret) << "]"; 
			SSL_free(ssl);
			close(connfd); 
			return;
		}

		if((sbio = BIO_new_socket(connfd, BIO_NOCLOSE)) == NULL){
			LOG(cbsdLog::CRITICAL) << "BIO_new_socket() failed"; 
			close(connfd);
			SSL_free(ssl); 
			//connectFails++; 
			return; 
		}

		SSL_set_bio(ssl,sbio,sbio);
	}else{
		LOG(cbsdLog::WARNING) << "Non-SSL connections are not implemented yet!";
		close(connfd);
		return;

// TODO!!
//		if ((ret=accept(connfd)) <= 0){
			//
//		}
	}
	EV_SET(&m_evSet, connfd, EVFILT_READ, EV_ADD, 0, 0, NULL);
	kevent(m_kq, &m_evSet, 1, NULL, 0, NULL);
	
//	if(m_flag_dont_accept){
//		if(m_flag_ssl_ready){
//			
//		}else{
//
//		}
//	}

	int tflags=0;
	if(m_flag_ssl_ready){
		X509		*cert = SSL_get_peer_certificate(ssl);
		X509_NAME_ENTRY	*certname = NULL;

	        if (cert == NULL){ 
			_connectFailed(connfd, ssl, "Error: Could not get a certificate from client."); 
			return; 
		}

		int common_name_loc = -1;
		common_name_loc = X509_NAME_get_index_by_NID(X509_get_subject_name(cert),NID_commonName, -1);
		certname = X509_NAME_get_entry(X509_get_subject_name(cert), common_name_loc);
		ASN1_STRING *common_name_asn1;

		if(certname == NULL || NULL == (common_name_asn1 = X509_NAME_ENTRY_get_data(certname))){ 
			X509_free(cert); 
			_connectFailed(connfd, ssl, "Error: Could not get CN from certificate.");
			return;
		}
		std::string name = std::string((char *) ASN1_STRING_get0_data(common_name_asn1));
		X509_free(cert);

		if(name.empty()){ 
			_connectFailed(connfd, ssl, "Error: Could not get name from certificate!"); 
			return;
		}

		tflags = fcntl(connfd, F_GETFL, 0);
		if(tflags == 0){ 
			_connectFailed(connfd, ssl, "Error: fcntl error."); 
			return; 
		}


	}


	cbsdSocket *item=(m_connect_cb_fn)(connfd, ssl, m_connect_cb_cl);
	if(item){
		m_connections[connfd]=item;

		if(m_flag_ssl_ready){
			fcntl(connfd, F_SETFL, tflags | O_NONBLOCK);
			item->socketEvent(cbsdSocket::SSSL, ssl);
		}		

		item->socketEvent(cbsdSocket::OPEN, &connfd);

		return;
	}

	_connectFailed(connfd, ssl, "Parent did not accept."); 


}

void cbsdListener::_handleDisconnect(int fd){

        m_mutex.lock();
        if(m_connections.find(fd) == m_connections.end()){ close(fd); m_mutex.unlock(); return; }
	cbsdSocket *c = m_connections[fd];

	c->socketEvent(cbsdSocket::CLOSE);

	EV_SET(&m_evSet, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);

        close(fd);
        m_connections.erase(fd);
        m_mutex.unlock();

	if(!c->isPersistent()) delete c;

	LOG(cbsdLog::DEBUG) << "Connection closed!";

}

void cbsdListener::listenThreadHandler(void){
	m_flag_thread_running=true;
//	m_flag_dont_accept=false;

	while(m_flag_thread_running){
		int nev = kevent(m_kq, NULL, 0, m_evList, 32, NULL);
		for (int i = 0; i < nev; i++) {
			int fd = (int)m_evList[i].ident;
			if (m_evList[i].flags & EV_EOF) _handleDisconnect(fd);
			else if(m_evList[i].flags & EV_ERROR){ LOG(cbsdLog::CRITICAL) << "Error kevent: " << strerror(m_evList[i].data); }
			else if (fd == m_fd) _handleAccept(fd);
			else if (m_evList[i].filter == EVFILT_READ || m_evList[i].filter == EVFILT_WRITE){
				m_mutex.lock();
				if(m_connections.find(fd) == m_connections.end()){ close(fd); m_mutex.unlock(); LOG(cbsdLog::CRITICAL) << "Invalid connection"; return; }
				m_mutex.unlock();
				if (m_evList[i].filter == EVFILT_READ) m_connections[fd]->socketEvent(cbsdSocket::READ); 
				else m_connections[fd]->socketEvent(cbsdSocket::WRITE);
			} else if (m_evList[i].filter == -1){ _handleDisconnect(fd);
			} else { LOG(cbsdLog::CRITICAL) << "Unknown kevent: "  << m_evList[i].filter; }
		}
	}
}


bool cbsdListener::Start(){

	if(!setupSSL("/etc/ssl/clusterca.crt", "/etc/ssl/Controller.crt", "/etc/ssl/Controller.key", "geheim")) return(false);
	if(!_initialize()) return(false);

	


	return(true);
}
