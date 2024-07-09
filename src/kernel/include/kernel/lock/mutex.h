#ifndef _KERNEL_LOCK_MUTEX_H_
#define _KERNEL_LOCK_MUTEX_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/lock/profiling.h>
#include <kernel/lock/rwlock.h>
#include <kernel/mp/event.h>
#include <kernel/mp/thread.h>
#include <kernel/types.h>



#define MUTEX_ACL_FLAG_DELETE 1
#define MUTEX_ACL_FLAG_QUERY 2
#define MUTEX_ACL_FLAG_IO 4



typedef struct _MUTEX{
	const char* name;
	rwlock_t lock;
	thread_t* holder;
	event_t* event;
	LOCK_PROFILING_DATA
} mutex_t;



typedef struct _MUTEX_HANDLE{
	handle_t handle;
	bool is_deleted;
	mutex_t* mutex;
} mutex_handle_t;



mutex_t* mutex_create(const char* name);



void mutex_delete(mutex_t* lock);



void mutex_acquire(mutex_t* lock);



void mutex_release(mutex_t* lock);



bool mutex_is_held(mutex_t* lock);



#endif
