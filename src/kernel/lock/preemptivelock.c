#include <kernel/lock/preemptivelock.h>
#include <kernel/lock/rwlock.h>
#include <kernel/memory/omm.h>
#include <kernel/mp/event.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



static omm_allocator_t* KERNEL_INIT_WRITE _preemptivelock_allocator=NULL;



KERNEL_EARLY_INIT(){
	_preemptivelock_allocator=omm_init("preemptivelock",sizeof(preemptivelock_t),8,4);
}



preemptivelock_t* preemptivelock_init(void){
	preemptivelock_t* out=omm_alloc(_preemptivelock_allocator);
	rwlock_init(&(out->lock));
	out->holder=NULL;
	out->event=event_create();
	return out;
}



void preemptivelock_deinit(preemptivelock_t* lock){
	if (lock->holder){
		panic("preemptivelock_deinit: lock is held");
	}
	event_delete(lock->event);
	omm_dealloc(_preemptivelock_allocator,lock);
}



void preemptivelock_acquire(preemptivelock_t* lock){
	rwlock_acquire_write(&(lock->lock));
	while (lock->holder){
		rwlock_release_write(&(lock->lock));
		event_await(lock->event,0);
		rwlock_acquire_write(&(lock->lock));
	}
	lock->holder=THREAD_DATA->header.current_thread;
	rwlock_release_write(&(lock->lock));
}



void preemptivelock_release(preemptivelock_t* lock){
	rwlock_acquire_write(&(lock->lock));
	if (lock->holder!=THREAD_DATA->header.current_thread){
		panic("preemptivelock_release: lock released by wrong thread");
	}
	lock->holder=NULL;
	rwlock_release_write(&(lock->lock));
}



bool preemptivelock_is_held(preemptivelock_t* lock){
	return !!lock->holder;
}
