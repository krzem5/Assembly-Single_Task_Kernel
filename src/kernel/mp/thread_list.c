#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/mp/thread_list.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "thread_list"



void thread_list_init(thread_list_t* out){
	spinlock_init(&(out->lock));
	out->head=NULL;
}



void thread_list_add(thread_list_t* thread_list,thread_t* thread){
	spinlock_acquire_exclusive(&(thread_list->lock));
	thread->thread_list_prev=NULL;
	thread->thread_list_next=thread_list->head;
	if (thread_list->head){
		thread_list->head->thread_list_prev=thread;
	}
	thread_list->head=thread;
	spinlock_release_exclusive(&(thread_list->lock));
}



_Bool thread_list_remove(thread_list_t* thread_list,thread_t* thread){
	spinlock_acquire_exclusive(&(thread_list->lock));
	if (thread->thread_list_prev){
		thread->thread_list_prev->thread_list_next=thread->thread_list_next;
	}
	else{
		thread_list->head=thread->thread_list_next;
	}
	if (thread->thread_list_next){
		thread->thread_list_next->thread_list_prev=thread->thread_list_prev;
	}
	spinlock_release_exclusive(&(thread_list->lock));
	return !thread_list->head;
}
