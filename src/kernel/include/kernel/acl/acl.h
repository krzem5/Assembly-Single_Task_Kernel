#ifndef _KERNEL_ACL_ACL_H_
#define _KERNEL_ACL_ACL_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/lock/spinlock.h>
#include <kernel/mp/process.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>



#define ACL_PROCESS_CACHE_SIZE 16



typedef struct _ACL{
	spinlock_t lock;
	handle_id_t cache[ACL_PROCESS_CACHE_SIZE];
	rb_tree_t tree;
} acl_t;



void acl_init(void);



acl_t* acl_create(void);



void acl_delete(acl_t* acl);



_Bool acl_check(acl_t* acl,process_t* process);



void acl_add(acl_t* acl,process_t* process);



void acl_remove(acl_t* acl,process_t* process);



#endif
