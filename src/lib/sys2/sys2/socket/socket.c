#include <sys2/error/error.h>
#include <sys2/fd/fd.h>
#include <sys2/socket/socket.h>
#include <sys2/syscall/kernel_syscalls.h>
#include <sys2/types.h>



SYS2_PUBLIC sys2_error_t sys2_socket_bind(sys2_fd_t fd,const void* address,u32 address_length){
	return _sys2_syscall_socket_bind(fd,address,address_length);
}



SYS2_PUBLIC sys2_error_t sys2_socket_connect(sys2_fd_t fd,const void* address,u32 address_length){
	return _sys2_syscall_socket_connect(fd,address,address_length);
}



SYS2_PUBLIC sys2_fd_t sys2_socket_create(sys2_socket_domain_t domain,sys2_socket_type_t type,sys2_socket_protocol_t protocol){
	return _sys2_syscall_socket_create(domain,type,protocol);
}



SYS2_PUBLIC sys2_error_t sys2_socket_create_pair(sys2_socket_domain_t domain,sys2_socket_type_t type,sys2_socket_protocol_t protocol,sys2_fd_t* pair){
	return _sys2_syscall_socket_create_pair(domain,type,protocol,pair);
}



SYS2_PUBLIC u64 sys2_socket_recv(sys2_fd_t fd,void* buffer,u32 buffer_length,u32 flags){
	return _sys2_syscall_socket_recv(fd,buffer,buffer_length,flags);
}



SYS2_PUBLIC u64 sys2_socket_send(sys2_fd_t fd,void* buffer,u32 buffer_length,u32 flags){
	return _sys2_syscall_socket_send(fd,buffer,buffer_length,flags);
}



SYS2_PUBLIC sys2_error_t sys2_socket_shutdown(sys2_fd_t fd,u32 flags){
	return _sys2_syscall_socket_shutdown(fd,flags);
}
