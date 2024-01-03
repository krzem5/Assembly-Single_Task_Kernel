#ifndef _SYS_MP_TIMER_H_
#define _SYS_MP_TIMER_H_ 1
#include <sys/error/error.h>
#include <sys/mp/event.h>
#include <sys/types.h>



#define SYS_TIMER_COUNT_ABSOLUTE_TIME 0
#define SYS_TIMER_COUNT_INFINITE 0xffffffffffffffffull



typedef u64 sys_timer_t;



sys_timer_t sys_timer_create(u64 interval,u64 count);



sys_error_t sys_timer_delete(sys_timer_t timer);



u64 sys_timer_get_deadline(sys_timer_t timer);



sys_event_t sys_timer_get_event(sys_timer_t timer);



u64 sys_timer_update(sys_timer_t timer,u64 interval,u64 count);



#endif
