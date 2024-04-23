#ifndef _KERNEL_RESOURCE_RESOURCE_H_
#define _KERNEL_RESOURCE_RESOURCE_H_ 1
#include <kernel/lock/spinlock.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>



typedef u64 resource_t;



typedef struct _RESOURCE_REGION{
	rb_tree_node_t rb_node;
	u64 length;
	bool is_used;
} resource_region_t;



typedef struct _RESOURCE_MANAGER{
	rb_tree_t tree;
	spinlock_t lock;
	resource_t max;
} resource_manager_t;



resource_manager_t* resource_manager_create(resource_t min,resource_t max);



void resource_manager_delete(resource_manager_t* resource_manager);



resource_t resource_alloc(resource_manager_t* resource_manager);



bool resource_dealloc(resource_manager_t* resource_manager,resource_t resource);



bool resource_is_used(resource_manager_t* resource_manager,resource_t resource);



#endif
