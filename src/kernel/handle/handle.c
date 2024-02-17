#include <kernel/acl/acl.h>
#include <kernel/handle/handle.h>
#include <kernel/handle/handle_list.h>
#include <kernel/kernel.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/notification/notification.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "handle"



static pmm_counter_descriptor_t _handle_descriptor_omm_pmm_counter=_PMM_COUNTER_INIT_STRUCT("omm_handle_descriptor");
static omm_allocator_t* _handle_descriptor_allocator=NULL;
static rb_tree_t _handle_type_tree;
static KERNEL_ATOMIC handle_type_t _handle_max_type=HANDLE_TYPE_ANY;

KERNEL_PUBLIC handle_type_t handle_handle_type=0;



KERNEL_PUBLIC handle_type_t handle_alloc(const char* name,handle_type_delete_callback_t delete_callback){
	if (!_handle_descriptor_allocator){
		omm_init_self();
		rb_tree_init(&_handle_type_tree);
		_handle_descriptor_allocator=omm_init("handle_descriptor",sizeof(handle_descriptor_t),8,2,&_handle_descriptor_omm_pmm_counter);
		omm_init_handle_type(_handle_descriptor_allocator);
		handle_handle_type=handle_alloc("handle",NULL);
		for (rb_tree_node_t* rb_node=rb_tree_iter_start(&_handle_type_tree);rb_node;rb_node=rb_tree_iter_next(&_handle_type_tree,rb_node)){
			handle_descriptor_t* descriptor=(handle_descriptor_t*)(((u64)rb_node)-__builtin_offsetof(handle_descriptor_t,rb_node));
			if (!descriptor->handle.rb_node.key){
				handle_new(descriptor,handle_handle_type,&(descriptor->handle));
				handle_finish_setup(&(descriptor->handle));
			}
		}
	}
	handle_type_t out=__atomic_add_fetch(&_handle_max_type,1,__ATOMIC_SEQ_CST);
	handle_descriptor_t* descriptor=omm_alloc(_handle_descriptor_allocator);
	descriptor->name=name;
	descriptor->var=NULL;
	descriptor->delete_callback=delete_callback;
	if (handle_handle_type){
		handle_new(descriptor,handle_handle_type,&(descriptor->handle));
	}
	else{
		descriptor->handle.rb_node.key=0;
	}
	spinlock_init(&(descriptor->lock));
	rb_tree_init(&(descriptor->tree));
	descriptor->count=0;
	descriptor->active_count=0;
	descriptor->rb_node.key=out;
	notification_dispatcher_init(&(descriptor->notification_dispatcher));
	rb_tree_insert_node_increasing(&_handle_type_tree,&(descriptor->rb_node));
	if (handle_handle_type){
		handle_finish_setup(&(descriptor->handle));
	}
	return out;
}



KERNEL_PUBLIC handle_descriptor_t* handle_get_descriptor(handle_type_t type){
	u64 out=(u64)rb_tree_lookup_node(&_handle_type_tree,type);
	return (out?(handle_descriptor_t*)(out-__builtin_offsetof(handle_descriptor_t,rb_node)):NULL);
}



KERNEL_PUBLIC void handle_new(void* object,handle_type_t type,handle_t* out){
	handle_descriptor_t* handle_descriptor=handle_get_descriptor(type);
	if (!handle_descriptor){
		panic("Invalid handle type");
	}
	out->object=object;
	out->rc=1;
	out->acl=NULL;
	out->handle_list=NULL;
	spinlock_acquire_exclusive(&(handle_descriptor->lock));
	out->rb_node.key=HANDLE_ID_CREATE(type,handle_descriptor->count);
	handle_descriptor->count++;
	handle_descriptor->active_count++;
	rb_tree_insert_node_increasing(&(handle_descriptor->tree),&(out->rb_node));
	spinlock_release_exclusive(&(handle_descriptor->lock));
}



KERNEL_PUBLIC void handle_finish_setup(handle_t* handle){
	handle_acquire(handle);
	handle_descriptor_t* handle_descriptor=handle_get_descriptor(HANDLE_ID_GET_TYPE(handle->rb_node.key));
	notification_dispatcher_dispatch(&(handle_descriptor->notification_dispatcher),handle,NOTIFICATION_TYPE_HANDLE_CREATE);
	handle_release(handle);
}



KERNEL_PUBLIC handle_t* handle_lookup_and_acquire(handle_id_t id,handle_type_t type){
	handle_descriptor_t* handle_descriptor=handle_get_descriptor(HANDLE_ID_GET_TYPE(id));
	if (!handle_descriptor||(type!=HANDLE_TYPE_ANY&&type!=HANDLE_ID_GET_TYPE(id))){
		return NULL;
	}
	spinlock_acquire_shared(&(handle_descriptor->lock));
	handle_t* out=(handle_t*)rb_tree_lookup_node(&(handle_descriptor->tree),id);
	if (out){
		handle_acquire(out);
	}
	spinlock_release_shared(&(handle_descriptor->lock));
	return out;
}



KERNEL_PUBLIC void handle_destroy(handle_t* handle){
	SPINLOOP(handle->rc>1);
	_handle_delete_internal(handle);
}



KERNEL_PUBLIC KERNEL_NOINLINE void _handle_delete_internal(handle_t* handle){
	if (handle->rc){
		return;
	}
	if (handle->handle_list){
		handle_list_pop(handle);
	}
	handle_descriptor_t* handle_descriptor=handle_get_descriptor(HANDLE_ID_GET_TYPE(handle->rb_node.key));
	notification_dispatcher_dispatch(&(handle_descriptor->notification_dispatcher),handle,NOTIFICATION_TYPE_HANDLE_DELETE);
	spinlock_acquire_exclusive(&(handle_descriptor->lock));
	rb_tree_remove_node(&(handle_descriptor->tree),&(handle->rb_node));
	spinlock_release_exclusive(&(handle_descriptor->lock));
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



KERNEL_PUBLIC _Bool handle_register_notification_listener(handle_type_t type,notification_listener_t* listener){
	handle_descriptor_t* handle_descriptor=handle_get_descriptor(type);
	if (!handle_descriptor){
		return 0;
	}
	spinlock_acquire_exclusive(&(handle_descriptor->lock));
	notification_dispatcher_add_listener(&(handle_descriptor->notification_dispatcher),listener);
	for (handle_t* handle=HANDLE_ITER_START(handle_descriptor);handle;handle=HANDLE_ITER_NEXT(handle_descriptor,handle)){
		listener->callback(handle,NOTIFICATION_TYPE_HANDLE_CREATE);
	}
	spinlock_release_exclusive(&(handle_descriptor->lock));
	return 1;
}



KERNEL_PUBLIC _Bool handle_unregister_notification_listener(handle_type_t type,notification_listener_t* listener){
	handle_descriptor_t* handle_descriptor=handle_get_descriptor(type);
	if (!handle_descriptor){
		return 0;
	}
	spinlock_acquire_exclusive(&(handle_descriptor->lock));
	notification_dispatcher_remove_listener(&(handle_descriptor->notification_dispatcher),listener);
	spinlock_release_exclusive(&(handle_descriptor->lock));
	return 1;
}
