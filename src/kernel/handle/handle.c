#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/notification/notification.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "handle"



HANDLE_DECLARE_TYPE(HANDLE,{
	panic("Unable to delete HANDLE_TYPE_HANDLE");
});



static rb_tree_t _handle_type_tree;



void handle_init(void){
	LOG("Initializing handle types...");
	rb_tree_init(&_handle_type_tree);
	handle_type_t handle_type_index=HANDLE_TYPE_ANY;
	for (handle_descriptor_t*const* descriptor=(void*)kernel_section_handle_start();(u64)descriptor<kernel_section_handle_end();descriptor++){
		handle_type_index++;
		handle_descriptor_t* handle_descriptor=*descriptor;
		*((*descriptor)->var)=handle_type_index;
		spinlock_init(&(handle_descriptor->lock));
		handle_descriptor->delete_callback=(*descriptor)->delete_callback;
		rb_tree_init(&(handle_descriptor->tree));
		notification_dispatcher_init(&(handle_descriptor->notification_dispatcher));
		handle_descriptor->count=0;
		handle_descriptor->active_count=0;
		handle_descriptor->rb_node.key=handle_type_index;
		rb_tree_insert_node_increasing(&_handle_type_tree,&(handle_descriptor->rb_node));
	}
	for (handle_descriptor_t*const* descriptor=(void*)kernel_section_handle_start();(u64)descriptor<kernel_section_handle_end();descriptor++){
		handle_descriptor_t* handle_descriptor=*descriptor;
		handle_new(handle_descriptor,HANDLE_TYPE_HANDLE,&(handle_descriptor->handle));
		handle_finish_setup(&(handle_descriptor->handle));
	}
}



handle_descriptor_t* handle_get_descriptor(handle_type_t type){
	u64 out=(u64)rb_tree_lookup_node(&_handle_type_tree,type);
	return (out?(handle_descriptor_t*)(out-__builtin_offsetof(handle_descriptor_t,rb_node)):NULL);
}



void handle_new(void* object,handle_type_t type,handle_t* out){
	handle_descriptor_t* handle_descriptor=handle_get_descriptor(type);
	if (!handle_descriptor){
		panic("Invalid handle type");
	}
	out->rc=1;
	out->object=object;
	spinlock_acquire_exclusive(&(handle_descriptor->lock));
	out->rb_node.key=HANDLE_ID_CREATE(type,handle_descriptor->count);
	handle_descriptor->count++;
	handle_descriptor->active_count++;
	rb_tree_insert_node_increasing(&(handle_descriptor->tree),&(out->rb_node));
	spinlock_release_exclusive(&(handle_descriptor->lock));
}



void handle_finish_setup(handle_t* handle){
	handle_acquire(handle);
	handle_descriptor_t* handle_descriptor=handle_get_descriptor(HANDLE_ID_GET_TYPE(handle->rb_node.key));
	notification_dispatcher_dispatch(&(handle_descriptor->notification_dispatcher),handle,NOTIFICATION_TYPE_HANDLE_CREATE);
	handle_release(handle);
}



handle_t* handle_lookup_and_acquire(handle_id_t id,handle_type_t type){
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



void handle_destroy(handle_t* handle){
	SPINLOOP(handle->rc>1);
	_handle_delete_internal(handle);
}



KERNEL_NOINLINE void _handle_delete_internal(handle_t* handle){
	if (handle->rc){
		return;
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
	handle_descriptor->delete_callback(handle);
}



_Bool handle_register_notification_listener(handle_type_t type,notification_listener_t* listener){
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



_Bool handle_unregister_notification_listener(handle_type_t type,notification_listener_t* listener){
	handle_descriptor_t* handle_descriptor=handle_get_descriptor(type);
	if (!handle_descriptor){
		return 0;
	}
	spinlock_acquire_exclusive(&(handle_descriptor->lock));
	notification_dispatcher_remove_listener(&(handle_descriptor->notification_dispatcher),listener);
	spinlock_release_exclusive(&(handle_descriptor->lock));
	return 1;
}
