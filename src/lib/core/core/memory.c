#include <core/memory.h>
#include <core/syscall.h>
#include <core/types.h>



void* memory_map(u64 length,u8 flags){
	return _syscall_memory_map(length,flags,0);
}



_Bool memory_unmap(void* address,u64 length){
	return _syscall_memory_unmap(address,length);
}
