#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/memory/mmap.h>
#include <kernel/mp/process.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "process"



static u64 _thread_next_handle_id=1;



process_t* process_new(void){
	process_t* out=kmm_alloc(sizeof(process_t));
	out->handle.id=_thread_next_handle_id;
	_thread_next_handle_id++;
	lock_init(&(out->lock));
	vmm_pagemap_init(&(out->pagemap));
	vmm_memory_map_init(&(out->mmap));
	lock_init(&(out->thread_list.lock));
	out->thread_list.head=NULL;
	return out;
}



void process_delete(process_t* process){
	ERROR("Unimplemented: process_delete");
}
