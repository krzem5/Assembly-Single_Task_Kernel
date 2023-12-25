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



u64 syscall_syscall_table_get_offset(const char* table_name){
	if (!syscall_get_string_length((u64)table_name)){
		return -1;
	}
	for (u32 i=0;i<_syscall_table_list_length;i++){
		if (_syscall_table_list[i]&&streq(_syscall_table_list[i]->name,table_name)){
			return ((u64)(_syscall_table_list[i]->index))<<32;
		}
	}
	return -1;
}



KERNEL_INIT(){
	LOG("Initializing syscall tables...");
	spinlock_init(&_syscall_table_list_lock);
	_syscall_table_allocator=omm_init("syscall_table",sizeof(syscall_table_t),8,1,pmm_alloc_counter("omm_syscall_table"));
	spinlock_init(&(_syscall_table_allocator->lock));
	syscall_create_table("linux",NULL,0);
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
