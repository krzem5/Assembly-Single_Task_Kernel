#include <kernel/kernel.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "spinlock"



static pmm_counter_descriptor_t _spinlock_profiling_buffer_pmm_counter=PMM_COUNTER_INIT_STRUCT("spinlock_profiling_buffer");



void spinlock_profiling_init(void){
	LOG("Initializing spinlock profiling data...");
	u16 max_lock_id=0;
	for (const spinlock_profiling_setup_descriptor_t*const* descriptor=(void*)kernel_section_spinlock_setup_start();(u64)descriptor<kernel_section_spinlock_setup_end();descriptor++){
		max_lock_id++;
		if ((*descriptor)->id){
			*((*descriptor)->id)=max_lock_id;
		}
	}
	INFO("%lu lock types, %lu lock operations",max_lock_id+1,(kernel_section_spinlock_end()-kernel_section_spinlock_start())/sizeof(void*));
	if (!max_lock_id){
		return;
	}
	u64 buffer_size=pmm_align_up_address((max_lock_id+1)*(kernel_section_spinlock_end()-kernel_section_spinlock_start())/sizeof(void*)*sizeof(spinlock_profiling_data_t));
	INFO("Lock profiling buffer size: %v",buffer_size);
	spinlock_profiling_data_t* data=(void*)(pmm_alloc(buffer_size>>PAGE_SIZE_SHIFT,&_spinlock_profiling_buffer_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	for (const spinlock_profiling_descriptor_t*const* descriptor=(void*)kernel_section_spinlock_start();(u64)descriptor<kernel_section_spinlock_end();descriptor++){
		*((*descriptor)->data)=data;
		data+=max_lock_id+1;
	}
}



const spinlock_profiling_setup_descriptor_t*const* spinlock_profiling_get_setup_descriptors(u32* count){
	if (count){
		*count=(kernel_section_spinlock_setup_end()-kernel_section_spinlock_setup_start())/sizeof(void*);
	}
	return (void*)kernel_section_spinlock_setup_start();
}



const spinlock_profiling_descriptor_t*const* spinlock_profiling_get_descriptors(u32* count){
	if (count){
		*count=(kernel_section_spinlock_end()-kernel_section_spinlock_start())/sizeof(void*);
	}
	return (void*)kernel_section_spinlock_start();
}
