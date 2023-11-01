#include <kernel/cpu/cpu.h>
#include <kernel/kernel.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mp/thread.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



#define MEMORY_COUNTER_DATA_NAME_LENGTH 64

#define MEMORY_OBJECT_ALLOCATOR_DATA_NAME_LENGTH 64



typedef struct _USER_MEMORY_RANGE{
	u64 base_address;
	u64 length;
} user_memory_range_t;



typedef struct _USER_MEMORY_COUNTER_DATA{
	char name[MEMORY_COUNTER_DATA_NAME_LENGTH];
	u64 count;
} user_memory_counter_data_t;



typedef struct _USER_MEMORY_OBJECT_ALLOCATOR_DATA{
	char name[MEMORY_OBJECT_ALLOCATOR_DATA_NAME_LENGTH];
	u64 allocation_count;
	u64 deallocation_count;
} user_memory_object_allocator_data_t;



static pmm_counter_descriptor_t _user_data_pmm_counter=PMM_COUNTER_INIT_STRUCT("user_data");



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
	mmap_region_t* out=mmap_reserve(&(THREAD_DATA->process->mmap),0,length,&_user_data_pmm_counter);
	if (out){
		vmm_reserve_pages(&(THREAD_DATA->process->pagemap),out->rb_node.key,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_USER|VMM_PAGE_FLAG_READWRITE,length>>PAGE_SIZE_SHIFT);
	}
	regs->rax=out->rb_node.key;
}



void syscall_memory_unmap(syscall_registers_t* regs){
	u64 length=pmm_align_up_address(regs->rsi);
	if (!mmap_release(&(THREAD_DATA->process->mmap),regs->rdi,length)){
		regs->rax=0;
		return;
	}
	vmm_release_pages(&(THREAD_DATA->process->pagemap),regs->rdi,length>>PAGE_SIZE_SHIFT);
	regs->rax=1;
}



void syscall_memory_counter_get_data(syscall_registers_t* regs){
	if (regs->rdx!=sizeof(user_memory_counter_data_t)||!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		return;
	}
	handle_t* handle=handle_lookup_and_acquire(regs->rdi,HANDLE_TYPE_PMM_COUNTER);
	if (!handle){
		regs->rax=0;
		return;
	}
	user_memory_counter_data_t* out=(void*)(regs->rsi);
	pmm_counter_descriptor_t* pmm_counter_descriptor=handle->object;
	strcpy(out->name,pmm_counter_descriptor->name,MEMORY_COUNTER_DATA_NAME_LENGTH);
	out->count=pmm_counter_descriptor->count;
	regs->rax=1;
}



void syscall_memory_object_allocator_get_data(syscall_registers_t* regs){
	if (regs->rdx!=sizeof(user_memory_object_allocator_data_t)||!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		return;
	}
	handle_t* handle=handle_lookup_and_acquire(regs->rdi,HANDLE_TYPE_OMM_ALLOCATOR);
	if (!handle){
		regs->rax=0;
		return;
	}
	user_memory_object_allocator_data_t* out=(void*)(regs->rsi);
	omm_allocator_t* omm_allocator=handle->object;
	strcpy(out->name,omm_allocator->name,MEMORY_OBJECT_ALLOCATOR_DATA_NAME_LENGTH);
	out->allocation_count=omm_allocator->allocation_count;
	out->deallocation_count=omm_allocator->deallocation_count;
	regs->rax=1;
}
