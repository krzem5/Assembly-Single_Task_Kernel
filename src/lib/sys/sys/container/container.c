#include <sys/container/container.h>
#include <sys/error/error.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



SYS_PUBLIC sys_container_t sys_container_create(void){
	return _sys_syscall_container_create();
}



SYS_PUBLIC sys_error_t sys_container_delete(sys_container_t container){
	return _sys_syscall_container_delete(container);
}



SYS_PUBLIC sys_error_t __attribute__((access(read_only,2,3),nonnull)) sys_container_add(sys_container_t container,const u64* handles,u64 handle_count){
	return _sys_syscall_container_add(container,handles,handle_count);
}
