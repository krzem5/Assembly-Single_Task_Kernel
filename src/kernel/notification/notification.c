#include <kernel/lock/rwlock.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/notification/notification.h>
#include <kernel/types.h>



static omm_allocator_t* _notification_listener_allocator=NULL;



KERNEL_PUBLIC void notification_dispatcher_init(notification_dispatcher_t* dispatcher){
	rwlock_init(&(dispatcher->lock));
	dispatcher->head=NULL;
}



KERNEL_PUBLIC void notification_dispatcher_deinit(notification_dispatcher_t* dispatcher){
	rwlock_acquire_write(&(dispatcher->lock));
	while (dispatcher->head){
		notification_listener_t* listener=dispatcher->head;
		dispatcher->head=listener->next;
		omm_dealloc(_notification_listener_allocator,listener);
	}
	rwlock_release_write(&(dispatcher->lock));
}



KERNEL_PUBLIC void notification_dispatcher_add_listener(notification_dispatcher_t* dispatcher,notification_listener_callback_t callback){
	if (!_notification_listener_allocator){
		_notification_listener_allocator=omm_init("notification_listener",sizeof(notification_listener_t),8,4);
		rwlock_init(&(_notification_listener_allocator->lock));
	}
	notification_listener_t* listener=omm_alloc(_notification_listener_allocator);
	listener->callback=callback;
	listener->prev=NULL;
	rwlock_acquire_write(&(dispatcher->lock));
	listener->next=dispatcher->head;
	if (dispatcher->head){
		dispatcher->head->prev=listener;
	}
	dispatcher->head=listener;
	rwlock_release_write(&(dispatcher->lock));
}



KERNEL_PUBLIC void notification_dispatcher_remove_listener(notification_dispatcher_t* dispatcher,notification_listener_callback_t callback){
	rwlock_acquire_write(&(dispatcher->lock));
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
	rwlock_release_write(&(dispatcher->lock));
}



KERNEL_PUBLIC void notification_dispatcher_dispatch(notification_dispatcher_t* dispatcher,void* object,u32 type){
	rwlock_acquire_read(&(dispatcher->lock));
	for (const notification_listener_t* listener=dispatcher->head;listener;listener=listener->next){
		listener->callback(object,type);
	}
	rwlock_release_read(&(dispatcher->lock));
}
