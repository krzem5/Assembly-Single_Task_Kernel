#include <kernel/kernel.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "syscall"



static spinlock_t _syscall_table_list_lock;
static omm_allocator_t* KERNEL_INIT_WRITE _syscall_table_allocator=NULL;

syscall_table_t*volatile* _syscall_table_list=NULL;
volatile u32 _syscall_table_list_length=0;



void KERNEL_NORETURN _syscall_invalid(syscall_reg_state_t* regs){
	ERROR("Invalid SYSCALL number: %lu",regs->rax);
	panic("Invalid SYSCALL");
}



void syscall_syscall_table_get_offset(syscall_reg_state_t* regs){
	regs->rax=-1;
	if (!syscall_get_string_length(regs->rdi)){
		return;
	}
	for (u32 i=0;i<_syscall_table_list_length;i++){
		if (_syscall_table_list[i]&&streq(_syscall_table_list[i]->name,(const char*)(regs->rdi))){
			regs->rax=((u64)(_syscall_table_list[i]->index))<<32;
			return;
		}
	}
}



void KERNEL_EARLY_EXEC syscall_init(void){
	LOG("Initializing syscall tables...");
	spinlock_init(&_syscall_table_list_lock);
	_syscall_table_allocator=omm_init("syscall_table",sizeof(syscall_table_t),8,1,pmm_alloc_counter("omm_syscall_table"));
	syscall_create_table("linux",(const syscall_callback_t*)_syscalls_linux_functions,_syscalls_linux_count);
	syscall_create_table("kernel",(const syscall_callback_t*)_syscalls_kernel_functions,_syscalls_kernel_count);
}



KERNEL_PUBLIC u32 syscall_create_table(const char* name,const syscall_callback_t* functions,u32 function_count){
	spinlock_acquire_exclusive(&_syscall_table_list_lock);
	syscall_table_t* table=omm_alloc(_syscall_table_allocator);
	table->name=name;
	table->functions=functions;
	table->function_count=function_count;
	table->index=_syscall_table_list_length;
	syscall_table_t** old_syscall_table_list=(syscall_table_t**)_syscall_table_list;
	syscall_table_t** new_syscall_table_list=amm_alloc((_syscall_table_list_length+1)*sizeof(syscall_table_t*));
	memcpy(new_syscall_table_list,old_syscall_table_list,_syscall_table_list_length*sizeof(syscall_table_t*));
	new_syscall_table_list[_syscall_table_list_length]=table;
	_syscall_table_list=new_syscall_table_list;
	_syscall_table_list_length++;
	amm_dealloc(old_syscall_table_list);
	spinlock_release_exclusive(&_syscall_table_list_lock);
	return table->index;
}



KERNEL_PUBLIC u64 syscall_get_user_pointer_max_length(u64 address){
	mmap_region_t* region=mmap_lookup(&(THREAD_DATA->process->mmap),address);
	if (!region||!(region->flags&MMAP_REGION_FLAG_VMM_USER)){
		return 0;
	}
	return region->rb_node.key+region->length-address;
}



KERNEL_PUBLIC u64 syscall_get_string_length(u64 address){
	u64 max_length=syscall_get_user_pointer_max_length(address);
	u64 length=0;
	for (;length<max_length&&*((const char*)(address+length));length++);
	return (length>=max_length?0:length);
}
