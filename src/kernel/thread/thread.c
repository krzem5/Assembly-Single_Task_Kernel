#include <kernel/cpu/cpu.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/umm.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/thread/thread.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "thread"



static tid_t _thread_next_pid=1;
static tid_t _thread_next_tid=1;



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



process_t* process_new(_Bool is_driver){
	process_t* out=kmm_alloc(sizeof(process_t));
	out->id=_thread_next_pid;
	_thread_next_pid++;
	lock_init(&(out->lock));
	out->is_driver=is_driver;
	vmm_pagemap_init(&(out->pagemap));
	vmm_memory_map_init(&(out->mmap));
	out->fs_gs_bases=kmm_alloc(cpu_count*sizeof(process_fs_gs_bases_t));
	for (u16 i=0;i<cpu_count;i++){
		(out->fs_gs_bases+i)->fs=0;
		(out->fs_gs_bases+i)->gs=(u64)(cpu_data+i);
	}
	_thread_list_init(out);
	return out;
}



void process_delete(process_t* process){
	panic("Unimplemented: process_delete",0);
}



thread_t* thread_new(process_t* process,u64 rip,u64 stack_size){
	stack_size=pmm_align_up_address(stack_size);
	thread_t* out=kmm_alloc(sizeof(thread_t));
	memset(out,0,sizeof(thread_t));
	out->id=_thread_next_tid;
	_thread_next_tid++;
	lock_init(&(out->lock));
	out->process=process;
	out->stack_bottom=vmm_memory_map_reserve(&(process->mmap),0,stack_size);
	if (!out->stack_bottom){
		panic("Unable to reserve thread stack",0);
	}
	vmm_reserve_pages(&(process->pagemap),out->stack_bottom,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_USER|VMM_PAGE_FLAG_READWRITE,stack_size>>PAGE_SIZE_SHIFT);
	out->stack_size=stack_size;
	out->state.rip=rip;
	out->state.rsp=out->stack_bottom+stack_size;
	out->state.cr3=process->pagemap.toplevel;
	out->state.cs=0x23;
	out->state.ds=0x1b;
	out->state.es=0x1b;
	out->state.ss=0x1b;
	out->state.rflags=0x00000202;
	out->priority=THREAD_PRIORITY_NORMAL;
	_thread_list_add(process,out);
	scheduler_enqueue_thread(out);
	return out;
}



void thread_delete(thread_t* thread){
	ERROR("Unimplemented: thread_delete");
	// vmm_memory_map_release(&(process->mmap),thread->stack_bottom,thread->stack_size);
	// vmm_release_pages(&(process->pagemap),out->stack_bottom,out->stack_size>>PAGE_SIZE_SHIFT);
	// _thread_list_remove(thread);
}
