#ifndef _KERNEL_MP_EVENT_H_
#define _KERNEL_MP_EVENT_H_ 1
#include <kernel/mp/_mp_types.h>



event_t* event_new(void);



void event_delete(event_t* event);



void event_dispatch(event_t* event,_Bool dispatch_all);



#endif
