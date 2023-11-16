#ifndef _SYS_THREAD_H_
#define _SYS_THREAD_H_ 1
#include <sys/types.h>



#define THREAD_PRIORITY_BACKGROUND 1
#define THREAD_PRIORITY_LOW 2
#define THREAD_PRIORITY_NORMAL 3
#define THREAD_PRIORITY_HIGH 4
#define THREAD_PRIORITY_REALTIME 5



typedef struct _THREAD{
	u32 id;
} thread_t;



static inline const __seg_gs thread_t* sys_thread_current(void){
	return NULL;
}



void __attribute__((noreturn)) sys_thread_stop(void);



u64 sys_thread_create(void (*func)(void*),void* arg,u64 stack_size);



u32 sys_thread_get_priority(u64 handle);



_Bool sys_thread_set_priority(u64 handle,u32 priority);



#endif
