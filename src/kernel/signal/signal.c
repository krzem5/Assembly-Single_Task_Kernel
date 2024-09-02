#include <kernel/error/error.h>
#include <kernel/exception/exception.h>
#include <kernel/handle/handle.h>
#include <kernel/isr/isr.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/mp/process_group.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/signal/signal.h>
#include <kernel/types.h>
#include <kernel/util/spinloop.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "signal"



static error_t _get_error(const signal_thread_state_t* state){
	return (state->pending?ERROR_SIGNAL(__builtin_ffsll(state->pending)-1):ERROR_INTERRUPTED);
}



static void _thread_wakeup_if_waiting(thread_t* thread){
	rwlock_acquire_write(&(thread->lock));
	if (thread->state!=THREAD_STATE_TYPE_AWAITING_EVENT){
		rwlock_release_write(&(thread->lock));
		return;
	}
	thread->state=THREAD_STATE_TYPE_NONE;
	thread->event_sequence_id++;
	thread->event_wakeup_index=0;
	rwlock_release_write(&(thread->lock));
	SPINLOOP(thread->reg_state.reg_state_not_present);
	thread->reg_state.gpr_state.rip=(u64)_exception_signal_interrupt_handler;
	thread->reg_state.gpr_state.rdi=_get_error(&(thread->signal_state));
	scheduler_enqueue_thread(thread);
}



static bool _dispatch_signal_to_thread(thread_t* thread,signal_t signal){
	if (thread->process==process_kernel){
		return 0;
	}
	if (signal==SIGNAL_KILL){
		// THREAD_STATE_TYPE_NONE, THREAD_STATE_TYPE_AWAITING_EVENT: <static interrupt (cs==0x08, fast path)>
		// THREAD_STATE_TYPE_QUEUED: <static interrupt + unschedule>
		// THREAD_STATE_TYPE_RUNNING: <live interrupt>
		// THREAD_STATE_TYPE_TERMINATED: <ignore>
		return 1;
	}
	rwlock_acquire_write(&(thread->signal_state.lock));
	if (thread->signal_state.mask&(1<<signal)){
		rwlock_release_write(&(thread->signal_state.lock));
		return 0;
	}
	thread->signal_state.pending|=1<<signal;
	_thread_wakeup_if_waiting(thread);
	rwlock_release_write(&(thread->signal_state.lock));
	event_dispatch(thread->signal_state.event,EVENT_DISPATCH_FLAG_DISPATCH_ALL|EVENT_DISPATCH_FLAG_SET_ACTIVE|EVENT_DISPATCH_FLAG_BYPASS_ACL);
	return 1;
}



static bool _dispatch_signal_to_process(process_t* process,signal_t signal){
	if (process==process_kernel){
		return 0;
	}
	if (signal==SIGNAL_KILL){
		// forward to all threads
		return 1;
	}
	event_t* event_to_dispatch=NULL;
	rwlock_acquire_write(&(process->signal_state.lock));
	if (process->signal_state.mask&(1<<signal)){
		rwlock_release_write(&(process->signal_state.lock));
		return 0;
	}
	rwlock_acquire_read(&(process->thread_list.lock));
	for (thread_t* thread=process->thread_list.head;thread;thread=thread->thread_list_next){
		rwlock_acquire_write(&(thread->signal_state.lock));
		if (!(thread->signal_state.mask&(1<<signal))){
			thread->signal_state.pending|=1<<signal;
			event_to_dispatch=thread->signal_state.event;
			_thread_wakeup_if_waiting(thread);
			rwlock_release_write(&(thread->signal_state.lock));
			goto _signal_dispatched;
		}
		rwlock_release_write(&(thread->signal_state.lock));
	}
	process->signal_state.pending|=1<<signal;
_signal_dispatched:
	rwlock_release_read(&(process->thread_list.lock));
	rwlock_release_write(&(process->signal_state.lock));
	if (event_to_dispatch){
		event_dispatch(event_to_dispatch,EVENT_DISPATCH_FLAG_DISPATCH_ALL|EVENT_DISPATCH_FLAG_SET_ACTIVE|EVENT_DISPATCH_FLAG_BYPASS_ACL);
	}
	return 1;
}



static bool _dispatch_signal_to_process_group(process_group_t* process_group,signal_t signal){
	if (process_group==process_kernel->process_group){
		return 0;
	}
	if (signal==SIGNAL_KILL){
		// forward to all processes
		return 1;
	}
	bool out=0;
	rwlock_acquire_read(&(process_group->lock));
	for (process_group_entry_t* entry=(process_group_entry_t*)rb_tree_iter_start(&(process_group->tree));entry;entry=(process_group_entry_t*)rb_tree_iter_next(&(process_group->tree),&(entry->rb_node))){
		handle_t* process_handle=handle_lookup_and_acquire(entry->rb_node.key,process_handle_type);
		if (!process_handle){
			continue;
		}
		out|=_dispatch_signal_to_process(KERNEL_CONTAINEROF(process_handle,process_t,handle),signal);
		handle_release(process_handle);
	}
	rwlock_release_read(&(process_group->lock));
	return out;
}



