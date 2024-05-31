#ifndef _KERNEL_TIMER_TIMER_H_
#define _KERNEL_TIMER_TIMER_H_ 1
#include <kernel/acl/acl.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/rwlock.h>
#include <kernel/mp/event.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>



#define TIMER_COUNT_ABSOLUTE_TIME 0
#define TIMER_COUNT_INFINITE 0xffffffffffffffffull

#define TIMER_ACL_FLAG_UPDATE 1
#define TIMER_ACL_FLAG_DELETE 2



typedef struct _TIMER{
	rb_tree_node_t rb_node;
	handle_t handle;
	rwlock_t lock;
	event_t* event;
	u64 interval;
	u64 count;
	bool is_deleted;
} timer_t;



extern handle_type_t timer_handle_type;



timer_t* timer_create(u64 interval,u64 count);



void timer_delete(timer_t* timer);



u64 timer_get_deadline(timer_t* timer);



void timer_update(timer_t* timer,u64 interval,u64 count,bool bypass_acl);



u32 timer_dispatch_timers(void);



#endif
