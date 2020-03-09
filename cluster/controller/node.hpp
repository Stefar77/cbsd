#ifndef NODE_HPP
#define NODE_HPP
#include <map>
#include "../common/socket.hpp"
#include "../shared/jail.hpp"

enum { 
	NCMD_PING=0, NCMD_PONG
} NODECMD_e;

enum { 
	ARCH_X86=0, ARCH_AMD64, ARCH_ARM32, ARCH_ARM64
} ARCHTYPE_e;

#define THREADING(cmd) cmd 
#include <thread>
class cbsdJail;
class cbsdNode: public cbsdSocket {
 friend class cbsdListener;
 friend class cbsdNodes;
 public:
	cbsdNode(const uint32_t id, const std::string name);
	~cbsdNode();


	/* Functions/Methods */
	inline std::string		&getName(){ return(m_name); }

	/* Events */
 	void				statsReceived();

 protected:
	bool 				isPersistent() override { return(true); } 


 private:
	/* Functions/Methods */
 	void				_hasConnected() override;
 	void				_hasDisconnected() override;
	void				_hasData(const std::string &data) override;
	void				_readyForData() override;


	void 				_handlePacket(char *packet, size_t len);
	int				_handleCommand(uint16_t command, uint8_t parameters, char *packet);

	/* Private variables */
	std::string			m_name;			// Name of the node

	uint32_t			m_id;			// Local flags for node
	uint32_t			m_flags;		// Local flags for node

	std::map<std::string, cbsdJail *> m_jails;
//	std::map<uint16_t, cbsdModule *> m_modules;		// Links to modules enabled on the jail.


/* do not change unions unless you also change the node parts of this! */
	union {				
		uint64_t		m_performance;
		struct{
			uint8_t		m_pcpu;
			uint8_t		m_pmem;
			uint16_t	m_temperature;
			uint16_t	m_no_jails;
			uint16_t	m_vms;

		};
	  };
	union {
		uint64_t		m_configuration;
		struct{
			uint8_t		m_arch;			// x86, x64, a32, a64, ...
			uint8_t		m_config_flags;		// 
			uint16_t	m_cores;		// >255 cores please, gimme! :-)
			uint32_t	m_memory;		// Only updates at node[daemon] (re)start
		};
	  };

};

#endif
