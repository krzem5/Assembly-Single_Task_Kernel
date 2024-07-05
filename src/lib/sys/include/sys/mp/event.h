#ifndef _SYS_MP_EVENT_H_
#define _SYS_MP_EVENT_H_ 1
#include <sys/error/error.h>
#include <sys/types.h>



#define SYS_EVENT_DISPATCH_FLAG_SET_ACTIVE 1
#define SYS_EVENT_DISPATCH_FLAG_DISPATCH_ALL 2



typedef u64 sys_event_t;



typedef struct _SYS_EVENT_QUERY_RESULT{
	sys_event_t eid;
	char name[256];
	bool is_active;
} sys_event_query_result_t;



sys_event_t sys_event_create(bool is_active);



sys_error_t sys_event_delete(sys_event_t event);



sys_error_t sys_event_dispatch(sys_event_t event,u32 dispatch_flags);



sys_error_t sys_event_set_active(sys_event_t event,bool is_active);



sys_event_t sys_event_iter_start(void);



sys_event_t sys_event_iter_next(sys_event_t event);



sys_error_t sys_event_iter_query(sys_event_t event,sys_event_query_result_t* out);



#endif
