#include <kernel/cpu/cpu.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mp/thread.h>
#include <kernel/sandbox/sandbox.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>



SANDBOX_DECLARE_TYPE(DISABLE_MEMORY_COUNTER_API);



void syscall_memory_map(syscall_registers_t* regs){
	u64 length=pmm_align_up_address(regs->rdi);
	u64 out=vmm_memory_map_reserve(&(THREAD_DATA->process->mmap),0,length);
	if (out){
		vmm_reserve_pages(&(THREAD_DATA->process->pagemap),out,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_USER|VMM_PAGE_FLAG_READWRITE,length>>PAGE_SIZE_SHIFT);
	}
	regs->rax=out;
}



void syscall_memory_unmap(syscall_registers_t* regs){
	u64 length=pmm_align_up_address(regs->rsi);
	if (!vmm_memory_map_release(&(THREAD_DATA->process->mmap),regs->rdi,length)){
		regs->rax=0;
		return;
	}
	vmm_release_pages(&(THREAD_DATA->process->pagemap),regs->rdi,length>>PAGE_SIZE_SHIFT);
	regs->rax=1;
}



void syscall_memory_get_counter_count(syscall_registers_t* regs){
	regs->rax=(sandbox_get(THREAD_DATA->sandbox,SANDBOX_FLAG_DISABLE_MEMORY_COUNTER_API)?0:pmm_get_counter_count());
}



void syscall_memory_get_counter(syscall_registers_t* regs){
	if (sandbox_get(THREAD_DATA->sandbox,SANDBOX_FLAG_DISABLE_MEMORY_COUNTER_API)||regs->rdx!=sizeof(pmm_counter_t)||!syscall_sanatize_user_memory(regs->rsi,regs->rdx)||!pmm_get_counter(regs->rdi,(pmm_counter_t*)(regs->rsi))){
		regs->rax=0;
		return;
	}
	regs->rax=1;
}



void syscall_memory_get_object_counter_count(syscall_registers_t* regs){
	regs->rax=(sandbox_get(THREAD_DATA->sandbox,SANDBOX_FLAG_DISABLE_MEMORY_COUNTER_API)?0:omm_allocator_count);
}



void syscall_memory_get_object_counter(syscall_registers_t* regs){
	if (sandbox_get(THREAD_DATA->sandbox,SANDBOX_FLAG_DISABLE_MEMORY_COUNTER_API)||regs->rdi>=omm_allocator_count||regs->rdx!=sizeof(omm_counter_t)||!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=0;
		return;
	}
	omm_allocator_t* allocator=omm_head_allocator;
	for (u32 i=0;i<regs->rdi;i++){
		allocator=allocator->next_allocator;
	}
	*((omm_counter_t*)(regs->rsi))=allocator->counter;
	regs->rax=1;
}
