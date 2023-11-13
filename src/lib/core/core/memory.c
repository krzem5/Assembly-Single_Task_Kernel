#include <core/memory.h>
#include <core/syscall.h>
#include <core/types.h>



void* memory_map(u64 length,u32 flags,u64 fd){
	return _syscall_memory_map(length,flags,fd);
}



_Bool memory_unmap(void* address,u64 length){
	return _syscall_memory_unmap(address,length);
}
