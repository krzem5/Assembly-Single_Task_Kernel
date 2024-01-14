#ifndef _SYS_HEAP_HEAP_H_
#define _SYS_HEAP_HEAP_H_ 1
#include <sys/types.h>



#define SYS_HEAP_GROW_SIZE 262144

#define SYS_HEAP_BLOCK_HEADER_FLAG_USED 1
#define SYS_HEAP_BLOCK_HEADER_FLAG_DEDICATED 2

#define SYS_HEAP_MIN_ALIGNMENT 16

#define SYS_HEAP_BUCKET_COUNT 12



typedef struct _SYS_HEAP_BLOCK_OFFSET{
	u32 prev_size;
	u32 offset;
} sys_heap_block_offset_t;



typedef struct _SYS_HEAP_BLOCK_HEADER{
	struct _SYS_HEAP_BLOCK_HEADER* prev;
	struct _SYS_HEAP_BLOCK_HEADER* next;
	u64 size_and_flags;
	sys_heap_block_offset_t offset;
	u8 ptr[];
} sys_heap_block_header_t;



typedef struct _SYS_HEAP_BUKET{
	sys_heap_block_header_t* head;
} sys_heap_bucket_t;



typedef struct _SYS_HEAP{
	sys_heap_bucket_t buckets[SYS_HEAP_BUCKET_COUNT];
	u16 bucket_bitmap;
} sys_heap_t;



void __sys_heap_init(void);



void __attribute__((access(write_only,1),nonnull)) sys_heap_init(sys_heap_t* out);



void __attribute__((access(read_write,1),nonnull)) sys_heap_deinit(sys_heap_t* heap);



void* __attribute__((access(read_write,1),alloc_size(2),warn_unused_result)) sys_heap_alloc(sys_heap_t* heap,u64 size);



void* __attribute__((access(read_write,1),alloc_align(2),alloc_size(2),warn_unused_result)) sys_heap_alloc_aligned(sys_heap_t* heap,u64 size,u64 alignment);



void* __attribute__((access(read_write,1),access(read_write,2),alloc_size(3),warn_unused_result)) sys_heap_realloc(sys_heap_t* heap,void* ptr,u64 size);



void __attribute__((access(read_write,1),access(read_write,2))) sys_heap_dealloc(sys_heap_t* heap,void* ptr);



#endif
