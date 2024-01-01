#include <sys2/error/error.h>
#include <sys2/fd/fd.h>
#include <sys2/memory/memory.h>
#include <sys2/syscall/kernel_syscalls.h>
#include <sys2/types.h>



SYS2_PUBLIC u64 sys2_memory_map(u64 length,u32 flags,sys2_fd_t fd){
	return _sys2_syscall_memory_map(length,flags,fd);
}



SYS2_PUBLIC sys2_error_t sys2_memory_change_flags(void* address,u64 length,u32 flags){
	return _sys2_syscall_memory_change_flags(address,length,flags);
}



SYS2_PUBLIC sys2_error_t sys2_memory_unmap(void* address,u64 length){
	return _sys2_syscall_memory_unmap(address,length);
}
