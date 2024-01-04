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

#define PMM_ALLOCATOR_BUCKET_COUNT 16
#define PMM_ALLOCATOR_MAX_REGION_SIZE_SHIFT (PMM_ALLOCATOR_BUCKET_COUNT+PAGE_SIZE_SHIFT)
#define PMM_ALLOCATOR_MAX_REGION_SIZE (1ull<<PMM_ALLOCATOR_MAX_REGION_SIZE_SHIFT)

#define PMM_LOW_ALLOCATOR_LIMIT 0x40000000ull

#define PMM_MEMORY_HINT_LOW_MEMORY 1

#define _PMM_COUNTER_INIT_STRUCT(name) {(name),0,HANDLE_INIT_STRUCT}



typedef struct _PMM_BLOCK_DESCRIPTOR{
	u64 data[2];
} pmm_block_descriptor_t;



typedef struct _PMM_ALLOCATOR_BUCKET{
	u64 head;
	u64 tail;
} pmm_allocator_bucket_t;



typedef struct _PMM_ALLOCATOR{
	pmm_allocator_bucket_t buckets[PMM_ALLOCATOR_BUCKET_COUNT];
	u16 bucket_bitmap;
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



extern handle_type_t pmm_counter_handle_type;
extern const pmm_load_balancer_stats_t* pmm_load_balancer_stats;



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



pmm_counter_descriptor_t* pmm_alloc_counter(const char* name);



u64 pmm_alloc(u64 count,pmm_counter_descriptor_t* counter,_Bool memory_hint);



void pmm_dealloc(u64 address,u64 count,pmm_counter_descriptor_t* counter);



#endif
