#ifndef _KERNEL_LOCK_PREEMPTIVELOCK_H_
#define _KERNEL_LOCK_PREEMPTIVELOCK_H_ 1
#include <kernel/lock/rwlock.h>
#include <kernel/mp/event.h>
#include <kernel/mp/thread.h>
#include <kernel/types.h>



typedef struct _PREEMPTIVELOCK{
	rwlock_t lock;
	thread_t* holder;
	event_t* event;
} preemptivelock_t;



preemptivelock_t* preemptivelock_init(void);



void preemptivelock_deinit(preemptivelock_t* lock);



void preemptivelock_acquire(preemptivelock_t* lock);



void preemptivelock_release(preemptivelock_t* lock);



bool preemptivelock_is_held(preemptivelock_t* lock);



#endif
