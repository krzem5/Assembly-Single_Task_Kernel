#include <kernel/lock/mutex.h>
#include <kernel/lock/profiling.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/mp/event.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "mutex"



static omm_allocator_t* KERNEL_INIT_WRITE _mutex_allocator=NULL;



KERNEL_EARLY_INIT(){
	_mutex_allocator=omm_init("kernel.mutex",sizeof(mutex_t),8,4);
}



KERNEL_PUBLIC mutex_t* mutex_init(void){
	mutex_t* out=omm_alloc(_mutex_allocator);
	rwlock_init(&(out->lock));
	out->holder=NULL;
	out->event=event_create();
	lock_profiling_init(LOCK_PROFILING_FLAG_PREEMPTIBLE,out);
	return out;
}



KERNEL_PUBLIC void mutex_deinit(mutex_t* lock){
	if (lock->holder){
		panic("mutex_deinit: lock is held");
	}
	event_delete(lock->event);
	omm_dealloc(_mutex_allocator,lock);
}



KERNEL_PUBLIC void mutex_acquire(mutex_t* lock){
	lock_profiling_acquire_start(lock);
	rwlock_acquire_write(&(lock->lock));
	while (lock->holder){
		rwlock_release_write(&(lock->lock));
		event_await(lock->event,0);
		rwlock_acquire_write(&(lock->lock));
	}
	lock->holder=THREAD_DATA->header.current_thread;
	rwlock_release_write(&(lock->lock));
	lock_profiling_acquire_end(lock);
}



KERNEL_PUBLIC void mutex_release(mutex_t* lock){
	rwlock_acquire_write(&(lock->lock));
	if (lock->holder!=THREAD_DATA->header.current_thread){
		panic("mutex_release: lock released by wrong thread");
	}
	lock->holder=NULL;
	rwlock_release_write(&(lock->lock));
	lock_profiling_release(lock);
	event_dispatch(lock->event,EVENT_DISPATCH_FLAG_BYPASS_ACL);
}



KERNEL_PUBLIC bool mutex_is_held(mutex_t* lock){
	return !!lock->holder;
}
