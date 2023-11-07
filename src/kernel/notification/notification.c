#include <kernel/lock/spinlock.h>
#include <kernel/notification/notification.h>
#include <kernel/types.h>



void notification_dispatcher_init(notification_dispatcher_t* dispatcher){
	spinlock_init(&(dispatcher->lock));
	dispatcher->head=NULL;
}



void notification_dispatcher_add_listener(notification_dispatcher_t* dispatcher,notification_listener_t* listener){
	spinlock_acquire_exclusive(&(dispatcher->lock));
	listener->prev=NULL;
	listener->next=dispatcher->head;
	if (dispatcher->head){
		dispatcher->head->prev=listener;
	}
	dispatcher->head=listener;
	spinlock_release_exclusive(&(dispatcher->lock));
}



void notification_dispatcher_remove_listener(notification_dispatcher_t* dispatcher,notification_listener_t* listener){
	spinlock_acquire_exclusive(&(dispatcher->lock));
	if (listener->prev){
		listener->prev->next=listener->next;
	}
	else{
		dispatcher->head=listener->next;
	}
	if (listener->next){
		listener->next->prev=listener->prev;
	}
	spinlock_release_exclusive(&(dispatcher->lock));
}



void notification_dispatcher_dispatch(notification_dispatcher_t* dispatcher,void* object,u32 type){
	spinlock_acquire_shared(&(dispatcher->lock));
	for (const notification_listener_t* listener=dispatcher->head;listener;listener=listener->next){
		listener->callback(object,type);
	}
	spinlock_release_shared(&(dispatcher->lock));
}
