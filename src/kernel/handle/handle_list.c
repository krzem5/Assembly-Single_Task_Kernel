#include <kernel/handle/handle.h>
#include <kernel/handle/handle_list.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "handle_list"



static void _pop_handle(handle_list_t* list,handle_t* handle){
	if (handle->handle_list!=list){
		return;
	}
	handle->handle_list=NULL;
	if (handle->handle_list_prev){
		handle->handle_list_prev->handle_list_next=handle->handle_list_next;
	}
	else{
		list->head=handle->handle_list_next;
	}
	if (handle->handle_list_next){
		handle->handle_list_next->handle_list_prev=handle->handle_list_prev;
	}
	else{
		list->tail=handle->handle_list_prev;
	}
}



KERNEL_PUBLIC void handle_list_init(handle_list_t* out){
	rwlock_init(&(out->lock));
	out->head=NULL;
	out->tail=NULL;
}



KERNEL_PUBLIC void handle_list_destroy(handle_list_t* list){
	return; // null pointer dereference (racing condition) is triggered during handle releases below
	while (1){
		rwlock_acquire_write(&(list->lock));
		handle_t* handle=list->head;
		if (!handle){
			rwlock_release_write(&(list->lock));
			return;
		}
		_pop_handle(list,handle);
		rwlock_release_write(&(list->lock));
		handle_release(handle);
	}
}



KERNEL_PUBLIC void handle_list_push(handle_list_t* list,handle_t* handle){
	if (handle->handle_list==list){
		return;
	}
	rwlock_acquire_write(&(list->lock));
	if (handle->handle_list){
		panic("Handle already in a list");
	}
	handle->handle_list=list;
	if (list->tail){
		list->tail->handle_list_next=handle;
	}
	else{
		list->head=handle;
	}
	handle->handle_list_prev=list->tail;
	handle->handle_list_next=NULL;
	list->tail=handle;
	rwlock_release_write(&(list->lock));
}



KERNEL_PUBLIC void handle_list_pop(handle_t* handle){
	handle_list_t* list=handle->handle_list;
	if (!list){
		return;
	}
	rwlock_acquire_write(&(list->lock));
	_pop_handle(list,handle);
	rwlock_release_write(&(list->lock));
}
