#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
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
		lock_init(&(handle_descriptor->lock));
		handle_descriptor->delete_callback=(*descriptor)->delete_callback;
		rb_tree_init(&(handle_descriptor->tree));
		handle_descriptor->count=0;
		handle_descriptor->active_count=0;
		handle_descriptor->rb_node.key=handle_type_index;
		rb_tree_insert_node_increasing(&_handle_type_tree,&(handle_descriptor->rb_node));
	}
	for (handle_descriptor_t*const* descriptor=(void*)kernel_section_handle_start();(u64)descriptor<kernel_section_handle_end();descriptor++){
		handle_descriptor_t* handle_descriptor=*descriptor;
		handle_new(handle_descriptor,HANDLE_TYPE_HANDLE,&(handle_descriptor->handle));
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
	lock_acquire_exclusive(&(handle_descriptor->lock));
	out->rb_node.key=HANDLE_ID_CREATE(type,handle_descriptor->count);
	handle_descriptor->count++;
	handle_descriptor->active_count++;
	rb_tree_insert_node_increasing(&(handle_descriptor->tree),&(out->rb_node));
	lock_release_exclusive(&(handle_descriptor->lock));
}



handle_t* handle_lookup_and_acquire(handle_id_t id,handle_type_t type){
	handle_descriptor_t* handle_descriptor=handle_get_descriptor(HANDLE_ID_GET_TYPE(id));
	if (!handle_descriptor||(type!=HANDLE_TYPE_ANY&&type!=HANDLE_ID_GET_TYPE(id))){
		return NULL;
	}
	lock_acquire_shared(&(handle_descriptor->lock));
	handle_t* out=(handle_t*)rb_tree_lookup_node(&(handle_descriptor->tree),id);
	if (out){
		handle_acquire(out);
	}
	lock_release_shared(&(handle_descriptor->lock));
	return out;
}



void handle_destroy(handle_t* handle){
	SPINLOOP(handle->rc>1);
	_handle_delete_internal(handle);
}



void _handle_delete_internal(handle_t* handle){
	if (handle->rc){
		return;
	}
	handle_descriptor_t* handle_descriptor=handle_get_descriptor(HANDLE_ID_GET_TYPE(handle->rb_node.key));
	lock_acquire_exclusive(&(handle_descriptor->lock));
	rb_tree_remove_node(&(handle_descriptor->tree),&(handle->rb_node));
	lock_release_exclusive(&(handle_descriptor->lock));
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
	handle_descriptor->active_count--;
#pragma GCC diagnostic pop
	handle_descriptor->delete_callback(handle);
}
