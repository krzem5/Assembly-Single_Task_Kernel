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
	memset(out->cache,0,ACL_PROCESS_CACHE_SIZE*sizeof(acl_cache_entry_t));
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
		return 0xffffffffffffffffull;
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



KERNEL_PUBLIC void acl_add(acl_t* acl,process_t* process,u64 flags){
	u64 key=HANDLE_ID_GET_INDEX(process->handle.rb_node.key);
	if (!key){
		return;
	}
	spinlock_acquire_exclusive(&(acl->lock));
	acl_tree_node_t* node=(acl_tree_node_t*)rb_tree_lookup_node(&(acl->tree),key);
	if (!node){
		node=omm_alloc(_acl_tree_node_allocator);
		node->rb_node.key=key;
		node->flags=0;
		rb_tree_insert_node(&(acl->tree),&(node->rb_node));
	}
	node->flags|=flags;
	if (!acl->cache[key&(ACL_PROCESS_CACHE_SIZE-1)].key){
		acl->cache[key&(ACL_PROCESS_CACHE_SIZE-1)].key=key;
		acl->cache[key&(ACL_PROCESS_CACHE_SIZE-1)].flags=node->flags;
	}
	spinlock_release_exclusive(&(acl->lock));
}



KERNEL_PUBLIC void acl_remove(acl_t* acl,process_t* process,u64 flags){
	u64 key=HANDLE_ID_GET_INDEX(process->handle.rb_node.key);
	if (!key){
		return;
	}
	spinlock_acquire_exclusive(&(acl->lock));
	if (acl->cache[key&(ACL_PROCESS_CACHE_SIZE-1)].key==key){
		acl->cache[key&(ACL_PROCESS_CACHE_SIZE-1)].flags&=~flags;
		if (!acl->cache[key&(ACL_PROCESS_CACHE_SIZE-1)].flags){
			acl->cache[key&(ACL_PROCESS_CACHE_SIZE-1)].key=0;
		}
	}
	acl_tree_node_t* node=(acl_tree_node_t*)rb_tree_lookup_node(&(acl->tree),key);
	if (node){
		node->flags&=~flags;
		if (!node->flags){
			rb_tree_remove_node(&(acl->tree),&(node->rb_node));
			omm_dealloc(_acl_tree_node_allocator,node);
		}
	}
	spinlock_release_exclusive(&(acl->lock));
}
