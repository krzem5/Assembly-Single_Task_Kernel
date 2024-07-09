#include <kernel/acl/acl.h>
#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/handle/handle_list.h>
#include <kernel/lock/mutex.h>
#include <kernel/lock/profiling.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/mp/event.h>
#include <kernel/mp/thread.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "mutex"



static omm_allocator_t* KERNEL_INIT_WRITE _mutex_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _mutex_handle_allocator=NULL;
static handle_type_t KERNEL_INIT_WRITE _mutex_handle_type=0;



static void _mutex_handle_destructor(handle_t* handle){
	mutex_handle_t* mutex_handle=KERNEL_CONTAINEROF(handle,mutex_handle_t,handle);
	if (!mutex_handle->is_deleted){
		panic("_mutex_handle_destructor: unreferenced mutex not deleted");
	}
	omm_dealloc(_mutex_handle_allocator,mutex_handle);
}



KERNEL_EARLY_INIT(){
	_mutex_allocator=omm_init("kernel.mutex",sizeof(mutex_t),8,4);
	rwlock_init(&(_mutex_allocator->lock));
	_mutex_handle_allocator=omm_init("kernel.mutex.handle",sizeof(mutex_handle_t),8,4);
	rwlock_init(&(_mutex_handle_allocator->lock));
	_mutex_handle_type=handle_alloc("kernel.mutex",HANDLE_DESCRIPTOR_FLAG_ALLOW_CONTAINER,_mutex_handle_destructor);
}



KERNEL_PUBLIC mutex_t* mutex_create(const char* name){
	mutex_t* out=omm_alloc(_mutex_allocator);
	out->name=name;
	rwlock_init(&(out->lock));
	out->holder=NULL;
	out->event=event_create("kernel.mutex",name);
	lock_profiling_init(LOCK_PROFILING_FLAG_PREEMPTIBLE,out);
	return out;
}



KERNEL_PUBLIC void mutex_delete(mutex_t* lock){
	if (lock->holder){
		panic("mutex_delete: lock is held");
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
	handle_acquire(&(lock->holder->handle));
	rwlock_release_write(&(lock->lock));
	lock_profiling_acquire_end(lock);
}



KERNEL_PUBLIC void mutex_release(mutex_t* lock){
	rwlock_acquire_write(&(lock->lock));
	if (lock->holder!=THREAD_DATA->header.current_thread){
		panic("mutex_release: lock released by wrong thread");
	}
	handle_release(&(lock->holder->handle));
	lock->holder=NULL;
	rwlock_release_write(&(lock->lock));
	lock_profiling_release(lock);
	event_dispatch(lock->event,EVENT_DISPATCH_FLAG_BYPASS_ACL);
}



KERNEL_PUBLIC bool mutex_is_held(mutex_t* lock){
	return !!lock->holder;
}



error_t syscall_mutex_create(void){
	mutex_handle_t* mutex_handle=omm_alloc(_mutex_handle_allocator);
	handle_new(_mutex_handle_type,&(mutex_handle->handle));
	mutex_handle->is_deleted=0;
	mutex_handle->handle.acl=acl_create();
	if (CPU_HEADER_DATA->current_thread){
		acl_set(mutex_handle->handle.acl,THREAD_DATA->process,0,MUTEX_ACL_FLAG_DELETE|MUTEX_ACL_FLAG_QUERY|MUTEX_ACL_FLAG_IO);
	}
	mutex_handle->mutex=mutex_create("user");
	return mutex_handle->handle.rb_node.key;
}
