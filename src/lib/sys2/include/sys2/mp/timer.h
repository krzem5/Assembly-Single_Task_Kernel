#ifndef _SYS2_MP_TIMER_H_
#define _SYS2_MP_TIMER_H_ 1
#include <sys2/error/error.h>
#include <sys2/mp/event.h>
#include <sys2/types.h>



#define SYS2_TIMER_COUNT_ABSOLUTE_TIME 0
#define SYS2_TIMER_COUNT_INFINITE 0xffffffffffffffffull



typedef u64 sys2_timer_t;



sys2_timer_t sys2_timer_create(u64 interval,u64 count);



sys2_error_t sys2_timer_delete(sys2_timer_t timer);



u64 sys2_timer_get_deadline(sys2_timer_t timer);



sys2_event_t sys2_timer_get_event(sys2_timer_t timer);



u64 sys2_timer_update(sys2_timer_t timer,u64 interval,u64 count);




#endif
