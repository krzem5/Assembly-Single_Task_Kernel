#ifndef _KERNEL_LOCK_RWLOCK_H_
#define _KERNEL_LOCK_RWLOCK_H_ 1
#include <kernel/types.h>



#define RWLOCK_INIT_STRUCT (0)



typedef u32 rwlock_t;



void (rwlock_init)(rwlock_t* out);



void (rwlock_acquire_write)(rwlock_t* lock);



void (rwlock_release_write)(rwlock_t* lock);



void (rwlock_acquire_read)(rwlock_t* lock);



void (rwlock_release_read)(rwlock_t* lock);



bool rwlock_is_held(rwlock_t* lock);



#ifndef KERNEL_RELEASE
#include <kernel/lock/profiling.h>
#define rwlock_init(lock) __lock_overload_type_function(rwlock_init,lock)
#define rwlock_acquire_write(lock) __lock_overload_acquire_function(rwlock_acquire_write,lock)
#define rwlock_acquire_read(lock) __lock_overload_acquire_function(rwlock_acquire_read,lock)
#define rwlock_release_write(lock) __lock_overload_release_function(rwlock_release_write,lock)
#define rwlock_release_read(lock) __lock_overload_release_function(rwlock_release_read,lock)
#endif



#endif
