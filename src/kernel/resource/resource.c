#include <kernel/resource/resource.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
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



KERNEL_PUBLIC resource_manager_t* resource_manager_create(resource_t min,resource_t max);



KERNEL_PUBLIC void resource_manager_delete(resource_manager_t* resource_manager);



KERNEL_PUBLIC resource_t resource_alloc(resource_manager_t* resource_manager);



KERNEL_PUBLIC _Bool resource_dealloc(resource_manager_t* resource_manager,resource_t resource);
