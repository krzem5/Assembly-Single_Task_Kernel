#ifndef _SYS_LOCK_H_
#define _SYS_LOCK_H_ 1
#include <sys/types.h>



#define LOCK_INIT_STRUCT (0)



typedef u32 lock_t;



void sys_lock_init(lock_t* out);



void sys_lock_acquire_exclusive(lock_t* lock);



void sys_lock_release_exclusive(lock_t* lock);



void sys_lock_acquire_shared(lock_t* lock);



void sys_lock_release_shared(lock_t* lock);



#endif
