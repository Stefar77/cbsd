/* main.cpp - Main function for CBSD Cluster Daemon
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

#include <unistd.h>		// sleep
#include <iostream>		// std::cout 
#include "nodes.hpp"
#include <csignal>
#include "../common/version.hpp"

cbsdNodes	*Nodes;
bool		keepRunning;

void signalHandler(int sig) {
	switch(sig){
		case SIGTERM: keepRunning=false; LOG(cbsdLog::WARNING) << "Terminate signal received, quitting!"; break;
		case SIGHUP: LOG(cbsdLog::INFO) << "Signal HUP received"; break;
		default: LOG(cbsdLog::INFO) << "Signal " << sig << " received!"; break;
	}
}


int main(int argc, char **argv){
	int	rc=0;
	keepRunning=true;
	signal(SIGHUP, signalHandler);  
	signal(SIGTERM, signalHandler);  

	LOGGER_INIT(cbsdLog::DEBUG, std::cout);

	LOG(cbsdLog::INFO) << "CBSD Controller daemon version " << VERSION;

	Nodes=new cbsdNodes();

	while(keepRunning){
		/*   
 		 *   I love this thread! 
		 *    - It just sleeps all the time...
		 */
		sleep(60);
	}

	LOG(cbsdLog::INFO) << "Shutting down controller!";
	delete Nodes;
	exit(rc);
}
