#ifndef _USER_THREAD_H_
#define _USER_THREAD_H_ 1
#include <user/types.h>



typedef struct _THREAD{
	u32 id;
} thread_t;



static inline const __seg_gs thread_t* thread_current(void){
	return NULL;
}



void __attribute__((noreturn)) thread_stop(void);



u32 __attribute__((returns_twice)) thread_create(u32 stack_size);



#endif
