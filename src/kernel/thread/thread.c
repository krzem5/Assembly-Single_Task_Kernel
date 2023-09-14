#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/thread/thread.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "thread"



static tid_t _thread_next_id=1;



thread_t* thread_new(process_t* process,vmm_pagemap_t* pagemap,u64 rip,u64 rsp){
	thread_t* out=kmm_alloc(sizeof(thread_t));
	memset(out,0,sizeof(thread_t));
	out->id=_thread_next_id;
	_thread_next_id++;
	lock_init(&(out->lock));
	out->process=process;
	out->pagemap=pagemap;
	out->state.rip=rip;
	out->state.rsp=rsp;
	return out;
}



void thread_delete(thread_t* thread);
