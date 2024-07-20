#ifndef _SYS_SIGNAL_SIGNAL_H_
#define _SYS_SIGNAL_SIGNAL_H_ 1
#include <sys/error/error.h>
#include <sys/mp/event.h>
#include <sys/types.h>



#define SYS_SIGNAL_KILL 0
#define SYS_SIGNAL_INTERRUPT 1
#define SYS_SIGNAL_MAX 63

#define SYS_SIGNAL_HANDLER_NONE NULL
#define SYS_SIGNAL_HANDLER_SYNC ((void*)1)



typedef u32 sys_signal_t;



typedef u64 sys_signal_mask_t;



typedef struct _SYS_SIGNAL_HANDLER_CONTEXT{
	u64 signal;
	u64 return_code;
	u64 rflags;
	u64 rip;
} sys_signal_handler_context_t;



sys_event_t sys_signal_get_event(void);



sys_error_t sys_signal_get_signal(void);



sys_signal_mask_t sys_signal_get_pending_signals(bool is_process);



sys_signal_mask_t sys_signal_get_mask(bool is_process_mask);



sys_error_t sys_signal_set_mask(sys_signal_mask_t mask,bool is_process_mask);



sys_error_t sys_signal_set_handler(void* handler);



sys_error_t sys_signal_dispatch(u64 handle,sys_signal_t signal);



#endif
