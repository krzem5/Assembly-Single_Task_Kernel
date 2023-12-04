#ifndef _KERNEL_MP_EVENT_H_
#define _KERNEL_MP_EVENT_H_ 1
#include <kernel/mp/_mp_types.h>
#include <kernel/types.h>



event_t* event_new(void);



void event_delete(event_t* event);



void event_dispatch(event_t* event,_Bool dispatch_all);



void event_await(event_t* event);



void event_await_multiple(event_t*const* events,u32 count);



#endif
