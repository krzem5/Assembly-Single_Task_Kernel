#ifndef _KERNEL_SCHEDULER_LOAD_BALANCER_H_
#define _KERNEL_SCHEDULER_LOAD_BALANCER_H_ 1
#include <kernel/lock/spinlock.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/_scheduler_types.h>
#include <kernel/types.h>



#define SCHEDULER_ROUND_ROBIN_PRIORITY_COUNT 3

#define SCHEDULER_PRIORITY_BACKGROUND 0
#define SCHEDULER_PRIORITY_LOW 1
#define SCHEDULER_PRIORITY_NORMAL 2
#define SCHEDULER_PRIORITY_HIGH 3
#define SCHEDULER_PRIORITY_REALTIME 4
#define SCHEDULER_PRIORITY_TERMINATED 255

#define SCHEDULER_PRIORITY_SHIFT 3

#define SCHEDULER_PRIORITY_MIN SCHEDULER_PRIORITY_BACKGROUND
#define SCHEDULER_PRIORITY_MAX SCHEDULER_PRIORITY_REALTIME

#define SCHEDULER_LOAD_BALANCER_THREAD_QUEUE_COUNT (SCHEDULER_PRIORITY_MAX+1)



typedef struct _SCHEDULER_LOAD_BALANCER_THREAD_QUEUE{
	spinlock_t lock;
	thread_t* head;
	thread_t* tail;
} scheduler_load_balancer_thread_queue_t;



typedef struct _SCHEDULER_LOAD_BALANCER_GROUP{
	union{
		struct{
			u16 length;
			u16 end;
		};
		struct _SCHEDULER_LOAD_BALANCER_GROUP* next_group;
	};
} scheduler_load_balancer_group_t;



typedef struct _SCHEDULER_LOAD_BALANCER_STATS{
	u64 added_thread_count;
	u64 free_slot_count;
	u64 used_slot_count;
} scheduler_load_balancer_stats_t;



typedef struct _SCHEDULER_LOAD_BALANCER_DATA{
	scheduler_load_balancer_stats_t stats;
	scheduler_load_balancer_group_t* group;
	scheduler_load_balancer_thread_queue_t queues[SCHEDULER_LOAD_BALANCER_THREAD_QUEUE_COUNT];
	u16 cpu_index;
	u8 queue_access_timing;
} scheduler_load_balancer_data_t;



typedef struct _SCHEDULER_LOAD_BALANCER{
	spinlock_t lock;
	scheduler_load_balancer_group_t* free_group;
	scheduler_load_balancer_data_t** priority_queue;
} scheduler_load_balancer_t;



void scheduler_load_balancer_init(void);



thread_t* scheduler_load_balancer_get(void);



void scheduler_load_balancer_add(thread_t* thread);



const scheduler_load_balancer_stats_t* scheduler_load_balancer_get_stats(u16 cpu_index);



#endif
