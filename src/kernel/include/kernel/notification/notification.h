#ifndef _KERNEL_NOTIFICATION_NOTIFICATION_H_
#define _KERNEL_NOTIFICATION_NOTIFICATION_H_ 1
#include <kernel/lock/spinlock.h>
#include <kernel/types.h>



typedef void (*notification_listener_callback_t)(void*,u32);



typedef struct _NOTIFICATION_LISTENER{
	struct _NOTIFICATION_LISTENER* prev;
	struct _NOTIFICATION_LISTENER* next;
	notification_listener_callback_t callback;
} notification_listener_t;



typedef struct _NOTIFICATION_DISPATCHER{
	spinlock_t lock;
	notification_listener_t* head;
} notification_dispatcher_t;



void notification_dispatcher_init(notification_dispatcher_t* notification_dispatcher);



#endif
