#ifndef MODULE_HPP
#define MODULE_HPP

#include "master.hpp"
#include "../common/moduleids.hpp"

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
#define EVENT_RECEIVE(...) void APPEND(MYNAME,MODULE_NAME)::moduleReceive(const uint16_t channel, const std::string &data){ __VA_ARGS__ }
#define EVENT_THREAD(...) void APPEND(MYNAME,MODULE_NAME)::moduleThread(){ __VA_ARGS__ }

#define MODULE_HPP_START(...) class APPEND(MYNAME,MODULE_NAME): public cbsdModule { friend class cbsdModule; public: APPEND(MYNAME,MODULE_NAME)(); ~APPEND(MYNAME,MODULE_NAME)(); private: void moduleUnloading() override; bool moduleLoaded() override; void moduleThread() override; void moduleReceive(const uint16_t channel, const std::string &data) override; __VA_ARGS__
#define MODULE_HPP_DONE(...) __VA_ARGS__ };


class cbsdMaster;
class cbsdModule {
 friend class cbsdMaster;

 public:
	cbsdModule(const uint16_t id, const std::string &name);
	virtual ~cbsdModule();

	/* Functions/Methods */
	inline std::string 	&getName(){ return(m_name); }
	inline uint16_t 	getID(){ return(m_id); }
	void			TransmitBuffered(const uint16_t code, const std::string &data);
	inline void		Disable(){ setEnabled(false); }
	inline void		Enable(){ setEnabled(true); }
	inline bool		isEnabled(){ return(m_is_enabled); }
	void 			setEnabled(bool state);


 protected:
	bool			 doLoad(cbsdMaster *master);
	void			 doUnload();


 private:
	cbsdMaster		*m_master;	// TODO: use statics?!
	std::string		 m_name;	

	/* Functions/Methods */
	virtual void		moduleUnloading()=0;
	virtual bool		moduleLoaded()=0;
	virtual void		moduleReceive(uint16_t channel, const std::string &data)=0;
	virtual void		moduleThread();

	std::thread		m_threadID;                     // ThreadID for the modules thread..
        void			threadHandler(void);
        inline std::thread	threadHandlerProc(void){return std::thread([=] { threadHandler(); });}

	uint16_t		m_id;				// Module ID number
	union {
	 uint16_t		m_flags;			// flags
	 struct{
	  uint16_t		m_flag_reserved:13;		// ...
	  uint16_t		m_is_enabled:1;			// Module is enabled
	  uint16_t		m_is_thread_stopping:1;		// Thread is stopping
	  uint16_t		m_is_thread_running:1;		// Thread is running
	 };
	};


};

#endif
