#ifndef _KERNEL_MEMORY_OMM_H_
#define _KERNEL_MEMORY_OMM_H_ 1
#include <kernel/lock/lock.h>
#include <kernel/memory/pmm.h>
#include <kernel/types.h>



#define OMM_COUNTER_NAME_LENGTH 16

#define OMM_ALLOCATOR_INIT_LATER_STRUCT {LOCK_INIT_STRUCT,0,NULL}
#define OMM_ALLOCATOR_IS_UNINITIALISED(allocator) (!(allocator)->object_size)

#define OMM_ALLOCATOR_INIT_STRUCT(name,object_size,alignment,page_count,memory_counter) \
	(omm_allocator_t){ \
		LOCK_INIT_STRUCT, \
		((object_size)+alignment-1)&(-(alignment)), \
		(void*)1, \
		(alignment), \
		(page_count), \
		(((page_count)<<PAGE_SIZE_SHIFT)-((sizeof(omm_page_header_t)+alignment-1)&(-alignment)))/(((object_size)+alignment-1)&(-(alignment))), \
		&(memory_counter), \
		NULL, \
		NULL, \
		NULL, \
		{ \
			(name), \
			0, \
			0 \
		} \
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



typedef struct _OMM_COUNTER{
	char name[OMM_COUNTER_NAME_LENGTH];
	u64 allocation_count;
	u64 deallocation_count;
} omm_counter_t;



typedef struct _OMM_ALLOCATOR{
	lock_t lock;
	u32 object_size;
	struct _OMM_ALLOCATOR* next_allocator;
	u32 alignment;
	u32 page_count;
	u32 max_used_count;
	u16* memory_counter;
	omm_page_header_t* page_free_head;
	omm_page_header_t* page_used_head;
	omm_page_header_t* page_full_head;
	omm_counter_t counter;
} omm_allocator_t;



extern u32 omm_allocator_count;
extern omm_allocator_t* omm_head_allocator;



void* omm_alloc(omm_allocator_t* allocator);



void omm_dealloc(omm_allocator_t* allocator,void* object);



#endif
