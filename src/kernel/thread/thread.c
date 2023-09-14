#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/thread/thread.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "thread"



static tid_t _thread_next_pid=1;
static tid_t _thread_next_tid=1;



process_t* process_new(void){
	process_t* out=kmm_alloc(sizeof(process_t));
	out->id=_thread_next_pid;
	_thread_next_pid++;
	lock_init(&(out->lock));
	vmm_pagemap_init(&(out->pagemap));
	out->fs_bases=NULL;
	out->gs_bases=NULL;
	return out;
}



void process_delete(process_t* process);



thread_t* thread_new(process_t* process,u64 rip,u64 rsp){
	thread_t* out=kmm_alloc(sizeof(thread_t));
	memset(out,0,sizeof(thread_t));
	out->id=_thread_next_tid;
	_thread_next_tid++;
	lock_init(&(out->lock));
	out->process=process;
	out->state.rip=rip;
	out->state.rsp=rsp;
	return out;
}



void thread_delete(thread_t* thread);
