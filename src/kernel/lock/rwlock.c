#include <kernel/lock/profiling.h>
#include <kernel/lock/rwlock.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#include <kernel/util/spinloop.h>



#define RWLOCK_LOCK_BIT 0
#define RWLOCK_READ_BIT 1
#define RWLOCK_FIRST_COUNTER_BIT 2



KERNEL_PUBLIC void rwlock_init(rwlock_t* out){
	__atomic_store_n(&(out->value),0,__ATOMIC_SEQ_CST);
	lock_profiling_init(out);
}



KERNEL_PUBLIC void rwlock_acquire_write(rwlock_t* lock){
	scheduler_pause();
	while (__builtin_expect(__atomic_fetch_or(&(lock->value),1<<RWLOCK_LOCK_BIT,__ATOMIC_SEQ_CST)&(1<<RWLOCK_LOCK_BIT),0)){
		__pause();
	}
	__atomic_store_n(&(lock->value),1<<RWLOCK_LOCK_BIT,__ATOMIC_SEQ_CST);
}



KERNEL_PUBLIC void rwlock_release_write(rwlock_t* lock){
	__atomic_store_n(&(lock->value),0,__ATOMIC_SEQ_CST);
	scheduler_resume();
}



KERNEL_PUBLIC void rwlock_acquire_read(rwlock_t* lock){
	scheduler_pause();
	while (1){
		u32 value=__atomic_fetch_or(&(lock->value),1<<RWLOCK_LOCK_BIT,__ATOMIC_SEQ_CST);
		if (value&(1<<RWLOCK_READ_BIT)){
			__atomic_add_fetch(&(lock->value),1<<RWLOCK_FIRST_COUNTER_BIT,__ATOMIC_SEQ_CST);
			return;
		}
		if (!(value&(1<<RWLOCK_LOCK_BIT))){
			__atomic_store_n(&(lock->value),(1<<RWLOCK_LOCK_BIT)|(1<<RWLOCK_READ_BIT)|(1<<RWLOCK_FIRST_COUNTER_BIT),__ATOMIC_SEQ_CST);
			return;
		}
		__pause();
	}
}



KERNEL_PUBLIC void rwlock_release_read(rwlock_t* lock){
	if (__atomic_sub_fetch(&(lock->value),1<<RWLOCK_FIRST_COUNTER_BIT,__ATOMIC_SEQ_CST)==((1<<RWLOCK_LOCK_BIT)|(1<<RWLOCK_READ_BIT))){
		__atomic_store_n(&(lock->value),0,__ATOMIC_SEQ_CST);
	}
	scheduler_resume();
}



KERNEL_PUBLIC bool rwlock_is_held(rwlock_t* lock){
	return !!lock->value;
}
