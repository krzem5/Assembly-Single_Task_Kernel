#ifndef _KERNEL_NOTIFICATION_NOTIFICATION_H_
#define _KERNEL_NOTIFICATION_NOTIFICATION_H_ 1
#include <kernel/lock/rwlock.h>
#include <kernel/types.h>



#define NOTIFICATION_TYPE_HANDLE_CREATE 1
#define NOTIFICATION_TYPE_HANDLE_DELETE 2

#define NOTIFICATION_TYPE_SHUTDOWN_POWEROFF 3
#define NOTIFICATION_TYPE_SHUTDOWN_RESTART 4

#define NOTIFICATION_TYPE_KEYRING_UPDATE 5



typedef struct _NOTIFICATION2{
	u64 object;
	u32 type;
} notification2_t;



typedef struct _NOTIFICATION2_CONTAINER{
	struct _NOTIFICATION2_CONTAINER* next;
	notification2_t data;
} notification2_container_t;



typedef struct _NOTIFICATION2_CONSUMER{
	struct _NOTIFICATION2_DISPATCHER* dispatcher;
	struct _NOTIFICATION2_CONSUMER* prev;
	struct _NOTIFICATION2_CONSUMER* next;
	rwlock_t lock;
	struct _EVENT* event;
	notification2_container_t* head;
	notification2_container_t* tail;
} notification2_consumer_t;



typedef struct _NOTIFICATION2_DISPATCHER{
	notification2_consumer_t* head;
	rwlock_t lock;
} notification2_dispatcher_t;



void notification2_dispatcher_init(notification2_dispatcher_t* dispatcher);



void notification2_dispatcher_deinit(notification2_dispatcher_t* dispatcher);



void notification2_dispatcher_dispatch(notification2_dispatcher_t* dispatcher,u64 object,u32 type);



notification2_consumer_t* notification2_consumer_create(notification2_dispatcher_t* dispatcher);



void notification2_consumer_delete(notification2_consumer_t* consumer);



bool notification2_consumer_get(notification2_consumer_t* consumer,bool wait,notification2_t* out);



#endif
