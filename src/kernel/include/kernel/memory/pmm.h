#ifndef _KERNEL_MEMORY_PMM_H_
#define _KERNEL_MEMORY_PMM_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/lock/spinlock.h>
#include <kernel/types.h>



#define PAGE_SIZE 4096
#define PAGE_SIZE_SHIFT 12
#define LARGE_PAGE_SIZE 2097152
#define LARGE_PAGE_SIZE_SHIFT 21
#define EXTRA_LARGE_PAGE_SIZE 1073741824
#define EXTRA_LARGE_PAGE_SIZE_SHIFT 30

#define PMM_ALLOCATOR_BLOCK_GROUP_COUNT 14
#define PMM_ALLOCATOR_MAX_REGION_SIZE_SHIFT (PMM_ALLOCATOR_BLOCK_GROUP_COUNT+1+PAGE_SIZE_SHIFT)
#define PMM_ALLOCATOR_MAX_REGION_SIZE (1ull<<PMM_ALLOCATOR_MAX_REGION_SIZE_SHIFT)

#define PMM_LOW_ALLOCATOR_LIMIT 0x40000000ull

#define PMM_MEMORY_HINT_LOW_MEMORY 1

#define PMM_COUNTER_INIT_STRUCT(name) {(name),0,HANDLE_INIT_STRUCT}



typedef struct _PMM_ALLOCATOR_PAGE_HEADER{
	struct _PMM_ALLOCATOR_PAGE_HEADER* prev;
	struct _PMM_ALLOCATOR_PAGE_HEADER* next;
	u8 idx;
	u8 _padding[7];
} pmm_allocator_page_header_t;



typedef struct _PMM_ALLOCATOR_BLOCK_GROUP{
	pmm_allocator_page_header_t* head;
	pmm_allocator_page_header_t* tail;
} pmm_allocator_block_group_t;



typedef struct _PMM_ALLOCATOR{
	pmm_allocator_block_group_t block_groups[PMM_ALLOCATOR_BLOCK_GROUP_COUNT];
	u16 block_group_bitmap;
	spinlock_t lock;
} pmm_allocator_t;



typedef struct _PMM_LOAD_BALANCER_STATS{
	KERNEL_ATOMIC u64 hit_count;
	KERNEL_ATOMIC u64 miss_count;
	KERNEL_ATOMIC u64 miss_locked_count;
} pmm_load_balancer_stats_t;



typedef struct _PMM_LOAD_BALANCER{
	spinlock_t lock;
	u32 index;
	pmm_load_balancer_stats_t stats;
} pmm_load_balancer_t;



typedef struct _PMM_COUNTER_DESCRIPTOR{
	const char* name;
	KERNEL_ATOMIC u64 count;
	handle_t handle;
} pmm_counter_descriptor_t;



extern handle_type_t HANDLE_TYPE_PMM_COUNTER;
extern pmm_load_balancer_stats_t* pmm_load_balancer_stats;



static KERNEL_INLINE u64 pmm_align_up_address(u64 base){
	return (base+PAGE_SIZE-1)&(-PAGE_SIZE);
}



static KERNEL_INLINE u64 pmm_align_up_address_large(u64 base){
	return (base+LARGE_PAGE_SIZE-1)&(-LARGE_PAGE_SIZE);
}



static KERNEL_INLINE u64 pmm_align_up_address_extra_large(u64 base){
	return (base+EXTRA_LARGE_PAGE_SIZE-1)&(-EXTRA_LARGE_PAGE_SIZE);
}



static KERNEL_INLINE u64 pmm_align_down_address(u64 base){
	return base&(-PAGE_SIZE);
}



static KERNEL_INLINE u64 pmm_align_down_address_large(u64 base){
	return base&(-LARGE_PAGE_SIZE);
}



static KERNEL_INLINE u64 pmm_align_down_address_extra_large(u64 base){
	return base&(-EXTRA_LARGE_PAGE_SIZE);
}



void pmm_init(void);



void pmm_init_high_mem(void);



u64 pmm_alloc(u64 count,pmm_counter_descriptor_t* counter,_Bool memory_hint);



void pmm_dealloc(u64 address,u64 count,pmm_counter_descriptor_t* counter);



#endif
