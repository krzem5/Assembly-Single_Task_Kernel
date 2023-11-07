#include <kernel/lock/spinlock.h>
#include <kernel/notification/notification.h>
#include <kernel/types.h>



void notification_dispatcher_init(notification_dispatcher_t* notification_dispatcher){
	spinlock_init(&(notification_dispatcher->lock));
	notification_dispatcher->head=NULL;
}
