#ifndef _KERNEL_NOTIFICATION_NOTIFICATION_H_
#define _KERNEL_NOTIFICATION_NOTIFICATION_H_ 1
#include <kernel/lock/spinlock.h>
#include <kernel/types.h>



#define NOTIFICATION_TYPE_HANDLE_CREATE 1
#define NOTIFICATION_TYPE_HANDLE_DELETE 2

#define NOTIFICATION_TYPE_SHUTDOWN_POWEROFF 3
#define NOTIFICATION_TYPE_SHUTDOWN_RESTART 4



typedef struct _NOTIFICATION_LISTENER{
	void (*callback)(void*,u32);
	struct _NOTIFICATION_LISTENER* prev;
	struct _NOTIFICATION_LISTENER* next;
} notification_listener_t;



typedef struct _NOTIFICATION_DISPATCHER{
	spinlock_t lock;
	notification_listener_t* head;
} notification_dispatcher_t;



void notification_dispatcher_init(notification_dispatcher_t* dispatcher);



void notification_dispatcher_add_listener(notification_dispatcher_t* dispatcher,notification_listener_t* listener);



void notification_dispatcher_remove_listener(notification_dispatcher_t* dispatcher,notification_listener_t* listener);



void notification_dispatcher_dispatch(notification_dispatcher_t* dispatcher,void* object,u32 type);



#endif
