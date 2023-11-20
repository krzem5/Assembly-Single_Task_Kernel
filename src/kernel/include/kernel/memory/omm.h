#ifndef _KERNEL_MEMORY_OMM_H_
#define _KERNEL_MEMORY_OMM_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/lock/spinlock.h>
#include <kernel/memory/pmm.h>
#include <kernel/types.h>



#define OMM_ALLOCATOR_INIT_STRUCT(name,object_size,alignment,page_count,pmm_counter) \
	(omm_allocator_t){ \
		(name), \
		HANDLE_INIT_STRUCT, \
		SPINLOCK_INIT_STRUCT, \
		((object_size)+(alignment)-1)&(-(alignment)), \
		(alignment), \
		(page_count), \
		(((page_count)<<PAGE_SIZE_SHIFT)-((sizeof(omm_page_header_t)+(alignment)-1)&(-(alignment))))/(((object_size)+(alignment)-1)&(-(alignment))), \
		(pmm_counter), \
		NULL, \
		NULL, \
		NULL, \
		0, \
		0 \
	}



typedef struct _OMM_OBJECT{
	struct _OMM_OBJECT* next;
} omm_object_t;



typedef struct _OMM_PAGE_HEADER{
	struct _OMM_PAGE_HEADER* prev;
	struct _OMM_PAGE_HEADER* next;
	omm_object_t* head;
	u32 object_size;
	u32 used_count;
} omm_page_header_t;



typedef struct _OMM_ALLOCATOR{
	const char* name;
	handle_t handle;
	spinlock_t lock;
	u32 object_size;
	u32 alignment;
	u32 page_count;
	u32 max_used_count;
	pmm_counter_descriptor_t* pmm_counter;
	omm_page_header_t* page_free_head;
	omm_page_header_t* page_used_head;
	omm_page_header_t* page_full_head;
	KERNEL_ATOMIC u64 allocation_count;
	KERNEL_ATOMIC u64 deallocation_count;
} omm_allocator_t;



extern handle_type_t HANDLE_TYPE_OMM_ALLOCATOR;



omm_allocator_t* omm_init(const char* name,u64 object_size,u64 alignment,u64 page_count,pmm_counter_descriptor_t* pmm_counter);



void* omm_alloc(omm_allocator_t* allocator);



void omm_dealloc(omm_allocator_t* allocator,void* object);



#endif
