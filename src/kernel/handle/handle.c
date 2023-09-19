#include <kernel/handle/handle.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "handle"



static lock_t _handle_global_lock=LOCK_INIT_STRUCT;
static handle_id_t _handle_next_id=1;
static handle_t* _handle_root=NULL;



void handle_new(void* object,u8 type,handle_t* out){
	// add to an AVL tree
	out->type=type;
	lock_init(&(out->lock));
	out->rc=1;
	out->object=object;
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
		panic("Unable to delete referenced handle",0);
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
	switch (handle->type){
		case HANDLE_TYPE_ANY:
			ERROR("any_delete");
			break;
		case HANDLE_TYPE_EVENT:
			event_delete(handle->object);
			break;
		case HANDLE_TYPE_THREAD:
			thread_delete(handle->object);
			break;
		case HANDLE_TYPE_PROCESS:
			process_delete(handle->object);
			break;
	}
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
