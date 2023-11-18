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



u64 syscall_get_user_pointer_max_length(u64 address){
	mmap_region_t* region=mmap_lookup(&(THREAD_DATA->process->mmap),address);
	if (!region||!(region->flags&MMAP_REGION_FLAG_VMM_USER)){
		return 0;
	}
	return region->rb_node.key+region->length-address;
}



u64 syscall_get_string_length(u64 address){
	u64 max_length=syscall_get_user_pointer_max_length(address);
	u64 length=0;
	for (;length<max_length&&*((const char*)(address+length));length++);
	return (length>=max_length?0:length);
}
