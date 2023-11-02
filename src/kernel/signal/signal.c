#include <kernel/isr/isr.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/signal/signal.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "signal"



void signal_send(thread_t* thread,isr_state_t* isr_state,signal_type_t type,const void* data,u32 size){
	scheduler_pause();
	spinlock_acquire_exclusive(&(thread->lock));
	if (!isr_state){
		isr_state=&(thread->gpr_state);
	}
	panic("signal_send");
	spinlock_release_exclusive(&(thread->lock));
	scheduler_resume();
}
