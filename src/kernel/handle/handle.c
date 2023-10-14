#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "handle"



static handle_t _handle_rb_nil_node={
	.rb_color=0,
	.rb_parent=NULL,
	.rb_prev=NULL,
	.rb_next=NULL
};

handle_type_data_t* handle_type_data;
handle_type_t handle_type_count;



static void _handle_add_node_to_tree(handle_t** root,handle_t* node){
	node->rb_parent=&_handle_rb_nil_node;
	node->rb_prev=NULL;
	node->rb_next=*root;
	if (*root){
		(*root)->rb_prev=node;
	}
	*root=node;
}



static handle_t* _handle_get_node_from_tree(handle_t* root,handle_id_t id){
	for (handle_t* handle=root;handle;handle=handle->rb_next){
		if (handle->id==id){
			return handle;
		}
	}
	return NULL;
}



static void _handle_remove_node_from_tree(handle_t** root,handle_t* node){
	if (node->rb_prev){
		node->rb_prev->rb_next=node->rb_next;
	}
	else{
		*root=node->rb_next;
	}
	if (node->rb_next){
		node->rb_next->rb_prev=node->rb_prev;
	}
}



void handle_init(void){
	LOG("Initializing handle types...");
	handle_type_count=HANDLE_TYPE_ANY+1;
	for (const handle_descriptor_t*const* descriptor=(void*)kernel_section_handle_start();(u64)descriptor<kernel_section_handle_end();descriptor++){
		if (*descriptor){
			*((*descriptor)->var)=handle_type_count;
			handle_type_count++;
		}
	}
	INFO("Handle type count: %u",handle_type_count);
	handle_type_data=kmm_alloc(handle_type_count*sizeof(handle_type_data_t));
	memset(handle_type_data->name,0,HANDLE_NAME_LENGTH);
	memcpy(handle_type_data->name,"any",3);
	lock_init(&(handle_type_data->lock));
	handle_type_data->delete_callback=NULL;
	handle_type_data->rb_root=NULL;
	handle_type_data->count=0;
	handle_type_data->active_count=0;
	for (const handle_descriptor_t*const* descriptor=(void*)kernel_section_handle_start();(u64)descriptor<kernel_section_handle_end();descriptor++){
		if (*descriptor){
			handle_type_data_t* type_data=handle_type_data+(*((*descriptor)->var));
			memcpy_lowercase(type_data->name,(*descriptor)->name,HANDLE_NAME_LENGTH);
			lock_init(&(type_data->lock));
			type_data->delete_callback=(*descriptor)->delete_callback;
			type_data->rb_root=NULL;
			type_data->count=0;
			type_data->active_count=0;
		}
	}
}



void handle_new(void* object,handle_type_t type,handle_t* out){
	if (type==HANDLE_TYPE_ANY||type>=handle_type_count){
		panic("Invalid handle type");
	}
	out->rc=1;
	out->object_offset=((s64)object)-((s64)out);
	if (((u64)out)+out->object_offset!=((u64)object)){
		panic("Wrong object offset");
	}
	handle_type_data_t* type_data=handle_type_data+type;
	lock_init(&(out->lock));
	lock_acquire_exclusive(&(type_data->lock));
	out->id=HANDLE_ID_CREATE(type,type_data->active_count);
	handle_type_data->active_count++;
	type_data->count++;
	type_data->active_count++;
	_handle_add_node_to_tree(&(type_data->rb_root),out);
	lock_release_exclusive(&(type_data->lock));
}



handle_t* handle_lookup_and_acquire(handle_id_t id,handle_type_t type){
	if (type!=HANDLE_TYPE_ANY&&type!=HANDLE_ID_GET_TYPE(id)){
		return NULL;
	}
	handle_type_data_t* type_data=handle_type_data+HANDLE_ID_GET_TYPE(id);
	lock_acquire_shared(&(type_data->lock));
	handle_t* out=_handle_get_node_from_tree(type_data->rb_root,id);
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
	handle_type_data_t* type_data=handle_type_data+HANDLE_ID_GET_TYPE(handle->id);
	lock_acquire_exclusive(&(type_data->lock));
	_handle_remove_node_from_tree(&(type_data->rb_root),handle);
	handle_type_data->active_count--;
	(handle_type_data+HANDLE_ID_GET_TYPE(handle->id))->active_count--;
	(handle_type_data+HANDLE_ID_GET_TYPE(handle->id))->delete_callback(handle);
	lock_release_exclusive(&(type_data->lock));
}
