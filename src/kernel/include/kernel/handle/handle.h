#ifndef _KERNEL_HANDLE_HANDLE_H_
#define _KERNEL_HANDLE_HANDLE_H_ 1
#include <kernel/lock/lock.h>
#include <kernel/types.h>



#define HANDLE_TYPE_ANY 0

#define HANDLE_NAME_LENGTH 16

#define HANDLE_ID_CREATE(type,index) ((type)|((index)<<16))
#define HANDLE_ID_GET_TYPE(handle_id) ((handle_id)&0xffff)
#define HANDLE_ID_GET_INDEX(handle_id) ((handle_id)>>16)

#define HANDLE_GET_OBJECT(handle) ((void*)(((u64)(handle))+(handle)->object_offset))

#define HANDLE_DECLARE_TYPE(name,delete_code) \
	handle_type_t HANDLE_TYPE_##name; \
	static void _handle_delete_callback_##name(handle_t* handle){delete_code;} \
	static const handle_descriptor_t _handle_descriptor_##name={ \
		#name, \
		&(HANDLE_TYPE_##name), \
		_handle_delete_callback_##name \
	}; \
	static const handle_descriptor_t*const __attribute__((used,section(".handle"))) _handle_descriptor_ptr_##name=&_handle_descriptor_##name;



typedef u16 handle_type_t;



typedef u64 handle_id_t;



typedef struct _HANDLE{
	handle_id_t id;
	lock_t lock;
	s16 object_offset;
	u8 rb_color;
	KERNEL_ATOMIC u64 rc;
	struct _HANDLE* rb_parent;
	union{
		struct{
			struct _HANDLE* rb_left;
			struct _HANDLE* rb_right;
		};
		struct _HANDLE* rb_nodes[2];
	};
} handle_t;



typedef void (*handle_type_delete_callback_t)(struct _HANDLE*);



typedef struct _HANDLE_DESCRIPTOR{
	char name[HANDLE_NAME_LENGTH];
	handle_type_t* var;
	handle_type_delete_callback_t delete_callback;
} handle_descriptor_t;



typedef struct _HANDLE_TYPE_DATA{
	char name[HANDLE_NAME_LENGTH];
	lock_t lock;
	handle_type_delete_callback_t delete_callback;
	handle_t* rb_root;
	KERNEL_ATOMIC handle_id_t count;
	KERNEL_ATOMIC handle_id_t active_count;
} handle_type_data_t;



extern handle_type_data_t* handle_type_data;
extern handle_type_t handle_type_count;



void handle_init(void);



void handle_new(void* object,handle_type_t type,handle_t* out);



handle_t* handle_lookup_and_acquire(handle_id_t id,handle_type_t type);



void _handle_delete_internal(handle_t* handle);



static KERNEL_INLINE void handle_acquire(handle_t* handle){
	handle->rc++;
}



static KERNEL_INLINE _Bool handle_release(handle_t* handle){
	handle->rc--;
	if (handle->rc){
		return 1;
	}
	_handle_delete_internal(handle);
	return 0;
}



#endif
