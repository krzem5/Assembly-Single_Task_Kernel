#ifndef _KERNEL_HANDLE_HANDLE_H_
#define _KERNEL_HANDLE_HANDLE_H_ 1
#include <kernel/lock/lock.h>
#include <kernel/types.h>



#define HANDLE_TYPE_NONE 0
#define HANDLE_TYPE_EVENT 1
#define HANDLE_TYPE_THREAD 2
#define HANDLE_TYPE_PROCESS 3



typedef struct _HANDLE{
	u8 type;
	lock_t lock;
	u64 id;
	void* object;
} handle_t;



void handle_new(void* object,u8 type,handle_t* out);



void handle_delete(handle_t* handle);



#endif
