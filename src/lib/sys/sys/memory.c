#include <sys/_kernel_syscall.h>
#include <sys/memory.h>
#include <sys/types.h>



SYS_PUBLIC void* sys_memory_map(u64 length,u32 flags,u64 fd){
	return (void*)_syscall_memory_map(length,flags,fd);
}



SYS_PUBLIC _Bool sys_memory_change_flags(void* address,u64 length,u32 flags){
	return _syscall_memory_change_flags(address,length,flags);
}



SYS_PUBLIC _Bool sys_memory_unmap(void* address,u64 length){
	return _syscall_memory_unmap(address,length);
}
