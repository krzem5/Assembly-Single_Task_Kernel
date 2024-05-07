#ifndef _KERNEL_LOCK_RWLOCK_H_
#define _KERNEL_LOCK_RWLOCK_H_ 1
#include <kernel/lock/profiling.h>
#include <kernel/types.h>



#define RWLOCK_INIT_STRUCT {0}



typedef struct _RWLOCK{
	KERNEL_ATOMIC u32 value;
#ifndef KERNEL_RELEASE
	lock_profiling_data_t __profiling_data;
#endif
} rwlock_t;



void rwlock_init(rwlock_t* out);



void rwlock_acquire_write(rwlock_t* lock);



void rwlock_release_write(rwlock_t* lock);



void rwlock_acquire_read(rwlock_t* lock);



void rwlock_release_read(rwlock_t* lock);



bool rwlock_is_held(rwlock_t* lock);



#endif
