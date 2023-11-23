#include <kernel/cpu/cpu.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/scheduler/cpu_mask.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "cpu_mask"



static omm_allocator_t* KERNEL_INIT_WRITE _scheduler_cpu_mask_allocator;
static u64 KERNEL_INIT_WRITE _scheduler_cpu_mask_last_bitmap_entry;

KERNEL_PUBLIC u32 KERNEL_INIT_WRITE cpu_mask_size;



void KERNEL_EARLY_EXEC cpu_mask_init(void){
	LOG("Initializing default CPU mask...");
	cpu_mask_size=((cpu_count+63)>>6)*sizeof(u64);
	_scheduler_cpu_mask_allocator=omm_init("cpu_mask",cpu_mask_size,8,1,pmm_alloc_counter("omm_cpu_mask"));
	spinlock_init(&(_scheduler_cpu_mask_allocator->lock));
	_scheduler_cpu_mask_last_bitmap_entry=((cpu_count&63)?(1ull<<(cpu_count&63))-1:0);
}



KERNEL_PUBLIC cpu_mask_t* cpu_mask_new(void){
	cpu_mask_t* out=omm_alloc(_scheduler_cpu_mask_allocator);
	for (u16 i=0;i<(cpu_count>>6);i++){
		out->bitmap[i]=0xffffffffffffffffull;
	}
	if (_scheduler_cpu_mask_last_bitmap_entry){
		out->bitmap[cpu_count>>6]=_scheduler_cpu_mask_last_bitmap_entry;
	}
	return out;
}



KERNEL_PUBLIC void cpu_mask_delete(cpu_mask_t* cpu_mask){
	omm_dealloc(_scheduler_cpu_mask_allocator,cpu_mask);
}
