#include <kernel/isr/isr.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/signal/signal.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "signal"



static pmm_counter_descriptor_t _signal_state_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_signal_state");
static omm_allocator_t _signal_state_allocator=OMM_ALLOCATOR_INIT_STRUCT("signal_state",sizeof(signal_state_t),8,2,&_signal_state_omm_pmm_counter);



void signal_send(thread_t* thread,isr_state_t* isr_state,signal_type_t type,u64 arg){
	scheduler_pause();
	spinlock_acquire_exclusive(&(thread->lock));
	if (thread->signal_state){
		return;
	}
	if (!isr_state){
		isr_state=&(thread->gpr_state);
	}
	if (isr_state->cs!=0x23){
		panic("signal_send: defer signal");
	}
	thread->signal_state=omm_alloc(&_signal_state_allocator);
	thread->signal_state->type=type;
	thread->signal_state->arg=arg;
	thread->signal_state->old_gpr_state=*isr_state;
	panic("signal_send");
	spinlock_release_exclusive(&(thread->lock));
	scheduler_resume();
}
