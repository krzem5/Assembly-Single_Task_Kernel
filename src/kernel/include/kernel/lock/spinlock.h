#ifndef _KERNEL_LOCK_SPINLOCK_H_
#define _KERNEL_LOCK_SPINLOCK_H_ 1
#include <kernel/types.h>



#define SPINLOCK_INIT_STRUCT (0)



typedef u32 spinlock_t;



void (spinlock_init)(spinlock_t* out);



void (spinlock_acquire_exclusive)(spinlock_t* lock);



void spinlock_release_exclusive(spinlock_t* lock);



void (spinlock_acquire_shared)(spinlock_t* lock);



void spinlock_release_shared(spinlock_t* lock);



_Bool spinlock_is_held(spinlock_t* lock);



#if KERNEL_DISABLE_ASSERT==0
#include <kernel/lock/_profiling_overload.h>
#define spinlock_init(lock) __lock_override_type_function(spinlock_init,(lock))
#define spinlock_acquire_exclusive(lock) __lock_override_data_function(spinlock_acquire_exclusive,(lock))
#define spinlock_acquire_shared(lock) __lock_override_data_function(spinlock_acquire_shared,(lock))
#endif



#endif
