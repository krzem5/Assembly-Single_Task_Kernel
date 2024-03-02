#include <kernel/cpu/cpu.h>
#include <kernel/isr/isr.h>
#include <kernel/isr/pf.h>
#include <kernel/log/log.h>
#include <kernel/mmap/mmap.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "pf"



_Bool pf_handle_fault(isr_state_t* isr_state){
	u64 address=pf_get_fault_address()&(-PAGE_SIZE);
	if (!address||(isr_state->error&1)||!CPU_HEADER_DATA->current_thread){
		return 0;
	}
	if (mmap_handle_pf((isr_state->cs==0x8&&(address>>63)?process_kernel:THREAD_DATA->process)->mmap,address)){
		pf_invalidate_tlb_entry(address);
		return 1;
	}
	return 1;
}
