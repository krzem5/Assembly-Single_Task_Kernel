#ifndef _KERNEL_ACL_ACL_H_
#define _KERNEL_ACL_ACL_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/lock/spinlock.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>



#define ACL_PROCESS_CACHE_SIZE 16 // power of 2



struct _PROCESS;



typedef struct _ACL_CACHE_ENTRY{
	u64 key;
	u64 flags;
} acl_cache_entry_t;



typedef struct _ACL_TREE_NODE{
	rb_tree_node_t rb_node;
	u64 flags;
} acl_tree_node_t;



typedef struct _ACL{
	spinlock_t lock;
	acl_cache_entry_t cache[ACL_PROCESS_CACHE_SIZE];
	rb_tree_t tree;
} acl_t;



void acl_init(void);



acl_t* acl_create(void);



void acl_delete(acl_t* acl);



u64 acl_get(acl_t* acl,struct _PROCESS* process);



void acl_add(acl_t* acl,struct _PROCESS* process,u64 flags);



void acl_remove(acl_t* acl,struct _PROCESS* process,u64 flags);



#endif
