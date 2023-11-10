#include <kernel/apic/lapic.h>
#include <kernel/clock/clock.h>
#include <kernel/cpu/cpu.h>
#include <kernel/cpu/local.h>
#include <kernel/fpu/fpu.h>
#include <kernel/isr/isr.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/msr/msr.h>
#include <kernel/scheduler/cpu_mask.h>
#include <kernel/scheduler/load_balancer.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/mp/thread.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "scheduler"



#define THREAD_TIMESLICE_US 5000



static _Bool KERNEL_INIT_WRITE _scheduler_enabled=0;
static KERNEL_INIT_WRITE CPU_LOCAL_DATA(scheduler_t,_scheduler_data);



void scheduler_init(void){
	LOG("Initializing scheduler...");
	cpu_mask_init();
	scheduler_load_balancer_init();
	for (u16 i=0;i<cpu_count;i++){
		(_scheduler_data+i)->current_timer_start=clock_get_ticks();
		(_scheduler_data+i)->current_timer=SCHEDULER_TIMER_NONE;
	}
}



void scheduler_enable(void){
	LOG("Enabling scheduler...");
	_scheduler_enabled=1;
}



void scheduler_pause(void){
	if (!_scheduler_enabled){
		return;
	}
	scheduler_t* scheduler=CPU_LOCAL(_scheduler_data);
	if (!scheduler->current_thread){
		return;
	}
	scheduler->pause_nested_count++;
	if (scheduler->pause_nested_count>1){
		return;
	}
	scheduler->pause_remaining_us=lapic_timer_stop();
	if (scheduler->pause_remaining_us){
		asm volatile("cli":::"memory");
	}
	scheduler->pause_start_ticks=clock_get_ticks();
}



void scheduler_resume(void){
	if (!_scheduler_enabled){
		return;
	}
	scheduler_t* scheduler=CPU_LOCAL(_scheduler_data);
	if (!scheduler->current_thread){
		return;
	}
	scheduler->pause_nested_count--;
	if (scheduler->pause_nested_count||!scheduler->pause_remaining_us){
		return;
	}
	u64 elapsed_us=(clock_ticks_to_time(clock_get_ticks()-scheduler->pause_start_ticks)+500)/1000;
	asm volatile("sti":::"memory");
	if (elapsed_us>=scheduler->pause_remaining_us){
		scheduler_yield();
	}
	else{
		lapic_timer_start(scheduler->pause_remaining_us-elapsed_us);
	}
}



void scheduler_isr_handler(isr_state_t* state){
	lapic_timer_stop();
	scheduler_set_timer(SCHEDULER_TIMER_SCHEDULER);
	scheduler_t* scheduler=CPU_LOCAL(_scheduler_data);
	scheduler->pause_nested_count=0;
	thread_t* current_thread=scheduler->current_thread;
	scheduler->current_thread=NULL;
	if (current_thread){
		spinlock_acquire_exclusive(&(current_thread->lock));
		msr_set_gs_base((u64)CPU_LOCAL(cpu_extra_data),0);
		CPU_LOCAL(cpu_extra_data)->tss.ist1=(u64)(CPU_LOCAL(cpu_extra_data)->pf_stack+(CPU_PAGE_FAULT_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT));
		if (current_thread->state.type==THREAD_STATE_TYPE_TERMINATED){
			vmm_switch_to_pagemap(&vmm_kernel_pagemap);
			spinlock_release_exclusive(&(current_thread->lock));
			thread_delete(current_thread);
		}
		else{
			current_thread->gpr_state=*state;
			fpu_save(current_thread->fpu_state);
			current_thread->state_not_present=0;
			spinlock_release_exclusive(&(current_thread->lock));
			if (current_thread->state.type==THREAD_STATE_TYPE_RUNNING){
				scheduler_enqueue_thread(current_thread);
			}
		}
	}
	current_thread=scheduler_load_balancer_get();
	if (current_thread){
		spinlock_acquire_exclusive(&(current_thread->lock));
		current_thread->header.index=CPU_HEADER_DATA->index;
		msr_set_gs_base((u64)current_thread,0);
		scheduler->current_thread=current_thread;
		*state=current_thread->gpr_state;
		CPU_LOCAL(cpu_extra_data)->tss.ist1=current_thread->pf_stack_bottom+(CPU_PAGE_FAULT_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT);
		msr_set_fs_base(current_thread->fs_gs_state.fs);
		msr_set_gs_base(current_thread->fs_gs_state.gs,1);
		fpu_restore(current_thread->fpu_state);
		vmm_switch_to_pagemap(&(current_thread->process->pagemap));
		current_thread->state.type=THREAD_STATE_TYPE_RUNNING;
		spinlock_release_exclusive(&(current_thread->lock));
	}
	else{
		vmm_switch_to_pagemap(&vmm_kernel_pagemap);
	}
	lapic_timer_start(THREAD_TIMESLICE_US);
	if (!scheduler->current_thread){
		scheduler_set_timer(SCHEDULER_TIMER_NONE);
		scheduler_task_wait_loop();
	}
	scheduler_set_timer((state->cs==0x08?SCHEDULER_TIMER_KERNEL:SCHEDULER_TIMER_USER));
}



void scheduler_enqueue_thread(thread_t* thread){
	scheduler_pause();
	spinlock_acquire_exclusive(&(thread->lock));
	if (thread->state.type==THREAD_STATE_TYPE_QUEUED){
		panic("Thread already queued");
	}
	scheduler_load_balancer_add(thread);
	thread->state.type=THREAD_STATE_TYPE_QUEUED;
	spinlock_release_exclusive(&(thread->lock));
	scheduler_resume();
}



const scheduler_timers_t* scheduler_get_timers(u16 cpu_index){
	if (cpu_index>=cpu_count){
		return NULL;
	}
	return &((_scheduler_data+cpu_index)->timers);
}



void scheduler_set_timer(u8 timer){
	scheduler_pause();
	scheduler_t* scheduler=CPU_LOCAL(_scheduler_data);
	u64 ticks=clock_get_ticks();
	scheduler->timers.data[scheduler->current_timer]+=ticks-scheduler->current_timer_start;
	scheduler->current_timer_start=ticks;
	scheduler->current_timer=timer;
	scheduler_resume();
}
