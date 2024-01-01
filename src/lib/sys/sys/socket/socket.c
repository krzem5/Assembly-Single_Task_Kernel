#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/socket/socket.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



SYS_PUBLIC sys_error_t sys_socket_bind(sys_fd_t fd,const void* address,u32 address_length){
	return _sys_syscall_socket_bind(fd,address,address_length);
}



SYS_PUBLIC sys_error_t sys_socket_connect(sys_fd_t fd,const void* address,u32 address_length){
	return _sys_syscall_socket_connect(fd,address,address_length);
}



SYS_PUBLIC sys_fd_t sys_socket_create(sys_socket_domain_t domain,sys_socket_type_t type,sys_socket_protocol_t protocol){
	return _sys_syscall_socket_create(domain,type,protocol);
}



SYS_PUBLIC sys_error_t sys_socket_create_pair(sys_socket_domain_t domain,sys_socket_type_t type,sys_socket_protocol_t protocol,sys_fd_t* pair){
	return _sys_syscall_socket_create_pair(domain,type,protocol,pair);
}



SYS_PUBLIC u64 sys_socket_recv(sys_fd_t fd,void* buffer,u32 buffer_length,u32 flags){
	return _sys_syscall_socket_recv(fd,buffer,buffer_length,flags);
}



SYS_PUBLIC u64 sys_socket_send(sys_fd_t fd,void* buffer,u32 buffer_length,u32 flags){
	return _sys_syscall_socket_send(fd,buffer,buffer_length,flags);
}



SYS_PUBLIC sys_error_t sys_socket_shutdown(sys_fd_t fd,u32 flags){
	return _sys_syscall_socket_shutdown(fd,flags);
}
