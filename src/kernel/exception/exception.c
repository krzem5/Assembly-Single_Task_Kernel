#include <kernel/exception/exception.h>
#include <kernel/lock/profiling.h>
#include <kernel/log/log.h>
#include <kernel/mp/thread.h>
#include <kernel/mp/thread.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "exception"



void exception_unwind(thread_t* thread){
	for (exception_unwind_frame_t* frame=thread->exception_unwind_frame;frame;frame=frame->next){
		frame->callback(frame->args);
	}
	lock_profiling_assert_empty(thread);
	thread->exception_unwind_frame=NULL;
}



void _exception_signal_interrupt_handler(u64 error){
	exception_unwind(THREAD_DATA->header.current_thread);
	if (THREAD_DATA->exception_is_user){
		_exception_user_return(error);
	}
	thread_terminate((void*)error);
}
