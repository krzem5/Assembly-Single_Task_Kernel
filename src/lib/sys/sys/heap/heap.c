#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/types.h>



static sys_heap_t _sys_heap_default;



void __sys_heap_init(void){
	sys_heap_init(&_sys_heap_default);
}



SYS_PUBLIC void sys_heap_init(sys_heap_t* out){
	for (u32 i=0;i<SYS_HEAP_BUCKET_COUNT;i++){
		out->buckets[i].head=NULL;
	}
}



SYS_PUBLIC void sys_heap_deinit(sys_heap_t* heap);



SYS_PUBLIC void* sys_heap_alloc(sys_heap_t* heap,u64 size){
	if (!size){
		return NULL;
	}
	if (!heap){
		heap=&_sys_heap_default;
	}
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
	return NULL;
}



SYS_PUBLIC void sys_heap_dealloc(sys_heap_t* heap,void* ptr){
	if (!ptr){
		return;
	}
	if (!heap){
		heap=&_sys_heap_default;
	}
}
