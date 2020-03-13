#ifndef OBJECTSTORAGE_HPP
#define OBJECTSTORAGE_HPP

class cbsdObjectStore {
 public:
	virtual ~cbsdObjectStore() {}

	/* Functions/Methods */
	virtual uint8_t					getPriority()=0;

	virtual std::map<std::string, std::string>	fetchObject(const std::string &container, const std::string &match, const std::string &val)=0;
	virtual bool					storeObject(const std::string &container, std::map<std::string, std::string> data)=0;

	virtual bool					hasContainer(const std::string &container) = 0;
};

#endif
