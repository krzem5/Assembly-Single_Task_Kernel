#ifndef _KERNEL_HANDLE_HANDLE_H_
#define _KERNEL_HANDLE_HANDLE_H_ 1
#include <kernel/lock/lock.h>
#include <kernel/types.h>



#define HANDLE_TYPE_ANY 0

#define HANDLE_DECLARE_TYPE(name,delete_code) \
	handle_type_t HANDLE_TYPE_##name; \
	static void _handle_delete_callback_##name(handle_t* handle){delete_code;} \
	static const handle_descriptor_t KERNEL_CORE_RDATA _handle_descriptor_##name={ \
		#name, \
		&(HANDLE_TYPE_##name), \
		_handle_delete_callback_##name \
	}; \
	static const handle_descriptor_t* __attribute__((used,section(".handle"))) _handle_descriptor_ptr_##name=&_handle_descriptor_##name;



typedef u16 handle_type_t;



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



typedef struct _HANDLE_DESCRIPTOR{
	const char* name;
	handle_type_t* var;
	void (*delete_fn)(handle_t*);
} handle_descriptor_t;



typedef struct _HANDLE_TYPE_DATA{
	const char* name;
	void (*delete_fn)(handle_t*);
	KERNEL_ATOMIC u64 count;
} handle_type_data_t;



extern handle_type_data_t* handle_type_data;
extern handle_type_t handle_type_count;



void handle_init(void);



void handle_new(void* object,handle_type_t type,handle_t* out);



void handle_delete(handle_t* handle);



handle_t* handle_lookup_and_acquire(handle_id_t id,handle_type_t type);



static KERNEL_INLINE void handle_acquire(handle_t* handle){
	handle->rc++;
}



static KERNEL_INLINE _Bool handle_release(handle_t* handle){
	handle->rc--;
	if (handle->rc){
		return 1;
	}
	handle_delete(handle);
	return 0;
}



#endif
