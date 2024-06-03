#ifndef _KERNEL_CONTAINER_CONTAINER_H_
#define _KERNEL_CONTAINER_CONTAINER_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/lock/mutex.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>



#define CONTAINER_ACL_FLAG_ACCESS 1
#define CONTAINER_ACL_FLAG_DELETE 2



typedef struct _CONTAINER_ENTRY{
	rb_tree_node_t rb_node;
} container_entry_t;



typedef struct _CONTAINER{
	handle_t handle;
	mutex_t* lock;
	rb_tree_t tree;
} container_t;



#endif
