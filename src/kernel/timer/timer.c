#include <kernel/acl/acl.h>
#include <kernel/clock/clock.h>
#include <kernel/cpu/cpu.h>
#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/handle/handle_list.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/event.h>
#include <kernel/mp/thread.h>
#include <kernel/timer/timer.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "timer"



static omm_allocator_t* KERNEL_INIT_WRITE _timer_allocator=NULL;
static rb_tree_t _timer_tree;

handle_type_t KERNEL_INIT_WRITE timer_handle_type=0;



static void _remove_from_chain(timer_t* timer){
	if (timer->prev){
		timer->prev->next=timer->next;
	}
	else if (timer->next){
		rb_tree_insert_node(&_timer_tree,&(timer->next->rb_node));
	}
	if (timer->next){
		timer->next->prev=timer->prev;
	}
	timer->prev=NULL;
	timer->next=NULL;
}



static void _schedule_timer(timer_t* timer){
	timer_t* chain=(timer_t*)rb_tree_lookup_or_insert_node(&_timer_tree,&(timer->rb_node));
	if (!chain){
		return;
	}
	if (chain->next){
		chain->next->prev=timer;
	}
	timer->prev=chain;
	timer->next=chain->next;
	chain->next=timer;
	rwlock_release_write(&(_timer_tree.lock));
}



static void _timer_handle_destructor(handle_t* handle){
	timer_t* timer=KERNEL_CONTAINEROF(handle,timer_t,handle);
	if (!timer->is_deleted){
		ERROR("DELETE %p",timer);
		if (!timer->prev&&timer->rb_node.key){
			rb_tree_remove_node(&_timer_tree,&(timer->rb_node));
		}
		_remove_from_chain(timer);
		event_delete(timer->event);
	}
	omm_dealloc(_timer_allocator,timer);
}



KERNEL_EARLY_INIT(){
	LOG("Initializing timers...");
	_timer_allocator=omm_init("kernel.timer",sizeof(timer_t),8,4);
	rwlock_init(&(_timer_allocator->lock));
	rb_tree_init(&_timer_tree);
	timer_handle_type=handle_alloc("kernel.timer",HANDLE_DESCRIPTOR_FLAG_ALLOW_CONTAINER,_timer_handle_destructor);
}



KERNEL_PUBLIC timer_t* timer_create(const char* name,u64 interval,u64 count){
	timer_t* out=omm_alloc(_timer_allocator);
	out->rb_node.key=0;
	out->name=name;
	out->prev=NULL;
	out->next=NULL;
	handle_new(timer_handle_type,&(out->handle));
	out->handle.acl=acl_create();
	if (CPU_HEADER_DATA->current_thread){
		acl_set(out->handle.acl,THREAD_DATA->process,0,TIMER_ACL_FLAG_UPDATE|TIMER_ACL_FLAG_DELETE);
	}
	rwlock_init(&(out->lock));
	out->event=event_create("kernel.timer");
	out->interval=0;
	out->count=0;
	out->is_deleted=0;
	timer_update(out,interval,count,1);
	return out;
}



KERNEL_PUBLIC void timer_delete(timer_t* timer){
	if (CPU_HEADER_DATA->current_thread&&!(acl_get(timer->handle.acl,THREAD_DATA->process)&TIMER_ACL_FLAG_DELETE)){
		return;
	}
	rwlock_acquire_write(&(timer->lock));
	if (timer->is_deleted){
		rwlock_release_write(&(timer->lock));
		return;
	}
	timer->is_deleted=1;
	if (!timer->prev&&timer->rb_node.key){
		rb_tree_remove_node(&_timer_tree,&(timer->rb_node));
	}
	_remove_from_chain(timer);
	event_delete(timer->event);
	rwlock_release_write(&(timer->lock));
	handle_release(&(timer->handle));
}



KERNEL_PUBLIC u64 timer_get_deadline(timer_t* timer){
	return timer->rb_node.key;
}



KERNEL_PUBLIC void timer_update(timer_t* timer,u64 interval,u64 count,bool bypass_acl){
	if (!bypass_acl&&CPU_HEADER_DATA->current_thread&&!(acl_get(timer->handle.acl,THREAD_DATA->process)&TIMER_ACL_FLAG_UPDATE)){
		return;
	}
	rwlock_acquire_write(&(timer->lock));
	if (timer->is_deleted){
		rwlock_release_write(&(timer->lock));
		return;
	}
	event_set_active(timer->event,0,0);
	if (!timer->prev&&timer->rb_node.key){
		rb_tree_remove_node(&_timer_tree,&(timer->rb_node));
	}
	timer->rb_node.key=0;
	_remove_from_chain(timer);
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
		_schedule_timer(timer);
	}
	rwlock_release_write(&(timer->lock));
}



u32 timer_dispatch_timers(void){
	u64 time=clock_get_time();
	timer_t* timer=(timer_t*)rb_tree_lookup_decreasing_node(&_timer_tree,time);
	if (!timer){
		goto _return;
	}
	if (!rwlock_try_acquire_write(&(timer->lock))){
		goto _return;
	}
	rb_tree_remove_node(&_timer_tree,&(timer->rb_node));
	_remove_from_chain(timer);
	event_dispatch(timer->event,EVENT_DISPATCH_FLAG_DISPATCH_ALL|EVENT_DISPATCH_FLAG_BYPASS_ACL);
	if (timer->count<=1){
		event_set_active(timer->event,1,0);
		timer->rb_node.key=0;
		timer->count=0;
	}
	else{
		timer->count--;
		timer->rb_node.key=time+timer->interval;
		_schedule_timer(timer);
	}
	rwlock_release_write(&(timer->lock));
_return:
	timer=(timer_t*)rb_tree_iter_start(&_timer_tree);
	return (timer?(timer->rb_node.key>time?(timer->rb_node.key-time)/1000:0):0xffffffff);
}



error_t syscall_timer_create(u64 interval,u64 count){
	timer_t* timer=timer_create("user",interval,count);
	handle_list_push(&(THREAD_DATA->process->handle_list),&(timer->handle));
	return timer->handle.rb_node.key;
}



error_t syscall_timer_delete(handle_id_t timer_handle){
	handle_t* handle=handle_lookup_and_acquire(timer_handle,timer_handle_type);
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	timer_t* timer=KERNEL_CONTAINEROF(handle,timer_t,handle);
	if (!(acl_get(timer->handle.acl,THREAD_DATA->process)&TIMER_ACL_FLAG_DELETE)){
		handle_release(handle);
		return ERROR_DENIED;
	}
	handle_list_pop(&(THREAD_DATA->process->handle_list),handle);
	timer_delete(timer);
	handle_release(handle);
	return ERROR_OK;
}



error_t syscall_timer_get_deadline(handle_id_t timer_handle){
	handle_t* handle=handle_lookup_and_acquire(timer_handle,timer_handle_type);
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	timer_t* timer=KERNEL_CONTAINEROF(handle,timer_t,handle);
	u64 out=timer_get_deadline(timer);
	handle_release(handle);
	return out;
}



error_t syscall_timer_update(handle_id_t timer_handle,u64 interval,u64 count){
	handle_t* handle=handle_lookup_and_acquire(timer_handle,timer_handle_type);
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	timer_t* timer=KERNEL_CONTAINEROF(handle,timer_t,handle);
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
	timer_t* timer=KERNEL_CONTAINEROF(handle,timer_t,handle);
	u64 out=timer->event->handle.rb_node.key;
	handle_release(handle);
	return out;
}
