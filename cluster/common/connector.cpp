/* connector.cpp - Connector class for CBSD Cluster Daemon
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
#include "connector.hpp"


extern int ssl_callback(char *buf, int size, int rwflag, void *u);


cbsdConnector::cbsdConnector(const std::string &name){
	m_name=name;							// My name..
	m_cert_pass=NULL;						// Make sure this is NULL initilialized
	m_ssl_ctx=NULL;							// No ssl context yet!
	m_sock=-1;
	m_flags=0;							// We start with all flags at 0!
	m_kq=kqueue();   						// ...
	socketpair(AF_UNIX, SOCK_STREAM, 0, m_sync_fd); 		// We use this to wake-up the thread when needed

	LOG(cbsdLog::DEBUG) << "Connector " << m_name << " loaded";
}

void cbsdConnector::_doUnload(){
	if(m_is_thread_running){
		LOG(cbsdLog::DEBUG) << "Connector " << m_name << " stopping thread";
		m_is_thread_stopping=true;	// Make the thread stop
		close(m_sync_fd[0]); 		// Should wake up thread
		threadID.join();		// We join
	}
}

cbsdConnector::~cbsdConnector(){
	if(m_is_thread_running) _doUnload();

	_ConnectFailed();			// Make sure we are disconnected..
	SSL_CTX_free(m_ssl_ctx);		// Free the context
	EVP_cleanup();				// Cleanup SSL
	m_ssl_ctx=NULL;				// ... why not ...

	if(NULL != m_cert_pass) free(m_cert_pass);

	LOG(cbsdLog::DEBUG) << "Connector " << m_name << " unloaded";
}


inline bool cbsdConnector::_ConnectFailed(){
	m_is_connected=false;
	if(NULL != m_ssl){					// Clean up SSL stuff
		SSL_shutdown(m_ssl); 				//
		SSL_free(m_ssl); 				// 
		m_ssl=NULL; 					// Make sure we do this once.
	}
	if(m_sock != -1){ close(m_sock); m_sock=-1; }		// Close the socket also
	return(false);						// We always return false..
}

bool cbsdConnector::Connect(const std::string &hostname, const uint16_t port){

	struct sockaddr_in             addr;

	if((addr.sin_addr.s_addr=inet_addr(hostname.c_str()))==-1){ LOG(cbsdLog::CRITICAL) << "Cannot parse hostname"; return(false); }
	if((m_sock=socket(PF_INET, SOCK_STREAM, 0))==-1){ LOG(cbsdLog::CRITICAL) << "Cannot create socket"; return(false); }
	addr.sin_family = AF_INET; addr.sin_port = htons(port);

	if (connect(m_sock, (struct sockaddr *) &addr, sizeof(addr)) == -1){ LOG(cbsdLog::CRITICAL) << "Connection failed"; return(_ConnectFailed()); } 

	if(m_is_ssl_ready){
		int ret;

		m_ssl = SSL_new(m_ssl_ctx);

		BIO *sbio = BIO_new_socket(m_sock, BIO_NOCLOSE); if(sbio == NULL) { LOG(cbsdLog::CRITICAL) << "Failed to create BIO";  return(_ConnectFailed()); }
		SSL_set_bio(m_ssl,sbio,sbio);
		if((ret=SSL_connect(m_ssl)) != 1){ 
			LOG(cbsdLog::CRITICAL) << "Handshake Error " << std::to_string(SSL_get_error(m_ssl,ret)); 
			/* ERR_print_errors_fp(stderr); */
			return(_ConnectFailed()); 
		}

		X509                *cert = SSL_get_peer_certificate(m_ssl);
		X509_NAME_ENTRY     *certname = NULL;

		if (cert == NULL){ LOG(cbsdLog::CRITICAL) << "Could not get a certificate from controller."; return(_ConnectFailed()); }

		int common_name_loc = -1;
		common_name_loc = X509_NAME_get_index_by_NID(X509_get_subject_name(cert),NID_commonName, -1);
		certname = X509_NAME_get_entry(X509_get_subject_name(cert), common_name_loc);
		if(certname != NULL){
			ASN1_STRING *common_name_asn1 = X509_NAME_ENTRY_get_data(certname);
			if(common_name_asn1 != NULL) {
				char *common_name_str = (char *) ASN1_STRING_get0_data(common_name_asn1);
				LOG(cbsdLog::DEBUG) << "Connected to '" + std::string(common_name_str) << "' with SSL";
			}else{
				X509_free(cert);
				LOG(cbsdLog::CRITICAL) << "Could not get a certificate from controller."; 
				return(_ConnectFailed()); 
			}

		}else{
			X509_free(cert);
			LOG(cbsdLog::CRITICAL) << "Could not get certificate name from controller.";
			return(_ConnectFailed()); 
		}
		X509_free(cert);
	}else{
		LOG(cbsdLog::CRITICAL) << "Non SSL sockets is not yet supported!";
		return(_ConnectFailed());
	}

	m_is_connected=true;
	if(!m_is_thread_running){
		EV_SET(&m_evSet, m_sync_fd[1], EVFILT_READ, EV_ADD, 0, 0, NULL); 
		if(-1 == kevent(m_kq, &m_evSet, 1, NULL, 0, NULL)) return(_ConnectFailed());
                threadID=threadHandlerProc(); // Start the thread...
	}

        EV_SET(&m_evSet, m_sock, EVFILT_READ, EV_ADD, 0, 0, NULL); 
	if(-1 == kevent(m_kq, &m_evSet, 1, NULL, 0, NULL)) return(_ConnectFailed());

	return(true);
}

