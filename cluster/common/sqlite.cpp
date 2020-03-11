/* sqlite.cpp - SQLite class for CBSD Cluster Daemon
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

#include "sqlite.hpp"

cbsdSQLite::cbsdSQLite() {
	LOG(cbsdLog::DEBUG) << "SQLite connection loaded";
}

cbsdSQLite::~cbsdSQLite() {
	Close();						// In case we still have stuff opened..
	LOG(cbsdLog::DEBUG) << "SQLite connection unloaded";
}


void cbsdSQLite::Close(){
	if(m_db){
		sqlite3_close(m_db);				// Clean-up any used resources
		m_db=NULL;
		LOG(cbsdLog::DEBUG) << "SQLite disconnected from dbfile";
	}

}

bool cbsdSQLite::Open(const std::string &filename){
	int	res;	// flags: SQLITE_OPEN_CREATE

        if (SQLITE_OK != (res = sqlite3_open_v2(filename.c_str(), &m_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_SHAREDCACHE, NULL))) {
		m_db=NULL; return(false); // This clearly didn't work..
	}

	_doQuery("PRAGMA mmap_size = 209715200;");

	LOG(cbsdLog::DEBUG) << "SQLite connected to dbfile: " << filename;
	return(true);
}

/* Private functions */
bool cbsdSQLite::_doQuery(const std::string &query){
	return(false);
}
