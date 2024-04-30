#ifndef _KERNEL_LOCK_BITLOCK_H_
#define _KERNEL_LOCK_BITLOCK_H_ 1
#include <kernel/types.h>



#define RWLOCK_INIT_STRUCT (0)



void bitlock_init(u32* field,u32 bit);



void bitlock_acquire(u32* field,u32 bit);



bool bitlock_try_acquire(u32* field,u32 bit);



void bitlock_release(u32* field,u32 bit);



bool bitlock_is_held(u32* field,u32 bit);



#endif
