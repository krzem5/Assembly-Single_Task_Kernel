#ifndef _KERNEL_HANDLE_HANDLE_H_
#define _KERNEL_HANDLE_HANDLE_H_ 1
#include <kernel/acl/acl.h>
#include <kernel/lock/rwlock.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>



#define HANDLE_TYPE_ANY 0

#define HANDLE_ID_CREATE(type,index) ((type)|((index)<<16))
#define HANDLE_ID_GET_TYPE(handle_id) ((handle_id)&0xffff)
#define HANDLE_ID_GET_INDEX(handle_id) ((handle_id)>>16)

#define HANDLE_DESCRIPTOR_FLAG_ALLOW_CONTAINER 1

#define HANDLE_INIT_STRUCT {.rb_node={.key=0}}

#define HANDLE_ITER_START(descriptor) ((handle_t*)rb_tree_iter_start(&((descriptor)->tree)))
#define HANDLE_ITER_NEXT(descriptor,handle) ((handle_t*)rb_tree_iter_next(&((descriptor)->tree),&((handle)->rb_node)))

#define _HANDLE_TEMPORARY_DESCRIPTOR_NAME_(a,b) a##b
#define _HANDLE_TEMPORARY_DESCRIPTOR_NAME(a,b) _HANDLE_TEMPORARY_DESCRIPTOR_NAME_(a,b)
#define HANDLE_FOREACH(type) handle_descriptor_t* _HANDLE_TEMPORARY_DESCRIPTOR_NAME(__descriptor,__LINE__)=handle_get_descriptor((type));if (_HANDLE_TEMPORARY_DESCRIPTOR_NAME(__descriptor,__LINE__))for (handle_t* handle=handle_iter_start(_HANDLE_TEMPORARY_DESCRIPTOR_NAME(__descriptor,__LINE__));handle;handle=handle_iter_next(_HANDLE_TEMPORARY_DESCRIPTOR_NAME(__descriptor,__LINE__),handle))



typedef u16 handle_type_t;



typedef u64 handle_id_t;



typedef struct _HANDLE{
	rb_tree_node_t rb_node;
	KERNEL_ATOMIC u64 rc;
	acl_t* acl;
} handle_t;



typedef void (*handle_type_delete_callback_t)(handle_t*);



typedef struct _HANDLE_DESCRIPTOR{
	const char* name;
	handle_type_delete_callback_t delete_callback;
	handle_t handle;
	rwlock_t lock;
	u32 flags;
	rb_tree_t tree;
	KERNEL_ATOMIC handle_id_t count;
	KERNEL_ATOMIC handle_id_t active_count;
	rb_tree_node_t rb_node;
} handle_descriptor_t;



extern handle_type_t handle_handle_type;



handle_type_t handle_alloc(const char* name,u32 flags,handle_type_delete_callback_t delete_callback);



handle_descriptor_t* handle_get_descriptor(handle_type_t type);



void handle_new(handle_type_t type,handle_t* out);



handle_t* handle_lookup_and_acquire(handle_id_t id,handle_type_t type);



void handle_destroy(handle_t* handle);



void _handle_delete_internal(handle_t* handle);



handle_t* handle_iter_start(handle_descriptor_t* handle_descriptor);



handle_t* handle_iter_next(handle_descriptor_t* handle_descriptor,handle_t* handle);



static KERNEL_INLINE void KERNEL_NOCOVERAGE handle_acquire(handle_t* handle){
	handle->rc++;
}



static KERNEL_INLINE bool KERNEL_NOCOVERAGE handle_release(handle_t* handle){
	handle->rc--;
	if (handle->rc){
		return 1;
	}
	_handle_delete_internal(handle);
	return 0;
}



#endif
