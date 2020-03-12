
// #ifdef MOD_RACCT
	struct RACCT_u{
	 union {
	  uint64_t		performance;
	  struct {
	   uint8_t		p_cpu;
	   uint8_t		p_mem;
	   uint16_t		temperature;
	   uint32_t		openfiles;
	  };
	 };
	};
// #endif
