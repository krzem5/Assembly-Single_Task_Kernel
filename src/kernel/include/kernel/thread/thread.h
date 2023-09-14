#ifndef _KERNEL_THREAD_THREAD_H_
#define _KERNEL_THREAD_THREAD_H_ 1
#include <kernel/isr/isr.h>
#include <kernel/lock/lock.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>



typedef u32 pid_t;



typedef u32 tid_t;



typedef struct _PROCESS{
	pid_t id;
	lock_t lock;
	vmm_pagemap_t pagemap;
	u64* fs_bases;
	u64* gs_bases;
} process_t;



typedef struct _THREAD{
	tid_t id;
	lock_t lock;
	process_t* process;
	vmm_pagemap_t* pagemap;
	isr_state_t state;
} thread_t;



process_t* process_new(void);



void process_delete(process_t* process);



thread_t* thread_new(process_t* process,u64 rip,u64 rsp);



void thread_delete(thread_t* thread);



#endif
