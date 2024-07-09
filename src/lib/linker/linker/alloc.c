#include <linker/alloc.h>
#include <linker/shared_object.h>
#include <sys/memory/memory.h>
#include <sys/types.h>



static void* _linker_alloc_default_allocator_base=NULL;
static u64 _linker_alloc_default_allocator_size=0;
static linker_allocator_backend_t _linker_alloc_allocator_backend=NULL;



static void* _default_allocator(void* null,void* ptr,u64 size){
	if (ptr){
		return NULL;
	}
	size=(size+sizeof(u64)-1)&(-sizeof(u64));
	if (_linker_alloc_default_allocator_size<size){
		_linker_alloc_default_allocator_size=sys_memory_align_up_address(size);
		_linker_alloc_default_allocator_base=(void*)sys_memory_map(_linker_alloc_default_allocator_size,SYS_MEMORY_FLAG_READ|SYS_MEMORY_FLAG_WRITE,0);
	}
	void* out=_linker_alloc_default_allocator_base;
	_linker_alloc_default_allocator_base+=size;
	_linker_alloc_default_allocator_size-=size;
	return out;
}



void* linker_alloc(u64 size){
	if (!_linker_alloc_allocator_backend){
		_linker_alloc_allocator_backend=_default_allocator;
	}
	return _linker_alloc_allocator_backend(NULL,NULL,size);
}



void linker_dealloc(void* ptr){
	if (!_linker_alloc_allocator_backend){
		_linker_alloc_allocator_backend=_default_allocator;
	}
	_linker_alloc_allocator_backend(NULL,ptr,0);
}



void linker_alloc_change_backend(linker_allocator_backend_t backend){
	_linker_alloc_allocator_backend=(backend?backend:_default_allocator);
}
