#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/mp/event.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "event"



static u64 _thread_next_handle_id=1;



event_t* event_new(void){
	event_t* out=kmm_alloc(sizeof(event_t));
	out->handle.id=_thread_next_handle_id;
	_thread_next_handle_id++;
	lock_init(&(out->lock));
	out->head=NULL;
	out->tail=NULL;
	return out;
}



void event_delete(event_t* event){
	ERROR("Unimplemented: event_delete");
}



void event_dispatch(event_t* event,_Bool dispatch_all){
	lock_acquire_exclusive(&(event->lock));
	while (event->head){
		thread_t* thread=event->head;
		event->head=thread->state.event.next;
		lock_acquire_exclusive(&(thread->state.lock));
		thread->state.type=THREAD_STATE_TYPE_NONE;
		thread->state.event.event=NULL;
		thread->state.event.next=NULL;
		lock_release_exclusive(&(thread->state.lock));
		SPINLOOP(thread->state_not_present);
		scheduler_enqueue_thread(thread);
		if (!dispatch_all){
			break;
		}
	}
	if (!event->head){
		event->tail=NULL;
	}
	lock_release_exclusive(&(event->lock));
}
