#include <kernel/apic/lapic.h>
#include <kernel/clock/clock.h>
#include <kernel/cpu/cpu.h>
#include <kernel/cpu/local.h>
#include <kernel/error/error.h>
#include <kernel/fpu/fpu.h>
#include <kernel/isr/isr.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/thread.h>
#include <kernel/msr/msr.h>
#include <kernel/scheduler/load_balancer.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/symbol/symbol.h>
#include <kernel/timer/timer.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "scheduler"



#define SCHEDULER_MIN_TIME_QUANTUM_US 5
#define SCHEDULER_PREEMPTION_DISABLED_QUANTUM_US 250



static bool KERNEL_INIT_WRITE _scheduler_enabled=0;
static CPU_LOCAL_DATA(scheduler_t,_scheduler_data);
static KERNEL_EARLY_WRITE u32 _scheduler_early_preemption_diabled_flag=0;

CPU_LOCAL_DATA(u32,_scheduler_preemption_disabled);
u32* _scheduler_preemption_disabled=&_scheduler_early_preemption_diabled_flag;



KERNEL_EARLY_INIT(){
	LOG("Initializing scheduler...");
	for (u16 i=0;i<cpu_count;i++){
		(_scheduler_data+i)->current_timer_start=clock_get_ticks();
		(_scheduler_data+i)->current_timer=SCHEDULER_TIMER_NONE;
	}
}



void _scheduler_ensure_no_locks(void){
	if (!(*CPU_LOCAL(_scheduler_preemption_disabled))){
		return;
	}
	return;
	asm volatile("cli":::"memory");
	scheduler_t* scheduler=CPU_LOCAL(_scheduler_data);
	if (!scheduler->current_thread){
		panic("Locks held while returning to usermode from scheduler");
	}
	WARN("%s: %u lock%s held",scheduler->current_thread->name->data,*CPU_LOCAL(_scheduler_preemption_disabled),(*CPU_LOCAL(_scheduler_preemption_disabled)==1?"":"s"));
	u64 rip=(u64)_scheduler_ensure_no_locks;
	u64 rbp=(u64)__builtin_frame_address(0);
	while (1){
		const symbol_t* symbol=symbol_lookup(rip);
		if (symbol){
			LOG("[%u] %s:%s+%u",CPU_HEADER_DATA->index,symbol->module,symbol->name->data,rip-symbol->rb_node.key);
		}
		else{
			LOG("[%u] %p",CPU_HEADER_DATA->index,rip);
		}
		if (!rbp){
			break;
		}
		if (!vmm_virtual_to_physical(&vmm_kernel_pagemap,rbp)||!vmm_virtual_to_physical(&vmm_kernel_pagemap,rbp+8)){
			LOG("[%u] <rbp: %p>",CPU_HEADER_DATA->index,rbp);
			break;
		}
		rip=*((u64*)(rbp+8));
		rbp=*((u64*)rbp);
	}
	panic("Locks held while returning to usermode from kernel thread");
}



void KERNEL_EARLY_EXEC scheduler_enable(void){
	LOG("Enabling scheduler...");
	_scheduler_enabled=1;
}



KERNEL_PUBLIC void scheduler_pause(void){
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



KERNEL_PUBLIC void scheduler_resume(void){
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
	if (*CPU_LOCAL(_scheduler_preemption_disabled)){
		// lapic_timer_start(SCHEDULER_PREEMPTION_DISABLED_QUANTUM_US);
		// return;
	}
	scheduler->pause_nested_count=0;
	thread_t* current_thread=scheduler->current_thread;
	scheduler->current_thread=NULL;
	if (current_thread){
		msr_set_gs_base((u64)CPU_LOCAL(cpu_extra_data),0);
		rwlock_acquire_write(&(current_thread->lock));
		CPU_LOCAL(cpu_extra_data)->tss.ist1=(u64)(CPU_LOCAL(cpu_extra_data)->pf_stack+(CPU_PAGE_FAULT_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT));
		if (current_thread->state==THREAD_STATE_TYPE_TERMINATED){
			vmm_switch_to_pagemap(&vmm_kernel_pagemap);
			rwlock_release_write(&(current_thread->lock));
			thread_delete(current_thread);
		}
		else{
			current_thread->reg_state.gpr_state=*state;
			fpu_save(current_thread->reg_state.fpu_state);
			current_thread->reg_state.reg_state_not_present=0;
			rwlock_release_write(&(current_thread->lock));
			if (current_thread->state==THREAD_STATE_TYPE_RUNNING){
				scheduler_enqueue_thread(current_thread);
			}
		}
	}
	u32 next_timer_time_us=timer_dispatch_timers();
	u32 time_us;
	current_thread=scheduler_load_balancer_get(&time_us);
	if (time_us>next_timer_time_us){
		time_us=next_timer_time_us;
		if (current_thread){
			current_thread->scheduler_early_yield=1;
		}
	}
	if (time_us<SCHEDULER_MIN_TIME_QUANTUM_US){
		time_us=SCHEDULER_MIN_TIME_QUANTUM_US;
	}
	if (current_thread){
		rwlock_acquire_write(&(current_thread->lock));
		current_thread->header.index=CPU_HEADER_DATA->index;
		*state=current_thread->reg_state.gpr_state;
		CPU_LOCAL(cpu_extra_data)->tss.ist1=current_thread->pf_stack_region->rb_node.key+(CPU_PAGE_FAULT_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT);
		msr_set_fs_base(current_thread->reg_state.fs_gs_state.fs);
		msr_set_gs_base(current_thread->reg_state.fs_gs_state.gs,1);
		fpu_restore(current_thread->reg_state.fpu_state);
		vmm_switch_to_pagemap(&(current_thread->process->pagemap));
		current_thread->state=THREAD_STATE_TYPE_RUNNING;
		rwlock_release_write(&(current_thread->lock));
	}
	else{
		vmm_switch_to_pagemap(&vmm_kernel_pagemap);
	}
	if (!current_thread){
		scheduler_set_timer(SCHEDULER_TIMER_NONE);
		lapic_timer_start(time_us);
		scheduler_task_wait_loop();
	}
	msr_set_gs_base((u64)current_thread,0);
	scheduler->current_thread=current_thread;
	scheduler_set_timer((state->cs==0x08?SCHEDULER_TIMER_KERNEL:SCHEDULER_TIMER_USER));
	lapic_timer_start(time_us);
}



KERNEL_PUBLIC void scheduler_enqueue_thread(thread_t* thread){
	scheduler_pause();
	rwlock_acquire_write(&(thread->lock));
	if (thread->state==THREAD_STATE_TYPE_QUEUED){
		panic("Thread already queued");
	}
	scheduler_load_balancer_add(thread);
	thread->state=THREAD_STATE_TYPE_QUEUED;
	rwlock_release_write(&(thread->lock));
	scheduler_resume();
}



KERNEL_PUBLIC const scheduler_timers_t* scheduler_get_timers(u16 cpu_index){
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



error_t syscall_scheduler_yield(void){
	THREAD_DATA->scheduler_early_yield=1;
	scheduler_yield();
	return ERROR_OK;
}
