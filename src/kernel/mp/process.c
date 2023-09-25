#include <kernel/handle/handle.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/omm.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread_list.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "process"



static omm_allocator_t _process_allocator=OMM_ALLOCATOR_INIT_STRUCT(sizeof(process_t),8,2);



static HANDLE_DECLARE_TYPE(PROCESS,{
	process_t* process=handle->object;
	if (process->thread_list.head){
		panic("Unterminated process not referenced");
	}
	omm_dealloc(&_process_allocator,process);
});



process_t* process_new(void){
	process_t* out=omm_alloc(&_process_allocator);
	handle_new(out,HANDLE_TYPE_PROCESS,&(out->handle));
	lock_init(&(out->lock));
	vmm_pagemap_init(&(out->pagemap));
	vmm_memory_map_init(&(out->mmap));
	thread_list_init(&(out->thread_list));
	return out;
}
