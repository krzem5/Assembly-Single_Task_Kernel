#ifndef _KERNEL_LOCK_LOCK_H_
#define _KERNEL_LOCK_LOCK_H_ 1
#include <kernel/types.h>



#define LOCK_INIT_STRUCT (0)



typedef u32 lock_t;



void lock_init(lock_t* out);



void lock_acquire_exclusive(lock_t* lock);



void lock_release_exclusive(lock_t* lock);



void lock_acquire_shared(lock_t* lock);



void lock_release_shared(lock_t* lock);



#endif
