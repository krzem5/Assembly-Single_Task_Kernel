#ifndef _KERNEL_SCHEDULER_SCHEDULER_H_
#define _KERNEL_SCHEDULER_SCHEDULER_H_ 1
#include <kernel/mp/thread.h>
#include <kernel/types.h>



#define SCHEDULER_TIMER_USER 0
#define SCHEDULER_TIMER_KERNEL 1
#define SCHEDULER_TIMER_SCHEDULER 2
#define SCHEDULER_TIMER_NONE 3

#define SCHEDULER_MAX_TIMER SCHEDULER_TIMER_NONE



typedef struct _SCHEDULER_TIMERS{
	u64 data[SCHEDULER_MAX_TIMER+1];
} scheduler_timers_t;



typedef struct _SCHEDULER{
	thread_t* current_thread;
	u32 pause_remaining_us;
	u32 pause_nested_count;
	u64 pause_start_ticks;
	scheduler_timers_t timers;
	u64 current_timer_start;
	u8 current_timer;
} scheduler_t;



void scheduler_init(void);



void scheduler_enable(void);



void scheduler_pause(void);



void scheduler_resume(void);



void scheduler_isr_handler(isr_state_t* state);



void scheduler_enqueue_thread(thread_t* thread);



const scheduler_timers_t* scheduler_get_timers(u16 cpu_index);



void scheduler_set_timer(u8 timer);



void scheduler_yield(void);



void KERNEL_NORETURN scheduler_task_wait_loop(void);



#endif
