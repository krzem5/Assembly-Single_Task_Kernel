#ifndef _USER_LOCK_H_
#define _USER_LOCK_H_ 1
#include <user/types.h>



#define LOCK_INIT_STRUCT (0)



typedef u32 lock_t;



void lock_init(lock_t* out);



void lock_acquire(lock_t* lock);



void lock_release(lock_t* lock);



void lock_acquire_multiple(lock_t* lock);



void lock_release_multiple(lock_t* lock);



#endif
