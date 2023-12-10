#ifndef _KERNEL_MP__MP_TYPES_H_
#define _KERNEL_MP__MP_TYPES_H_ 1
#include <kernel/acl/acl.h>
#include <kernel/cpu/cpu.h>
#include <kernel/handle/handle.h>
#include <kernel/id/group.h>
#include <kernel/id/user.h>
#include <kernel/isr/_isr_types.h>
#include <kernel/lock/profiling.h>
#include <kernel/lock/spinlock.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/vmm.h>
#include <kernel/scheduler/_scheduler_types.h>
#include <kernel/scheduler/cpu_mask.h>
#include <kernel/types.h>



typedef u8 thread_state_t;



typedef struct _EVENT_THREAD_CONTAINER{
	struct _THREAD* thread;
	struct _EVENT_THREAD_CONTAINER* prev;
	struct _EVENT_THREAD_CONTAINER* next;
	u64 sequence_id;
	u64 index;
} event_thread_container_t;



typedef struct _EVENT{
	handle_t handle;
	acl_t* acl;
	spinlock_t lock;
	_Bool is_active;
	event_thread_container_t* head;
	event_thread_container_t* tail;
} event_t;



typedef struct _THREAD_LIST{
	spinlock_t lock;
	struct _THREAD* head;
} thread_list_t;



typedef struct _PROCESS{
	handle_t handle;
	acl_t* acl;
	spinlock_t lock;
	vmm_pagemap_t pagemap;
	mmap_t mmap;
	thread_list_t thread_list;
	string_t* name;
	string_t* image;
	uid_t uid;
	gid_t gid;
	event_t* event;
} process_t;



typedef struct _THREAD_FS_GS_STATE{
	u64 fs;
	u64 gs;
} thread_fs_gs_state_t;



typedef struct _THREAD{
	cpu_header_t header;
	handle_t handle;
	acl_t* acl;
	spinlock_t lock;
	process_t* process;
	string_t* name;
	vmm_pagemap_t* pagemap;
	mmap_region_t* user_stack_region;
	mmap_region_t* kernel_stack_region;
	mmap_region_t* pf_stack_region;
	struct{
		_Bool reg_state_not_present;
		isr_state_t gpr_state;
		thread_fs_gs_state_t fs_gs_state;
		void* fpu_state;
	} reg_state;
	cpu_mask_t* cpu_mask;
	KERNEL_ATOMIC scheduler_priority_t priority;
	thread_state_t state;
	u64 event_sequence_id;
	u64 event_wakeup_index;
	struct _THREAD* thread_list_prev;
	struct _THREAD* thread_list_next;
	struct _THREAD* scheduler_load_balancer_thread_queue_next;
#if KERNEL_DISABLE_ASSERT==0
	lock_profiling_thread_data_t __lock_profiling_data;
#endif
} thread_t;



#endif
