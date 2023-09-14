#include <kernel/cpu/cpu.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/memory/umm.h>
#include <kernel/thread/thread.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "thread"



static tid_t _thread_next_pid=1;
static tid_t _thread_next_tid=1;

process_t* process_kernel;



static void _thread_list_init(process_t* process){
	lock_init(&(process->thread_list.lock));
	process->thread_list.head=NULL;
}



static void _thread_list_add(process_t* process,thread_t* thread){
	lock_acquire_exclusive(&(process->thread_list.lock));
	process->thread_list.head=NULL;
	thread->thread_list_prev=NULL;
	thread->thread_list_next=process->thread_list.head;
	if (process->thread_list.head){
		process->thread_list.head->thread_list_prev=thread;
	}
	process->thread_list.head=thread;
	lock_release_exclusive(&(process->thread_list.lock));
}



process_t* process_new(_Bool is_kernel_process){
	process_t* out=kmm_alloc(sizeof(process_t));
	out->id=_thread_next_pid;
	_thread_next_pid++;
	lock_init(&(out->lock));
	out->ring=(is_kernel_process?0:3);
	vmm_pagemap_init(&(out->pagemap));
	if (!is_kernel_process){
		umm_init_pagemap(&(out->pagemap));
	}
	out->fs_gs_bases=kmm_alloc(cpu_count*sizeof(process_fs_gs_bases_t));
	for (u16 i=0;i<cpu_count;i++){
		(out->fs_gs_bases+i)->fs=0;
		(out->fs_gs_bases+i)->gs=(u64)(cpu_data+i);
	}
	_thread_list_init(out);
	return out;
}



void process_delete(process_t* process);



void process_init_kernel_process(u64 entry_point){
	LOG("Initializing kernel process...");
	process_kernel=process_new(1);
	thread_new(process_kernel,entry_point,0x1000);
	// panic("AAA",0);
}



thread_t* thread_new(process_t* process,u64 rip,u64 stack_size){
	thread_t* out=kmm_alloc(sizeof(thread_t));
	memset(out,0,sizeof(thread_t));
	out->id=_thread_next_tid;
	_thread_next_tid++;
	lock_init(&(out->lock));
	out->process=process;
	out->state.rip=rip;
	out->state.rsp=0;
	out->stack_size=stack_size;
	_thread_list_add(process,out);
	return out;
}



void thread_delete(thread_t* thread);
