#include <kernel/lock/rwlock.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/event.h>
#include <kernel/mp/thread.h>
#include <kernel/notification/notification.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



static omm_allocator_t* _notification_consumer_allocator=NULL;
static omm_allocator_t* _notification_container_allocator=NULL;



KERNEL_EARLY_INIT(){
	_notification_consumer_allocator=omm_init("kernel.notification.consumer",sizeof(notification_consumer_t),8,4);
	_notification_container_allocator=omm_init("kernel.notification.container",sizeof(notification_container_t),8,4);
}



KERNEL_PUBLIC void notification_dispatcher_init(notification_dispatcher_t* dispatcher){
	dispatcher->head=NULL;
	rwlock_init(&(dispatcher->lock));
}



KERNEL_PUBLIC void notification_dispatcher_deinit(notification_dispatcher_t* dispatcher){
	panic("notification_dispatcher_deinit");
}



KERNEL_PUBLIC void notification_dispatcher_dispatch(notification_dispatcher_t* dispatcher,u64 object,u32 type){
	if (!_notification_container_allocator){
		return;
	}
	rwlock_acquire_write(&(dispatcher->lock));
	for (notification_consumer_t* consumer=dispatcher->head;consumer;consumer=consumer->next){
		notification_container_t* container=omm_alloc(_notification_container_allocator);
		container->next=NULL;
		container->data.object=object;
		container->data.type=type;
		rwlock_acquire_write(&(consumer->lock));
		if (consumer->tail){
			consumer->tail->next=container;
		}
		else{
			consumer->head=container;
		}
		consumer->tail=container;
		event_dispatch(consumer->event,EVENT_DISPATCH_FLAG_SET_ACTIVE|EVENT_DISPATCH_FLAG_BYPASS_ACL);
		rwlock_release_write(&(consumer->lock));
	}
	rwlock_release_write(&(dispatcher->lock));
}



KERNEL_PUBLIC notification_consumer_t* notification_consumer_create(notification_dispatcher_t* dispatcher){
	notification_consumer_t* out=omm_alloc(_notification_consumer_allocator);
	out->dispatcher=dispatcher;
	out->prev=NULL;
	rwlock_init(&(out->lock));
	out->event=event_create("kernel.notification.consumer");
	out->head=NULL;
	out->tail=NULL;
	rwlock_acquire_write(&(dispatcher->lock));
	out->next=dispatcher->head;
	if (dispatcher->head){
		dispatcher->head->prev=out;
	}
	dispatcher->head=out;
	rwlock_release_write(&(dispatcher->lock));
	return out;
}



KERNEL_PUBLIC void notification_consumer_delete(notification_consumer_t* consumer){
	panic("notification_consumer_delete");
}



KERNEL_PUBLIC bool notification_consumer_get(notification_consumer_t* consumer,bool wait,notification_t* out){
	rwlock_acquire_write(&(consumer->lock));
	while (!consumer->head){
		rwlock_release_write(&(consumer->lock));
		if (!wait){
			return 0;
		}
		event_await(consumer->event,0);
		rwlock_acquire_write(&(consumer->lock));
	}
	notification_container_t* container=consumer->head;
	consumer->head=container->next;
	if (!consumer->head){
		consumer->tail=NULL;
		event_set_active(consumer->event,0,1);
	}
	rwlock_release_write(&(consumer->lock));
	*out=container->data;
	omm_dealloc(_notification_container_allocator,container);
	return 1;
}
