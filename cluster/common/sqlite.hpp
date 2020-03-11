#ifndef SQLITE_HPP
#define SQLITE_HPP

#include "../common/log.hpp"
#define DEFSQLDELIMER "|"
#define DBPOSTFIX ".sqlite"
#include "sqlite3.h"
#include <vector>

extern cbsdLog *Log;


class cbsdSQLite {
 public:
	cbsdSQLite();
	~cbsdSQLite();

	bool			 Open(const std::string &filename);
	void			 Close();
	std::string		 getValue(const std::string &query);
	std::vector<std::string> getValues(const std::string &query);


 private:
	bool 			 _doQuery(const std::string &query);

	sqlite3			*m_db;				// SQLite Resource
	std::string		 m_filename;			// Filename of the database

};

#endif



