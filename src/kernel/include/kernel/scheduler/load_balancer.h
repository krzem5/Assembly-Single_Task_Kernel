#ifndef _KERNEL_SCHEDULER_LOAD_BALANCER_H_
#define _KERNEL_SCHEDULER_LOAD_BALANCER_H_ 1
#include <kernel/lock/lock.h>
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

#define SCHEDULER_PRIORITY_MIN SCHEDULER_PRIORITY_BACKGROUND
#define SCHEDULER_PRIORITY_MAX SCHEDULER_PRIORITY_REALTIME

#define SCHEDULER_QUEUE_COUNT (SCHEDULER_PRIORITY_MAX+1)



typedef struct _SCHEDULER_LOAD_BALANCER_THREAD_QUEUE{
	lock_t lock;
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



typedef struct _SCHEDULER_LOAD_BALANCER_DATA{
	u64 counter;
	scheduler_load_balancer_group_t* group;
	scheduler_load_balancer_thread_queue_t queues[SCHEDULER_QUEUE_COUNT];
	u8 round_robin_timing;
	u16 cpu_index;
} scheduler_load_balancer_data_t;



typedef struct _SCHEDULER_LOAD_BALANCER{
	lock_t lock;
	scheduler_load_balancer_group_t* free_group;
	scheduler_load_balancer_data_t** priority_queue;
} scheduler_load_balancer_t;



void scheduler_load_balancer_init(void);



thread_t* scheduler_load_balancer_get(void);



scheduler_load_balancer_thread_queue_t* scheduler_load_balancer_get_queues(scheduler_priority_t priority);



#endif
