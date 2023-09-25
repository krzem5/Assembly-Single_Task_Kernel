#include <kernel/apic/lapic.h>
#include <kernel/cpu/cpu.h>
#include <kernel/cpu/local.h>
#include <kernel/fpu/fpu.h>
#include <kernel/isr/isr.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/msr/msr.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/mp/thread.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "scheduler"



#define THREAD_TIMESLICE_US 5000



static const scheduler_priority_t _scheduler_queue_priority_queue_access_pattern[SCHEDULER_ROUND_ROBIN_PRIORITY_COUNT][SCHEDULER_ROUND_ROBIN_PRIORITY_COUNT]={
	{SCHEDULER_PRIORITY_LOW,SCHEDULER_PRIORITY_HIGH,SCHEDULER_PRIORITY_NORMAL},
	{SCHEDULER_PRIORITY_NORMAL,SCHEDULER_PRIORITY_HIGH,SCHEDULER_PRIORITY_LOW},
	{SCHEDULER_PRIORITY_HIGH,SCHEDULER_PRIORITY_NORMAL,SCHEDULER_PRIORITY_LOW},
};

static _Bool KERNEL_CORE_DATA _scheduler_enabled=0;
static CPU_LOCAL_DATA(scheduler_t,_scheduler_data);
static scheduler_queues_t _scheduler_queues;



static void _queue_init(scheduler_queue_t* queue){
	lock_init(&(queue->lock));
	queue->head=NULL;
	queue->tail=NULL;
}



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
	for (u8 i=0;i<SCHEDULER_QUEUE_COUNT;i++){
		_queue_init(_scheduler_queues.data+i);
	}
}



void scheduler_enable(void){
	_scheduler_enabled=1;
}



void KERNEL_CORE_CODE scheduler_pause(void){
	if (_scheduler_enabled&&CPU_LOCAL(_scheduler_data)->current_thread){
		if (!CPU_LOCAL(_scheduler_data)->nested_pause_count){
			CPU_LOCAL(_scheduler_data)->remaining_us=lapic_timer_stop();
		}
		CPU_LOCAL(_scheduler_data)->nested_pause_count++;
	}
}



void KERNEL_CORE_CODE scheduler_resume(void){
	if (_scheduler_enabled&&CPU_LOCAL(_scheduler_data)->current_thread){
		CPU_LOCAL(_scheduler_data)->nested_pause_count--;
		if (!CPU_LOCAL(_scheduler_data)->nested_pause_count){
			lapic_timer_start(CPU_LOCAL(_scheduler_data)->remaining_us);
		}
	}
}



void scheduler_isr_handler(isr_state_t* state){
	lapic_timer_stop();
	scheduler_t* scheduler=CPU_LOCAL(_scheduler_data);
	scheduler->nested_pause_count=0;
	thread_t* current_thread=scheduler->current_thread;
	scheduler->current_thread=NULL;
	thread_t* new_thread=_try_pop_from_queue(_scheduler_queues.data+SCHEDULER_PRIORITY_REALTIME);
	if (!new_thread){
		scheduler_priority_t start_priority=SCHEDULER_PRIORITY_HIGH;
		if (!(scheduler->round_robin_timing&15)){
			start_priority=SCHEDULER_PRIORITY_LOW;
		}
		else if (!(scheduler->round_robin_timing&3)){
			start_priority=SCHEDULER_PRIORITY_NORMAL;
		}
		scheduler->round_robin_timing++;
		const scheduler_priority_t* pattern=_scheduler_queue_priority_queue_access_pattern[start_priority-SCHEDULER_PRIORITY_LOW];
		for (u8 i=0;!new_thread&&i<SCHEDULER_ROUND_ROBIN_PRIORITY_COUNT;i++){
			new_thread=_try_pop_from_queue(_scheduler_queues.data+pattern[i]);
		}
	}
	if (!new_thread){
		new_thread=_try_pop_from_queue(_scheduler_queues.data+SCHEDULER_PRIORITY_BACKGROUND);
	}
	if (current_thread&&(new_thread||current_thread->state.type!=THREAD_STATE_TYPE_RUNNING)){
		lock_acquire_exclusive(&(current_thread->lock));
		msr_set_gs_base(CPU_LOCAL(cpu_extra_data),0);
		CPU_LOCAL(cpu_extra_data)->tss.ist1=(u64)(CPU_LOCAL(cpu_extra_data)->pf_stack+(CPU_PAGE_FAULT_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT));
		current_thread->gpr_state=*state;
		current_thread->fs_gs_state.fs=(u64)msr_get_fs_base();
		current_thread->fs_gs_state.gs=(u64)msr_get_gs_base(1);
		fpu_save(current_thread->fpu_state);
		current_thread->state_not_present=0;
		lock_release_exclusive(&(current_thread->lock));
		if (current_thread->state.type==THREAD_STATE_TYPE_RUNNING){
			scheduler_enqueue_thread(current_thread);
		}
		current_thread=NULL;
	}
	if (new_thread){
		lock_acquire_exclusive(&(new_thread->lock));
		new_thread->header.index=CPU_HEADER_DATA->index;
		msr_set_gs_base(new_thread,0);
		scheduler->current_thread=new_thread;
		*state=new_thread->gpr_state;
		CPU_LOCAL(cpu_extra_data)->tss.ist1=new_thread->pf_stack_bottom+(CPU_PAGE_FAULT_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT);
		msr_set_fs_base((void*)(new_thread->fs_gs_state.fs));
		msr_set_gs_base((void*)(new_thread->fs_gs_state.gs),1);
		fpu_restore(new_thread->fpu_state);
		vmm_switch_to_pagemap(&(new_thread->process->pagemap));
		new_thread->state.type=THREAD_STATE_TYPE_RUNNING;
		lock_release_exclusive(&(new_thread->lock));
	}
	else{
		scheduler->current_thread=current_thread;
	}
	lapic_timer_start(THREAD_TIMESLICE_US);
	if (!scheduler->current_thread){
		scheduler_task_wait_loop();
	}
}



void scheduler_enqueue_thread(thread_t* thread){
	scheduler_pause();
	lock_acquire_exclusive(&(thread->lock));
	if (thread->state.type==THREAD_STATE_TYPE_QUEUED){
		panic("Thread already queued");
	}
	scheduler_priority_t priority=thread->priority;
	if (priority<SCHEDULER_PRIORITY_MIN||priority>SCHEDULER_PRIORITY_MAX){
		WARN("Unknown thread priority '%u'",priority);
		priority=SCHEDULER_PRIORITY_NORMAL;
	}
	scheduler_queue_t* queue=_scheduler_queues.data+priority;
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
	lock_release_exclusive(&(thread->lock));
	scheduler_resume();
}



void scheduler_dequeue_thread(_Bool save_registers){
	lapic_timer_stop();
	CPU_LOCAL(cpu_extra_data)->tss.ist1=(u64)(CPU_LOCAL(cpu_extra_data)->pf_stack+(CPU_PAGE_FAULT_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT));
	if (!save_registers){
		msr_set_gs_base(CPU_LOCAL(cpu_extra_data),0);
		CPU_LOCAL(_scheduler_data)->current_thread=NULL;
	}
	else{
		scheduler_start();
	}
}
