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
	m_name=name;
	m_flag_ssl_ready=false;
	m_cert_pass=NULL;
	m_ssl_ctx=NULL;	
	LOG(cbsdLog::DEBUG) << "Connector " << m_name << " loaded";
}

cbsdConnector::~cbsdConnector(){

	SSL_CTX_free(m_ssl_ctx);		// Free the context
	EVP_cleanup();				// Cleanup SSL
	m_ssl_ctx=NULL;				// ... why not ...

	if(NULL != m_cert_pass) free(m_cert_pass);

	LOG(cbsdLog::DEBUG) << "Connector " << m_name << " unloaded";
}


inline bool cbsdConnector::_ConnectFailed(){
	if(NULL != m_ssl){
		SSL_shutdown(m_ssl); 
		SSL_free(m_ssl); 
		m_ssl=NULL; 
	}
	if(m_sock != -1){ close(m_sock); m_sock=-1; }
	return(false);	// Always return false..
}

bool cbsdConnector::Connect(const std::string &hostname, const uint16_t port){

	 struct sockaddr_in             addr;

	if((addr.sin_addr.s_addr=inet_addr(hostname.c_str()))==-1){ LOG(cbsdLog::CRITICAL) << "Cannot parse hostname"; return(false); }
	if((m_sock=socket(PF_INET, SOCK_STREAM, 0))==-1){ LOG(cbsdLog::CRITICAL) << "Cannot create socket"; return(false); }
	addr.sin_family = AF_INET; addr.sin_port = htons(port);

	if (connect(m_sock, (struct sockaddr *) &addr, sizeof(addr)) == -1){ LOG(cbsdLog::CRITICAL) << "Connection failed"; return(_ConnectFailed()); } 

	if(m_flag_ssl_ready){
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
		return(true);
	}

	LOG(cbsdLog::CRITICAL) << "Non SSL sockets is not yet supported!";
	return(false);
}

bool cbsdConnector::setupSSL(const std::string &ca, const std::string &cert, const std::string &key, const std::string &pass){
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

	
	return((m_flag_ssl_ready=true));
}

bool cbsdConnector::Transmit(const std::string &data){
	if(m_ssl){
		int ret=SSL_write(m_ssl, data.c_str(), data.size()); 
		if(ret != data.size()){ LOG(cbsdLog::WARNING) << "Incomplete write!"; }else return(true);
		
	}
	
	return(false);

}
