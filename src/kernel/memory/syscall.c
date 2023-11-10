#include <kernel/isr/isr.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/thread.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>



static pmm_counter_descriptor_t _user_data_pmm_counter=PMM_COUNTER_INIT_STRUCT("user_data");



void syscall_memory_map(isr_state_t* regs){
	u64 length=pmm_align_up_address(regs->rdi);
	mmap_region_t* out=mmap_alloc(&(THREAD_DATA->process->mmap),0,length,&_user_data_pmm_counter,MMAP_REGION_FLAG_VMM_NOEXECUTE|MMAP_REGION_FLAG_VMM_USER|MMAP_REGION_FLAG_VMM_READWRITE,NULL);
	regs->rax=(out?out->rb_node.key:0);
}



void syscall_memory_unmap(isr_state_t* regs){
	u64 length=pmm_align_up_address(regs->rsi);
	regs->rax=mmap_dealloc(&(THREAD_DATA->process->mmap),regs->rdi,length);
}
