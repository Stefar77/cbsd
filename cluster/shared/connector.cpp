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
#ifdef ISNODE
#include <map>
#include "connector.hpp"
#else
#include "../controller/cbsd.hpp"
#endif

extern int ssl_callback(char *buf, int size, int rwflag, void *u);

#ifndef IFDEBUG
#define IFDEBUG(__VA_ARGS__)
#endif

cbsdConnector::cbsdConnector(const std::string &name){
	m_name=name;							// My name..
	m_cert_pass=NULL;						// Make sure this is NULL initilialized
	m_ssl_ctx=NULL;							// No ssl context yet!
	m_ssl=NULL;							// 
	m_sock=-1;
	m_flags=0;							// We start with all flags at 0!
	m_kq=kqueue();   						// ...
	socketpair(AF_UNIX, SOCK_STREAM, 0, m_sync_fd); 		// We use this to wake-up the thread when needed

	IFDEBUG(Log(cbsdLog::DEBUG, "Connector loaded");)
}

void cbsdConnector::_doUnload(){
	if(m_is_thread_running){
		IFDEBUG(Log(cbsdLog::DEBUG, "stopping thread");)
		m_is_thread_stopping=true;	// Make the thread stop
		close(m_sync_fd[0]); 		// Should wake up thread
		threadID.join();		// We join
	}
}

cbsdConnector::~cbsdConnector(){
	if(m_is_thread_running) _doUnload();

	_ConnectFailed("Unloading");		// Make sure we are disconnected..
	SSL_CTX_free(m_ssl_ctx);		// Free the context
	EVP_cleanup();				// Cleanup SSL
	m_ssl_ctx=NULL;				// ... why not ...

	if(NULL != m_cert_pass) free(m_cert_pass);

	IFDEBUG(Log(cbsdLog::DEBUG, "Connector unloaded");)
}

void cbsdConnector::Disconnect(){
	_ConnectFailed("Disconnected"); // for now.
}

inline bool cbsdConnector::_ConnectFailed(const std::string &reason){
	Log(cbsdLog::WARNING, "Disconnected [" + reason + "]");

	if(NULL != m_ssl){					// Clean up SSL stuff
		SSL_shutdown(m_ssl); 				//
		SSL_free(m_ssl); 				// 
		m_ssl=NULL; 					// Make sure we do this once.
	}
	if(m_sock != -1){ close(m_sock); m_sock=-1; }		// Close the socket also

	if(m_is_connected){
		m_is_connected=false;
		_Disconnected();				// User event
	}

	return(false);						// We always return false..
}

bool cbsdConnector::Connect(const std::string &hostname, const uint16_t port){
	m_hostname=hostname;
	m_port=port;
	return(Connect());
}

bool cbsdConnector::Connect(){
	struct sockaddr_in	addr;

	if((addr.sin_addr.s_addr=inet_addr(m_hostname.c_str()))==-1){ Log(cbsdLog::CRITICAL, "Cannot parse hostname"); return(false); }
	if((m_sock=socket(PF_INET, SOCK_STREAM, 0))==-1){ Log(cbsdLog::CRITICAL, "Cannot create socket"); return(false); }
	addr.sin_family = AF_INET; addr.sin_port = htons(m_port);

	if (connect(m_sock, (struct sockaddr *) &addr, sizeof(addr)) == -1) return(_ConnectFailed("connect() failed")); 

	if(m_is_ssl_ready){
		int ret;

		m_ssl = SSL_new(m_ssl_ctx);
		if(!m_ssl) return(_ConnectFailed("Failed to create SSL"));

		BIO *sbio = BIO_new_socket(m_sock, BIO_NOCLOSE); 
		if(sbio == NULL) return(_ConnectFailed("Failed to create BIO")); 

		SSL_set_bio(m_ssl,sbio,sbio);
		if((ret=SSL_connect(m_ssl)) != 1){ 
			//Log(cbsdLog::CRITICAL, "Handshake Error " + std::to_string(SSL_get_error(m_ssl,ret))); 
			/* ERR_print_errors_fp(stderr); */
			return(_ConnectFailed("Handshake error")); 
		}

		X509                *cert = SSL_get_peer_certificate(m_ssl);
		X509_NAME_ENTRY     *certname = NULL;

		if (cert == NULL) return(_ConnectFailed("Could not get a certificate.")); 

		int common_name_loc = -1;
		common_name_loc = X509_NAME_get_index_by_NID(X509_get_subject_name(cert),NID_commonName, -1);
		certname = X509_NAME_get_entry(X509_get_subject_name(cert), common_name_loc);
		if(certname != NULL){
			ASN1_STRING *common_name_asn1 = X509_NAME_ENTRY_get_data(certname);
			if(common_name_asn1 != NULL) {
				char *common_name_str = (char *) ASN1_STRING_get0_data(common_name_asn1);
				Log(cbsdLog::DEBUG, "Connected to '" + std::string(common_name_str) + "' with SSL");
			}else{
				X509_free(cert);
			//	Log(cbsdLog::CRITICAL, "Could not get a certificate from controller."); 
				return(_ConnectFailed("Unable to get certificate.")); 
			}

		}else{
			X509_free(cert);
//			Log(cbsdLog::CRITICAL, "Could not get certificate name from controller.");
			return(_ConnectFailed("Unable to get certificate name")); 
		}
		X509_free(cert);
	}

	m_is_connected=true;
	if(!m_is_thread_running){
		EV_SET(&m_evSet, m_sync_fd[1], EVFILT_READ, EV_ADD, 0, 0, NULL); 
		if(-1 == kevent(m_kq, &m_evSet, 1, NULL, 0, NULL)) return(_ConnectFailed("kevent failed for sync"));
                threadID=threadHandlerProc(); // Start the thread...
	}

        EV_SET(&m_evSet, m_sock, EVFILT_READ, EV_ADD, 0, 0, NULL); 
	if(-1 == kevent(m_kq, &m_evSet, 1, NULL, 0, NULL)) return(_ConnectFailed("kevent failed"));

	m_was_connected=true;

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
		Log(cbsdLog::CRITICAL, "Failed to initalize SSL!"); // ERR_print_errors_fp(stderr); 
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
		Log(cbsdLog::CRITICAL, "Error; Failed to initalize SSL CA-chain!"); // ERR_print_errors_fp(stderr);
		return(false);
	}
	if (SSL_CTX_use_certificate_file(m_ssl_ctx, cert.c_str(), SSL_FILETYPE_PEM) <= 0){
		Log(cbsdLog::CRITICAL, "Failed to initalize SSL CRT!"); // ERR_print_errors_fp(stderr);
		return(false);
	}
	if (SSL_CTX_use_PrivateKey_file(m_ssl_ctx, key.c_str(), SSL_FILETYPE_PEM) <= 0 ){ 
		Log(cbsdLog::CRITICAL, "Failed to initalize SSL key");
		return(false); 
	}
	if(SSL_CTX_check_private_key(m_ssl_ctx) != 1){ 
		Log(cbsdLog::CRITICAL, "Certificate does not match key!"); 
		return(false); 
	}
	if(!SSL_CTX_load_verify_locations(m_ssl_ctx, ca.c_str(), NULL)){ 
		Log(cbsdLog::CRITICAL, "Failed to initalize SSL CA");
		return(false); 
	}

	
	return((m_is_ssl_ready=true));
}

