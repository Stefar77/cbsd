
// #ifdef MOD_RACCT
	typedef struct{
	 union {
	  uint64_t		performance;
	  struct {
	   uint8_t		p_cpu;
	   uint8_t		p_mem;
	   uint16_t		temperature;
	   uint32_t		openfiles;
	  };
	 };
	}RACCT_u;


	typedef struct{				// Initialization packet Node -> Controller
	 uint32_t	head;
	 uint16_t	version;
	 uint16_t	modules;


	 uint16_t	jails;			/* Thou shall not have more then 65535 active jails! */
	 uint16_t	bhyves;
	 uint32_t	uptime;

	 uint32_t	memory;
	 uint16_t	cores;
	 uint8_t	arch;
	 uint8_t	not_used_yet;

	 /* uint16_t	module[modules] */

	 /* uint32_t	jail_jid[jails] 
	    uint32_t	jail_gid[jails] */

	 /* uint32_t	bhyve_pid[bhyves] 
	    uint32_t	bhyve_gid[bhyves] */
	}CNINIT_t;


// #endif
