#include <kernel/acl/acl.h>
#include <kernel/clock/clock.h>
#include <kernel/cpu/cpu.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/event.h>
#include <kernel/mp/thread.h>
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
	out->acl=acl_create();
	if (CPU_HEADER_DATA->current_thread){
		acl_add(out->acl,THREAD_DATA->process,TIMER_ACL_FLAG_UPDATE|TIMER_ACL_FLAG_DELETE);
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
	if (CPU_HEADER_DATA->current_thread&&!(acl_get(timer->acl,THREAD_DATA->process)&TIMER_ACL_FLAG_DELETE)){
		return;
	}
	spinlock_acquire_exclusive(&(timer->lock));
	if (timer->rb_node.key){
		rb_tree_remove_node(&_timer_tree,&(timer->rb_node));
	}
	handle_destroy(&(timer->handle));
	event_delete(timer->event);
	acl_delete(timer->acl);
	omm_dealloc(_timer_allocator,timer);
}



KERNEL_PUBLIC u64 timer_get_deadline(timer_t* timer){
	return timer->rb_node.key;
}



KERNEL_PUBLIC void timer_update(timer_t* timer,u64 interval,u64 count,_Bool bypass_acl){
	if (!bypass_acl&&CPU_HEADER_DATA->current_thread&&!(acl_get(timer->acl,THREAD_DATA->process)&TIMER_ACL_FLAG_UPDATE)){
		return;
	}
	spinlock_acquire_exclusive(&(timer->lock));
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
}



void timer_dispatch_timers(void){
	u64 time=clock_get_time();
	timer_t* timer=(timer_t*)rb_tree_lookup_decreasing_node(&_timer_tree,time);
	if (!timer){
		return;
	}
	spinlock_acquire_exclusive(&(timer->lock));
	rb_tree_remove_node(&_timer_tree,&(timer->rb_node));
	event_dispatch(timer->event,EVENT_DISPATCH_FLAG_DISPATCH_ALL|EVENT_DISPATCH_FLAG_BYPASS_ACL);
	if (timer->count<=1){
		timer->rb_node.key=0;
		timer->count=0;
	}
	else{
		timer->count--;
		timer->rb_node.key=time+timer->interval;
		rb_tree_insert_node(&_timer_tree,&(timer->rb_node));
	}
	spinlock_release_exclusive(&(timer->lock));
}
