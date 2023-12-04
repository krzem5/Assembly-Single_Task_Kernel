#include <kernel/clock/clock.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/event.h>
#include <kernel/timer/timer.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "timer"



static omm_allocator_t* KERNEL_INIT_WRITE _timer_allocator=NULL;
static rb_tree_t _timer_tree;

handle_type_t timer_handle_type=0;



void KERNEL_EARLY_EXEC timer_init(void){
	_timer_allocator=omm_init("timer",sizeof(timer_t),8,4,pmm_alloc_counter("omm_timer"));
	rb_tree_init(&_timer_tree);
	timer_handle_type=handle_alloc("timer",NULL);
}



KERNEL_PUBLIC timer_t* timer_create(u64 interval,u64 count);



KERNEL_PUBLIC void timer_delete(timer_t* timer);



KERNEL_PUBLIC u64 timer_get_deadline(timer_t* timer);



KERNEL_PUBLIC _Bool timer_update(timer_t* timer,u64 interval,u64 count);



void timer_dispatch_timers(void){
	u64 time=clock_get_time();
	timer_t* timer=(timer_t*)rb_tree_lookup_decreasing_node(&_timer_tree,time);
	if (!timer){
		return;
	}
	rb_tree_remove_node(&_timer_tree,&(timer->rb_node));
	event_dispatch(timer->event,1);
	if (timer->count<=1){
		timer->count=0;
		return;
	}
	timer->count--;
	timer->rb_node.key=time+timer->interval;
	rb_tree_insert_node(&_timer_tree,&(timer->rb_node));
}
