#include <kernel/apic/lapic.h>
#include <kernel/cpu/cpu.h>
#include <kernel/cpu/local.h>
#include <kernel/fpu/fpu.h>
#include <kernel/isr/isr.h>
#include <kernel/lock/lock.h>
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



static _Bool KERNEL_CORE_DATA _scheduler_enabled=0;
static CPU_LOCAL_DATA(scheduler_t,_scheduler_data);



void scheduler_init(void){
	LOG("Initializing scheduler...");
	cpu_mask_init();
	scheduler_load_balancer_init();
}



void scheduler_enable(void){
	_scheduler_enabled=1;
}



void KERNEL_CORE_CODE scheduler_pause(void){
	if (_scheduler_enabled&&CPU_LOCAL(_scheduler_data)->current_thread){
		if (!CPU_LOCAL(_scheduler_data)->pause_nested_count){
			CPU_LOCAL(_scheduler_data)->pause_remaining_us=lapic_timer_stop();
			asm volatile("cli":::"memory");
		}
		CPU_LOCAL(_scheduler_data)->pause_nested_count++;
	}
}



void KERNEL_CORE_CODE scheduler_resume(void){
	if (_scheduler_enabled&&CPU_LOCAL(_scheduler_data)->current_thread){
		CPU_LOCAL(_scheduler_data)->pause_nested_count--;
		if (!CPU_LOCAL(_scheduler_data)->pause_nested_count){
			lapic_timer_start(CPU_LOCAL(_scheduler_data)->pause_remaining_us);
			asm volatile("sti":::"memory");
		}
	}
}



void scheduler_isr_handler(isr_state_t* state){
	lapic_timer_stop();
	scheduler_t* scheduler=CPU_LOCAL(_scheduler_data);
	scheduler->pause_nested_count=0;
	thread_t* current_thread=scheduler->current_thread;
	scheduler->current_thread=NULL;
	if (current_thread){
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
	}
	current_thread=scheduler_load_balancer_get();
	if (current_thread){
		lock_acquire_exclusive(&(current_thread->lock));
		current_thread->header.index=CPU_HEADER_DATA->index;
		msr_set_gs_base(current_thread,0);
		scheduler->current_thread=current_thread;
		*state=current_thread->gpr_state;
		CPU_LOCAL(cpu_extra_data)->tss.ist1=current_thread->pf_stack_bottom+(CPU_PAGE_FAULT_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT);
		msr_set_fs_base((void*)(current_thread->fs_gs_state.fs));
		msr_set_gs_base((void*)(current_thread->fs_gs_state.gs),1);
		fpu_restore(current_thread->fpu_state);
		vmm_switch_to_pagemap(&(current_thread->process->pagemap));
		current_thread->state.type=THREAD_STATE_TYPE_RUNNING;
		lock_release_exclusive(&(current_thread->lock));
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
	scheduler_load_balancer_add(thread);
	thread->state.type=THREAD_STATE_TYPE_QUEUED;
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
