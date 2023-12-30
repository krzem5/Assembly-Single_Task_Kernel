#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/resource/resource.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "resource"



static omm_allocator_t* _resource_manager_allocator=NULL;
static omm_allocator_t* _resource_region_allocator=NULL;



KERNEL_INIT(){
	LOG("Initializing resources...");
	_resource_manager_allocator=omm_init("resource_manager",sizeof(resource_manager_t),8,1,pmm_alloc_counter("omm_resource_manager"));
	spinlock_init(&(_resource_manager_allocator->lock));
	_resource_region_allocator=omm_init("resource_region",sizeof(resource_region_t),8,1,pmm_alloc_counter("omm_resource_region"));
	spinlock_init(&(_resource_region_allocator->lock));
}



KERNEL_PUBLIC resource_manager_t* resource_manager_create(resource_t min,resource_t max){
	if (!min){
		min=1;
	}
	if (min>=max){
		panic("resource_manager_create: invalid arguments");
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



KERNEL_PUBLIC void resource_manager_delete(resource_manager_t* resource_manager);



KERNEL_PUBLIC resource_t resource_alloc(resource_manager_t* resource_manager){
	spinlock_acquire_exclusive(&(resource_manager->lock));
	resource_region_t* prev_region=NULL;
	resource_region_t* region=(void*)rb_tree_lookup_increasing_node(&(resource_manager->tree),0);
	if (region->is_used){
		prev_region=region;
		region=(void*)rb_tree_lookup_node(&(resource_manager->tree),region->rb_node.key+region->length);
		if (!region){
			spinlock_release_exclusive(&(resource_manager->lock));
			return 0;
		}
		KERNEL_ASSERT(region->is_used);
	}
	KERNEL_ASSERT(region->length);
	resource_t out=region->rb_node.key;
	rb_tree_remove_node(&(resource_manager->tree),&(region->rb_node));
	if (!prev_region){
		prev_region=omm_alloc(_resource_region_allocator);
		prev_region->rb_node.key=out;
		prev_region->length=0;
		prev_region->is_used=1;
	}
	prev_region->length++;
	region->rb_node.key++;
	region->length--;
	if (region->length){
		rb_tree_insert_node(&(resource_manager->tree),&(region->rb_node));
	}
	else{
		omm_dealloc(_resource_region_allocator,region);
		if (out+1<resource_manager->max){
			resource_region_t* next_region=(void*)rb_tree_lookup_node(&(resource_manager->tree),out+1);
			KERNEL_ASSERT(next_region);
			KERNEL_ASSERT(next_region->is_used);
			rb_tree_remove_node(&(resource_manager->tree),&(next_region->rb_node));
			prev_region->length+=next_region->length;
			omm_dealloc(_resource_region_allocator,next_region);
		}
	}
	spinlock_release_exclusive(&(resource_manager->lock));
	return out;
}



KERNEL_PUBLIC _Bool resource_dealloc(resource_manager_t* resource_manager,resource_t resource);
