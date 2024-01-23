#include <kernel/acl/acl.h>
#include <kernel/clock/clock.h>
#include <kernel/cpu/cpu.h>
#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/handle/handle_list.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/event.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/timer/timer.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "timer"



static omm_allocator_t* KERNEL_INIT_WRITE _timer_allocator=NULL;
static rb_tree_t _timer_tree;

handle_type_t timer_handle_type=0;



KERNEL_EARLY_INIT(){
	LOG("Initializing timers...");
	_timer_allocator=omm_init("timer",sizeof(timer_t),8,4,pmm_alloc_counter("omm_timer"));
	spinlock_init(&(_timer_allocator->lock));
	rb_tree_init(&_timer_tree);
	timer_handle_type=handle_alloc("timer",NULL);
}



KERNEL_PUBLIC timer_t* timer_create(u64 interval,u64 count){
	timer_t* out=omm_alloc(_timer_allocator);
	out->rb_node.key=0;
	handle_new(out,timer_handle_type,&(out->handle));
	out->handle.acl=acl_create();
	if (CPU_HEADER_DATA->current_thread){
		acl_set(out->handle.acl,THREAD_DATA->process,0,TIMER_ACL_FLAG_UPDATE|TIMER_ACL_FLAG_DELETE);
	}
	spinlock_init(&(out->lock));
	out->event=event_create();
	out->interval=0;
	out->count=0;
	handle_finish_setup(&(out->handle));
	timer_update(out,interval,count,1);
	return out;
}



KERNEL_PUBLIC void timer_delete(timer_t* timer){
	if (CPU_HEADER_DATA->current_thread&&!(acl_get(timer->handle.acl,THREAD_DATA->process)&TIMER_ACL_FLAG_DELETE)){
		return;
	}
	scheduler_pause();
	spinlock_acquire_exclusive(&(timer->lock));
	if (timer->rb_node.key){
		rb_tree_remove_node(&_timer_tree,&(timer->rb_node));
	}
	handle_destroy(&(timer->handle));
	event_delete(timer->event);
	omm_dealloc(_timer_allocator,timer);
	scheduler_resume();
}



KERNEL_PUBLIC u64 timer_get_deadline(timer_t* timer){
	return timer->rb_node.key;
}



KERNEL_PUBLIC void timer_update(timer_t* timer,u64 interval,u64 count,_Bool bypass_acl){
	if (!bypass_acl&&CPU_HEADER_DATA->current_thread&&!(acl_get(timer->handle.acl,THREAD_DATA->process)&TIMER_ACL_FLAG_UPDATE)){
		return;
	}
	scheduler_pause();
	spinlock_acquire_exclusive(&(timer->lock));
	event_set_active(timer->event,0,0);
	if (timer->rb_node.key){
		rb_tree_remove_node(&_timer_tree,&(timer->rb_node));
	}
	if (count==TIMER_COUNT_ABSOLUTE_TIME){
		timer->rb_node.key=interval;
		timer->count=0;
	}
	else{
		timer->rb_node.key=clock_get_time()+interval;
		timer->interval=interval;
		timer->count=count;
	}
	if (timer->rb_node.key){
		rb_tree_insert_node(&_timer_tree,&(timer->rb_node));
	}
	spinlock_release_exclusive(&(timer->lock));
	scheduler_resume();
}



void timer_dispatch_timers(void){
	u64 time=clock_get_time();
	timer_t* timer=(timer_t*)rb_tree_lookup_decreasing_node(&_timer_tree,time);
	if (!timer){
		return;
	}
	scheduler_pause();
	spinlock_acquire_exclusive(&(timer->lock));
	rb_tree_remove_node(&_timer_tree,&(timer->rb_node));
	event_dispatch(timer->event,EVENT_DISPATCH_FLAG_DISPATCH_ALL|EVENT_DISPATCH_FLAG_BYPASS_ACL);
	if (timer->count<=1){
		event_set_active(timer->event,1,0);
		timer->rb_node.key=0;
		timer->count=0;
	}
	else{
		timer->count--;
		timer->rb_node.key=time+timer->interval;
		rb_tree_insert_node(&_timer_tree,&(timer->rb_node));
	}
	spinlock_release_exclusive(&(timer->lock));
	scheduler_resume();
}



error_t syscall_timer_create(u64 interval,u64 count){
	timer_t* timer=timer_create(interval,count);
	handle_list_push(&(THREAD_DATA->process->handle_list),&(timer->handle));
	return timer->handle.rb_node.key;
}



error_t syscall_timer_delete(handle_id_t timer_handle){
	handle_t* handle=handle_lookup_and_acquire(timer_handle,timer_handle_type);
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	timer_t* timer=handle->object;
	if (!(acl_get(timer->handle.acl,THREAD_DATA->process)&TIMER_ACL_FLAG_DELETE)){
		handle_release(handle);
		return ERROR_DENIED;
	}
	handle_release(handle);
	timer_delete(timer);
	return ERROR_OK;
}



error_t syscall_timer_get_deadline(handle_id_t timer_handle){
	handle_t* handle=handle_lookup_and_acquire(timer_handle,timer_handle_type);
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	timer_t* timer=handle->object;
	u64 out=timer_get_deadline(timer);
	handle_release(handle);
	return out;
}



error_t syscall_timer_update(handle_id_t timer_handle,u64 interval,u64 count){
	handle_t* handle=handle_lookup_and_acquire(timer_handle,timer_handle_type);
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	timer_t* timer=handle->object;
	if (!(acl_get(timer->handle.acl,THREAD_DATA->process)&TIMER_ACL_FLAG_UPDATE)){
		handle_release(handle);
		return ERROR_DENIED;
	}
	timer_update(timer,interval,count,0);
	u64 out=timer_get_deadline(timer);
	handle_release(handle);
	return out;
}



error_t syscall_timer_get_event(handle_id_t timer_handle){
	handle_t* handle=handle_lookup_and_acquire(timer_handle,timer_handle_type);
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	timer_t* timer=handle->object;
	u64 out=timer->event->handle.rb_node.key;
	handle_release(handle);
	return out;
}
