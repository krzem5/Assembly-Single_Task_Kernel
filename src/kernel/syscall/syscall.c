#include <kernel/isr/isr.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mp/thread.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "syscall"



void syscall_invalid(isr_state_t* regs){
	ERROR("Invalid SYSCALL number: %lu",regs->rax);
	panic("Invalid SYSCALL");
}



_Bool syscall_sanatize_user_memory(u64 address,u64 size){
	if (!address||!size||(address|size|(address+size))>=VMM_HIGHER_HALF_ADDRESS_OFFSET){
		return 0;
	}
	return vmm_is_user_accessible(&(THREAD_DATA->process->pagemap),address,pmm_align_up_address(size)>>PAGE_SIZE_SHIFT);
}
