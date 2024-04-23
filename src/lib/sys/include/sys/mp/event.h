#ifndef _SYS_MP_EVENT_H_
#define _SYS_MP_EVENT_H_ 1
#include <sys/error/error.h>
#include <sys/types.h>



#define SYS_EVENT_DISPATCH_FLAG_SET_ACTIVE 1
#define SYS_EVENT_DISPATCH_FLAG_DISPATCH_ALL 2



typedef u64 sys_event_t;



sys_event_t sys_event_create(bool is_active);



sys_error_t sys_event_delete(sys_event_t event);



sys_error_t sys_event_dispatch(sys_event_t event,u32 dispatch_flags);



sys_error_t sys_event_set_active(sys_event_t event,bool is_active);



#endif
