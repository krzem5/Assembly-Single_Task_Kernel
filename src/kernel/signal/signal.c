#include <kernel/error/error.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/mp/event.h>
#include <kernel/mp/thread.h>
#include <kernel/signal/signal.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "signal"



void signal_process_state_init(signal_process_state_t* state){
	rwlock_init(&(state->lock));
	state->mask=0;
	state->pending=0;
}



void signal_thread_state_init(signal_thread_state_t* state){
	rwlock_init(&(state->lock));
	state->mask=0;
	state->pending=0;
	state->event=event_create("kernel.signal");
}



void signal_thread_state_deinit(signal_thread_state_t* state){
	event_delete(state->event);
	state->event=NULL;
}



error_t syscall_signal_get_event(void){
	return THREAD_DATA->signal_state.event->handle.rb_node.key;
}



error_t syscall_signal_get_signal(void){
	rwlock_acquire_write(&(THREAD_DATA->header.current_thread->signal_state.lock));
	error_t out=ERROR_NO_DATA;
	if (THREAD_DATA->signal_state.pending){
		out=__builtin_ffsll(THREAD_DATA->signal_state.pending)-1;
		THREAD_DATA->signal_state.pending&=THREAD_DATA->signal_state.pending-1;
	}
	rwlock_release_write(&(THREAD_DATA->header.current_thread->signal_state.lock));
	return out;
}



error_t syscall_signal_get_pending_signals(u32 is_process){
	error_t out=0;
	if (is_process){
		rwlock_acquire_read(&(THREAD_DATA->process->signal_state.lock));
		out=THREAD_DATA->process->signal_state.pending;
		rwlock_release_read(&(THREAD_DATA->process->signal_state.lock));
	}
	else{
		rwlock_acquire_read(&(THREAD_DATA->header.current_thread->signal_state.lock));
		out=THREAD_DATA->signal_state.pending;
		rwlock_release_read(&(THREAD_DATA->header.current_thread->signal_state.lock));
	}
	return out;
}



error_t syscall_signal_get_mask(u32 is_process_mask){
	return (is_process_mask?THREAD_DATA->process->signal_state.mask:THREAD_DATA->signal_state.mask);
}



error_t syscall_signal_set_mask(u64 mask,u32 is_process_mask){
	if (is_process_mask){
		rwlock_acquire_write(&(THREAD_DATA->process->signal_state.lock));
		THREAD_DATA->process->signal_state.mask=mask;
		rwlock_release_write(&(THREAD_DATA->process->signal_state.lock));
	}
	else{
		rwlock_acquire_write(&(THREAD_DATA->header.current_thread->signal_state.lock));
		THREAD_DATA->signal_state.mask=mask;
		rwlock_release_write(&(THREAD_DATA->header.current_thread->signal_state.lock));
	}
	return ERROR_OK;
}
