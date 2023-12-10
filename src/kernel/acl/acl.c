#include <kernel/acl/acl.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/process.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "acl"



static omm_allocator_t* _acl_allocator=NULL;
static omm_allocator_t* _acl_tree_node_allocator=NULL;



void acl_init(void){
	LOG("Initializing access control lists...");
	_acl_allocator=omm_init("acl",sizeof(acl_t),8,4,pmm_alloc_counter("omm_acl"));
	_acl_tree_node_allocator=omm_init("acl_tree_node",sizeof(rb_tree_node_t),8,4,pmm_alloc_counter("omm_acl_tree_node"));
}



KERNEL_PUBLIC acl_t* acl_create(void){
	acl_t* out=omm_alloc(_acl_allocator);
	spinlock_init(&(out->lock));
	memset(out->cache,0,ACL_PROCESS_CACHE_SIZE*sizeof(handle_id_t));
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



KERNEL_PUBLIC _Bool acl_check(acl_t* acl,process_t* process){
	handle_id_t key=HANDLE_ID_GET_INDEX(process->handle.rb_node.key);
	_Bool out=0;
	spinlock_acquire_exclusive(&(acl->lock));
	if (acl->cache[key&(ACL_PROCESS_CACHE_SIZE-1)]==key){
		out=1;
	}
	else{
		rb_tree_node_t* rb_node=rb_tree_lookup_node(&(acl->tree),key);
		if (rb_node){
			acl->cache[key&(ACL_PROCESS_CACHE_SIZE-1)]=key;
			out=1;
		}
	}
	spinlock_release_exclusive(&(acl->lock));
	return out;
}



KERNEL_PUBLIC void acl_add(acl_t* acl,process_t* process){
	handle_id_t key=HANDLE_ID_GET_INDEX(process->handle.rb_node.key);
	spinlock_acquire_exclusive(&(acl->lock));
	if (!acl->cache[key&(ACL_PROCESS_CACHE_SIZE-1)]){
		acl->cache[key&(ACL_PROCESS_CACHE_SIZE-1)]=key;
	}
	rb_tree_node_t* rb_node=rb_tree_lookup_node(&(acl->tree),key);
	if (!rb_node){
		rb_node=omm_alloc(_acl_tree_node_allocator);
		rb_node->key=key;
		rb_tree_insert_node(&(acl->tree),rb_node);
	}
	spinlock_release_exclusive(&(acl->lock));
}



KERNEL_PUBLIC void acl_remove(acl_t* acl,process_t* process){
	handle_id_t key=HANDLE_ID_GET_INDEX(process->handle.rb_node.key);
	spinlock_acquire_exclusive(&(acl->lock));
	if (acl->cache[key&(ACL_PROCESS_CACHE_SIZE-1)]==key){
		acl->cache[key&(ACL_PROCESS_CACHE_SIZE-1)]=0;
	}
	rb_tree_node_t* rb_node=rb_tree_lookup_node(&(acl->tree),key);
	if (rb_node){
		rb_tree_remove_node(&(acl->tree),rb_node);
		omm_dealloc(_acl_tree_node_allocator,rb_node);
	}
	spinlock_release_exclusive(&(acl->lock));
}
