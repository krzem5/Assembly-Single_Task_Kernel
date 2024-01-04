#ifndef _KERNEL_LOCK_BITLOCK_H_
#define _KERNEL_LOCK_BITLOCK_H_ 1
#include <kernel/types.h>



#define SPINLOCK_INIT_STRUCT (0)



void bitlock_init(u32* field,u32 bit);



void bitlock_acquire_exclusive(u32* field,u32 bit);



void bitlock_release_exclusive(u32* field,u32 bit);



_Bool bitlock_is_held(u32* field,u32 bit);



#endif
