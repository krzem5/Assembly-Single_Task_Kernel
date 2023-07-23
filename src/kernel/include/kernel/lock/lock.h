#ifndef _KERNEL_LOCK_LOCK_H_
#define _KERNEL_LOCK_LOCK_H_ 1
#include <kernel/types.h>



#define LOCK_INIT_STRUCT (0)



typedef u32 lock_t;



void lock_init(lock_t* out);



void lock_acquire(lock_t* lock);



void lock_release(lock_t* lock);



#endif
