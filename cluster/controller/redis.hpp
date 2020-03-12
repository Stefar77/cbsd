#ifndef REDIS_HPP
#define REDIS_HPP

#include <vector>
#include <map>
#include "../shared/connector.hpp"

extern cbsdLog *Log;

class cbsdRedis: public cbsdConnector {
 public:
	cbsdRedis(const std::string &host, uint16_t port, const std::string &password, uint32_t database);
	~cbsdRedis();

	/* Queue functions */
	uint32_t		Publish(const std::string &queue, const std::string &event);

	/* HASH functions */
	std::string		 hGet(const std::string &hash, const std::string &key);
	uint32_t		 hSet(const std::string &hash, const std::string &key, const std::string &val);
	uint32_t		 hSet(const std::string &hash, std::map<std::string, std::string> vals);
	uint32_t		 hDel(const std::string &hash, const std::string &key);
	uint32_t		 hDel(const std::string &hash, const std::vector<std::string> &keys);
	uint32_t		 hLen(const std::string &hash);
	uint32_t		 hExists(const std::string &hash);


 private:
	bool 			_doConnect();
	std::string 		_doRequest(std::vector<std::string> oplist);
	bool			_handleData(const std::string &data) override;
	uint32_t		_intResult(const std::string &data);

	std::condition_variable	 m_cv;
	std::mutex		 m_mutex;

	std::string		 m_response;			// Last response..
	std::string		 m_host;			// Redis host/ip to connect to
	std::string		 m_password;			// Password for Redis database
	uint32_t		 m_database;			// Database number
	uint16_t		 m_port;			// Port number
	union	{
		uint16_t	 m_flags;
// We could just as well use the flags of the parent..
//		struct {
//		 uint16_t	 m_reserved_flags:15;
//		 uint16_t	 m_is_connected:1;
//		};
	};

};

#endif
