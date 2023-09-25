#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "handle"



static lock_t _handle_global_lock=LOCK_INIT_STRUCT;
static handle_id_t _handle_next_id=1;
static handle_t* _handle_root=NULL;

handle_type_data_t* handle_type_data;
handle_type_t handle_type_count;



void handle_init(void){
	handle_type_count=HANDLE_TYPE_ANY+1;
	for (const handle_descriptor_t*const* descriptor=(void*)(kernel_get_handle_start()+kernel_get_offset());(u64)descriptor<(kernel_get_handle_end()+kernel_get_offset());descriptor++){
		if (*descriptor){
			*((*descriptor)->var)=handle_type_count;
			handle_type_count++;
		}
	}
	handle_type_data=kmm_alloc(handle_type_count*sizeof(handle_type_data_t));
	handle_type_data->name="ANY";
	handle_type_data->delete_fn=NULL;
	handle_type_data->count=0;
	for (const handle_descriptor_t*const* descriptor=(void*)(kernel_get_handle_start()+kernel_get_offset());(u64)descriptor<(kernel_get_handle_end()+kernel_get_offset());descriptor++){
		if (*descriptor){
			handle_type_data_t* type_data=handle_type_data+(*((*descriptor)->var));
			type_data->name=(*descriptor)->name;
			type_data->delete_fn=(*descriptor)->delete_fn;
			type_data->count=0;
		}
	}
}



void handle_new(void* object,handle_type_t type,handle_t* out){
	// add to an AVL tree
	out->type=type;
	lock_init(&(out->lock));
	out->rc=1;
	out->object=object;
	handle_type_data->count++;
	(handle_type_data+type)->count++;
	lock_acquire_exclusive(&_handle_global_lock);
	out->id=_handle_next_id;
	_handle_next_id++;
	out->prev=NULL;
	out->next=_handle_root;
	if (_handle_root){
		_handle_root->prev=out;
	}
	_handle_root=out;
	lock_release_exclusive(&_handle_global_lock);
}



void handle_delete(handle_t* handle){
	if (handle->rc){
		panic("Unable to delete referenced handle");
	}
	lock_acquire_exclusive(&_handle_global_lock);
	if (handle->prev){
		handle->prev->next=handle->next;
	}
	else{
		_handle_root=handle->next;
	}
	if (handle->next){
		handle->next->prev=handle->prev;
	}
	lock_release_exclusive(&_handle_global_lock);
	handle_type_data->count--;
	(handle_type_data+handle->type)->count--;
	(handle_type_data+handle->type)->delete_fn(handle);
}



handle_t* handle_lookup_and_acquire(handle_id_t id,handle_type_t type){
	handle_t* out=NULL;
	lock_acquire_shared(&_handle_global_lock);
	for (handle_t* handle=_handle_root;handle;handle=handle->next){
		if (handle->id==id){
			if (handle->type==type||type==HANDLE_TYPE_ANY){
				handle_acquire(handle);
				out=handle;
			}
			break;
		}
	}
	lock_release_shared(&_handle_global_lock);
	return out;
}
