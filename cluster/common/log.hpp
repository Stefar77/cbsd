#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>
#include <string>
#include <functional>


class Delegate {
public:
	Delegate(std::ostream& os, std::function<void(std::ostream&)> endlineStrategy);
	~Delegate();
	Delegate(const Delegate&) = delete;
	Delegate& operator=(const Delegate&) = delete;
	Delegate(Delegate&&) = default;
	Delegate& operator=(Delegate&&) = default;
	template<class T> inline Delegate& operator<<(T&& output){ ostream << std::forward<T>(output); return *this; }
 private:
	std::ostream&	ostream;
	std::function<void(std::ostream&)> endline;
};

class cbsdLog {
 public:
	static cbsdLog& instance(uint8_t globalLogLevel = cbsdLog::NONE, std::ostream& output = std::cout);
	~cbsdLog();

        static std::mutex		mutex;			// My lock

	cbsdLog(cbsdLog const&) = delete;
	void operator=(cbsdLog const&) = delete;

	Delegate log(uint8_t level);

	uint8_t getGlobalLogLevel();

	enum { 
		NONE=0,			//
		FATAL,			//
		CRITICAL,		//
		ERROR,			//
		WARNING,		//
		INFO,			//
		DEBUG			//
	};

 protected:
	void timeStamp();
	void logLevelStamp(uint8_t level);
	std::function<void(std::ostream&)> endlineStrategy(uint8_t level);
	std::string logLevelString(uint8_t level);

 private:
	cbsdLog();
	cbsdLog(uint8_t logLevel, std::ostream& output);

	uint8_t			m_level;
	std::ostream& 		m_log_stream;


};

#define LOGGER_INIT(logLevel, output) cbsdLog::instance(logLevel, output); 
#define LOG(level) if(cbsdLog::instance().getGlobalLogLevel() >= (level) && (level) > cbsdLog::NONE) cbsdLog::instance().log(level)


#endif
