#ifndef _KERNEL_HANDLE_HANDLE_LIST_H_
#define _KERNEL_HANDLE_HANDLE_LIST_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/lock/rwlock.h>
#include <kernel/types.h>



typedef struct _HANDLE_LIST{
	rwlock_t lock;
	handle_t* head;
	handle_t* tail;
} handle_list_t;



void handle_list_init(handle_list_t* out);



void handle_list_destroy(handle_list_t* out);



void handle_list_push(handle_list_t* list,handle_t* handle);



void handle_list_pop(handle_t* handle);



#endif
