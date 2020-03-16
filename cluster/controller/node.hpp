#ifndef NODE_HPP
#define NODE_HPP

#include "cbsd.hpp"

enum { 
	NCMD_PING=0, NCMD_PONG
} NODECMD_e;

enum { 
	ARCH_X86=0, ARCH_AMD64, ARCH_ARM32, ARCH_ARM64
} ARCHTYPE_e;

#define THREADING(cmd) cmd 
#include <thread>
class cbsdJail;
class cbsdNodes;
class cbsdModule;
class cbsdNode: public cbsdSocket {
 friend class cbsdListener;
 friend class cbsdNodes;
 public:
	cbsdNode(const uint32_t id, const std::string name);
	~cbsdNode();


	/* Functions/Methods */
	inline std::string		&getName(){ return(m_name); }

	inline bool			isOnline(){ return(m_is_authenticated); }
	inline bool			isFresh(){ return((std::time(0) - m_last_seen) < 2); }
	inline bool			isStale(){ return((std::time(0) - m_last_seen) > 4); }

	/* Events */
// 	void				statsReceived();
	void				setPerfdata(const uint16_t what, uint64_t val);
	std::string 			getJSON(uint32_t flags);



 protected:
	bool 				isPersistent() override { return(true); } 
 	void				_hasDisconnected() override;

 private:
	CBSDDBITEMI(cbsdNode)										// Default stuff
	/* Functions/Methods */
	bool 				_doAuth(std::string &data, const uint16_t channel);		// Helper
	bool 				_doSysOp(std::string &data);

 	void				_hasConnected() override;
	void				_hasData(const std::string &data) override;
	void				_readyForData() override;

//	void 				_handlePacket(char *packet, size_t len);
//	int				_handleCommand(uint16_t command, uint8_t parameters, char *packet);

	/* Private variables */
	std::string			 m_name;		// Name of the node
	TASKS_MAP			 m_tasks;		// Tasks running on this node
	uint32_t			 m_id;			// Global ID of the node (for databases etc)
	std::time_t			 m_last_seen;		// Last packet..

	union {				
		uint32_t		 m_flags;		// All flags combined
		struct {
		 uint32_t		 m_has_error:1;		// Node had some error user did not ack yet
		 uint32_t		 m_has_warning:1;	// Node had a warning user did not ack yet
		 uint32_t		 m_has_negotiated:1;	// Node has nogotiated the modules and stuff

		 uint32_t		 m_reserved_flags:10;	// not used yet

		 uint32_t		 m_is_maintenance:1;	// Node is in maintenance modus, do not alert
		 uint32_t		 m_is_authenticated:1;	// Node is authenticated
		 uint32_t		 m_is_connected:1;

		};
	};

	std::map<std::string, cbsdJail *> m_jails;
	MODULES_MAP			 m_modules;		// Links to modules enabled on the jail.


/* do not change unions unless you also change the node parts of this! */
	union {	
		uint64_t		m_performance;
		struct{
			uint8_t		m_pcpu;
			uint8_t		m_pmem;
			uint16_t	m_temperature;
			uint32_t	m_openfiles;

		};
	  };
	union {
		uint64_t		m_configuration[2];
		struct{
			uint8_t		m_arch;			// x86, x64, a32, a64, ...
			uint8_t		m_config_flags;		// 
			uint16_t	m_cores;		// >255 cores please, gimme! :-)
			uint32_t	m_memory;		// Only updates at node[daemon] (re)start
			uint32_t	m_uptime;		// 
			uint32_t	m_not_used;		// 
		};
	  };

};

#endif
