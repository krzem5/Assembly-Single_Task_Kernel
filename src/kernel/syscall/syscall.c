#include <kernel/error/error.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/string.h>
#define KERNEL_LOG_NAME "syscall"



static rwlock_t _syscall_table_list_lock;
static omm_allocator_t* KERNEL_INIT_WRITE _syscall_table_allocator=NULL;

syscall_table_t*volatile* _syscall_table_list=NULL;
volatile u32 _syscall_table_list_length=0;



error_t syscall_syscall_table_get_offset(KERNEL_USER_POINTER const char* table_name){
	u64 string_length=syscall_get_string_length((const char*)table_name);
	if (!string_length){
		return ERROR_INVALID_ARGUMENT(0);
	}
	for (u32 i=0;i<_syscall_table_list_length;i++){
		if (_syscall_table_list[i]&&str_equal(_syscall_table_list[i]->name,(const char*)table_name)){
			return ((u64)(_syscall_table_list[i]->index))<<32;
		}
	}
	return ERROR_NOT_FOUND;
}



KERNEL_INIT(){
	LOG("Creating syscall tables...");
	rwlock_init(&_syscall_table_list_lock);
	_syscall_table_allocator=omm_init("kernel.syscall_table",sizeof(syscall_table_t),8,1);
	rwlock_init(&(_syscall_table_allocator->lock));
	syscall_create_table("linux",NULL,0);
	syscall_create_table("kernel",_syscall_kernel_functions,_syscall_kernel_count);
}



KERNEL_PUBLIC u32 syscall_create_table(const char* name,const syscall_callback_t* functions,u32 function_count){
	rwlock_acquire_write(&_syscall_table_list_lock);
	syscall_table_t* table=omm_alloc(_syscall_table_allocator);
	table->name=name;
	table->functions=functions;
	table->function_count=function_count;
	table->index=_syscall_table_list_length;
	syscall_table_t** old_syscall_table_list=(syscall_table_t**)_syscall_table_list;
	syscall_table_t** new_syscall_table_list=amm_alloc((_syscall_table_list_length+1)*sizeof(syscall_table_t*));
	mem_copy(new_syscall_table_list,old_syscall_table_list,_syscall_table_list_length*sizeof(syscall_table_t*));
	new_syscall_table_list[_syscall_table_list_length]=table;
	_syscall_table_list=new_syscall_table_list;
	_syscall_table_list_length++;
	amm_dealloc(old_syscall_table_list);
	rwlock_release_write(&_syscall_table_list_lock);
	return table->index;
}



KERNEL_PUBLIC bool syscall_update_table(u32 index,const syscall_callback_t* functions,u32 function_count){
	rwlock_acquire_write(&_syscall_table_list_lock);
	if (index>=_syscall_table_list_length){
		rwlock_release_write(&_syscall_table_list_lock);
		return 0;
	}
	syscall_table_t* table=_syscall_table_list[index];
	table->function_count=0;
	table->functions=functions;
	table->function_count=function_count;
	rwlock_release_write(&_syscall_table_list_lock);
	return 1;
}



KERNEL_PUBLIC u64 syscall_get_user_pointer_max_length(const void* address){
	mmap_region_t* region=mmap_lookup(THREAD_DATA->process->mmap,(u64)address);
	if (!region||!(region->flags&MMAP_REGION_FLAG_VMM_USER)){
		return 0;
	}
	return region->rb_node.key+region->length-((u64)address);
}



KERNEL_PUBLIC u64 syscall_get_string_length(const void* address){
	u64 max_length=syscall_get_user_pointer_max_length(address);
	if (!max_length){
		return 0;
	}
	u64 length=0;
	for (;length<max_length&&*((const char*)(address+length));length++);
	return (length>=max_length?0:length);
}
