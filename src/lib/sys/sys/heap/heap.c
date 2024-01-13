#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/memory/memory.h>
#include <sys/mp/thread.h>
#include <sys/types.h>



static sys_heap_t _sys_heap_default;



static inline u32 _get_bucket_index(u64 size){
	return 31-__builtin_clz(size)/*+(!!(size&(size-1)))*/-__builtin_ffs(SYS_HEAP_MIN_ALIGNMENT);
}



static inline void _heap_block_header_init(u64 size,u32 flags,sys_heap_block_header_t* header){
	header->prev=NULL;
	header->next=NULL;
	header->size_and_flags=size|flags;
	header->offset.offset=sizeof(sys_heap_block_header_t);
}



static inline u64 _heap_block_header_get_size(const sys_heap_block_header_t* header){
	return header->size_and_flags&(-SYS_HEAP_MIN_ALIGNMENT);
}



static inline void _insert_block(sys_heap_t* heap,sys_heap_block_header_t* header){
	u32 i=_get_bucket_index(_heap_block_header_get_size(header));
	if (i>=SYS_HEAP_BUCKET_COUNT){
		i=SYS_HEAP_BUCKET_COUNT-1;
	}
	header->prev=NULL;
	header->next=heap->buckets[i].head;
	if (header->next){
		header->next->prev=header;
	}
	else{
		heap->bucket_bitmap|=1<<i;
	}
	heap->buckets[i].head=header;
}



static inline void _remove_block(sys_heap_t* heap,sys_heap_block_header_t* header,u32 i){
	if (header->next){
		header->next->prev=header->prev;
	}
	if (header->prev){
		header->prev->next=header->next;
	}
	else{
		heap->buckets[i].head=header->next;
		if (!header->next){
			heap->bucket_bitmap&=~(1<<i);
		}
	}
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
		_heap_block_header_init(size,SYS_HEAP_BLOCK_HEADER_FLAG_USED|SYS_HEAP_BLOCK_HEADER_FLAG_DEDICATED,header);
		return header->ptr;
	}
	u32 i=_get_bucket_index(size);
	if (!(heap->bucket_bitmap>>i)){
_grow_heap:
		void* data=(void*)sys_memory_map(SYS_HEAP_GROW_SIZE,SYS_MEMORY_FLAG_READ|SYS_MEMORY_FLAG_WRITE,0);
		_heap_block_header_init(sizeof(sys_heap_block_header_t),SYS_HEAP_BLOCK_HEADER_FLAG_USED,data);
		_heap_block_header_init(SYS_HEAP_GROW_SIZE-2*sizeof(sys_heap_block_header_t),0,data+sizeof(sys_heap_block_header_t));
		_heap_block_header_init(sizeof(sys_heap_block_header_t),SYS_HEAP_BLOCK_HEADER_FLAG_USED,data+SYS_HEAP_GROW_SIZE-sizeof(sys_heap_block_header_t));
		_insert_block(heap,data+sizeof(sys_heap_block_header_t));
	}
	sys_heap_block_header_t* header=NULL;
	if (heap->bucket_bitmap&(1<<i)){
		for (header=heap->buckets[i].head;header;header=header->next){
			if (_heap_block_header_get_size(header)>=size){
				goto _header_found;
			}
		}
	}
	if (!(heap->bucket_bitmap>>(i+1))){
		goto _grow_heap;
	}
	i=__builtin_ffs(heap->bucket_bitmap>>(i+1))+i;
	header=heap->buckets[i].head;
_header_found:
	_remove_block(heap,header,i);
	u64 total_size=_heap_block_header_get_size(header);
	if (total_size<=size+sizeof(sys_heap_block_header_t)){
		size=total_size;
	}
	else{
		_heap_block_header_init(total_size-size,0,((void*)header)+size);
		_insert_block(heap,((void*)header)+size);
	}
	_heap_block_header_init(size,SYS_HEAP_BLOCK_HEADER_FLAG_USED,header);
	return header->ptr;
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
	u64 offset=((const sys_heap_block_offset_t*)(ptr-sizeof(sys_heap_block_offset_t)))->offset;
	sys_heap_block_header_t* header=ptr-offset;
	if (header->size_and_flags&SYS_HEAP_BLOCK_HEADER_FLAG_DEDICATED){
		sys_memory_unmap(header,header->size_and_flags&(-SYS_HEAP_MIN_ALIGNMENT));
		return;
	}
	header->size_and_flags&=~SYS_HEAP_BLOCK_HEADER_FLAG_USED;
	_insert_block(heap,header);
	sys_io_print("sys_heap_dealloc: %p [%x]\n",ptr,header->size_and_flags);//sys_thread_stop(0);
}
