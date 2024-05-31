#ifndef _KERNEL_NOTIFICATION_NOTIFICATION_H_
#define _KERNEL_NOTIFICATION_NOTIFICATION_H_ 1
#include <kernel/lock/rwlock.h>
#include <kernel/types.h>



#define NOTIFICATION_TYPE_HANDLE_CREATE 1
#define NOTIFICATION_TYPE_HANDLE_DELETE 2

#define NOTIFICATION_TYPE_SHUTDOWN_POWEROFF 3
#define NOTIFICATION_TYPE_SHUTDOWN_RESTART 4

#define NOTIFICATION_TYPE_KEYRING_UPDATE 5



typedef struct _NOTIFICATION{
	u64 object;
	u32 type;
} notification_t;



typedef struct _NOTIFICATION_CONTAINER{
	struct _NOTIFICATION_CONTAINER* next;
	notification_t data;
} notification_container_t;



typedef struct _NOTIFICATION_CONSUMER{
	struct _NOTIFICATION_DISPATCHER* dispatcher;
	struct _NOTIFICATION_CONSUMER* prev;
	struct _NOTIFICATION_CONSUMER* next;
	rwlock_t lock;
	struct _EVENT* event;
	notification_container_t* head;
	notification_container_t* tail;
} notification_consumer_t;



typedef struct _NOTIFICATION_DISPATCHER{
	notification_consumer_t* head;
	rwlock_t lock;
} notification_dispatcher_t;



void notification_dispatcher_init(notification_dispatcher_t* dispatcher);



void notification_dispatcher_deinit(notification_dispatcher_t* dispatcher);



void notification_dispatcher_dispatch(notification_dispatcher_t* dispatcher,u64 object,u32 type);



notification_consumer_t* notification_consumer_create(notification_dispatcher_t* dispatcher);



void notification_consumer_delete(notification_consumer_t* consumer);



bool notification_consumer_get(notification_consumer_t* consumer,bool wait,notification_t* out);



#endif
