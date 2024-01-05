#include <kernel/handle/handle.h>
#include <kernel/handle/handle_list.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "handle_list"



KERNEL_PUBLIC void handle_list_init(handle_list_t* out){
	spinlock_init(&(out->lock));
	out->head=NULL;
	out->tail=NULL;
}



KERNEL_PUBLIC void handle_list_destroy(handle_list_t* list){
	while (list->head){
		handle_t* handle=list->head;
		handle_list_pop(handle);
		handle_release(handle);
	}
}



KERNEL_PUBLIC void handle_list_push(handle_list_t* list,handle_t* handle){
	spinlock_acquire_exclusive(&(list->lock));
	handle->handle_list=list;
	if (list->tail){
		list->tail->handle_list_next=handle;
	}
	else{
		list->head=handle;
	}
	handle->handle_list_prev=list->tail;
	list->tail=handle;
	spinlock_release_exclusive(&(list->lock));
}



KERNEL_PUBLIC void handle_list_pop(handle_t* handle){
	if (!handle->handle_list){
		return;
	}
	spinlock_acquire_exclusive(&(handle->handle_list->lock));
	if (handle->handle_list_prev){
		handle->handle_list_prev->handle_list_next=handle->handle_list_next;
	}
	else{
		handle->handle_list->head=handle->handle_list_next;
	}
	if (handle->handle_list_next){
		handle->handle_list_next->handle_list_prev=handle->handle_list_prev;
	}
	else{
		handle->handle_list->tail=handle->handle_list_prev;
	}
	spinlock_release_exclusive(&(handle->handle_list->lock));
}
