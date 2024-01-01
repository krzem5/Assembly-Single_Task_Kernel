#ifndef _SYS2_MP_THREAD_H_
#define _SYS2_MP_THREAD_H_ 1
#include <sys2/error/error.h>
#include <sys2/mp/event.h>
#include <sys2/types.h>



#define SYS2_THREAD_PRIORITY_BACKGROUND 0
#define SYS2_THREAD_PRIORITY_LOW 1
#define SYS2_THREAD_PRIORITY_NORMAL 2
#define SYS2_THREAD_PRIORITY_HIGH 3
#define SYS2_THREAD_PRIORITY_REALTIME 4
#define SYS2_THREAD_PRIORITY_TERMINATED 255



typedef u64 sys2_thread_t;



typedef u64 sys2_thread_priority_t;



u64 sys2_thread_await_events(const sys2_event_t* events,u32 count);



sys2_thread_t sys2_thread_create(void* func,void* arg,u64 stack_size);



sys2_error_t thread_get_cpu_mask(sys2_thread_t thread,void* cpumask,u32 cpumask_size);



sys2_thread_priority_t sys2_thread_get_priority(sys2_thread_t thread);



sys2_thread_t sys2_thread_get_handle(void);



sys2_error_t sys2_thread_set_cpu_mask(sys2_thread_t thread,void* cpumask,u32 cpumask_size);



sys2_error_t sys2_thread_set_priority(sys2_thread_t thread,sys2_thread_priority_t priority);



sys2_error_t sys2_thread_stop(sys2_thread_t thread);



#endif
