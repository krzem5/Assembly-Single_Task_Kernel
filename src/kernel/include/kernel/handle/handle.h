#ifndef _KERNEL_HANDLE_HANDLE_H_
#define _KERNEL_HANDLE_HANDLE_H_ 1
#include <kernel/lock/spinlock.h>
#include <kernel/notification/notification.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>



#define HANDLE_TYPE_ANY 0

#define HANDLE_ID_CREATE(type,index) ((type)|((index)<<16))
#define HANDLE_ID_GET_TYPE(handle_id) ((handle_id)&0xffff)
#define HANDLE_ID_GET_INDEX(handle_id) ((handle_id)>>16)

#define HANDLE_INIT_STRUCT {.rb_node={.key=0}}

#define HANDLE_ITER_START(descriptor) ((handle_t*)rb_tree_iter_start(&((descriptor)->tree)))
#define HANDLE_ITER_NEXT(descriptor,handle) ((handle_t*)rb_tree_iter_next(&((descriptor)->tree),&((handle)->rb_node)))
#define HANDLE_FOREACH(type) handle_descriptor_t* __descriptor=handle_get_descriptor((type));if (__descriptor)for (handle_t* handle=HANDLE_ITER_START(__descriptor);handle;handle=HANDLE_ITER_NEXT(__descriptor,handle))



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
	spinlock_t lock;
	rb_tree_t tree;
	KERNEL_ATOMIC handle_id_t count;
	KERNEL_ATOMIC handle_id_t active_count;
	rb_tree_node_t rb_node;
	notification_dispatcher_t notification_dispatcher;
} handle_descriptor_t;



extern handle_type_t handle_handle_type;



handle_type_t handle_alloc(const char* name,handle_type_delete_callback_t delete_callback);



handle_descriptor_t* handle_get_descriptor(handle_type_t type);



void handle_new(void* object,handle_type_t type,handle_t* out);



void handle_finish_setup(handle_t* handle);



handle_t* handle_lookup_and_acquire(handle_id_t id,handle_type_t type);



void handle_destroy(handle_t* handle);



void _handle_delete_internal(handle_t* handle);



_Bool handle_register_notification_listener(handle_type_t type,notification_listener_t* listener);



_Bool handle_unregister_notification_listener(handle_type_t type,notification_listener_t* listener);



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
