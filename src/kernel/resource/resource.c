#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/resource/resource.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "resource"



static omm_allocator_t* _resource_manager_allocator=NULL;
static omm_allocator_t* _resource_region_allocator=NULL;



KERNEL_INIT(){
	LOG("Initializing resources...");
	_resource_manager_allocator=omm_init("resource_manager",sizeof(resource_manager_t),8,1);
	spinlock_init(&(_resource_manager_allocator->lock));
	_resource_region_allocator=omm_init("resource_region",sizeof(resource_region_t),8,1);
	spinlock_init(&(_resource_region_allocator->lock));
}



KERNEL_PUBLIC resource_manager_t* resource_manager_create(resource_t min,resource_t max){
	if (!min){
		min=1;
	}
	if (min>=max){
		ERROR("resource_manager_create: invalid arguments");
		return NULL;
	}
	resource_manager_t* out=omm_alloc(_resource_manager_allocator);
	rb_tree_init(&(out->tree));
	spinlock_init(&(out->lock));
	out->max=max;
	resource_region_t* region=omm_alloc(_resource_region_allocator);
	region->rb_node.key=min;
	region->length=max-min+1;
	region->is_used=0;
	rb_tree_insert_node(&(out->tree),&(region->rb_node));
	return out;
}



KERNEL_PUBLIC void resource_manager_delete(resource_manager_t* resource_manager){
	for (resource_t value=1;value<=resource_manager->max;){
		resource_region_t* region=(resource_region_t*)rb_tree_lookup_increasing_node(&(resource_manager->tree),value);
		value=region->rb_node.key+region->length;
		rb_tree_remove_node(&(resource_manager->tree),&(region->rb_node));
		omm_dealloc(_resource_region_allocator,region);
	}
	omm_dealloc(_resource_manager_allocator,resource_manager);
}



KERNEL_PUBLIC resource_t resource_alloc(resource_manager_t* resource_manager){
	spinlock_acquire_exclusive(&(resource_manager->lock));
	resource_region_t* prev_region=NULL;
	resource_region_t* region=(void*)rb_tree_lookup_increasing_node(&(resource_manager->tree),1);
	if (region->is_used){
		prev_region=region;
		region=(void*)rb_tree_lookup_node(&(resource_manager->tree),region->rb_node.key+region->length);
		if (!region){
			spinlock_release_exclusive(&(resource_manager->lock));
			return 0;
		}
		KERNEL_ASSERT(!region->is_used);
	}
	resource_t out=region->rb_node.key;
	rb_tree_remove_node(&(resource_manager->tree),&(region->rb_node));
	if (!prev_region){
		prev_region=omm_alloc(_resource_region_allocator);
		prev_region->rb_node.key=out;
		prev_region->length=0;
		prev_region->is_used=1;
		rb_tree_insert_node(&(resource_manager->tree),&(prev_region->rb_node));
	}
	prev_region->length++;
	region->length--;
	if (region->length){
		region->rb_node.key++;
		rb_tree_insert_node(&(resource_manager->tree),&(region->rb_node));
	}
	else{
		omm_dealloc(_resource_region_allocator,region);
		resource_region_t* next_region=(void*)rb_tree_lookup_node(&(resource_manager->tree),out+1);
		if (next_region){
			rb_tree_remove_node(&(resource_manager->tree),&(next_region->rb_node));
			prev_region->length+=next_region->length;
			omm_dealloc(_resource_region_allocator,next_region);
		}
	}
	spinlock_release_exclusive(&(resource_manager->lock));
	return out;
}



KERNEL_PUBLIC _Bool resource_dealloc(resource_manager_t* resource_manager,resource_t resource){
	if (!resource||resource>resource_manager->max){
		return 0;
	}
	spinlock_acquire_exclusive(&(resource_manager->lock));
	resource_region_t* region=(void*)rb_tree_lookup_decreasing_node(&(resource_manager->tree),resource);
	if (!region||!region->is_used){
		spinlock_release_exclusive(&(resource_manager->lock));
		return 0;
	}
	KERNEL_ASSERT(region->rb_node.key+region->length>resource);
	if (resource==region->rb_node.key){
		rb_tree_remove_node(&(resource_manager->tree),&(region->rb_node));
		resource_region_t* prev_region=(void*)rb_tree_lookup_decreasing_node(&(resource_manager->tree),resource-1);
		if (prev_region){
			prev_region->length++;
		}
		else{
			prev_region=omm_alloc(_resource_region_allocator);
			prev_region->rb_node.key=resource;
			prev_region->length=1;
			prev_region->is_used=0;
			rb_tree_insert_node(&(resource_manager->tree),&(prev_region->rb_node));
		}
		region->length--;
		if (region->length){
			region->rb_node.key++;
			rb_tree_insert_node(&(resource_manager->tree),&(region->rb_node));
		}
		else{
			omm_dealloc(_resource_region_allocator,region);
			region=(void*)rb_tree_lookup_increasing_node(&(resource_manager->tree),resource+1);
			if (region){
				rb_tree_remove_node(&(resource_manager->tree),&(region->rb_node));
				prev_region->length+=region->length;
				omm_dealloc(_resource_region_allocator,region);
			}
		}
	}
	else if (resource==region->rb_node.key+region->length-1){
		resource_region_t* next_region=(void*)rb_tree_lookup_node(&(resource_manager->tree),resource+1);
		if (next_region){
			rb_tree_remove_node(&(resource_manager->tree),&(next_region->rb_node));
		}
		else{
			next_region=omm_alloc(_resource_region_allocator);
			next_region->length=0;
			next_region->is_used=0;
		}
		next_region->rb_node.key=resource;
		next_region->length++;
		rb_tree_insert_node(&(resource_manager->tree),&(next_region->rb_node));
		region->length--;
		KERNEL_ASSERT(region->length);
	}
	else{
		resource_region_t* new_unused_region=omm_alloc(_resource_region_allocator);
		new_unused_region->rb_node.key=resource;
		new_unused_region->length=1;
		new_unused_region->is_used=0;
		rb_tree_insert_node(&(resource_manager->tree),&(new_unused_region->rb_node));
		resource_region_t* new_used_region=omm_alloc(_resource_region_allocator);
		new_used_region->rb_node.key=resource+1;
		new_used_region->length=region->rb_node.key+region->length-resource-1;
		new_used_region->is_used=1;
		rb_tree_insert_node(&(resource_manager->tree),&(new_used_region->rb_node));
		region->length=resource-region->rb_node.key;
	}
	spinlock_release_exclusive(&(resource_manager->lock));
	return 1;
}



KERNEL_PUBLIC _Bool resource_is_used(resource_manager_t* resource_manager,resource_t resource){
	spinlock_acquire_shared(&(resource_manager->lock));
	resource_region_t* region=(void*)rb_tree_lookup_decreasing_node(&(resource_manager->tree),resource);
	_Bool out=(region&&region->is_used);
	spinlock_release_shared(&(resource_manager->lock));
	return out;
}
