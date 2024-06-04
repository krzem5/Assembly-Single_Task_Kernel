#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/mp/process.h>
#include <kernel/mp/process_group.h>
#include <kernel/mp/thread.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "process_group"



static omm_allocator_t* KERNEL_INIT_WRITE _process_group_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _process_group_entry_allocator=NULL;

KERNEL_PUBLIC handle_type_t KERNEL_INIT_WRITE process_group_handle_type;



static void _process_group_handle_destructor(handle_t* handle){
	process_group_t* process_group=KERNEL_CONTAINEROF(handle,process_group_t,handle);
	if (process_group->tree.root){
		panic("Nonempty process group not referenced");
	}
	omm_dealloc(_process_group_allocator,process_group);
}



KERNEL_EARLY_EARLY_INIT(){
	_process_group_allocator=omm_init("kernel.process.group",sizeof(process_group_t),8,2);
	rwlock_init(&(_process_group_allocator->lock));
	_process_group_entry_allocator=omm_init("kernel.process.group.entry",sizeof(process_group_entry_t),8,2);
	rwlock_init(&(_process_group_entry_allocator->lock));
	process_group_handle_type=handle_alloc("kernel.process.group",HANDLE_DESCRIPTOR_FLAG_ALLOW_CONTAINER,_process_group_handle_destructor);
}



KERNEL_PUBLIC process_group_t* process_group_create(process_t* process){
	process_group_t* out=omm_alloc(_process_group_allocator);
	handle_new(process_group_handle_type,&(out->handle));
	rwlock_init(&(out->lock));
	rb_tree_init(&(out->tree));
	process_group_join(out,process);
	return out;
}



KERNEL_PUBLIC void process_group_join(process_group_t* group,process_t* process){
	if (process->process_group){
		process_group_leave(process->process_group,process);
	}
	rwlock_acquire_write(&(group->lock));
	if (rb_tree_lookup_node(&(group->tree),process->handle.rb_node.key)){
		panic("process_group_join: racing condition");
	}
	handle_acquire(&(group->handle));
	process_group_entry_t* entry=omm_alloc(_process_group_entry_allocator);
	entry->rb_node.key=process->handle.rb_node.key;
	rb_tree_insert_node(&(group->tree),&(entry->rb_node));
	process->process_group=group;
	rwlock_release_write(&(group->lock));
}



KERNEL_PUBLIC void process_group_leave(process_group_t* group,process_t* process){
	rwlock_acquire_write(&(group->lock));
	process_group_entry_t* entry=(process_group_entry_t*)rb_tree_lookup_node(&(group->tree),process->handle.rb_node.key);
	if (!entry){
		rwlock_release_write(&(group->lock));
		return;
	}
	rb_tree_remove_node(&(group->tree),&(entry->rb_node));
	omm_dealloc(_process_group_entry_allocator,entry);
	process->process_group=NULL;
	rwlock_release_write(&(group->lock));
	handle_release(&(group->handle));
}



error_t syscall_process_group_get(handle_id_t process_handle){
	if (!process_handle){
		process_handle=THREAD_DATA->process->handle.rb_node.key;
	}
	handle_t* handle=handle_lookup_and_acquire(process_handle,process_handle_type);
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	process_group_t* group=KERNEL_CONTAINEROF(handle,process_t,handle)->process_group;
	error_t out=(group?group->handle.rb_node.key:0);
	handle_release(handle);
	return out;
}



error_t syscall_process_group_set(handle_id_t process_handle,handle_id_t process_group_handle){
	if (!process_handle){
		process_handle=THREAD_DATA->process->handle.rb_node.key;
	}
	handle_t* handle=handle_lookup_and_acquire(process_handle,process_handle_type);
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	process_t* process=KERNEL_CONTAINEROF(handle,process_t,handle);
	panic("syscall_process_group_set");(void)process;
	handle_release(handle);
	return ERROR_OK;
}



error_t syscall_process_group_get_next(handle_id_t process_group){
	handle_descriptor_t* handle_descriptor=handle_get_descriptor(process_group_handle_type);
	rb_tree_node_t* rb_node=rb_tree_lookup_increasing_node(&(handle_descriptor->tree),(process_group?process_group+1:0));
	return (rb_node?rb_node->key:0);
}



error_t syscall_process_group_iter(handle_id_t process_group_handle,handle_id_t process){
	handle_t* handle=handle_lookup_and_acquire(process_group_handle,process_group_handle_type);
	if (!handle){
		return 0;
	}
	process_group_t* process_group=KERNEL_CONTAINEROF(handle,process_group_t,handle);
	rwlock_acquire_read(&(process_group->lock));
	rb_tree_node_t* rb_node=rb_tree_lookup_increasing_node(&(process_group->tree),(process?process+1:0));
	error_t out=(rb_node?rb_node->key:0);
	rwlock_release_read(&(process_group->lock));
	handle_release(handle);
	return out;
}
