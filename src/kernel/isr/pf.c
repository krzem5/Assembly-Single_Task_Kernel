#include <kernel/cpu/cpu.h>
#include <kernel/isr/isr.h>
#include <kernel/isr/pf.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/types.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "pf"



_Bool pf_handle_fault(isr_state_t* isr_state){
	u64 address=pf_get_fault_address()&(-PAGE_SIZE);
	if (!address||(isr_state->error&1)||!CPU_HEADER_DATA->current_thread){
		return 0;
	}
	if (mmap2_handle_pf((isr_state->cs==0x8&&(address>>63)?process_kernel:THREAD_DATA->process)->mmap2,address)){
		pf_invalidate_tlb_entry(address);
		return 1;
	}
	mmap_region_t* region=mmap_lookup((isr_state->cs==0x8&&(address>>63)?&(process_kernel->mmap):&(THREAD_DATA->process->mmap)),address);
	if (!region||!region->pmm_counter||((region->flags&MMAP_REGION_FLAG_STACK)&&address==region->rb_node.key)){
		return 0;
	}
	u64 physical_address=pmm_alloc(1,region->pmm_counter,0);
	if (region->file){
		vfs_node_read(region->file,address-region->rb_node.key+(region->flags>>MMAP_REGION_FILE_OFFSET_SHIFT),(void*)(physical_address+VMM_HIGHER_HALF_ADDRESS_OFFSET),PAGE_SIZE,0);
	}
	vmm_map_page(&(THREAD_DATA->process->pagemap),physical_address,address,mmap_get_vmm_flags(region));
	pf_invalidate_tlb_entry(address);
	return 1;
}
