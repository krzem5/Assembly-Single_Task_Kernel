#include <kernel/lock/spinlock.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/notification/notification.h>
#include <kernel/types.h>



static omm_allocator_t* _notification_listener_allocator=NULL;



KERNEL_PUBLIC void notification_dispatcher_init(notification_dispatcher_t* dispatcher){
	spinlock_init(&(dispatcher->lock));
	dispatcher->head=NULL;
}



KERNEL_PUBLIC void notification_dispatcher_deinit(notification_dispatcher_t* dispatcher){
	spinlock_acquire_exclusive(&(dispatcher->lock));
	while (dispatcher->head){
		notification_listener_t* listener=dispatcher->head;
		dispatcher->head=listener->next;
		omm_dealloc(_notification_listener_allocator,listener);
	}
	spinlock_release_exclusive(&(dispatcher->lock));
}



KERNEL_PUBLIC void notification_dispatcher_add_listener(notification_dispatcher_t* dispatcher,notification_listener_callback_t callback){
	if (!_notification_listener_allocator){
		_notification_listener_allocator=omm_init("notification_listener",sizeof(notification_listener_t),8,4);
		spinlock_init(&(_notification_listener_allocator->lock));
	}
	notification_listener_t* listener=omm_alloc(_notification_listener_allocator);
	listener->callback=callback;
	listener->prev=NULL;
	spinlock_acquire_exclusive(&(dispatcher->lock));
	listener->next=dispatcher->head;
	if (dispatcher->head){
		dispatcher->head->prev=listener;
	}
	dispatcher->head=listener;
	spinlock_release_exclusive(&(dispatcher->lock));
}



KERNEL_PUBLIC void notification_dispatcher_remove_listener(notification_dispatcher_t* dispatcher,notification_listener_callback_t callback){
	spinlock_acquire_exclusive(&(dispatcher->lock));
	for (notification_listener_t* listener=dispatcher->head;listener;listener=listener->next){
		if (listener->callback!=callback){
			continue;
		}
		if (listener->prev){
			listener->prev->next=listener->next;
		}
		else{
			dispatcher->head=listener->next;
		}
		if (listener->next){
			listener->next->prev=listener->prev;
		}
		omm_dealloc(_notification_listener_allocator,listener);
		break;
	}
	spinlock_release_exclusive(&(dispatcher->lock));
}



KERNEL_PUBLIC void notification_dispatcher_dispatch(notification_dispatcher_t* dispatcher,void* object,u32 type){
	spinlock_acquire_shared(&(dispatcher->lock));
	for (const notification_listener_t* listener=dispatcher->head;listener;listener=listener->next){
		listener->callback(object,type);
	}
	spinlock_release_shared(&(dispatcher->lock));
}
