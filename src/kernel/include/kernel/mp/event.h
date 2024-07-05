#ifndef _KERNEL_MP_EVENT_H_
#define _KERNEL_MP_EVENT_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/mp/_mp_types.h>
#include <kernel/types.h>



#define EVENT_DISPATCH_FLAG_DISPATCH_ALL 1
#define EVENT_DISPATCH_FLAG_SET_ACTIVE 2
#define EVENT_DISPATCH_FLAG_BYPASS_ACL 4

#define EVENT_ACL_FLAG_DISPATCH 1
#define EVENT_ACL_FLAG_DELETE 2
#define EVENT_ACL_FLAG_QUERY 4



typedef struct _EVENT_QUERY_USER_DATA{
	u64 eid;
	char name[256];
	bool is_active;
} event_query_user_data_t;



extern handle_type_t event_handle_type;



event_t* event_create(const char* name,const char* resource);



void event_delete(event_t* event);



void event_dispatch(event_t* event,u32 flags);



void event_await(event_t* event,bool is_io_wait);



u32 event_await_multiple(event_t*const* events,u32 count);



u32 event_await_multiple_handles(const handle_id_t* handles,u32 count);



void event_set_active(event_t* event,bool is_active,bool bypass_acl);



void event_await_thread_irq(thread_t* thread,event_t* event);



#endif
