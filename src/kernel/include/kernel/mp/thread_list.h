#ifndef _KERNEL_MP_THREAD_LIST_H_
#define _KERNEL_MP_THREAD_LIST_H_ 1
#include <kernel/mp/_mp_types.h>



void thread_list_init(thread_list_t* out);



void thread_list_add(thread_list_t* thread_list,thread_t* thread);



bool thread_list_remove(thread_list_t* thread_list,thread_t* thread);



#endif
