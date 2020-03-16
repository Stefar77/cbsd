/* msgbus.cpp - Messagebus class for CBSD Cluster Node 
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
 *
 * -------------------------------------------------------------------------------
 * 
 *  Makes a socket in jails so they could talk to each other without networking
 *  - Jails do not have to be running on the same node.
 */

#include <iostream>		// std::cout 
#include "msgbus.hpp"
#include <unistd.h>

// Name is in the HPP file! If you clone this don't forget to change is and register it in the moduleid.hpp file!

MODULE_START()
EVENT_LOADED( return(true); )
EVENT_UNLOAD( LOG(cbsdLog::DEBUG) << "Trying to stop MSGBUS thread"; )
EVENT_THREAD(
	/* Thread of the module
	 *  No parameters, get's looped so make sure you sleep or wait on something!
	 */

	sleep(10); // We do nothing here.. 

)
EVENT_RECEIVE( 
	/* Receive event from the Controller
	 *  const std::string &data
	 *  const uint16_t channel
	 */

//	LOG(cbsdLog::DEBUG) << "Received [" << data << "] from controller on channel " << std::to_string(channel) << "."; 
)

MODULE_STOP( LOG(cbsdLog::DEBUG) << "MSGBUS module unloaded nicely!"; )


/*
  - uint16_t Socket# [channel]
  - std::string name
  -- **vector**
  --: uint32_t Jail# - [File flags]
  --: ...


int cbsdMSGBUS::createSocket(path){
	if (stat(path.c_str(), &st) == 0 && unlink(path.c_str()) != 0) return(-1);

	int fd=socket(PF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) return(false); // Failed to create socket
	if (fcntl(fd, F_SETFD, FD_CLOEXEC) == -1) { close(fd); return(-1); }
	
	struct sockaddr_un sockaddr;
	sockaddr.sun_family = AF_UNIX;
	strncpy(sockaddr.sun_path, path.c_str(), sizeof(sockaddr.sun_path) - 1);
	sockaddr.sun_path[sizeof(sockaddr.sun_path) - 1] = '\0';

	if (bind(fd, reinterpret_cast<struct sockaddr *>(&sockaddr), sizeof(sockaddr)) < 0) { close(fd); return(-1); }

	if (0 != chmod(path.c_str(), 0660)){ close(fd); return(-1); }

	if (0 != listen(fd, 3)) { close(fd); return(-1); }

	return(fd);	
}


*/
