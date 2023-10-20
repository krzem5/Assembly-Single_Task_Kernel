#ifndef _KERNEL_HANDLE_HANDLE_H_
#define _KERNEL_HANDLE_HANDLE_H_ 1
#include <kernel/lock/lock.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>



#define HANDLE_TYPE_ANY 0

#define HANDLE_NAME_LENGTH 16

#define HANDLE_ID_CREATE(type,index) ((type)|((index)<<16))
#define HANDLE_ID_GET_TYPE(handle_id) ((handle_id)&0xffff)
#define HANDLE_ID_GET_INDEX(handle_id) ((handle_id)>>16)

#define HANDLE_INIT_STRUCT {.rb_node={.key=0}}

#define HANDLE_DECLARE_TYPE(name,delete_code) \
	handle_type_t HANDLE_TYPE_##name; \
	static void _handle_delete_callback_##name(handle_t* handle){delete_code;} \
	static handle_descriptor_t _handle_descriptor_##name={ \
		#name, \
		&(HANDLE_TYPE_##name), \
		_handle_delete_callback_##name \
	}; \
	static handle_descriptor_t*const __attribute__((used,section(".handle"))) _handle_descriptor_ptr_##name=&_handle_descriptor_##name;

#define HANDLE_ITER_START(descriptor) ((handle_t*)rb_tree_iter_start(&((descriptor)->tree)))
#define HANDLE_ITER_NEXT(descriptor,handle) ((handle_t*)rb_tree_iter_next(&((descriptor)->tree),&((handle)->rb_node)))
#define HANDLE_FOREACH(type) handle_descriptor_t* __descriptor=handle_get_descriptor((type));for (handle_t* handle=HANDLE_ITER_START(__descriptor);handle;handle=HANDLE_ITER_NEXT(__descriptor,handle))



typedef u16 handle_type_t;



typedef u64 handle_id_t;



typedef struct _HANDLE{
	rb_tree_node_t rb_node;
	void* object;
	KERNEL_ATOMIC u64 rc;
} handle_t;



typedef void (*handle_type_delete_callback_t)(handle_t*);



typedef struct _HANDLE_DESCRIPTOR{
	const char* name;
	handle_type_t* var;
	handle_type_delete_callback_t delete_callback;
	handle_t handle;
	lock_t lock;
	rb_tree_t tree;
	KERNEL_ATOMIC handle_id_t count;
	KERNEL_ATOMIC handle_id_t active_count;
	rb_tree_node_t rb_node;
} handle_descriptor_t;



extern handle_type_t HANDLE_TYPE_HANDLE;



void handle_init(void);



handle_descriptor_t* handle_get_descriptor(handle_type_t type);



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
