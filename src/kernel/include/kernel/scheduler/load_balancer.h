#ifndef _KERNEL_SCHEDULER_LOAD_BALANCER_H_
#define _KERNEL_SCHEDULER_LOAD_BALANCER_H_ 1
#include <kernel/lock/spinlock.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/_scheduler_types.h>
#include <kernel/types.h>



#define SCHEDULER_PRIORITY_BACKGROUND 0
#define SCHEDULER_PRIORITY_LOW 1
#define SCHEDULER_PRIORITY_NORMAL 2
#define SCHEDULER_PRIORITY_HIGH 3
#define SCHEDULER_PRIORITY_REALTIME 4
#define SCHEDULER_PRIORITY_TERMINATED 255

#define SCHEDULER_PRIORITY_MIN SCHEDULER_PRIORITY_BACKGROUND
#define SCHEDULER_PRIORITY_MAX SCHEDULER_PRIORITY_REALTIME

#define SCHEDULER_LOAD_BALANCER_QUEUE_COUNT 64



typedef struct _SCHEDULER_LOAD_BALANCER_THREAD_QUEUE{
	spinlock_t lock;
	thread_t* head;
	thread_t* tail;
} scheduler_load_balancer_thread_queue_t;



typedef struct _SCHEDULER_LOAD_BALANCER_STATS{
	u64 added_thread_count;
	u64 free_slot_count;
	u64 used_slot_count;
} scheduler_load_balancer_stats_t;



thread_t* scheduler_load_balancer_get(void);



void scheduler_load_balancer_add(thread_t* thread);



const scheduler_load_balancer_stats_t* scheduler_load_balancer_get_stats(u16 cpu_index);



#endif
