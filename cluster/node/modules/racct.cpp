/* racct.cpp - RACCT class for CBSD Cluster Node
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

#include <iostream>		// std::cout 
#include "racct.hpp"
#include <unistd.h>
#include <sys/sysctl.h>

MODULE_START(m_pcpu=0; m_pmem=0;)
EVENT_LOADED( _getHosterStats(); return(true); )
EVENT_UNLOAD( LOG(cbsdLog::DEBUG) << "Trying to stop RACCT thread"; )
EVENT_THREAD(
	sleep(1);		// Important!! This will loop until the node is stopped or module unloaded!

	_getHosterStats();

	// Debug / temporary...
	int tmp1=0|m_pcpu, tmp2=0|m_pmem;
	std::string data = "CPU=" + std::to_string(tmp1) + ",MEM=" + std::to_string(tmp2) + ",FILES=" + std::to_string(m_openfiles) + ",TEMP=" + std::to_string(m_temperature);

	/* Transmit to the Controller on channel 1 */
	TransmitBuffered(1, data);
)
EVENT_RECEIVE( LOG(cbsdLog::DEBUG) << "Received [" << data << "] from controller on channel " << std::to_string(channel) << "."; )

MODULE_STOP( LOG(cbsdLog::DEBUG) << "RACCT module unloaded nicely!"; )


// User functions...

void	cbsdRACCT::_getHosterStats(void){


	m_pcpu=cbsdUtils::getPCPU();	 		// Host CPU
	m_pmem=cbsdUtils::getPMEM();			// Host Memory
	m_temperature=cbsdUtils::cpuTemperature();	// Temperature
	m_openfiles=cbsdUtils::getOpenFiles();		// Open files



}

