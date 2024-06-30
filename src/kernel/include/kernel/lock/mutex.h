#ifndef _KERNEL_LOCK_MUTEX_H_
#define _KERNEL_LOCK_MUTEX_H_ 1
#include <kernel/lock/profiling.h>
#include <kernel/lock/rwlock.h>
#include <kernel/mp/event.h>
#include <kernel/mp/thread.h>
#include <kernel/types.h>



typedef struct _MUTEX{
	const char* name;
	rwlock_t lock;
	thread_t* holder;
	event_t* event;
	LOCK_PROFILING_DATA
} mutex_t;



mutex_t* mutex_init(const char* name);



void mutex_deinit(mutex_t* lock);



void mutex_acquire(mutex_t* lock);



void mutex_release(mutex_t* lock);



bool mutex_is_held(mutex_t* lock);



#endif
