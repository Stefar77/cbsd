#ifndef MODULE_HPP
#define MODULE_HPP

#define MODULES_MAP std::map<uint16_t, cbsdModule *>

#include "nodes.hpp"
#include "../common/moduleids.hpp"
#include "../config.hpp"



#define APPEND_STR(A, B) A ## B
#define APPEND(A, B) APPEND_STR(A, B)
#define Q(A) #A
#define QUOTE(A) Q(A)
#define MKSTR(A) QUOTE(A)
#define MYNAME cbsd
#define MOD_PRE MID_
#define MODULE_START(...) APPEND(MYNAME,MODULE_NAME)::APPEND(MYNAME,MODULE_NAME)(): cbsdModule(APPEND(MOD_PRE, MODULE_NAME),MKSTR(MODULE_NAME)) { __VA_ARGS__ } 
#define MODULE_STOP(...) APPEND(MYNAME,MODULE_NAME)::~APPEND(MYNAME,MODULE_NAME)() { __VA_ARGS__ } 
#define EVENT_LOADED(...) bool APPEND(MYNAME,MODULE_NAME)::moduleLoaded(){ __VA_ARGS__ }
#define EVENT_UNLOAD(...) void APPEND(MYNAME,MODULE_NAME)::moduleUnloading(){ __VA_ARGS__ }
#define EVENT_RECEIVE(...) void APPEND(MYNAME,MODULE_NAME)::moduleReceive(cbsdNode *node, const uint16_t channel, const std::string &data){ __VA_ARGS__ }
#define EVENT_THREAD(...) void APPEND(MYNAME,MODULE_NAME)::moduleThread(){ __VA_ARGS__ }

#define MODULE_HPP_START(...) class APPEND(MYNAME,MODULE_NAME): public cbsdModule { friend class cbsdNodes; public: APPEND(MYNAME,MODULE_NAME)(); ~APPEND(MYNAME,MODULE_NAME)(); private: void moduleUnloading() override; bool moduleLoaded() override; void moduleThread() override; void moduleReceive(cbsdNode *node, const uint16_t channel, const std::string &data) override; __VA_ARGS__
#define MODULE_HPP_DONE(...) __VA_ARGS__ };

class cbsdNode;
class cbsdModule {
 friend class cbsdNode;
 friend class cbsdNodes;

 public:
	cbsdModule(const uint16_t id, const std::string &name);
	virtual ~cbsdModule();

	/* Functions/Methods */
	inline std::string 	&getName(){ return(m_name); }
	inline uint16_t 	getID(){ return(m_id); }
	bool			transmitRaw(cbsdNode *node, const std::string &data);
	bool			Transmit(cbsdNode *node, uint16_t channel, const std::string &data);
	void			Disable(){ m_flag_thread_disabled=true; }
	void			Enable(){ m_flag_thread_disabled=false; }
	bool			isEnabled(){ return(!m_flag_thread_disabled); }
	void			Log(const uint8_t level, const std::string &data);
        void			Log(const uint8_t level, std::map<std::string,std::string> data);

 protected:
	bool			 Init();
	void			 doUnload();


 private:
	cbsdNodes		*m_nodes;	// TODO: use statics?!
	std::string		 m_name;	

	/* Functions/Methods */
	virtual void		moduleUnloading()=0;
	virtual bool		moduleLoaded()=0;
	virtual void		moduleReceive(cbsdNode *node, const uint16_t type, const std::string &data)=0;
	virtual void		moduleThread();


	std::thread		m_threadID;                     // ThreadID for the modules thread..
        void			threadHandler(void);

	uint16_t		m_id;				// Module ID number
	union {
	 uint16_t		m_flags;			// flags
	 struct{
	  uint16_t		m_flag_reserved:13;		// ...
	  uint16_t		m_flag_thread_disabled:1;	// Thread is disabled
	  uint16_t		m_flag_thread_stopping:1;	// Thread is stopping
	  uint16_t		m_flag_thread_running:1;	// Thread is running
	 };
	};


};

#endif
