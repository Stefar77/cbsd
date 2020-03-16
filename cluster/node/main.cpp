/* main.cpp - Main function for CBSD Node Daemon
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
#include <csignal>
#include "../common/version.hpp"
#include "master.hpp"
#include "modules/all.hpp"
#include "../config.hpp"
#include "../common/sqlite.hpp"

cbsdMaster	*master;
bool		keepRunning;


/* Initialize statics */
uint64_t cbsdUtils::page_size=sysconf(_SC_PAGE_SIZE);
uint64_t cbsdUtils::total_memory=(sysconf(_SC_PHYS_PAGES) * cbsdUtils::page_size);
uint32_t cbsdUtils::total_cores=sysconf(_SC_NPROCESSORS_ONLN);


void signalHandler(int sig) {
	switch(sig){
		case SIGTERM: keepRunning=false; LOG(cbsdLog::WARNING) << "Signal TERM received, quitting!"; break;
		case SIGHUP: LOG(cbsdLog::INFO) << "Signal HUP received!"; break;
		default: LOG(cbsdLog::INFO) << "Signal " << sig << " received!";
	}

}


int main(int argc, char **argv){
	int	rc=0;

	keepRunning=true;
	signal(SIGHUP, signalHandler);  
	signal(SIGTERM, signalHandler);  


	LOGGER_INIT(cbsdLog::LoggingLevel, std::cout);

	LOG(cbsdLog::INFO) << "CBSD Node daemon version " << VERSION;
	LOG(cbsdLog::INFO) << "CBSD Root is at '" << CBSDROOT "'";

	{	// Enclose this so sql get's unloaded after we are done!
		cbsdSQLite	*sql=new cbsdSQLite();
		if(!sql->Open(CBSDROOT "/var/db/local.sqlite")){
			LOG(cbsdLog::WARNING) << "Failed to open local sqlite database!";
		}else{
			LOG(cbsdLog::DEBUG) << "SQLite: " << sql->getValue("SELECT jid FROM jails WHERE jname='jail1'");

		}
	}




	master=new cbsdMaster();
	master->loadModule(new cbsdRACCT());			// Load modules first, then add contoller so it can negotiate.
	master->loadModule(new cbsdMSGBUS());

	keepRunning=master->doSetup(ControllerIP, ControllerPORT);

	while(keepRunning){
		/*   
 		 *   I love this thread! 
		 *    - It just sleeps all the time...
		 */
		sleep(1);
	}

	LOG(cbsdLog::INFO) << "Shutting down node daemon!";
	delete master;
	exit(rc);
}


//#define ClusterCA /etc/ssl/clusterca.crt
//#define ControllerCRT /etc/ssl/Controller.crt
//#define ControllerKEY /etc/ssl/Controller.key
//#define NodeCRT /etc/ssl/GUI.crt
//#define NodeKEY /etc/ssl/GUI.key
//#define LoggingLevel DEBUG


