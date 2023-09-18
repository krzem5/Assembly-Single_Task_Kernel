#include <kernel/cpu/cpu.h>
#include <kernel/cpu/local.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "cpulocal"



void cpu_local_init(void){
	LOG("Allocating per-cpu data...");
	for (const cpu_local_data_descriptor_t*const* descriptor=(void*)(kernel_get_cpu_local_start()+kernel_get_offset());(u64)descriptor<(kernel_get_cpu_local_end()+kernel_get_offset());descriptor++){
		if ((*descriptor)->var){
			*((*descriptor)->var)=kmm_alloc((*descriptor)->size*cpu_count);
		}
	}
}
