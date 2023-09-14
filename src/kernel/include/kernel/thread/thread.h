#ifndef _KERNEL_THREAD_THREAD_H_
#define _KERNEL_THREAD_THREAD_H_ 1
#include <kernel/isr/isr.h>
#include <kernel/lock/lock.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>



#define THREAD_PRIORITY_TERMINATED 0
#define THREAD_PRIORITY_BACKGROUND 1
#define THREAD_PRIORITY_LOW 2
#define THREAD_PRIORITY_NORMAL 3
#define THREAD_PRIORITY_HIGH 4
#define THREAD_PRIORITY_REALTIME 5



typedef u8 thread_priority_t;



typedef u32 pid_t;



typedef u32 tid_t;



typedef struct _PROCESS_FS_GS_BASES{
	u64 fs;
	u64 gs;
} process_fs_gs_bases_t;



typedef struct _THREAD_LIST{
	lock_t lock;
	struct _THREAD* head;
} thread_list_t;



typedef struct _PROCESS{
	pid_t id;
	lock_t lock;
	_Bool is_driver;
	vmm_pagemap_t pagemap;
	vmm_memory_map_t mmap;
	process_fs_gs_bases_t* fs_gs_bases;
	thread_list_t thread_list;
} process_t;



typedef struct _THREAD{
	tid_t id;
	lock_t lock;
	process_t* process;
	vmm_pagemap_t* pagemap;
	u64 stack_bottom;
	u64 stack_size;
	isr_state_t state;
	thread_priority_t priority;
	struct _THREAD* thread_list_prev;
	struct _THREAD* thread_list_next;
	struct _THREAD* scheduler_queue_next;
} thread_t;



process_t* process_new(_Bool is_driver);



void process_delete(process_t* process);



thread_t* thread_new(process_t* process,u64 rip,u64 stack_size);



void thread_delete(thread_t* thread);



#endif
