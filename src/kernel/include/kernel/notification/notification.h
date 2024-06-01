#ifndef _KERNEL_NOTIFICATION_NOTIFICATION_H_
#define _KERNEL_NOTIFICATION_NOTIFICATION_H_ 1
#include <kernel/lock/rwlock.h>
#include <kernel/mp/event.h>
#include <kernel/types.h>



typedef struct _NOTIFICATION{
	u32 type;
	u32 length;
	void* data;
} notification_t;



typedef struct _NOTIFICATION_CONTAINER{
	struct _NOTIFICATION_CONTAINER* next;
	notification_t data;
	KERNEL_ATOMIC u64 rc;
} notification_container_t;



typedef struct _NOTIFICATION_CONSUMER{
	rwlock_t lock;
	struct _NOTIFICATION_DISPATCHER* dispatcher;
	struct _NOTIFICATION_CONSUMER* prev;
	struct _NOTIFICATION_CONSUMER* next;
	notification_container_t* last;
	event_t* event;
} notification_consumer_t;



typedef struct _NOTIFICATION_DISPATCHER{
	rwlock_t lock;
	notification_container_t* head;
	notification_container_t* tail;
	notification_consumer_t* consumer_head;
} notification_dispatcher_t;



notification_dispatcher_t* notification_dispatcher_create(void);



void notification_dispatcher_delete(notification_dispatcher_t* dispatcher);



void notification_dispatcher_dispatch(notification_dispatcher_t* dispatcher,u32 type,const void* data,u32 length);



notification_consumer_t* notification_consumer_create(notification_dispatcher_t* dispatcher);



void notification_consumer_delete(notification_consumer_t* consumer);



bool notification_consumer_get(notification_consumer_t* consumer,bool wait,notification_t* out);



#endif
