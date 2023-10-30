#include <kernel/cpu/cpu.h>
#include <kernel/cpu/local.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "cpulocal"



static pmm_counter_descriptor_t _cpu_local_pmm_counter=PMM_COUNTER_INIT_STRUCT("cpu_local");



void cpu_local_init(void){
	LOG("Allocating cpu-local data...");
	u32 total_size=0;
	for (const cpu_local_data_descriptor_t*const* descriptor=(void*)kernel_section_cpu_local_start();(u64)descriptor<kernel_section_cpu_local_end();descriptor++){
		total_size+=((*descriptor)->size*cpu_count+7)&0xfffffff8;
	}
	INFO("CPU-local data size: %v",pmm_align_up_address(total_size));
	void* buffer=(void*)(pmm_alloc_zero(pmm_align_up_address(total_size)>>PAGE_SIZE_SHIFT,&_cpu_local_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	for (const cpu_local_data_descriptor_t*const* descriptor=(void*)kernel_section_cpu_local_start();(u64)descriptor<kernel_section_cpu_local_end();descriptor++){
		*((*descriptor)->var)=buffer;
		buffer+=((*descriptor)->size*cpu_count+7)&0xfffffff8;
	}
}
