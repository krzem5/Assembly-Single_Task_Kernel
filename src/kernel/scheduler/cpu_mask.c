#include <kernel/cpu/cpu.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/scheduler/cpu_mask.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "cpu_mask"



static pmm_counter_descriptor_t _scheduler_cpu_mask_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_cpu_mask");
static omm_allocator_t _scheduler_cpu_mask_allocator=OMM_ALLOCATOR_INIT_LATER_STRUCT;
static u64 _scheduler_last_bitmap_entry;

u32 cpu_mask_size;



void cpu_mask_init(void){
	LOG("Initializing default CPU mask...");
	cpu_mask_size=((cpu_count+63)>>6)*sizeof(u64);
	_scheduler_cpu_mask_allocator=OMM_ALLOCATOR_INIT_STRUCT("cpu_mask",cpu_mask_size,8,1,&_scheduler_cpu_mask_omm_pmm_counter);
	_scheduler_last_bitmap_entry=((cpu_count&63)?(1ull<<(cpu_count&63))-1:0);
}



cpu_mask_t* cpu_mask_new(void){
	cpu_mask_t* out=omm_alloc(&_scheduler_cpu_mask_allocator);
	for (u16 i=0;i<(cpu_count>>6);i++){
		out->bitmap[i]=0xffffffffffffffffull;
	}
	if (_scheduler_last_bitmap_entry){
		out->bitmap[cpu_count>>6]=_scheduler_last_bitmap_entry;
	}
	return out;
}



void cpu_mask_delete(cpu_mask_t* cpu_mask){
	omm_dealloc(&_scheduler_cpu_mask_allocator,cpu_mask);
}
