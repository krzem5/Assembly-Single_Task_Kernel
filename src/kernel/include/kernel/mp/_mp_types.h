#ifndef _KERNEL_MP__MP_TYPES_H_
#define _KERNEL_MP__MP_TYPES_H_ 1
#include <kernel/cpu/cpu.h>
#include <kernel/handle/handle.h>
#include <kernel/isr/_isr_types.h>
#include <kernel/lock/lock.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>



typedef u8 thread_priority_t;



typedef u8 thread_state_type_t;



typedef struct _EVENT{
	handle_t handle;
	lock_t lock;
	struct _THREAD* head;
	struct _THREAD* tail;
} event_t;



typedef struct _PROCESS_THREAD_LIST{
	lock_t lock;
	struct _THREAD* head;
} process_thread_list_t;



typedef struct _PROCESS{
	handle_t handle;
	lock_t lock;
	vmm_pagemap_t pagemap;
	vmm_memory_map_t mmap;
	process_thread_list_t thread_list;
} process_t;



typedef struct _THREAD_FS_GS_STATE{
	u64 fs;
	u64 gs;
} thread_fs_gs_state_t;



typedef struct _THREAD_STATE{
	thread_state_type_t type;
	union{
		struct{
			event_t* event;
			struct _THREAD* next;
		} event;
	};
} thread_state_t;



typedef struct _THREAD{
	cpu_header_t header;
	handle_t handle;
	lock_t lock;
	process_t* process;
	vmm_pagemap_t* pagemap;
	u64 user_stack_bottom;
	u64 kernel_stack_bottom;
	u64 pf_stack_bottom;
	u64 stack_size;
	isr_state_t gpr_state;
	thread_fs_gs_state_t fs_gs_state;
	void* fpu_state;
	volatile thread_priority_t priority;
	_Bool state_not_present;
	thread_state_t state;
	struct _THREAD* thread_list_prev;
	struct _THREAD* thread_list_next;
	struct _THREAD* scheduler_queue_next;
} thread_t;



#endif
