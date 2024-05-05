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



void __rwlock_init(rwlock_t* out);



void __rwlock_acquire_write(rwlock_t* lock);



void __rwlock_release_write(rwlock_t* lock);



void __rwlock_acquire_read(rwlock_t* lock);



void __rwlock_release_read(rwlock_t* lock);



bool rwlock_is_held(rwlock_t* lock);



static KERNEL_INLINE KERNEL_NOCOVERAGE void (rwlock_init)(rwlock_t* out){
	__rwlock_init(out);
}



static KERNEL_INLINE KERNEL_NOCOVERAGE void (rwlock_acquire_write)(rwlock_t* lock){
	__rwlock_acquire_write(lock);
}



static KERNEL_INLINE KERNEL_NOCOVERAGE void (rwlock_release_write)(rwlock_t* lock){
	__rwlock_release_write(lock);
}



static KERNEL_INLINE KERNEL_NOCOVERAGE void (rwlock_acquire_read)(rwlock_t* lock){
	__rwlock_acquire_read(lock);
}



static KERNEL_INLINE KERNEL_NOCOVERAGE void (rwlock_release_read)(rwlock_t* lock){
	__rwlock_release_read(lock);
}



#ifndef KERNEL_RELEASE
#define rwlock_init(lock) __lock_overload_type_function(rwlock_init,lock)
#define rwlock_acquire_write(lock) __lock_overload_acquire_function(rwlock_acquire_write,lock)
#define rwlock_acquire_read(lock) __lock_overload_acquire_function(rwlock_acquire_read,lock)
#define rwlock_release_write(lock) __lock_overload_release_function(rwlock_release_write,lock)
#define rwlock_release_read(lock) __lock_overload_release_function(rwlock_release_read,lock)
#endif



#endif
