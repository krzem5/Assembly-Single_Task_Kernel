#include <kernel/cpu/cpu.h>
#include <kernel/kernel.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mp/thread.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>



typedef struct _USER_MEMORY_RANGE{
	u64 base_address;
	u64 length;
} user_memory_range_t;



void syscall_memory_get_range(syscall_registers_t* regs){
	if (regs->rdi>=kernel_data.mmap_size||regs->rdx!=sizeof(user_memory_range_t)||!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=0;
		return;
	}
	user_memory_range_t* out=(void*)(regs->rsi);
	out->base_address=(kernel_data.mmap+regs->rdi)->base;
	out->length=(kernel_data.mmap+regs->rdi)->length;
	regs->rax=1;
}



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
	regs->rax=0;
}



void syscall_memory_get_counter(syscall_registers_t* regs){
	if (regs->rdx!=0x11223344||!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=0;
		return;
	}
	regs->rax=0;
}



void syscall_memory_get_object_counter_count(syscall_registers_t* regs){
	regs->rax=omm_allocator_count;
}



void syscall_memory_get_object_counter(syscall_registers_t* regs){
	if (regs->rdi>=omm_allocator_count||regs->rdx!=sizeof(omm_counter_t)||!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
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
