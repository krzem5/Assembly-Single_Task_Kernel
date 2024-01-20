#include <linker/alloc.h>
#include <linker/shared_object.h>
#include <sys/memory/memory.h>
#include <sys/types.h>



static void* _alloc_default_allocator_base=NULL;
static u64 _alloc_default_allocator_size=0;
static allocator_backend_t _alloc_allocator_backend=NULL;



static void* _default_allocator(void* null,u64 size){
	size=(size+sizeof(u64)-1)&(-sizeof(u64));
	if (_alloc_default_allocator_size<size){
		_alloc_default_allocator_size=sys_memory_align_up_address(size);
		_alloc_default_allocator_base=(void*)sys_memory_map(_alloc_default_allocator_size,SYS_MEMORY_FLAG_READ|SYS_MEMORY_FLAG_WRITE,0);
	}
	void* out=_alloc_default_allocator_base;
	_alloc_default_allocator_base+=size;
	_alloc_default_allocator_size-=size;
	return out;
}



void* alloc(u64 size){
	if (!_alloc_allocator_backend){
		_alloc_allocator_backend=_default_allocator;
	}
	return _alloc_allocator_backend(NULL,size);
}



void alloc_change_backend(allocator_backend_t backend){
	_alloc_allocator_backend=(backend?backend:_default_allocator);
}
