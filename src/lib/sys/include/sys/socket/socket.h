#ifndef _SYS_SOCKET_SOCKET_H_
#define _SYS_SOCKET_SOCKET_H_ 1
#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/types.h>



#define SYS_SOCKET_DOMAIN_NONE 0
#define SYS_SOCKET_DOMAIN_INET 1
#define SYS_SOCKET_DOMAIN_INET6 2
#define SYS_SOCKET_DOMAIN_UNIX 3

#define SYS_SOCKET_TYPE_NONE 0
#define SYS_SOCKET_TYPE_STREAM 1
#define SYS_SOCKET_TYPE_DGRAM 2

#define SYS_SOCKET_PROTOCOL_NONE 0
#define SYS_SOCKET_PROTOCOL_TCP 6
#define SYS_SOCKET_PROTOCOL_UDP 17

#define SYS_SOCKET_SHUTDOWN_FLAG_READ 0x01
#define SYS_SOCKET_SHUTDOWN_FLAG_WRITE 0x02



typedef u8 sys_socket_domain_t;



typedef u8 sys_socket_type_t;



typedef u8 sys_socket_protocol_t;



sys_error_t sys_socket_bind(sys_fd_t fd,const void* address,u32 address_length);



sys_error_t sys_socket_connect(sys_fd_t fd,const void* address,u32 address_length);



sys_fd_t sys_socket_create(sys_socket_domain_t domain,sys_socket_type_t type,sys_socket_protocol_t protocol);



sys_error_t sys_socket_create_pair(sys_socket_domain_t domain,sys_socket_type_t type,sys_socket_protocol_t protocol,sys_fd_t* pair);



u64 sys_socket_recv(sys_fd_t fd,void* buffer,u32 buffer_length,u32 flags);



u64 sys_socket_send(sys_fd_t fd,void* buffer,u32 buffer_length,u32 flags);



sys_error_t sys_socket_shutdown(sys_fd_t fd,u32 flags);



#endif
