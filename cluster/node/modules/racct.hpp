#ifndef RACCT_HPP
#define RACCT_HPP

#include "../master.hpp"

#define MODULE_NAME RACCT
MODULE_HPP_START(

	void    			_getHosterStats(void);

	uint8_t				m_pcpu;
	uint8_t				m_pmem;
	uint16_t			m_temperature;
	uint32_t			m_openfiles;
)

//
// you could also put your defines here..
//


MODULE_HPP_DONE()
#endif

