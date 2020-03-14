#ifndef MACROS_HPP
#define MACROS_HPP


#define CBSDDBCLASS(cname, iname, pname) \
	void cname::Log(const uint8_t level, const std::string &data){ std::map<std::string,std::string> item; item["msg"]=data; Log(level, item); }\
	void cname::Log(const uint8_t level, std::map<std::string,std::string> data){ CBSD->Log(level, data); } \
	void cname::Publish(const std::string &key, const std::string &val){ std::map<std::string,std::string> item; item[key]=val; Publish(item); }\
	void cname::Publish(std::map<std::string,std::string> data){ CBSD->Publish(data); } \
	iname	*cname::find(const std::string &name){ iname *item=NULL; m_mutex.lock(); for (std::map<uint32_t, iname *>::iterator it = pname.begin(); it != pname.end(); it++){ if(it->second->getName() == name){ item=it->second; break; } } m_mutex.unlock(); if(item) return(item); return(NULL); } \
	iname   *cname::find(const uint32_t id){ iname *item=NULL; m_mutex.lock(); std::map<uint32_t, iname *>::iterator it = pname.find(id); if(it != pname.end()) item=it->second; m_mutex.unlock(); if(item) return(item); return(NULL); }

        // TODO: Check external
        // map<std::string, std::string> res=Datastore->fetch("tasks", "id", std::to_string(id));
        // map<std::string, std::string> res=Datastore->fetch("tasks", "name", name); // TODO!!


#define CBSDDBITEM(cname, iname, pname) \
	void cname::Log(const uint8_t level, const std::string &data){ std::map<std::string,std::string> item; item["msg"]=data; Log(level, item); } \
	void cname::Log(const uint8_t level, std::map<std::string,std::string> data){ data[iname]=m_name; pname->Log(level, data); } \
	void cname::Publish(const std::string &key, const std::string &val){ std::map<std::string,std::string> item; item[key]=val; Publish(item); }\
	void cname::Publish(std::map<std::string,std::string> data){ data[iname]=m_name; pname->Publish(data); } \


#define CBSDDBCLASSI(iname) \
 public: \
	iname			*find(const std::string &name); \
	iname			*find(const uint32_t id); \
 protected: \
	void			 Log(const uint8_t level, const std::string &data); \
	void			 Log(const uint8_t level, std::map<std::string,std::string> data); \
	void			 Publish(const std::string &key, const std::string &val); \
	void			 Publish(std::map<std::string,std::string> data); \


#define CBSDDBITEMI(iname) \
	void	 Log(const uint8_t level, const std::string &data); \
	void	 Log(const uint8_t level, std::map<std::string,std::string> data); \
	void	 Publish(const std::string &key, const std::string &val); \
	void	 Publish(std::map<std::string,std::string> data); 


#define SMAPIT	std::map<std::string,std::string>::iterator 




#ifdef WITH_DEBUG
#define IFDEBUG(...) __VA_ARGS__
#else
#define IFDEBUG(...) 
#endif
#endif
