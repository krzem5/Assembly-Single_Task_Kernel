#include <kernel/handle/handle.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/memory/mmap.h>
#include <kernel/mp/process.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "process"



process_t* process_new(void){
	process_t* out=kmm_alloc(sizeof(process_t));
	handle_new(out,HANDLE_TYPE_PROCESS,&(out->handle));
	lock_init(&(out->lock));
	vmm_pagemap_init(&(out->pagemap));
	vmm_memory_map_init(&(out->mmap));
	lock_init(&(out->thread_list.lock));
	out->thread_list.head=NULL;
	return out;
}



void process_delete(process_t* process){
	handle_release(&(process->handle));
	ERROR("Unimplemented: process_delete");
}
