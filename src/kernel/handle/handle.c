#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "handle"



// static lock_t _handle_global_lock=LOCK_INIT_STRUCT;
// static handle_id_t _handle_next_id=1;
// static handle_t* _handle_root=NULL;

handle_type_data_t* handle_type_data;
handle_type_t handle_type_count;



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
	handle_type_data->root=NULL;
	handle_type_data->count=0;
	handle_type_data->active_count=0;
	for (const handle_descriptor_t*const* descriptor=(void*)kernel_section_handle_start();(u64)descriptor<kernel_section_handle_end();descriptor++){
		if (*descriptor){
			handle_type_data_t* type_data=handle_type_data+(*((*descriptor)->var));
			memcpy_lowercase(type_data->name,(*descriptor)->name,HANDLE_NAME_LENGTH);
			lock_init(&(type_data->lock));
			type_data->delete_callback=(*descriptor)->delete_callback;
			type_data->root=NULL;
			type_data->count=0;
			type_data->active_count=0;
		}
	}
}



void handle_new(void* object,handle_type_t type,handle_t* out){
	// add to an AVL tree
	if (type==HANDLE_TYPE_ANY||type>=handle_type_count){
		panic("Invalid handle type");
	}
	handle_type_data_t* type_data=handle_type_data+type;
	lock_init(&(out->lock));
	out->rc=1;
	out->object=object;
	lock_acquire_exclusive(&(type_data->lock));
	out->id=HANDLE_ID_CREATE(type,type_data->active_count);
	handle_type_data->active_count++;
	type_data->count++;
	type_data->active_count++;
	out->prev=NULL;
	out->next=type_data->root;
	if (type_data->root){
		type_data->root->prev=out;
	}
	type_data->root=out;
	lock_release_exclusive(&(type_data->lock));
	// lock_init(&(out->lock));
	// out->type=type;
	// out->rc=1;
	// out->object=object;
	// handle_type_data->count++;
	// (handle_type_data+type)->count++;
	// lock_acquire_exclusive(&_handle_global_lock);
	// out->id=_handle_next_id;
	// _handle_next_id++;
	// out->prev=NULL;
	// out->next=_handle_root;
	// if (_handle_root){
	// 	_handle_root->prev=out;
	// }
	// _handle_root=out;
	// lock_release_exclusive(&_handle_global_lock);
}



handle_t* handle_lookup_and_acquire(handle_id_t id,handle_type_t type){
	if (type!=HANDLE_TYPE_ANY&&type!=HANDLE_ID_GET_TYPE(id)){
		return NULL;
	}
	handle_type_data_t* type_data=handle_type_data+HANDLE_ID_GET_TYPE(id);
	handle_t* out=NULL;
	lock_acquire_shared(&(type_data->lock));
	for (handle_t* handle=type_data->root;handle;handle=handle->next){
		if (handle->id==id){
			handle_acquire(handle);
			out=handle;
			break;
		}
	}
	lock_release_shared(&(type_data->lock));
	return out;
}



void _handle_delete_internal(handle_t* handle){
	if (handle->rc){
		panic("Unable to delete referenced handle");
	}
	handle_type_data_t* type_data=handle_type_data+HANDLE_ID_GET_TYPE(handle->id);
	lock_acquire_exclusive(&(type_data->lock));
	if (handle->prev){
		handle->prev->next=handle->next;
	}
	else{
		type_data->root=handle->next;
	}
	if (handle->next){
		handle->next->prev=handle->prev;
	}
	handle_type_data->active_count--;
	(handle_type_data+HANDLE_ID_GET_TYPE(handle->id))->active_count--;
	(handle_type_data+HANDLE_ID_GET_TYPE(handle->id))->delete_callback(handle);
	lock_release_exclusive(&(type_data->lock));
}
