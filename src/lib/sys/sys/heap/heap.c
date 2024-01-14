#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/memory/memory.h>
#include <sys/mp/thread.h>
#include <sys/types.h>



static sys_heap_t _sys_heap_default;



static inline u32 _get_bucket_index(u64 size){
	u32 out=31-__builtin_clz(size/SYS_HEAP_MIN_ALIGNMENT);
	return (out>=SYS_HEAP_BUCKET_COUNT?SYS_HEAP_BUCKET_COUNT-1:out);
}



static inline void _heap_block_header_init(u64 size,u32 prev_size,u32 flags,sys_heap_block_header_t* header){
	header->prev=NULL;
	header->next=NULL;
	header->size_and_flags=size|flags;
	header->offset.prev_size=prev_size;
	header->offset.offset=sizeof(sys_heap_block_header_t);
}



static inline u64 _heap_block_header_get_size(const sys_heap_block_header_t* header){
	return header->size_and_flags&(-SYS_HEAP_MIN_ALIGNMENT);
}



static inline u64 _heap_block_header_get_prev_size(const sys_heap_block_header_t* header){
	return header->offset.prev_size;
}



static inline void _insert_block(sys_heap_t* heap,sys_heap_block_header_t* header){
	u32 i=_get_bucket_index(_heap_block_header_get_size(header));
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



static inline void _remove_block(sys_heap_t* heap,sys_heap_block_header_t* header){
	u32 i=_get_bucket_index(_heap_block_header_get_size(header));
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
		size=sys_memory_align_up_address(size);
		sys_heap_block_header_t* header=(sys_heap_block_header_t*)sys_memory_map(size,SYS_MEMORY_FLAG_READ|SYS_MEMORY_FLAG_WRITE,0);
		_heap_block_header_init(size,0,SYS_HEAP_BLOCK_HEADER_FLAG_USED|SYS_HEAP_BLOCK_HEADER_FLAG_DEDICATED,header);
		return header->ptr;
	}
	u32 i=_get_bucket_index(size);
	if (!(heap->bucket_bitmap>>i)){
_grow_heap:
		void* data=(void*)sys_memory_map(SYS_HEAP_GROW_SIZE,SYS_MEMORY_FLAG_READ|SYS_MEMORY_FLAG_WRITE,0);
		_heap_block_header_init(sizeof(sys_heap_block_header_t),0,SYS_HEAP_BLOCK_HEADER_FLAG_USED,data);
		_heap_block_header_init(SYS_HEAP_GROW_SIZE-2*sizeof(sys_heap_block_header_t),sizeof(sys_heap_block_header_t),0,data+sizeof(sys_heap_block_header_t));
		_heap_block_header_init(sizeof(sys_heap_block_header_t),SYS_HEAP_GROW_SIZE-2*sizeof(sys_heap_block_header_t),SYS_HEAP_BLOCK_HEADER_FLAG_USED,data+SYS_HEAP_GROW_SIZE-sizeof(sys_heap_block_header_t));
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
	header=heap->buckets[__builtin_ffs(heap->bucket_bitmap>>(i+1))+i].head;
_header_found:
	_remove_block(heap,header);
	u64 total_size=_heap_block_header_get_size(header);
	if (total_size<=size+sizeof(sys_heap_block_header_t)){
		size=total_size;
	}
	else{
		_heap_block_header_init(total_size-size,size,0,((void*)header)+size);
		_insert_block(heap,((void*)header)+size);
	}
	_heap_block_header_init(size,_heap_block_header_get_prev_size(header),SYS_HEAP_BLOCK_HEADER_FLAG_USED,header);
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
	u64 offset=((const sys_heap_block_offset_t*)(ptr-sizeof(sys_heap_block_offset_t)))->offset;
	sys_heap_block_header_t* header=ptr-offset;
	if (size>=(SYS_HEAP_MIN_ALIGNMENT<<SYS_HEAP_BUCKET_COUNT)){
		size=sys_memory_align_up_address(size);
		if (_heap_block_header_get_size(header)==size){
			return ptr;
		}
		if (size<_heap_block_header_get_size(header)){
			sys_io_print("sys_heap_realloc: shrink dedicated block\n");sys_thread_stop(0);
		}
		sys_heap_block_header_t* new_header=(sys_heap_block_header_t*)sys_memory_map(size,SYS_MEMORY_FLAG_READ|SYS_MEMORY_FLAG_WRITE,0);
		_heap_block_header_init(size,0,SYS_HEAP_BLOCK_HEADER_FLAG_USED|SYS_HEAP_BLOCK_HEADER_FLAG_DEDICATED,new_header);
		sys_memory_copy(ptr,new_header->ptr,_heap_block_header_get_size(header)-sizeof(sys_heap_block_header_t));
		sys_heap_dealloc(heap,ptr);
		return new_header->ptr;
	}
	if (header->size_and_flags&SYS_HEAP_BLOCK_HEADER_FLAG_DEDICATED){
		size-=sizeof(sys_heap_block_header_t);
		void* out=sys_heap_alloc(heap,size);
		sys_memory_copy(ptr,out,size);
		sys_memory_unmap(header,_heap_block_header_get_size(header));
		return out;
	}
	if (_heap_block_header_get_size(header)==size){
		return ptr;
	}
	sys_heap_block_header_t* next_header=((void*)header)+_heap_block_header_get_size(header);
	if (size<_heap_block_header_get_size(header)){
		u64 extra_space=_heap_block_header_get_size(header)-size;
		if (extra_space<=sizeof(sys_heap_block_header_t)){
			return ptr;
		}
		next_header->offset.prev_size=extra_space;
		header->size_and_flags=size|SYS_HEAP_BLOCK_HEADER_FLAG_USED;
		header=((void*)header)+size;
		_heap_block_header_init(extra_space,size,0,header);
		if (!(next_header->size_and_flags&SYS_HEAP_BLOCK_HEADER_FLAG_USED)){
			_remove_block(heap,next_header);
			header->size_and_flags+=_heap_block_header_get_size(next_header);
			next_header=((void*)next_header)+_heap_block_header_get_size(next_header);
			next_header->offset.prev_size=_heap_block_header_get_size(header);
		}
		_insert_block(heap,header);
		return ptr;
	}
	if ((next_header->size_and_flags&SYS_HEAP_BLOCK_HEADER_FLAG_USED)||_heap_block_header_get_size(header)+_heap_block_header_get_size(next_header)<size||1){
		void* out=sys_heap_alloc(heap,size-sizeof(sys_heap_block_header_t));
		sys_memory_copy(ptr,out,_heap_block_header_get_size(header)-sizeof(sys_heap_block_header_t));
		sys_heap_dealloc(heap,ptr);
		return out;
	}
	sys_io_print("sys_heap_realloc: merge with next block\n");sys_thread_stop(0);
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
		sys_memory_unmap(header,_heap_block_header_get_size(header));
		return;
	}
	header->size_and_flags&=~SYS_HEAP_BLOCK_HEADER_FLAG_USED;
	sys_heap_block_header_t* next_header=((void*)header)+_heap_block_header_get_size(header);
	if (!(next_header->size_and_flags&SYS_HEAP_BLOCK_HEADER_FLAG_USED)){
		_remove_block(heap,next_header);
		header->size_and_flags+=_heap_block_header_get_size(next_header);
		next_header=((void*)next_header)+_heap_block_header_get_size(next_header);
		next_header->offset.prev_size=_heap_block_header_get_size(header);
	}
	sys_heap_block_header_t* prev_header=((void*)header)-_heap_block_header_get_prev_size(header);
	if (!(prev_header->size_and_flags&SYS_HEAP_BLOCK_HEADER_FLAG_USED)){
		// merge prev_header and header
	}
	_insert_block(heap,header);
}
