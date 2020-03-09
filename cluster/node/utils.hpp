/* utils.hpp - Utils class for CBSD Cluster Node
 *
 * Copyright (c) 2020, Stefan Rink <stefanrink at yahoo dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/sysctl.h>
#include <sys/param.h>
#include <sys/jail.h>
#include <jail.h>


class cbsdUtils {
        enum { CP_USER=0, CP_NICE, CP_SYS, CP_INTR, CP_IDLE, CPUSTATES };

 public:
	inline static uint32_t  getUptime(void){ timeval temp; size_t len=sizeof(temp); if(0 != sysctlbyname("kern.boottime", &temp, &len, NULL, 0)) return 0; return(temp.tv_sec);  }
	inline static uint32_t  getOpenFiles(void){ uint32_t openfiles; size_t len=sizeof(openfiles); sysctlbyname("kern.openfiles", &openfiles, &len, NULL, 0); return(openfiles); }
	inline static uint8_t   getPCPU(void){ static long cpu_last[CPUSTATES]={0,0,0,0,0}; long cur[CPUSTATES], sum=0; size_t cur_sz = sizeof cur; if (sysctlbyname("kern.cp_time", &cur, &cur_sz, NULL, 0) < 0) return(0); for (int state = 0; state<CPUSTATES; state++){ long tmp = cur[state]; cur[state] -= cpu_last[state]; cpu_last[state] = tmp; sum += cur[state]; } double util = 100.0L - (100.0L * cur[CP_IDLE] / (sum ? (double) sum : 1.0L)); return((uint8_t)util); }
        inline static uint8_t	getPMEM(void){ uint64_t realmem = 0 ; uint64_t active_count = 0; uint64_t wire_count = 0 ; size_t var_len = sizeof(realmem); int i = sysctlbyname("hw.realmem",&realmem, &var_len, NULL, 0); if (i != 0 && errno == ENOENT) return(0); i = sysctlbyname("vm.stats.vm.v_active_count",&active_count, &var_len, NULL, 0); if (i != 0 && errno == ENOENT) return(0); i = sysctlbyname("vm.stats.vm.v_wire_count",&wire_count, &var_len, NULL, 0); if (i != 0 && errno == ENOENT) return(0); uint64_t wire_size=cbsdUtils::page_size*wire_count; uint64_t freemem=realmem-(cbsdUtils::page_size*active_count)-wire_size; uint64_t mem_use=realmem - freemem; uint64_t calc_pmem=100.0 * mem_use / realmem; if(calc_pmem > 100) calc_pmem=100; return(0xFF & calc_pmem); }


	inline static uint32_t  getMemory(void){ return(cbsdUtils::total_memory  / 1024 / 1024); }
	inline static uint16_t  cpuCores(void){ return(0xFFFF & cbsdUtils::total_cores); }

	inline static uint32_t  cpuTemperature(void){ uint32_t temp; size_t len=sizeof(temp); if(0 != sysctlbyname("dev.cpu.0.temperature", &temp, &len, NULL, 0)) return 0; return(temp);   }

	inline static uint8_t   getArch(void){ return(3); }


	inline static std::map<uint32_t, std::string>  getRunningJails(void){
		int jflags=0, jid=0;
		static struct jailparam params[3];

		/* Add the parameters to print. */
		jailparam_init(&params[0], "jid");
		jailparam_init(&params[1], "name");
		jailparam_init(&params[2], "lastjid");
		jailparam_import_raw(&params[2], &jid, sizeof(int));

		std::map<uint32_t, std::string> items;
		items.clear();

		std::cout << "Jails..\n";
		while((jid = jailparam_get(params, 3, jflags)) >= 0) items[*(int *)params[0].jp_value]=std::string((char *)params[1].jp_value);
		jailparam_free(params, 3);

		return(items);
	}







	static uint64_t page_size;
	static uint64_t total_memory;
	static uint32_t total_cores;

/*
 * 1   - x68
 * 2   - x64
 * 4   - ARM32
 * 8   - ARM64
 * 16  -
 * 32  -
 * 64  -
 * 128 -
 */



};







