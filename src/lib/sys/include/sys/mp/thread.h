#ifndef _SYS_MP_THREAD_H_
#define _SYS_MP_THREAD_H_ 1
#include <sys/error/error.h>
#include <sys/mp/event.h>
#include <sys/mp/process.h>
#include <sys/types.h>



#define SYS_THREAD_PRIORITY_BACKGROUND 0
#define SYS_THREAD_PRIORITY_LOW 1
#define SYS_THREAD_PRIORITY_NORMAL 2
#define SYS_THREAD_PRIORITY_HIGH 3
#define SYS_THREAD_PRIORITY_REALTIME 4
#define SYS_THREAD_PRIORITY_TERMINATED 255

#define SYS_THREAD_STATE_TYPE_NONE 0
#define SYS_THREAD_STATE_TYPE_QUEUED 1
#define SYS_THREAD_STATE_TYPE_RUNNING 2
#define SYS_THREAD_STATE_TYPE_AWAITING_EVENT 3
#define SYS_THREAD_STATE_TYPE_TERMINATED 255

#define SYS_THREAD_ITER_ALL_PROCESSES 0



typedef u64 sys_thread_t;



typedef u64 sys_thread_priority_t;



typedef struct _SYS_THREAD_QUERY_RESULT{
	sys_process_t pid;
	sys_thread_t tid;
	char name[256];
	u8 state;
	u8 priority;
	u8 scheduler_priority;
	void* return_value;
} sys_thread_query_result_t;



u64 sys_thread_await_event(sys_event_t event);



u64 __attribute__((access(read_only,1,2),nonnull)) sys_thread_await_events(const sys_event_t* events,u32 count);



sys_error_t __attribute__((nonnull(1))) sys_thread_create(void* func,void* arg,void* stack);



sys_thread_priority_t sys_thread_get_priority(sys_thread_t thread);



sys_thread_t sys_thread_get_handle(void);



sys_error_t sys_thread_set_priority(sys_thread_t thread,sys_thread_priority_t priority);



sys_error_t sys_thread_start(sys_thread_t thread);



sys_error_t sys_thread_stop(sys_thread_t thread,void* return_value);



void* sys_thread_get_return_value(sys_thread_t thread);



sys_thread_t sys_thread_iter_start(sys_process_t process);



sys_thread_t sys_thread_iter_next(sys_process_t process,sys_thread_t thread);



sys_error_t sys_thread_query(sys_thread_t thread,sys_thread_query_result_t* out);



sys_error_t sys_thread_set_name(sys_thread_t thread,const char* name);



#endif
