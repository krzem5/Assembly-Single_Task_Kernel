#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/memory/memory.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



SYS_PUBLIC u64 sys_memory_map(u64 length,u32 flags,sys_fd_t fd){
	return _sys_syscall_memory_map(length,flags,fd);
}



SYS_PUBLIC sys_error_t sys_memory_change_flags(void* address,u64 length,u32 flags){
	return _sys_syscall_memory_change_flags(address,length,flags);
}



SYS_PUBLIC sys_error_t sys_memory_unmap(void* address,u64 length){
	return _sys_syscall_memory_unmap(address,length);
}
