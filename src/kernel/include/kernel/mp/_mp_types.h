#ifndef _KERNEL_MP__MP_TYPES_H_
#define _KERNEL_MP__MP_TYPES_H_ 1
#include <kernel/cpu/cpu.h>
#include <kernel/handle/handle.h>
#include <kernel/id/group.h>
#include <kernel/id/user.h>
#include <kernel/isr/_isr_types.h>
#include <kernel/lock/spinlock.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/vmm.h>
#include <kernel/scheduler/_scheduler_types.h>
#include <kernel/scheduler/cpu_mask.h>
#include <kernel/signal/_signal_types.h>
#include <kernel/types.h>



typedef u8 thread_state_type_t;



typedef struct _EVENT_THREAD_CONTAINER{
	struct _THREAD* thread;
	struct _EVENT_THREAD_CONTAINER* prev;
	struct _EVENT_THREAD_CONTAINER* next;
} event_thread_container_t;



typedef struct _EVENT{
	handle_t handle;
	spinlock_t lock;
	struct _THREAD* head;
	struct _THREAD* tail;
	event_thread_container_t* head_NEW;
	event_thread_container_t* tail_NEW;
} event_t;



typedef struct _THREAD_LIST{
	spinlock_t lock;
	struct _THREAD* head;
} thread_list_t;



typedef struct _PROCESS{
	handle_t handle;
	spinlock_t lock;
	vmm_pagemap_t pagemap;
	mmap_t mmap;
	thread_list_t thread_list;
	u64 signal_handler;
	string_t* name;
	string_t* image;
	uid_t uid;
	gid_t gid;
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
	spinlock_t lock;
	process_t* process;
	vmm_pagemap_t* pagemap;
	mmap_region_t* user_stack_region;
	mmap_region_t* kernel_stack_region;
	mmap_region_t* pf_stack_region;
	isr_state_t gpr_state;
	thread_fs_gs_state_t fs_gs_state;
	void* fpu_state;
	cpu_mask_t* cpu_mask;
	KERNEL_ATOMIC scheduler_priority_t priority;
	_Bool state_not_present;
	thread_state_t state;
	signal_state_t* signal_state;
	struct _THREAD* thread_list_prev;
	struct _THREAD* thread_list_next;
	struct _THREAD* scheduler_load_balancer_thread_queue_next;
} thread_t;



#endif
