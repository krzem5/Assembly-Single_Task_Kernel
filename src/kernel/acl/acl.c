#include <kernel/acl/acl.h>
#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/id/flags.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#define KERNEL_LOG_NAME "acl"



static omm_allocator_t* _acl_allocator=NULL;
static omm_allocator_t* _acl_tree_node_allocator=NULL;
static KERNEL_ATOMIC acl_request_callback_t _acl_request_callback=NULL;



void KERNEL_EARLY_EXEC acl_init(void){
	LOG("Initializing access control lists...");
	_acl_allocator=omm_init("acl",sizeof(acl_t),8,4,pmm_alloc_counter("omm_acl"));
	spinlock_init(&(_acl_allocator->lock));
	_acl_tree_node_allocator=omm_init("acl_tree_node",sizeof(acl_tree_node_t),8,4,pmm_alloc_counter("omm_acl_tree_node"));
	spinlock_init(&(_acl_tree_node_allocator->lock));
}



KERNEL_PUBLIC acl_t* acl_create(void){
	acl_t* out=omm_alloc(_acl_allocator);
	spinlock_init(&(out->lock));
	mem_fill(out->cache,ACL_PROCESS_CACHE_SIZE*sizeof(acl_cache_entry_t),0);
	rb_tree_init(&(out->tree));
	return out;
}



KERNEL_PUBLIC void acl_delete(acl_t* acl){
	spinlock_acquire_exclusive(&(acl->lock));
	for (rb_tree_node_t* rb_node=rb_tree_iter_start(&(acl->tree));rb_node;){
		rb_tree_node_t* next_rb_node=rb_tree_iter_next(&(acl->tree),rb_node);
		rb_tree_remove_node(&(acl->tree),rb_node);
		omm_dealloc(_acl_tree_node_allocator,rb_node);
		rb_node=next_rb_node;
	}
	spinlock_release_exclusive(&(acl->lock));
	omm_dealloc(_acl_allocator,acl);
}



KERNEL_PUBLIC u64 acl_get(acl_t* acl,process_t* process){
	u64 key=HANDLE_ID_GET_INDEX(process->handle.rb_node.key);
	if (!key){
		return ACL_PERMISSION_MASK;
	}
	u64 out=0;
	spinlock_acquire_exclusive(&(acl->lock));
	if (acl->cache[key&(ACL_PROCESS_CACHE_SIZE-1)].key==key){
		out=acl->cache[key&(ACL_PROCESS_CACHE_SIZE-1)].flags;
	}
	else{
		acl_tree_node_t* node=(acl_tree_node_t*)rb_tree_lookup_node(&(acl->tree),key);
		if (node){
			out=node->flags;
			acl->cache[key&(ACL_PROCESS_CACHE_SIZE-1)].key=key;
			acl->cache[key&(ACL_PROCESS_CACHE_SIZE-1)].flags=out;
		}
	}
	spinlock_release_exclusive(&(acl->lock));
	return out;
}



KERNEL_PUBLIC void acl_set(acl_t* acl,struct _PROCESS* process,u64 clear,u64 set){
	u64 key=HANDLE_ID_GET_INDEX(process->handle.rb_node.key);
	if (!key){
		return;
	}
	spinlock_acquire_exclusive(&(acl->lock));
	acl_tree_node_t* node=(acl_tree_node_t*)rb_tree_lookup_node(&(acl->tree),key);
	if (!node&&set){
		node=omm_alloc(_acl_tree_node_allocator);
		node->rb_node.key=key;
		node->flags=set;
		rb_tree_insert_node(&(acl->tree),&(node->rb_node));
	}
	else if (node){
		node->flags=(node->flags&(~clear))|set;
		if (!node->flags){
			rb_tree_remove_node(&(acl->tree),&(node->rb_node));
			omm_dealloc(_acl_tree_node_allocator,node);
			node=NULL;
		}
	}
	acl_cache_entry_t* cache_entry=acl->cache+(key&(ACL_PROCESS_CACHE_SIZE-1));
	if (node&&(!cache_entry->key||cache_entry->key==node->rb_node.key)){
		cache_entry->key=node->rb_node.key;
		cache_entry->flags=node->flags;
	}
	else if (!node&&cache_entry->key==key){
		cache_entry->key=0;
	}
	spinlock_release_exclusive(&(acl->lock));
}



