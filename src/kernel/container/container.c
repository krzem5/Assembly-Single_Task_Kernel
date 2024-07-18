#include <kernel/acl/acl.h>
#include <kernel/container/container.h>
#include <kernel/exception/exception.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/mutex.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/syscall/syscall.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#define KERNEL_LOG_NAME "container"



static omm_allocator_t* KERNEL_INIT_WRITE _container_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _container_entry_allocator=NULL;
static handle_type_t KERNEL_INIT_WRITE _container_handle_type=0;



static void _container_handle_destructor(handle_t* handle){
	container_t* data=KERNEL_CONTAINEROF(handle,container_t,handle);
	while (1){
		container_entry_t* entry=(container_entry_t*)rb_tree_iter_start(&(data->tree));
		if (!entry){
			break;
		}
		rb_tree_remove_node(&(data->tree),&(entry->rb_node));
		handle_t* handle=handle_lookup_and_acquire(entry->rb_node.key,HANDLE_TYPE_ANY);
		if (handle&&handle_release(handle)){
			handle_release(handle);
		}
		omm_dealloc(_container_entry_allocator,entry);
	}
	mutex_delete(data->lock);
	omm_dealloc(_container_allocator,data);
}



KERNEL_INIT(){
	LOG("Initializing containers...");
	_container_allocator=omm_init("kernel.container",sizeof(container_t),8,4);
	rwlock_init(&(_container_allocator->lock));
	_container_entry_allocator=omm_init("kernel.container.entry",sizeof(container_entry_t),8,4);
	rwlock_init(&(_container_entry_allocator->lock));
	_container_handle_type=handle_alloc("kernel.container",0,_container_handle_destructor);
}



error_t syscall_container_create(void){
	container_t* out=omm_alloc(_container_allocator);
	handle_new(_container_handle_type,&(out->handle));
	out->handle.acl=acl_create();
	acl_set(out->handle.acl,THREAD_DATA->process,0,CONTAINER_ACL_FLAG_ACCESS|CONTAINER_ACL_FLAG_DELETE);
	handle_list_push(&(THREAD_DATA->process->handle_list),&(out->handle));
	out->lock=mutex_create("kernel.container");
	rb_tree_init(&(out->tree));
	return out->handle.rb_node.key;
}



error_t syscall_container_delete(handle_id_t container){
	handle_t* container_handle=handle_lookup_and_acquire(container,_container_handle_type);
	if (!container_handle){
		return ERROR_INVALID_HANDLE;
	}
	if (!(acl_get(container_handle->acl,THREAD_DATA->process)&CONTAINER_ACL_FLAG_DELETE)){
		handle_release(container_handle);
		return ERROR_DENIED;
	}
	handle_release(container_handle);
	handle_release(container_handle);
	return ERROR_OK;
}



KERNEL_AWAITS error_t syscall_container_add(handle_id_t container,KERNEL_USER_POINTER const handle_id_t* handles,u64 handle_count){
	handle_t* container_handle=handle_lookup_and_acquire(container,_container_handle_type);
	if (!container_handle){
		return ERROR_INVALID_HANDLE;
	}
	if (!(acl_get(container_handle->acl,THREAD_DATA->process)&CONTAINER_ACL_FLAG_DELETE)){
		handle_release(container_handle);
		return ERROR_DENIED;
	}
	if (!handle_count){
		handle_release(container_handle);
		return ERROR_INVALID_ARGUMENT(2);
	}
	if (handle_count*sizeof(handle_id_t)>syscall_get_user_pointer_max_length((const void*)handles)){
		handle_release(container_handle);
		return ERROR_INVALID_ARGUMENT(1);
	}
	handle_id_t* buffer=amm_alloc(handle_count*sizeof(handle_id_t));
	mem_copy(buffer,(const void*)handles,handle_count*sizeof(handle_id_t));
	container_t* data=KERNEL_CONTAINEROF(container_handle,container_t,handle);
	exception_unwind_push(container_handle,buffer){
		container_t* data=KERNEL_CONTAINEROF(EXCEPTION_UNWIND_ARG(0),container_t,handle);
		mutex_release(data->lock);
		mutex_release(EXCEPTION_UNWIND_ARG(0));
		amm_dealloc(EXCEPTION_UNWIND_ARG(1));
	}
	mutex_acquire(data->lock);
	error_t out=ERROR_OK;
	for (u64 i=0;i<handle_count;i++){
		if (rb_tree_lookup_node(&(data->tree),buffer[i])){
			out=ERROR_ALREADY_PRESENT;
			continue;
		}
		const handle_descriptor_t* descriptor=handle_get_descriptor(HANDLE_ID_GET_TYPE(buffer[i]));
		if (!descriptor||!(descriptor->flags&HANDLE_DESCRIPTOR_FLAG_ALLOW_CONTAINER)){
			out=ERROR_UNSUPPORTED_OPERATION;
			continue;
		}
		handle_t* handle=handle_lookup_and_acquire(buffer[i],HANDLE_TYPE_ANY);
		if (!handle){
			out=ERROR_INVALID_HANDLE;
			continue;
		}
		container_entry_t* entry=omm_alloc(_container_entry_allocator);
		entry->rb_node.key=buffer[i];
		rb_tree_insert_node(&(data->tree),&(entry->rb_node));
	}
	exception_unwind_pop();
	mutex_release(data->lock);
	handle_release(container_handle);
	amm_dealloc(buffer);
	return out;
}
