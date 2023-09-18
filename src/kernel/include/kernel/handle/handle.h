#ifndef _KERNEL_HANDLE_HANDLE_H_
#define _KERNEL_HANDLE_HANDLE_H_ 1
#include <kernel/lock/lock.h>
#include <kernel/types.h>



#define HANDLE_TYPE_ANY 0
#define HANDLE_TYPE_EVENT 1
#define HANDLE_TYPE_THREAD 2
#define HANDLE_TYPE_PROCESS 3



typedef u8 handle_type_t;



typedef u64 handle_id_t;



typedef struct _HANDLE{
	handle_type_t type;
	lock_t lock;
	handle_id_t id;
	u64 rc;
	void* object;
	struct _HANDLE* prev;
	struct _HANDLE* next;
} handle_t;



void handle_new(void* object,handle_type_t type,handle_t* out);



void handle_delete(handle_t* handle);



handle_t* handle_lookup_and_acquire(handle_id_t id,handle_type_t type);



static inline void handle_acquire(handle_t* handle){
	handle->rc++;
}



static inline void handle_release(handle_t* handle){
	handle->rc--;
	if (!handle->rc){
		handle_delete(handle);
	}
}



#endif
