#include <kernel/acl/acl.h>
#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/kernel/kernel.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/syscall/syscall.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/spinloop.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "handle"



static omm_allocator_t* KERNEL_INIT_WRITE _handle_descriptor_allocator=NULL;
static rb_tree_t _handle_type_tree;
static KERNEL_ATOMIC handle_type_t _handle_max_type=HANDLE_TYPE_ANY;

KERNEL_PUBLIC handle_type_t KERNEL_INIT_WRITE handle_handle_type=0;



KERNEL_PUBLIC handle_type_t handle_alloc(const char* name,u32 flags,handle_type_delete_callback_t delete_callback){
	if (!_handle_descriptor_allocator){
		omm_init_self();
		rb_tree_init(&_handle_type_tree);
		_handle_descriptor_allocator=omm_init("kernel.handle.descriptor",sizeof(handle_descriptor_t),8,2);
		omm_init_handle_type(_handle_descriptor_allocator);
		handle_handle_type=handle_alloc("kernel.handle",0,NULL);
		for (rb_tree_node_t* rb_node=rb_tree_iter_start(&_handle_type_tree);rb_node;rb_node=rb_tree_iter_next(&_handle_type_tree,rb_node)){
			handle_descriptor_t* descriptor=KERNEL_CONTAINEROF(rb_node,handle_descriptor_t,rb_node);
			if (!descriptor->handle.rb_node.key){
				handle_new(handle_handle_type,&(descriptor->handle));
			}
		}
	}
	handle_type_t out=__atomic_add_fetch(&_handle_max_type,1,__ATOMIC_SEQ_CST);
	handle_descriptor_t* descriptor=omm_alloc(_handle_descriptor_allocator);
	descriptor->name=name;
	descriptor->delete_callback=delete_callback;
	if (handle_handle_type){
		handle_new(handle_handle_type,&(descriptor->handle));
	}
	else{
		descriptor->handle.rb_node.key=0;
	}
	rwlock_init(&(descriptor->lock));
	descriptor->flags=flags;
	rb_tree_init(&(descriptor->tree));
	descriptor->count=0;
	descriptor->active_count=0;
	descriptor->rb_node.key=out;
	rb_tree_insert_node(&_handle_type_tree,&(descriptor->rb_node));
	return out;
}



KERNEL_PUBLIC handle_descriptor_t* handle_get_descriptor(handle_type_t type){
	u64 out=(u64)rb_tree_lookup_node(&_handle_type_tree,type);
	return (out?KERNEL_CONTAINEROF(out,handle_descriptor_t,rb_node):NULL);
}



KERNEL_PUBLIC void handle_new(handle_type_t type,handle_t* out){
	handle_descriptor_t* handle_descriptor=handle_get_descriptor(type);
	if (!handle_descriptor){
		panic("Invalid handle type");
	}
	out->rc=1;
	out->acl=NULL;
	rwlock_acquire_write(&(handle_descriptor->lock));
	out->rb_node.key=HANDLE_ID_CREATE(type,handle_descriptor->count);
	handle_descriptor->count++;
	handle_descriptor->active_count++;
	rb_tree_insert_node(&(handle_descriptor->tree),&(out->rb_node));
	rwlock_release_write(&(handle_descriptor->lock));
}



KERNEL_PUBLIC handle_t* handle_lookup_and_acquire(handle_id_t id,handle_type_t type){
	handle_descriptor_t* handle_descriptor=handle_get_descriptor(HANDLE_ID_GET_TYPE(id));
	if (!handle_descriptor||(type!=HANDLE_TYPE_ANY&&type!=HANDLE_ID_GET_TYPE(id))){
		return NULL;
	}
	rwlock_acquire_read(&(handle_descriptor->lock));
	handle_t* out=(handle_t*)rb_tree_lookup_node(&(handle_descriptor->tree),id);
	if (out){
		handle_acquire(out);
	}
	rwlock_release_read(&(handle_descriptor->lock));
	return out;
}



KERNEL_PUBLIC void handle_destroy(handle_t* handle){
	SPINLOOP(handle->rc>1);
	handle->rc=0;
	_handle_delete_internal(handle);
}



KERNEL_PUBLIC KERNEL_NOINLINE void _handle_delete_internal(handle_t* handle){
	if (handle->rc){
		return;
	}
	handle_descriptor_t* handle_descriptor=handle_get_descriptor(HANDLE_ID_GET_TYPE(handle->rb_node.key));
	rwlock_acquire_write(&(handle_descriptor->lock));
	if (handle->rc){
		rwlock_release_write(&(handle_descriptor->lock));
		return;
	}
	rb_tree_remove_node(&(handle_descriptor->tree),&(handle->rb_node));
	rwlock_release_write(&(handle_descriptor->lock));
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
	handle_descriptor->active_count--;
#pragma GCC diagnostic pop
	if (handle->acl){
		acl_delete(handle->acl);
		handle->acl=NULL;
	}
	if (handle_descriptor->delete_callback){
		handle_descriptor->delete_callback(handle);
	}
}



KERNEL_PUBLIC handle_t* handle_iter_start(handle_descriptor_t* handle_descriptor){
	rwlock_acquire_read(&(handle_descriptor->lock));
	handle_t* out=(handle_t*)rb_tree_iter_start(&(handle_descriptor->tree));
	if (out){
		handle_acquire(out);
	}
	rwlock_release_read(&(handle_descriptor->lock));
	return out;
}



KERNEL_PUBLIC handle_t* handle_iter_next(handle_descriptor_t* handle_descriptor,handle_t* handle){
	rwlock_acquire_read(&(handle_descriptor->lock));
	handle_t* out=(handle_t*)rb_tree_iter_next(&(handle_descriptor->tree),&(handle->rb_node));
	handle_release(handle);
	if (out){
		handle_acquire(out);
	}
	rwlock_release_read(&(handle_descriptor->lock));
	return out;
}



error_t syscall_handle_get_name(handle_id_t handle,KERNEL_USER_POINTER char* buffer,u32 buffer_length){
	if (buffer_length>syscall_get_user_pointer_max_length((char*)buffer)){
		return ERROR_INVALID_ARGUMENT(1);
	}
	handle_descriptor_t* handle_descriptor=handle_get_descriptor(HANDLE_ID_GET_TYPE(handle));
	if (!handle_descriptor){
		return ERROR_NOT_FOUND;
	}
	u32 length=smm_length(handle_descriptor->name)+1;
	if (buffer_length<length){
		return ERROR_NO_SPACE;
	}
	mem_copy((char*)buffer,handle_descriptor->name,length);
	return ERROR_OK;
}
