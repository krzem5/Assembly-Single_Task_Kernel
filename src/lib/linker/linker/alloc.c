#include <linker/alloc.h>
#include <sys/heap/heap.h>
#include <sys/memory/memory.h>
#include <sys/types.h>



static sys_heap_t _linker_alloc_heap;
static bool _linker_alloc_initialized=0;



void* linker_alloc(u64 size){
	if (!_linker_alloc_initialized){
		_linker_alloc_initialized=1;
		sys_heap_init(&_linker_alloc_heap);
	}
	return sys_heap_alloc(&_linker_alloc_heap,size);
}



void linker_dealloc(void* ptr){
	if (!_linker_alloc_initialized){
		return;
	}
	sys_heap_dealloc(&_linker_alloc_heap,ptr);
}
