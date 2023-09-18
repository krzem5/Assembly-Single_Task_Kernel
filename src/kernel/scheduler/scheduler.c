#include <kernel/apic/lapic.h>
#include <kernel/cpu/cpu.h>
#include <kernel/fpu/fpu.h>
#include <kernel/isr/isr.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/msr/msr.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/thread/thread.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "scheduler"



#define THREAD_TIMESLICE_US 5000



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



void scheduler_pause(void){
	lapic_timer_stop();
}



scheduler_t* scheduler_new(void){
	scheduler_t* out=kmm_alloc(sizeof(scheduler_t));
	out->current_thread=NULL;
	out->priority_timing=0;
	return out;
}



void scheduler_isr_handler(isr_state_t* state){
	lapic_timer_stop();
	scheduler_t* scheduler=CPU_HEADER_DATA->cpu_data->scheduler;
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
	if (scheduler->current_thread&&(new_thread||scheduler->current_thread->state.type!=THREAD_STATE_TYPE_EXECUTING)){
		msr_set_gs_base(CPU_HEADER_DATA->cpu_data,0);
		scheduler->current_thread->gpr_state=*state;
		scheduler->current_thread->fs_gs_state.fs=(u64)msr_get_fs_base();
		scheduler->current_thread->fs_gs_state.gs=(u64)msr_get_gs_base(1);
		fpu_save(scheduler->current_thread->fpu_state);
		if (scheduler->current_thread->state.type==THREAD_STATE_TYPE_EXECUTING){
			scheduler_enqueue_thread(scheduler->current_thread);
		}
		scheduler->current_thread=NULL;
	}
	if (new_thread){
		new_thread->header.index=CPU_HEADER_DATA->index;
		new_thread->header.cpu_data=CPU_HEADER_DATA->cpu_data;
		msr_set_gs_base(new_thread,0);
		scheduler->current_thread=new_thread;
		*state=new_thread->gpr_state;
		CPU_HEADER_DATA->cpu_data->tss.ist1=new_thread->pf_stack;
		msr_set_fs_base((void*)(new_thread->fs_gs_state.fs));
		msr_set_gs_base((void*)(new_thread->fs_gs_state.gs),1);
		fpu_restore(new_thread->fpu_state);
		vmm_switch_to_pagemap(&(new_thread->process->pagemap));
		lock_acquire_exclusive(&(new_thread->state.lock));
		new_thread->state.type=THREAD_STATE_TYPE_EXECUTING;
		lock_release_exclusive(&(new_thread->state.lock));
	}
	lapic_timer_start(THREAD_TIMESLICE_US);
	if (!scheduler->current_thread){
		scheduler_task_wait_loop();
	}
}



void scheduler_enqueue_thread(thread_t* thread){
	lock_acquire_exclusive(&(thread->state.lock));
	if (thread->state.type==THREAD_STATE_TYPE_EXECUTING){
		panic("Thread already queued",0);
	}
	u32 remaining_us=lapic_timer_stop();
	scheduler_queue_t* queue=NULL;
	switch (thread->priority){
		case THREAD_PRIORITY_BACKGROUND:
			queue=&(_scheduler_queues.background_queue);
			break;
		case THREAD_PRIORITY_LOW:
			queue=_scheduler_queues.priority_queues;
			break;
		default:
			WARN("Unknown thread priority '%u'",thread->priority);
		case THREAD_PRIORITY_NORMAL:
			queue=_scheduler_queues.priority_queues+1;
			break;
		case THREAD_PRIORITY_HIGH:
			queue=_scheduler_queues.priority_queues+2;
			break;
		case THREAD_PRIORITY_REALTIME:
			queue=&(_scheduler_queues.realtime_queue);
			break;
	}
	lock_acquire_exclusive(&(queue->lock));
	if (queue->tail){
		queue->tail->scheduler_queue_next=thread;
	}
	else{
		queue->head=thread;
	}
	queue->tail=thread;
	thread->scheduler_queue_next=NULL;
	thread->state.type=THREAD_STATE_TYPE_QUEUED;
	lock_release_exclusive(&(queue->lock));
	lock_release_exclusive(&(thread->state.lock));
	lapic_timer_start(remaining_us);
}



void scheduler_dequeue_thread(_Bool save_registers){
	lapic_timer_stop();
	if (!save_registers){
		msr_set_gs_base(CPU_HEADER_DATA->cpu_data,0);
		CPU_HEADER_DATA->cpu_data->scheduler->current_thread=NULL;
	}
	scheduler_start();
}
