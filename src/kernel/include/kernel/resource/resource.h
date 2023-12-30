#ifndef _KERNEL_RESOURCE_RESOURCE_H_
#define _KERNEL_RESOURCE_RESOURCE_H_ 1
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>



typedef u32 resource_t;



typedef struct _RESOURCE_REGION{
	rb_tree_node_t rb_node;
	u32 length;
	_Bool is_used;
} resource_region_t;



typedef struct _RESOURCE_MANAGER{
	rb_tree_t tree;
} resource_manager_t;



resource_manager_t* resource_manager_create(resource_t min,resource_t max);



void resource_manager_delete(resource_manager_t* resource_manager);



resource_t resource_alloc(resource_manager_t* resource_manager);



_Bool resource_dealloc(resource_manager_t* resource_manager,resource_t resource);



#endif
