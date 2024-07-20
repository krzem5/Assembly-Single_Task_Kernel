#ifndef _KERNEL_SIGNAL_SIGNAL_H_
#define _KERNEL_SIGNAL_SIGNAL_H_ 1
#include <kernel/error/error.h>
#include <kernel/lock/rwlock.h>
#include <kernel/types.h>



#define SIGNAL_KILL 0
#define SIGNAL_INTERRUPT 1
#define SIGNAL_MAX 63

#define SIGNAL_HANDLER_NONE NULL
#define SIGNAL_HANDLER_SYNC ((void*)1)



struct _EVENT;



typedef u32 signal_t;



typedef struct _SIGNAL_PROCESS_STATE{
	rwlock_t lock;
	u64 mask;
	u64 pending;
} signal_process_state_t;



typedef struct _SIGNAL_THREAD_STATE{
	rwlock_t lock;
	u64 mask;
	u64 pending;
	struct _EVENT* event;
	KERNEL_USER_POINTER void* handler;
	u64 handler_ctx;
} signal_thread_state_t;



void signal_process_state_init(signal_process_state_t* state);



void signal_thread_state_init(signal_thread_state_t* state);



void signal_thread_state_deinit(signal_thread_state_t* state);



error_t signal_thread_state_get_error(signal_thread_state_t* state);



error_t signal_dispatch(handle_id_t handle,signal_t signal);



#endif
