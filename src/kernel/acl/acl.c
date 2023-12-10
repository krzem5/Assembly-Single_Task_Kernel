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



KERNEL_PUBLIC void acl_delete(acl_t* acl);



KERNEL_PUBLIC _Bool acl_check(acl_t* acl,process_t* process);



KERNEL_PUBLIC void acl_add(acl_t* acl,process_t* process);



KERNEL_PUBLIC void acl_remove(acl_t* acl,process_t* process);
