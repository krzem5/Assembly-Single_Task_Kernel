#ifndef _KERNEL_MP_THREAD_H_
#define _KERNEL_MP_THREAD_H_ 1
#include <kernel/mp/_mp_types.h>
#include <kernel/types.h>



#define THREAD_PRIORITY_TERMINATED 0
#define THREAD_PRIORITY_BACKGROUND 1
#define THREAD_PRIORITY_LOW 2
#define THREAD_PRIORITY_NORMAL 3
#define THREAD_PRIORITY_HIGH 4
#define THREAD_PRIORITY_REALTIME 5

#define THREAD_PRIORITY_MIN THREAD_PRIORITY_BACKGROUND
#define THREAD_PRIORITY_MAX THREAD_PRIORITY_REALTIME

#define THREAD_STATE_TYPE_NONE 0
#define THREAD_STATE_TYPE_QUEUED 1
#define THREAD_STATE_TYPE_RUNNING 2
#define THREAD_STATE_TYPE_AWAITING_EVENT 3
#define THREAD_STATE_TYPE_TERMINATED 255

#define THREAD_DATA ((volatile __seg_gs thread_t*)NULL)



extern handle_type_t HANDLE_TYPE_THREAD;



thread_t* thread_new(process_t* process,u64 rip,u64 stack_size);



void thread_delete(thread_t* thread);



void KERNEL_NORETURN thread_terminate(void);



void thread_await_event(event_t* event);



#endif
