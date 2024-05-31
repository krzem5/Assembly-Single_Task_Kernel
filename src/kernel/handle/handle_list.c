#include <kernel/handle/handle.h>
#include <kernel/handle/handle_list.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "handle_list"



static omm_allocator_t* KERNEL_INIT_WRITE _handle_list_entry_allocator=NULL;



KERNEL_EARLY_EARLY_INIT(){
	_handle_list_entry_allocator=omm_init("kernel.handle_list.entry",sizeof(rb_tree_node_t),8,4);
}



KERNEL_PUBLIC void handle_list_init(handle_list_t* out){
	rwlock_init(&(out->lock));
	rb_tree_init(&(out->tree));
}



KERNEL_PUBLIC void handle_list_destroy(handle_list_t* list){
	while (1){
		rwlock_acquire_write(&(list->lock));
		rb_tree_node_t* rb_node=rb_tree_iter_start(&(list->tree));
		if (!rb_node){
			rwlock_release_write(&(list->lock));
			return;
		}
		rb_tree_remove_node(&(list->tree),rb_node);
		rwlock_release_write(&(list->lock));
		handle_t* handle=handle_lookup_and_acquire(rb_node->key,HANDLE_TYPE_ANY);
		omm_dealloc(_handle_list_entry_allocator,rb_node);
		if (handle&&handle_release(handle)){
			handle_release(handle);
		}
	}
}



KERNEL_PUBLIC void handle_list_push(handle_list_t* list,handle_t* handle){
	rwlock_acquire_write(&(list->lock));
	rb_tree_node_t* rb_node=omm_alloc(_handle_list_entry_allocator);
	rb_node->key=handle->rb_node.key;
	rb_tree_insert_node(&(list->tree),rb_node);
	rwlock_release_write(&(list->lock));
}
