#ifndef _KERNEL_SCHEDULER_SCHEDULER_H_
#define _KERNEL_SCHEDULER_SCHEDULER_H_ 1
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



typedef struct _SCHEDULER_QUEUE{
	lock_t lock;
	thread_t* head;
	thread_t* tail;
} scheduler_queue_t;



typedef struct _SCHEDULER_QUEUES{
	scheduler_queue_t data[SCHEDULER_QUEUE_COUNT];
} scheduler_queues_t;



typedef struct _SCHEDULER{
	thread_t* current_thread;
	u32 remaining_us;
	u32 nested_pause_count;
	u8 round_robin_timing;
} scheduler_t;



void scheduler_init(void);



void scheduler_enable(void);



void scheduler_pause(void);



void scheduler_resume(void);



scheduler_t* scheduler_new(void);



void scheduler_isr_handler(isr_state_t* state);



void scheduler_enqueue_thread(thread_t* thread);



void scheduler_dequeue_thread(_Bool save_registers);



void scheduler_start(void);



void KERNEL_NORETURN scheduler_task_wait_loop(void);



#endif
