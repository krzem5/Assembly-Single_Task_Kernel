#include <kernel/handle/handle.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/mp/process.h>
#include <kernel/mp/process_group.h>
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



KERNEL_PUBLIC process_group_t* process_group_create(void){
	process_group_t* out=omm_alloc(_process_group_allocator);
	handle_new(process_group_handle_type,&(out->handle));
	rwlock_init(&(out->lock));
	rb_tree_init(&(out->tree));
	return out;
}



KERNEL_PUBLIC void process_group_join(process_group_t* group,process_t* process){
	rwlock_acquire_write(&(group->lock));
	if (rb_tree_lookup_node(&(group->tree),process->handle.rb_node.key)){
		rwlock_release_write(&(group->lock));
		return;
	}
	process_group_entry_t* entry=omm_alloc(_process_group_entry_allocator);
	entry->rb_node.key=process->handle.rb_node.key;
	rb_tree_insert_node(&(group->tree),&(entry->rb_node));
	rwlock_release_write(&(group->lock));
}



KERNEL_PUBLIC void process_group_leave(process_group_t* group,process_t* process){
	rwlock_acquire_write(&(group->lock));
	process_group_entry_t* entry=(process_group_entry_t*)rb_tree_lookup_node(&(group->tree),process->handle.rb_node.key);
	if (entry){
		rb_tree_remove_node(&(group->tree),&(entry->rb_node));
		omm_dealloc(_process_group_entry_allocator,entry);
	}
	rwlock_release_write(&(group->lock));
}