bool cbsdConnector::setupSSL(const std::string &ca, const std::string &cert, const std::string &key, const std::string &pass){
	if(m_is_ssl_ready) return(true);	// Pass trough or error?!

	SSL_load_error_strings();		// Initialize SSL errros
        SSL_library_init();			// Initialize Library
        OpenSSL_add_ssl_algorithms();		// Do that..

	const SSL_METHOD *method=TLS_method();
        RAND_seed("CBSD-ROCKS!",10);

	if((m_ssl_ctx=SSL_CTX_new(method))==NULL){ 
		LOG(cbsdLog::CRITICAL) << "Failed to initalize SSL! "; // ERR_print_errors_fp(stderr); 
		return(false); 
	}

//	SSL_CTX_set_ecdh_auto(m_ssl_ctx, 1);
//	SSL_CTX_set_cipher_list(m_ssl_ctx, "HIGH:!aNULL:!kRSA:!RC4:!MD5");

        SSL_CTX_set_default_passwd_cb(m_ssl_ctx, ssl_callback);
	if(!pass.empty()){
		m_cert_pass=strdup(pass.c_str());
		SSL_CTX_set_default_passwd_cb_userdata(m_ssl_ctx, m_cert_pass);
	}
	if (SSL_CTX_use_certificate_chain_file(m_ssl_ctx, ca.c_str()) <= 0){
		LOG(cbsdLog::CRITICAL) << "Error; Failed to initalize SSL CA-chain!"; // ERR_print_errors_fp(stderr);
		return(false);
	}
	if (SSL_CTX_use_certificate_file(m_ssl_ctx, cert.c_str(), SSL_FILETYPE_PEM) <= 0){
		LOG(cbsdLog::CRITICAL) << "Failed to initalize SSL CRT!"; // ERR_print_errors_fp(stderr);
		return(false);
	}
	if (SSL_CTX_use_PrivateKey_file(m_ssl_ctx, key.c_str(), SSL_FILETYPE_PEM) <= 0 ){ 
		LOG(cbsdLog::CRITICAL) << "Failed to initalize SSL key";
		return(false); 
	}
	if(SSL_CTX_check_private_key(m_ssl_ctx) != 1){ 
		LOG(cbsdLog::CRITICAL) << "Certificate does not match key!"; 
		return(false); 
	}
	if(!SSL_CTX_load_verify_locations(m_ssl_ctx, ca.c_str(), NULL)){ 
		LOG(cbsdLog::CRITICAL) << "Failed to initalize SSL CA";
		return(false); 
	}

	
	return((m_is_ssl_ready=true));
}

bool cbsdConnector::TransmitRaw(const std::string &data){
	if(m_ssl){
		int ret=SSL_write(m_ssl, data.c_str(), data.size()); 
		if(ret != data.size()){ LOG(cbsdLog::WARNING) << "Incomplete write!"; _ConnectFailed(); }else return(true);
		
	}
	return(false);

}

void cbsdConnector::threadHandler(void){
	if(m_is_thread_running) return;			// should never happen.. but ok..
	LOG(cbsdLog::DEBUG) << "Starting connector thread handler";

	m_is_thread_running=true;
	while(!m_is_thread_stopping){
		LOG(cbsdLog::DEBUG) << "Connection thread waiting for events";

		int nev = kevent(m_kq, NULL, 0, m_evList, 32, NULL);		// Fetch k-queue events..
		for (int i = 0; i < nev; i++) {
			int fd = (int)m_evList[i].ident;
			if (m_evList[i].flags & EV_EOF){
				if(fd == m_sock){
					EV_SET(&m_evSet, m_sock, 0, EV_DELETE, 0, 0, NULL); kevent(m_kq, &m_evSet, 1, NULL, 0, NULL);
					_ConnectFailed();
					LOG(cbsdLog::WARNING) << "Server has disconnected/died!";
				} // else we are shutting down!
				LOG(cbsdLog::WARNING) << "Server has shutting down!";
				break;
			}else if(m_evList[i].flags & EV_ERROR){
				LOG(cbsdLog::WARNING) << "Connection has kevent error!";
			}else if (m_evList[i].filter == EVFILT_READ && fd == m_sock){
				std::string data=std::string();				// Te dump it in a std::string..
				char buffer[2048];
				int bytes_read=SSL_read(m_ssl, buffer, sizeof(buffer));
                                data.append(buffer, bytes_read);
                                if(!_handleData(data)){
					LOG(cbsdLog::WARNING) << "Connection was dropped due to invalid data!";
					EV_SET(&m_evSet, m_sock, 0, EV_DELETE, 0, 0, NULL); kevent(m_kq, &m_evSet, 1, NULL, 0, NULL);
					_ConnectFailed();
				}
			}else{
				LOG(cbsdLog::WARNING) << "Connection has unknown kevent error!";
			}
		}
	}
	m_is_thread_running=false;
	m_is_thread_stopping=false;

	LOG(cbsdLog::DEBUG) << "Stopped connector thread handler";
}


