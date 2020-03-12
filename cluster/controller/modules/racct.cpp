/* racct.cpp - RACCT class for CBSD Cluster Daemon
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
 *  Collects RACCT statistics from jails, bhyves and the node.
 *
 */

#include "racct.hpp"

// Look in the node/modules/racct.cpp for more help.

MODULE_START( )
EVENT_LOADED( return(true); )
EVENT_UNLOAD( Log(cbsdLog::DEBUG, "Trying to stop RACCT thread"); )
EVENT_THREAD(
	sleep(10);		// Important!! This will loop until the node is stopped or module unloaded!

	Log(cbsdLog::DEBUG, "Doing RACCT controller task...");

//	TransmitBuffered(data);
)
EVENT_RECEIVE( 
	/* Reveice data from a node 
	 *  const std::string &data
	 *  const uint16_t    channel
	 *  cbsdNode 	     *node;
	 */

	switch(channel){
		case 1: { RACCT_u *perf=(RACCT_u *)data.data(); node->setPerfdata(0, perf->performance); } break;
		default:
		{
			std::map<std::string, std::string> item;
			item["node"]=node->getName();
			item["channel"]=std::to_string(channel);
			item["msg"]="Received [" + data + "]";
			Log(cbsdLog::DEBUG, item);
		}
		break;
	}
//	Transmit(node, 1, "Tesing!");

)
MODULE_STOP( Log(cbsdLog::DEBUG, "RACCT module unloaded nicely!"); )