KERNEL_PUBLIC _Bool acl_register_request_callback(acl_request_callback_t callback){
	if (callback&&_acl_request_callback){
		ERROR("ACL request callback already registered");
		return 0;
	}
	_acl_request_callback=callback;
	return 1;
}



error_t syscall_acl_get_permissions(handle_id_t handle_id,handle_id_t process_handle_id){
	handle_t* handle=handle_lookup_and_acquire(handle_id,HANDLE_ID_GET_TYPE(handle_id));
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	if (!handle->acl){
		return ERROR_NO_ACL;
	}
	handle_t* process_handle=NULL;
	if (process_handle_id){
		process_handle=handle_lookup_and_acquire(process_handle_id,process_handle_type);
		if (!process_handle){
			handle_release(handle);
			return ERROR_INVALID_HANDLE;
		}
	}
	u64 out=acl_get(handle->acl,(process_handle?process_handle->object:THREAD_DATA->process));
	if (process_handle){
		handle_release(process_handle);
	}
	handle_release(handle);
	return out;
}



error_t syscall_acl_set_permissions(handle_id_t handle_id,handle_id_t process_handle_id,u64 clear,u64 set){
	if (clear&(~ACL_PERMISSION_MASK)){
		return ERROR_INVALID_ARGUMENT(2);
	}
	if (set&(~ACL_PERMISSION_MASK)){
		return ERROR_INVALID_ARGUMENT(3);
	}
	handle_t* handle=handle_lookup_and_acquire(handle_id,HANDLE_ID_GET_TYPE(handle_id));
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	if (!handle->acl){
		return ERROR_NO_ACL;
	}
	handle_t* process_handle=NULL;
	if (process_handle_id){
		process_handle=handle_lookup_and_acquire(process_handle_id,process_handle_type);
		if (!process_handle){
			handle_release(handle);
			return ERROR_INVALID_HANDLE;
		}
	}
	if (!(process_get_id_flags()&ID_FLAG_ROOT_ACL)){
		set&=acl_get(handle->acl,THREAD_DATA->process);
	}
	acl_set(handle->acl,(process_handle?process_handle->object:THREAD_DATA->process),clear,set);
	if (process_handle){
		handle_release(process_handle);
	}
	handle_release(handle);
	return ERROR_OK;
}



error_t syscall_acl_request_permissions(handle_id_t handle_id,handle_id_t process_handle_id,u64 flags){
	if (flags&(~ACL_PERMISSION_MASK)){
		return ERROR_INVALID_ARGUMENT(2);
	}
	handle_t* handle=handle_lookup_and_acquire(handle_id,HANDLE_ID_GET_TYPE(handle_id));
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	if (!handle->acl){
		return ERROR_NO_ACL;
	}
	handle_t* process_handle=NULL;
	if (process_handle_id){
		process_handle=handle_lookup_and_acquire(process_handle_id,process_handle_type);
		if (!process_handle){
			handle_release(handle);
			return ERROR_INVALID_HANDLE;
		}
	}
	process_t* process=(process_handle?process_handle->object:THREAD_DATA->process);
	LOG("'%s' requested permissions '%x' for handle '%s'",process->name->data,flags,handle_get_descriptor(HANDLE_ID_GET_TYPE(handle->rb_node.key))->name);
	acl_request_callback_t callback=_acl_request_callback;
	u64 out=(callback?callback(handle,process,flags):ERROR_DENIED);
	if (out==ERROR_OK){
		acl_set(handle->acl,process,0,flags);
	}
	if (process_handle){
		handle_release(process_handle);
	}
	handle_release(handle);
	return out;
}
