#ifndef _KERNEL_THREAD_THREAD_H_
#define _KERNEL_THREAD_THREAD_H_ 1
#include <kernel/cpu/_cpu_types.h>
#include <kernel/isr/_isr_types.h>
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

#define THREAD_STATE_TYPE_NONE 0
#define THREAD_STATE_TYPE_QUEUED 1
#define THREAD_STATE_TYPE_EXECUTING 2
#define THREAD_STATE_TYPE_AWAITING_EVENT 3
#define THREAD_STATE_TYPE_TERMINATED 255

#define THREAD_DATA ((volatile __seg_gs thread_t*)NULL)



typedef u8 thread_priority_t;



typedef u8 thread_state_type_t;



typedef u64 eid_t;



typedef u64 pid_t;



typedef u64 tid_t;



typedef struct _EVENT{
	eid_t id;
	lock_t lock;
	struct _THREAD* head;
	struct _THREAD* tail;
} event_t;



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
	thread_list_t thread_list;
} process_t;



typedef struct _THREAD_FS_GS_STATE{
	u64 fs;
	u64 gs;
} thread_fs_gs_state_t;



typedef struct _THREAD_STATE{
	thread_state_type_t type;
	lock_t lock;
	union{
		struct{
			event_t* event;
			struct _THREAD* next;
		} event;
	};
} thread_state_t;



typedef struct _THREAD{
	cpu_header_t header;
	tid_t id;
	lock_t lock;
	process_t* process;
	vmm_pagemap_t* pagemap;
	u64 user_stack_bottom;
	u64 kernel_stack_bottom;
	u64 pf_stack_bottom;
	u64 stack_size;
	isr_state_t gpr_state;
	u64 pf_stack;
	thread_fs_gs_state_t fs_gs_state;
	void* fpu_state;
	thread_priority_t priority;
	_Bool state_not_present;
	thread_state_t state;
	struct _THREAD* thread_list_prev;
	struct _THREAD* thread_list_next;
	struct _THREAD* scheduler_queue_next;
} thread_t;



process_t* process_new(_Bool is_driver);



void process_delete(process_t* process);



thread_t* thread_new(process_t* process,u64 rip,u64 stack_size);



void thread_delete(thread_t* thread);



void KERNEL_NORETURN thread_terminate(void);



void thread_await_event(event_t* event);



event_t* event_new(void);



void event_delete(event_t* event);



void event_signal(event_t* event,_Bool dispatch_all);



#endif
