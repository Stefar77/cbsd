#ifndef JAIL_HPP
#define JAIL_HPP

#ifdef ISNODE
#include "../node/master.hpp"
class cbsdJail {
 friend class cbsdMaster;
#else
#include "../controller/node.hpp"
class cbsdNode;
class cbsdJail {
 friend class cbsdNode;
#endif

 public:
#ifdef ISNODE
	cbsdJail(const uint32_t id, const std::string &name, const std::string &hostname, const std::string &path, const std::string &ethernet);
#else
	cbsdJail(cbsdNode *node, const uint32_t id, const std::string &name, const std::string &hostname, const std::string &path, const std::string &ethernet);
#endif
	~cbsdJail();

	/* Functions/Methods */
	inline uint32_t 	 getJID(){ return(m_jid); }
	inline std::string 	&getName(){ return(m_name); }
	inline std::string 	&getHostname(){ return(m_hostname); }
	inline std::string 	&getPath(){ return(m_path); }
	inline std::string 	&getEthernet(){ return(m_ethernet); }
#ifndef ISNODE
	inline cbsdNode 	*getNode(){ return(m_node); }
#endif
	inline uint32_t 	 getID(){ return(m_id); }

 protected:
#ifdef ISNODE
	void    		 gatherStats();			// Gather RACCT data for jail
#endif
	void			 doUnload();


 private:
	std::string		 m_name;			// Name of the jail
	std::string		 m_hostname;			// Full hostname of the jail
	std::string		 m_path;			// Root path of the jail
	std::string		 m_ethernet;			// Ethernet configuration

#ifdef ISNODE
	cbsdMaster		*m_master;			// TODO: use statics?!
#else
	cbsdNode		*m_node;			// Node this jail is running on
#endif
	uint32_t		 m_id;				// Jail-ID in cluster database
	uint32_t		 m_jid;				// Jail-ID if running or 0.
	time_t			 m_started;			// Time the jail was started

	uint16_t		 m_reserved;			// ...
	union {
	 uint16_t		 m_flags;			// flags
	 struct{
	  uint16_t		 m_flag_reserved:15;		// ...
	  uint16_t		 m_flag_stats_initialized:1;	// We sending stats?
	 };
	};

	union {							// Generic jail stats
	 uint64_t		 m_stats[6];			// For easy access
	 union {
		uint8_t		 m_pcpu;			// Percentage CPU used
		uint8_t		 m_pmem;			// Percentage memory used
		uint16_t	 m_datasize;			// Datasize in KB (/ 1024)
		uint16_t	 m_stacksize;			// Stacksize in KB
		uint16_t	 m_read_bps;			// BPS 

		uint32_t	 m_cputime;			// CPU time used by jail
		uint32_t	 m_write_bps;			// BPS

		uint32_t	 m_openfiles;			// Files opened by this jail
		uint32_t	 m_maxproc;			// Processes in the jail

		uint32_t	 m_read_iops;			// IOPS read
		uint32_t	 m_write_iops;			// IOPS write

		uint32_t	 m_memory_used;			//  
		uint32_t	 m_vmemory_used;		// 

		uint32_t	 m_wallclock;			//  
		uint32_t	 m_jail_flags;			// ...

	 };
	};
};

#endif
