#include <user/memory.h>
#include <user/syscall.h>
#include <user/types.h>



void* memory_map(u64 length,u8 flags){
	return _syscall_memory_map(length,flags);
}



_Bool memory_unmap(void* address,u64 length){
	return _syscall_memory_unmap(address,length);
}



u32 memory_get_counter_count(void){
	return _syscall_memory_get_counter_count();
}



_Bool memory_get_counter(u32 counter,memory_counter_t* out){
	return _syscall_memory_get_counter(counter,out,sizeof(memory_counter_t));
}



u32 memory_get_object_counter_count(void){
	return _syscall_memory_get_object_counter_count();
}



_Bool memory_get_object_counter(u32 counter,memory_object_counter_t* out){
	return _syscall_memory_get_object_counter(counter,out,sizeof(memory_object_counter_t));
}
