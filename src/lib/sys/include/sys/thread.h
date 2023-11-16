#ifndef _SYS_THREAD_H_
#define _SYS_THREAD_H_ 1
#include <sys/types.h>



#define SYS_THREAD_PRIORITY_BACKGROUND 1
#define SYS_THREAD_PRIORITY_LOW 2
#define SYS_THREAD_PRIORITY_NORMAL 3
#define SYS_THREAD_PRIORITY_HIGH 4
#define SYS_THREAD_PRIORITY_REALTIME 5



typedef struct _SYS_THREAD{
	u32 id;
} sys_thread_t;



static inline const __seg_gs sys_thread_t* sys_thread_current(void){
	return NULL;
}



void __attribute__((noreturn)) sys_thread_stop(void);



u64 sys_thread_create(void (*func)(void*),void* arg,u64 stack_size);



u32 sys_thread_get_priority(u64 handle);



_Bool sys_thread_set_priority(u64 handle,u32 priority);



#endif
