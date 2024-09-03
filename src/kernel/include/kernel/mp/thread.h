#ifndef _KERNEL_MP_THREAD_H_
#define _KERNEL_MP_THREAD_H_ 1
#include <kernel/mp/_mp_types.h>
#include <kernel/types.h>



#define THREAD_STATE_TYPE_NONE 0
#define THREAD_STATE_TYPE_QUEUED 1
#define THREAD_STATE_TYPE_RUNNING 2
#define THREAD_STATE_TYPE_AWAITING_EVENT 3
#define THREAD_STATE_TYPE_TERMINATED 255

#define THREAD_PRIORITY_FIXED 254

#define THREAD_ACL_FLAG_TERMINATE 1
#define THREAD_ACL_FLAG_CONFIG 2

#define THREAD_DATA ((volatile __seg_gs thread_t*)NULL)



typedef struct _THREAD_QUERY_USER_DATA{
	u64 pid;
	u64 tid;
	char name[256];
	u8 state;
	u8 priority;
	u8 scheduler_priority;
} thread_query_user_data_t;



extern handle_type_t thread_handle_type;



thread_t* thread_create_user_thread(process_t* process,u64 rip,u64 rsp);



thread_t* thread_create_kernel_thread(process_t* process,const char* name,void* func,u8 arg_count,...);



void thread_delete(thread_t* thread);



void KERNEL_NORETURN thread_terminate(void* return_value);



void thread_terminate_remote_locked(thread_t* thread,void* return_value);



void KERNEL_NORETURN _thread_bootstrap_kernel_thread(void);



#endif
