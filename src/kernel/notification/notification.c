#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/mp/event.h>
#include <kernel/mp/thread.h>
#include <kernel/notification/notification.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "notification"



static omm_allocator_t* _notification_dispatcher_allocator=NULL;
static omm_allocator_t* _notification_consumer_allocator=NULL;
static omm_allocator_t* _notification_container_allocator=NULL;



KERNEL_EARLY_INIT(){
	_notification_dispatcher_allocator=omm_init("kernel.notification.dispatcher",sizeof(notification_dispatcher_t),8,1);
	rwlock_init(&(_notification_dispatcher_allocator->lock));
	_notification_consumer_allocator=omm_init("kernel.notification.consumer",sizeof(notification_consumer_t),8,2);
	rwlock_init(&(_notification_consumer_allocator->lock));
	_notification_container_allocator=omm_init("kernel.notification.container",sizeof(notification_container_t),8,8);
	rwlock_init(&(_notification_container_allocator->lock));
}



KERNEL_PUBLIC notification_dispatcher_t* notification_dispatcher_create(void){
	notification_dispatcher_t* out=omm_alloc(_notification_dispatcher_allocator);
	rwlock_init(&(out->lock));
	out->head=omm_alloc(_notification_container_allocator);
	out->head->next=NULL;
	out->head->rc=0;
	out->tail=out->head;
	out->consumer_head=NULL;
	return out;
}



KERNEL_PUBLIC void notification_dispatcher_delete(notification_dispatcher_t* dispatcher){
	panic("notification_dispatcher_delete");
}



KERNEL_PUBLIC void notification_dispatcher_dispatch(notification_dispatcher_t* dispatcher,u32 type,const void* data,u32 length){
	notification_container_t* container=omm_alloc(_notification_container_allocator);
	container->next=NULL;
	container->data.type=type;
	container->data.length=length;
	container->data.data=amm_alloc(length);
	mem_copy(container->data.data,data,length);
	container->rc=0;
	rwlock_acquire_write(&(dispatcher->lock));
	dispatcher->tail->next=container;
	dispatcher->tail=container;
	for (notification_consumer_t* consumer=dispatcher->consumer_head;consumer;consumer=consumer->next){
		event_dispatch(consumer->event,EVENT_DISPATCH_FLAG_DISPATCH_ALL|EVENT_DISPATCH_FLAG_SET_ACTIVE|EVENT_DISPATCH_FLAG_BYPASS_ACL);
	}
	rwlock_release_write(&(dispatcher->lock));
}



KERNEL_PUBLIC notification_consumer_t* notification_consumer_create(notification_dispatcher_t* dispatcher){
	notification_consumer_t* out=omm_alloc(_notification_consumer_allocator);
	rwlock_init(&(out->lock));
	out->dispatcher=dispatcher;
	out->prev=NULL;
	out->last=dispatcher->head;
	out->event=event_create("kernel.notification.consumer");
	dispatcher->head->rc++;
	rwlock_acquire_write(&(dispatcher->lock));
	out->next=dispatcher->consumer_head;
	if (dispatcher->consumer_head){
		dispatcher->consumer_head->prev=out;
	}
	dispatcher->consumer_head=out;
	rwlock_release_write(&(dispatcher->lock));
	return out;
}



KERNEL_PUBLIC void notification_consumer_delete(notification_consumer_t* consumer){
	panic("notification_consumer_delete");
}



KERNEL_PUBLIC bool notification_consumer_get(notification_consumer_t* consumer,bool wait,notification_t* out){
	rwlock_acquire_write(&(consumer->lock));
	while (!consumer->last->next){
		rwlock_release_write(&(consumer->lock));
		if (!wait){
			return 0;
		}
		event_await(consumer->event,0);
		rwlock_acquire_write(&(consumer->lock));
	}
	notification_container_t* last_container=consumer->last;
	consumer->last=last_container->next;
	consumer->last->rc++;
	*out=consumer->last->data;
	last_container->rc--;
	rwlock_release_write(&(consumer->lock));
	if (!consumer->last->next){
		rwlock_acquire_write(&(consumer->dispatcher->lock));
		if (!consumer->last->next){
			event_set_active(consumer->event,0,1);
		}
		rwlock_release_write(&(consumer->dispatcher->lock));
	}
	if (last_container->rc){
		return 1;
	}
	rwlock_acquire_write(&(consumer->dispatcher->lock));
	notification_container_t* head=consumer->dispatcher->head;
	while (head->next&&!head->next->rc){
		notification_container_t* next=head->next;
		head->next=next->next;
		amm_dealloc(next->data.data);
		omm_dealloc(_notification_container_allocator,next);
	}
	rwlock_release_write(&(consumer->dispatcher->lock));
	return 1;
}
