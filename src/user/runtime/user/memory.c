#include <user/memory.h>
#include <user/syscall.h>
#include <user/types.h>



_Bool memory_get_range(u32 index,memory_range_t* out){
	return _syscall_memory_get_range(index,out,sizeof(memory_range_t));
}



void* memory_map(u64 length,u8 flags){
	return _syscall_memory_map(length,flags);
}



_Bool memory_unmap(void* address,u64 length){
	return _syscall_memory_unmap(address,length);
}



_Bool memory_counter_get_data(u64 handle,memory_counter_data_t* out){
	return _syscall_memory_counter_get_data(handle,out,sizeof(memory_counter_data_t));
}



_Bool memory_object_allocator_get_data(u64 handle,memory_object_allocator_data_t* out){
	return _syscall_memory_object_allocator_get_data(handle,out,sizeof(memory_object_allocator_data_t));
}
