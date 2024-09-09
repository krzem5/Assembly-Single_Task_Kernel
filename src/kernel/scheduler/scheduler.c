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
#include <kernel/mp/process.h>
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
#define SCHEDULER_IRQ_THREAD_START_TIME_QUANTUM_US 1000 // slightly higher to decrease responce time of file-backed memory



static CPU_LOCAL_DATA(scheduler_t,_scheduler_data);

bool KERNEL_INIT_WRITE scheduler_enabled=0;



static void _switch_thread(isr_state_t* state,thread_t* new_thread){
	lock_profiling_assert_empty(NULL);
	scheduler_set_timer(SCHEDULER_TIMER_SCHEDULER);
	scheduler_t* scheduler=CPU_LOCAL(_scheduler_data);
	scheduler->is_irq_context=1;
	scheduler->pause_nested_count=0;
	thread_t* current_thread=scheduler->current_thread;
	scheduler->current_thread=NULL;
	if (current_thread){
		msr_set_gs_base((u64)CPU_LOCAL(cpu_extra_data),0);
		rwlock_acquire_write(&(current_thread->lock));
		if (current_thread->scheduler_kill_thread){
			panic("Kill thread");
		}
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
	u32 time_us=SCHEDULER_IRQ_THREAD_START_TIME_QUANTUM_US;
	current_thread=(new_thread?new_thread:scheduler_load_balancer_get(&time_us));
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
	lock_profiling_assert_empty(NULL);
	scheduler->is_irq_context=0;
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



KERNEL_EARLY_INIT(){
	for (u16 i=0;i<cpu_count;i++){
		(_scheduler_data+i)->current_timer_start=clock_get_ticks();
		(_scheduler_data+i)->current_timer=SCHEDULER_TIMER_NONE;
	}
}



void KERNEL_EARLY_EXEC scheduler_enable(void){
	LOG("Enabling scheduler...");
	scheduler_enabled=1;
}



KERNEL_PUBLIC void scheduler_pause(void){
	if (!scheduler_enabled){
		return;
	}
	scheduler_t* scheduler=CPU_LOCAL(_scheduler_data);
	if (!scheduler->current_thread||scheduler->is_irq_context){
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



KERNEL_PUBLIC void scheduler_resume(bool yield_if_possible){
	if (!scheduler_enabled){
		return;
	}
	scheduler_t* scheduler=CPU_LOCAL(_scheduler_data);
	if (!scheduler->current_thread||scheduler->is_irq_context){
		return;
	}
	scheduler->pause_nested_count--;
	if (scheduler->pause_nested_count||!scheduler->pause_remaining_us){
		return;
	}
	u64 elapsed_us=(clock_ticks_to_time(clock_get_ticks()-scheduler->pause_start_ticks)+500)/1000;
	if (yield_if_possible){
		asm volatile("sti":::"memory");
	}
	if (elapsed_us>=scheduler->pause_remaining_us){
		if (yield_if_possible){
			scheduler_yield();
			return;
		}
		scheduler->pause_remaining_us=elapsed_us+SCHEDULER_MIN_TIME_QUANTUM_US;
	}
	lapic_timer_start(scheduler->pause_remaining_us-elapsed_us);
}



void scheduler_set_irq_context(bool is_irq_context){
	if (!scheduler_enabled){
		return;
	}
	CPU_LOCAL(_scheduler_data)->is_irq_context=is_irq_context;
}



void scheduler_isr_handler(isr_state_t* state){
	lapic_timer_stop();
	scheduler_t* scheduler=CPU_LOCAL(_scheduler_data);
	if (scheduler->is_irq_context){
		WARN("Thread: %s",scheduler->current_thread->name->data);
		panic("Scheduler called in an irq context");
	}
	_switch_thread(state,NULL);
}



void scheduler_irq_return_after_thread(isr_state_t* state,thread_t* thread){
	lapic_timer_stop();
	scheduler_t* scheduler=CPU_LOCAL(_scheduler_data);
	if (!scheduler->is_irq_context){
		panic("scheduler_irq_return_after_thread: called outside an irq context");
	}
	if (thread->state!=THREAD_STATE_TYPE_NONE){
		panic("scheduler_irq_return_after_thread: invalid thread state");
	}
	if (!scheduler->current_thread){
		panic("scheduler_irq_return_after_thread: no current thread");
	}
	event_await_thread_irq(scheduler->current_thread,thread->termination_event);
	_switch_thread(state,thread);
}



KERNEL_PUBLIC void scheduler_enqueue_thread(thread_t* thread){
	rwlock_acquire_write(&(thread->lock));
	if (thread->state==THREAD_STATE_TYPE_QUEUED){
		rwlock_release_write(&(thread->lock));
		return;
	}
	scheduler_load_balancer_add(thread);
	thread->state=THREAD_STATE_TYPE_QUEUED;
	rwlock_release_write(&(thread->lock));
}



void scheduler_dequeue_thread_locked(thread_t* thread){
	if (thread->state!=THREAD_STATE_TYPE_QUEUED){
		return;
	}
	scheduler_load_balancer_remove(thread);
	thread->state=THREAD_STATE_TYPE_NONE;
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
	scheduler_resume(1);
}



error_t syscall_scheduler_yield(void){
	THREAD_DATA->scheduler_early_yield=1;
	scheduler_yield();
	return ERROR_OK;
}
