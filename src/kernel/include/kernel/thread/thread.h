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

#define THREAD_STATE_NONE 0
#define THREAD_STATE_QUEUED 1
#define THREAD_STATE_EXECUTING 2
#define THREAD_STATE_TERMINATED 3



typedef u8 thread_priority_t;



typedef u8 thread_state_t;



typedef u64 pid_t;



typedef u64 tid_t;



typedef struct _THREAD_LIST{
	lock_t lock;
	struct _THREAD* head;
} thread_list_t;



typedef struct _PROCESS{
	pid_t id;
	lock_t lock;
	_Bool is_driver;
	vmm_pagemap_t user_pagemap;
	vmm_pagemap_t kernel_pagemap;
	vmm_memory_map_t mmap;
	thread_list_t thread_list;
} process_t;



typedef struct _THREAD_CPU_STATE{
	u64 kernel_rsp;
	u64 user_rsp;
	u64 kernel_cr3;
	u64 tss_ist1;
	u64 tss_ist2;
} therad_cpu_state_t;



typedef struct _THREAD_FS_GS_STATE{
	u64 fs;
	u64 gs;
} thread_fs_gs_state_t;



typedef struct _THREAD{
	tid_t id;
	lock_t lock;
	process_t* process;
	vmm_pagemap_t* pagemap;
	u64 stack_bottom;
	u64 stack_size;
	isr_state_t gpr_state;
	therad_cpu_state_t cpu_state;
	thread_fs_gs_state_t fs_gs_state;
	void* fpu_state;
	thread_priority_t priority;
	thread_state_t state;
	struct _THREAD* thread_list_prev;
	struct _THREAD* thread_list_next;
	struct _THREAD* scheduler_queue_next;
} thread_t;



process_t* process_new(_Bool is_driver);



void process_delete(process_t* process);



thread_t* thread_new(process_t* process,u64 rip,u64 stack_size);



void thread_delete(thread_t* thread);



#endif
