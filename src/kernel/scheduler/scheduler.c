#include <kernel/cpu/cpu.h>
#include <kernel/isr/isr.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/thread/thread.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "scheduler"



static scheduler_queues_t _scheduler_queues;



static thread_t* _try_pop_from_queue(scheduler_queue_t* queue){
	if (!queue->head){
		return NULL;
	}
	lock_acquire_exclusive(&(queue->lock));
	if (!queue->head){
		lock_release_exclusive(&(queue->lock));
		return NULL;
	}
	thread_t* out=queue->head;
	queue->head=out->scheduler_queue_next;
	if (queue->tail==out){
		queue->tail=NULL;
	}
	lock_release_exclusive(&(queue->lock));
	return out;
}



void scheduler_init(void){
	LOG("Initializing scheduler...");
	lock_init(&(_scheduler_queues.background_queue.lock));
	for (u8 i=0;i<SCHEDULER_PRIORITY_QUEUE_COUNT;i++){
		lock_init(&((_scheduler_queues.priority_queues+i)->lock));
	}
	lock_init(&(_scheduler_queues.realtime_queue.lock));
}



scheduler_t* scheduler_new(void){
	scheduler_t* out=kmm_alloc(sizeof(scheduler_t));
	out->current_thread=NULL;
	out->priority_timing=0;
	return out;
}



void scheduler_isr_handler(isr_state_t* state){
	scheduler_t* scheduler=CPU_DATA->scheduler;
	if (scheduler->current_thread){
		scheduler->current_thread->state=*state;
		scheduler->current_thread=NULL;
	}
	thread_t* new_thread=_try_pop_from_queue(&(_scheduler_queues.realtime_queue));
	if (!new_thread){
		u8 priority=2;
		if (!(scheduler->priority_timing&15)){
			priority=0;
		}
		else if (!(scheduler->priority_timing&3)){
			priority=1;
		}
		u8 i=priority;
		do{
			new_thread=_try_pop_from_queue(_scheduler_queues.priority_queues+i);
			i=(i?i-1:SCHEDULER_PRIORITY_QUEUE_COUNT-1);
		} while (!new_thread&&i!=priority);
		scheduler->priority_timing++;
	}
	if (!new_thread){
		new_thread=_try_pop_from_queue(&(_scheduler_queues.background_queue));
	}
	if (!new_thread){
		ERROR("scheduler_isr_handler: no thread");
		return;
	}
	scheduler->current_thread=new_thread;
	ERROR("scheduler_isr_handler: new thread");
	for (;;);
}