void cbsdConnector::_Disconnected(){ }
bool cbsdConnector::_Reconnect(){ return(Connect()); }


bool cbsdConnector::TransmitRaw(const std::string &data){
	int ret;

	if(!m_is_connected){
		if(!m_was_connected || m_is_reconnecting) return(false);
		m_is_reconnecting=true;
		bool test=_Reconnect();
		m_is_reconnecting=false;
		if(!test) return(false);	// Cannot transmit without connection..
	}

	if(m_ssl){
		ret=SSL_write(m_ssl, data.c_str(), data.size()); 	
	}else{
		ret=write(m_sock, data.c_str(), data.size()); 
	}
	if(ret == data.size()) return(true);

	_ConnectFailed("Incomplete write!"); 
	return(false);
}

void cbsdConnector::threadHandler(void){
	if(m_is_thread_running) return;			// should never happen.. but ok..
	IFDEBUG(Log(cbsdLog::DEBUG, "Starting thread handler");)

	m_is_thread_running=true;
	while(!m_is_thread_stopping){
		int nev = kevent(m_kq, NULL, 0, m_evList, 32, NULL);		// Fetch k-queue events..
		for (int i = 0; i < nev; i++) {
			int fd = (int)m_evList[i].ident;
			if (m_evList[i].flags & EV_EOF){
				if(fd == m_sock){
					EV_SET(&m_evSet, m_sock, 0, EV_DELETE, 0, 0, NULL); kevent(m_kq, &m_evSet, 1, NULL, 0, NULL);
					_ConnectFailed("Server disconnected");
//					LOG(cbsdLog::WARNING) << "Server has disconnected/died!";
				} // else we are shutting down!
//				LOG(cbsdLog::WARNING) << "Server is shutting down!";
				break;
			}else if(m_evList[i].flags & EV_ERROR){
				LOG(cbsdLog::WARNING) << "Connection has kevent error!";
			}else if (m_evList[i].filter == EVFILT_READ && fd == m_sock){
				std::string data=std::string();				// Te dump it in a std::string..
				char buffer[2048];
				int bytes_read;
				if(m_ssl){
					bytes_read=SSL_read(m_ssl, buffer, sizeof(buffer));
				}else{
					bytes_read=read(fd, buffer, sizeof(buffer));
				}
                                data.append(buffer, bytes_read);
                                if(!_handleData(data)){
					LOG(cbsdLog::WARNING) << "Connection was dropped due to invalid data!";
					EV_SET(&m_evSet, m_sock, 0, EV_DELETE, 0, 0, NULL); kevent(m_kq, &m_evSet, 1, NULL, 0, NULL);
					_ConnectFailed("Invalid data");
				}
			}else{
				LOG(cbsdLog::WARNING) << "Connection has unknown kevent error!";
			}
		}
	}
	m_is_thread_running=false;
	m_is_thread_stopping=false;

	IFDEBUG(Log(cbsdLog::DEBUG, "Stopped thread handler");)
}


void cbsdConnector::Log(const uint8_t level, const std::string &data){
	std::map<std::string,std::string> item;
	item["msg"]=data;
	Log(level, item);
}
        
void cbsdConnector::Log(const uint8_t level, std::map<std::string,std::string> data){
	data["module"]=m_name;
#ifdef ISNODE
	data["node"]="SuperBSD";	// TEMPORARY
 
	std::string msg="";
	for (std::map<std::string,std::string>::iterator it = data.begin(); it != data.end(); it++){
		msg.append(it->first+"='"+it->second+"' ");
        }

        LOG(level) << msg;
#else
	CBSD->Log(level, data);
#endif
}

