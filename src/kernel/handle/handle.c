#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "handle"



HANDLE_DECLARE_TYPE(HANDLE,{
	panic("Unable to delete HANDLE_TYPE_HANDLE");
});



handle_type_data_t* handle_type_data;
handle_type_t handle_type_count;



void handle_init(void){
	LOG("Initializing handle types...");
	handle_type_count=HANDLE_TYPE_ANY+1;
	for (const handle_descriptor_t*const* descriptor=(void*)kernel_section_handle_start();(u64)descriptor<kernel_section_handle_end();descriptor++){
		*((*descriptor)->var)=handle_type_count;
		handle_type_count++;
	}
	INFO("Handle type count: %u",handle_type_count);
	handle_type_data=kmm_alloc(handle_type_count*sizeof(handle_type_data_t));
	memset(handle_type_data->name,0,HANDLE_NAME_LENGTH);
	memcpy(handle_type_data->name,"any",3);
	lock_init(&(handle_type_data->lock));
	handle_type_data->delete_callback=NULL;
	rb_tree_init(&(handle_type_data->handle_tree));
	handle_type_data->count=0;
	handle_type_data->active_count=0;
	for (const handle_descriptor_t*const* descriptor=(void*)kernel_section_handle_start();(u64)descriptor<kernel_section_handle_end();descriptor++){
		handle_type_data_t* type_data=handle_type_data+(*((*descriptor)->var));
		memcpy_lowercase(type_data->name,(*descriptor)->name,HANDLE_NAME_LENGTH);
		lock_init(&(type_data->lock));
		type_data->delete_callback=(*descriptor)->delete_callback;
		rb_tree_init(&(type_data->handle_tree));
		type_data->count=0;
		type_data->active_count=0;
	}
	for (const handle_descriptor_t*const* descriptor=(void*)kernel_section_handle_start();(u64)descriptor<kernel_section_handle_end();descriptor++){
		handle_type_data_t* type_data=handle_type_data+(*((*descriptor)->var));
		handle_new(type_data,HANDLE_TYPE_HANDLE,&(type_data->handle));
	}
}



void handle_new(void* object,handle_type_t type,handle_t* out){
	if (type==HANDLE_TYPE_ANY||type>=handle_type_count){
		panic("Invalid handle type");
	}
	out->rc=1;
	out->object=object;
	handle_type_data_t* type_data=handle_type_data+type;
	lock_acquire_exclusive(&(type_data->lock));
	out->rb_node.key=HANDLE_ID_CREATE(type,type_data->count);
	type_data->count++;
	type_data->active_count++;
	rb_tree_insert_node_increasing(&(type_data->handle_tree),&(out->rb_node));
	lock_release_exclusive(&(type_data->lock));
	handle_type_data->active_count++;
}



handle_t* handle_lookup_and_acquire(handle_id_t id,handle_type_t type){
	if (type!=HANDLE_TYPE_ANY&&type!=HANDLE_ID_GET_TYPE(id)){
		return NULL;
	}
	handle_type_data_t* type_data=handle_type_data+HANDLE_ID_GET_TYPE(id);
	lock_acquire_shared(&(type_data->lock));
	handle_t* out=(handle_t*)rb_tree_lookup_node(&(type_data->handle_tree),id);
	if (out){
		handle_acquire(out);
	}
	lock_release_shared(&(type_data->lock));
	return out;
}



void _handle_delete_internal(handle_t* handle){
	if (handle->rc){
		return;
	}
	handle_type_data_t* type_data=handle_type_data+HANDLE_ID_GET_TYPE(handle->rb_node.key);
	lock_acquire_exclusive(&(type_data->lock));
	rb_tree_remove_node(&(type_data->handle_tree),&(handle->rb_node));
	lock_release_exclusive(&(type_data->lock));
	handle_type_data->active_count--;
	(handle_type_data+HANDLE_ID_GET_TYPE(handle->rb_node.key))->active_count--;
	(handle_type_data+HANDLE_ID_GET_TYPE(handle->rb_node.key))->delete_callback(handle);
}
