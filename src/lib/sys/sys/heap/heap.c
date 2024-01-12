#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/memory/memory.h>
#include <sys/mp/thread.h>
#include <sys/types.h>



static sys_heap_t _sys_heap_default;



static inline void sys_heap_block_header_init(u64 size,u32 flags,sys_heap_block_header_t* header){
	header->prev=NULL;
	header->next=NULL;
	header->size_and_flags=size|flags;
	header->offset.offset=sizeof(sys_heap_block_header_t);
}



static inline u32 _get_bucket_index(u64 size){
	return 31-__builtin_clz(size)+(!!(size&(size-1)))-__builtin_ffs(SYS_HEAP_MIN_ALIGNMENT);
}



void __sys_heap_init(void){
	sys_heap_init(&_sys_heap_default);
}



SYS_PUBLIC void sys_heap_init(sys_heap_t* out){
	for (u32 i=0;i<SYS_HEAP_BUCKET_COUNT;i++){
		out->buckets[i].head=NULL;
	}
	out->bucket_bitmap=0;
}



SYS_PUBLIC void sys_heap_deinit(sys_heap_t* heap);



SYS_PUBLIC void* sys_heap_alloc(sys_heap_t* heap,u64 size){
	if (!size){
		return NULL;
	}
	if (!heap){
		heap=&_sys_heap_default;
	}
	size=(size+sizeof(sys_heap_block_header_t)+SYS_HEAP_MIN_ALIGNMENT-1)&(-SYS_HEAP_MIN_ALIGNMENT);
	if (size>=(SYS_HEAP_MIN_ALIGNMENT<<SYS_HEAP_BUCKET_COUNT)){
		sys_heap_block_header_t* header=(sys_heap_block_header_t*)sys_memory_map(size,SYS_MEMORY_FLAG_READ|SYS_MEMORY_FLAG_WRITE,0);
		sys_heap_block_header_init(size,SYS_HEAP_BLOCK_HEADER_FLAG_DEDICATED,header);
		return header->ptr;
	}
	u32 i=_get_bucket_index(size);
	if (!(heap->bucket_bitmap>>i)){
		sys_heap_block_header_t* header=(sys_heap_block_header_t*)sys_memory_map(SYS_HEAP_MIN_ALIGNMENT<<SYS_HEAP_BUCKET_COUNT,SYS_MEMORY_FLAG_READ|SYS_MEMORY_FLAG_WRITE,0);
		sys_heap_block_header_init(SYS_HEAP_MIN_ALIGNMENT<<SYS_HEAP_BUCKET_COUNT,0,header);
		heap->buckets[SYS_HEAP_BUCKET_COUNT-1].head=header;
		heap->bucket_bitmap|=1<<(SYS_HEAP_BUCKET_COUNT-1);
	}
	u32 j=__builtin_ffs(heap->bucket_bitmap>>i)+i-1;
	sys_io_print("sys_heap_alloc: %v [%u:%u]\n",size,i,j);sys_thread_stop(0);
	return NULL;
}



SYS_PUBLIC void* sys_heap_alloc_aligned(sys_heap_t* heap,u64 size,u64 alignment);



SYS_PUBLIC void* sys_heap_realloc(sys_heap_t* heap,void* ptr,u64 size){
	if (!ptr){
		return sys_heap_alloc(heap,size);
	}
	if (!size){
		sys_heap_dealloc(heap,ptr);
		return NULL;
	}
	if (!heap){
		heap=&_sys_heap_default;
	}
	size=(size+sizeof(sys_heap_block_header_t)+SYS_HEAP_MIN_ALIGNMENT-1)&(-SYS_HEAP_MIN_ALIGNMENT);
	u32 i=_get_bucket_index(size);
	sys_io_print("sys_heap_alloc: %p -> %v [%u]\n",ptr,size,i);sys_thread_stop(0);
	return NULL;
}



SYS_PUBLIC void sys_heap_dealloc(sys_heap_t* heap,void* ptr){
	if (!ptr){
		return;
	}
	if (!heap){
		heap=&_sys_heap_default;
	}
	sys_io_print("sys_heap_dealloc: %p\n",ptr);sys_thread_stop(0);
}