u64 _signal_return_from_syscall(u64 rax){
	if (!THREAD_DATA->signal_state.pending||THREAD_DATA->signal_state.handler==SIGNAL_HANDLER_SYNC){
		return rax;
	}
	rwlock_acquire_write(&(THREAD_DATA->header.current_thread->signal_state.lock));
	signal_t signal=__builtin_ffsll(THREAD_DATA->signal_state.pending)-1;
	THREAD_DATA->signal_state.pending&=THREAD_DATA->signal_state.pending-1;
	KERNEL_USER_POINTER void* handler=THREAD_DATA->signal_state.handler;
	u64 handler_ctx=THREAD_DATA->signal_state.handler_ctx;
	rwlock_release_write(&(THREAD_DATA->header.current_thread->signal_state.lock));
	if (handler==SIGNAL_HANDLER_NONE){
		asm volatile("sti":::"memory");
		thread_terminate((void*)ERROR_SIGNAL(signal));
	}
	volatile u64* args=(void*)(__builtin_frame_address(0)+16);
	args[0]=args[8];
	args[1]=args[9];
	args[2]=rax;
	args[3]=signal;
	args[4]=handler_ctx;
	args[8]=(u64)handler;
	args[9]=0x0000000202;
	return 0;
}



void signal_process_state_init(signal_process_state_t* state){
	rwlock_init(&(state->lock));
	state->mask=0;
	state->pending=0;
}



void signal_thread_state_init(signal_thread_state_t* state){
	rwlock_init(&(state->lock));
	state->mask=0;
	state->pending=0;
	state->event=event_create("kernel.signal",NULL);
	state->handler=SIGNAL_HANDLER_NONE;
}



void signal_thread_state_deinit(signal_thread_state_t* state){
	event_delete(state->event);
	state->event=NULL;
}



error_t signal_thread_state_get_error(signal_thread_state_t* state){
	rwlock_acquire_read(&(state->lock));
	error_t out=_get_error(state);
	rwlock_release_read(&(state->lock));
	return out;
}



KERNEL_PUBLIC error_t signal_dispatch(handle_id_t handle,signal_t signal){
	if (signal==SIGNAL_KILL){
		panic("signal_dispatch: SIGNAL_KILL");
	}
	if (signal>SIGNAL_MAX){
		return ERROR_INVALID_ARGUMENT(1);
	}
	if (HANDLE_ID_GET_TYPE(handle)==thread_handle_type){
		handle_t* thread_handle=handle_lookup_and_acquire(handle,thread_handle_type);
		if (!thread_handle){
			return ERROR_INVALID_HANDLE;
		}
		error_t out=(_dispatch_signal_to_thread(KERNEL_CONTAINEROF(thread_handle,thread_t,handle),signal)?ERROR_OK:ERROR_MASKED);
		handle_release(thread_handle);
		return out;
	}
	if (HANDLE_ID_GET_TYPE(handle)==process_handle_type){
		handle_t* process_handle=handle_lookup_and_acquire(handle,process_handle_type);
		if (!process_handle){
			return ERROR_INVALID_HANDLE;
		}
		error_t out=(_dispatch_signal_to_process(KERNEL_CONTAINEROF(process_handle,process_t,handle),signal)?ERROR_OK:ERROR_MASKED);
		handle_release(process_handle);
		return out;
	}
	if (HANDLE_ID_GET_TYPE(handle)==process_group_handle_type){
		handle_t* process_group_handle=handle_lookup_and_acquire(handle,process_group_handle_type);
		if (!process_group_handle){
			return ERROR_INVALID_HANDLE;
		}
		error_t out=(_dispatch_signal_to_process_group(KERNEL_CONTAINEROF(process_group_handle,process_group_t,handle),signal)?ERROR_OK:ERROR_MASKED);
		handle_release(process_group_handle);
		return out;
	}
	return ERROR_INVALID_HANDLE;
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
	bool dispatch_event=0;
	rwlock_acquire_write(&(THREAD_DATA->process->signal_state.lock));
	if (is_process_mask){
		THREAD_DATA->process->signal_state.mask=mask;
		THREAD_DATA->process->signal_state.pending&=~mask;
	}
	else{
		rwlock_acquire_write(&(THREAD_DATA->header.current_thread->signal_state.lock));
		THREAD_DATA->signal_state.mask=mask;
		THREAD_DATA->signal_state.pending&=~mask;
		u64 pending=THREAD_DATA->process->signal_state.pending&(~mask);
		THREAD_DATA->process->signal_state.pending^=pending;
		THREAD_DATA->signal_state.pending|=pending;
		if (pending){
			dispatch_event=1;
		}
		rwlock_release_write(&(THREAD_DATA->header.current_thread->signal_state.lock));
	}
	rwlock_release_write(&(THREAD_DATA->process->signal_state.lock));
	if (dispatch_event){
		event_dispatch(THREAD_DATA->signal_state.event,EVENT_DISPATCH_FLAG_DISPATCH_ALL|EVENT_DISPATCH_FLAG_SET_ACTIVE|EVENT_DISPATCH_FLAG_BYPASS_ACL);
	}
	return ERROR_OK;
}



error_t syscall_signal_set_handler(KERNEL_USER_POINTER void* handler,u64 ctx){
	rwlock_acquire_write(&(THREAD_DATA->header.current_thread->signal_state.lock));
	THREAD_DATA->signal_state.handler=handler;
	THREAD_DATA->signal_state.handler_ctx=ctx;
	rwlock_release_write(&(THREAD_DATA->header.current_thread->signal_state.lock));
	return ERROR_OK;
}



error_t syscall_signal_dispatch(handle_id_t handle,signal_t signal){
	return signal_dispatch(handle,signal);
}



error_t syscall_signal_return(u64 rip,u64 rflags,u64 rax){
	volatile u64* args=(void*)(__builtin_frame_address(0)+16);
	args[8]=rip;
	args[9]=(rflags&0x00000cd5)|0x00000202;
	return rax;
}
