/* log.cpp - Log class for CBSD Cluster Daemons
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
#include <iomanip>
#include "log.hpp"

std::mutex  	    cbsdLog::mutex;

cbsdLog::~cbsdLog() { }
cbsdLog &cbsdLog::instance(uint8_t globalLogLevel, std::ostream& output){static cbsdLog instance(globalLogLevel, output); return(instance); }
Delegate cbsdLog::log(uint8_t level){ /* cbsdLog::mutex.lock(); */ timeStamp(); logLevelStamp(level); return Delegate(m_log_stream, endlineStrategy(level)); }
uint8_t cbsdLog::getGlobalLogLevel(){ return this->m_level; }
void cbsdLog::logLevelStamp(uint8_t level) { m_log_stream << logLevelString(level) + ": "; }
void cbsdLog::timeStamp(){ 
	auto time = std::time(nullptr);
	m_log_stream << std::put_time(std::localtime(&time), "%F %T%z "); 
}

std::string cbsdLog::logLevelString(uint8_t level) {
    switch (level) {
        case FATAL: return "FATAL";
        case CRITICAL: return "CRITICAL";
        case DEBUG: return "DEBUG";
        case INFO:  return "INFO";
        case WARNING: return "WARN";
        case ERROR: return "ERROR";
        case NONE: break;
        default: break;
    }
    return "";
}

std::function<void(std::ostream&)> cbsdLog::endlineStrategy(uint8_t level) {
    std::function<void(std::ostream&)> endlineNoFlush = [](std::ostream& os) { os << "\n"; };
    std::function<void(std::ostream&)> endlineFlush = [](std::ostream& os) { os << std::endl; };

    switch (level) {
	    case CRITICAL: return endlineNoFlush;
	    case DEBUG: return endlineNoFlush;
	    case FATAL: return endlineNoFlush;
	    case INFO:  return endlineNoFlush;
	    case WARNING: return endlineFlush;
	    case ERROR: return endlineFlush;
	    case NONE: break;
	    default: break;
    }
	return(NULL);
}

cbsdLog::cbsdLog(uint8_t globalLogLevel, std::ostream& output) : m_level(globalLogLevel), m_log_stream(output) { }
Delegate::Delegate(std::ostream & os, std::function<void(std::ostream&)> endline) : ostream(os), endline(endline) { }
Delegate::~Delegate() { endline(ostream); /* cbsdLog::mutex.unlock(); */ }


