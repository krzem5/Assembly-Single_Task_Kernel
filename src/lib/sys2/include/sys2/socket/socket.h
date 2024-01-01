#ifndef _SYS2_SOCKET_SOCKET_H_
#define _SYS2_SOCKET_SOCKET_H_ 1
#include <sys2/error/error.h>
#include <sys2/fd/fd.h>
#include <sys2/types.h>



#define SOCKET_DOMAIN_NONE 0
#define SOCKET_DOMAIN_INET 1
#define SOCKET_DOMAIN_INET6 2
#define SOCKET_DOMAIN_UNIX 3

#define SOCKET_TYPE_NONE 0
#define SOCKET_TYPE_STREAM 1
#define SOCKET_TYPE_DGRAM 2

#define SOCKET_PROTOCOL_NONE 0
#define SOCKET_PROTOCOL_TCP 6
#define SOCKET_PROTOCOL_UDP 17

#define SYS2_SOCKET_SHUTDOWN_FLAG_READ 0x01
#define SYS2_SOCKET_SHUTDOWN_FLAG_WRITE 0x02



typedef u8 sys2_socket_domain_t;



typedef u8 sys2_socket_type_t;



typedef u8 sys2_socket_protocol_t;



sys2_error_t sys2_socket_bind(sys2_fd_t fd,const void* address,u32 address_length);



sys2_error_t sys2_socket_connect(sys2_fd_t fd,const void* address,u32 address_length);



sys2_fd_t sys2_socket_create(sys2_socket_domain_t domain,sys2_socket_type_t type,sys2_socket_protocol_t protocol);



sys2_error_t sys2_socket_create_pair(sys2_socket_domain_t domain,sys2_socket_type_t type,sys2_socket_protocol_t protocol,sys2_fd_t* pair);



u64 sys2_socket_recv(sys2_fd_t fd,void* buffer,u32 buffer_length,u32 flags);



u64 sys2_socket_send(sys2_fd_t fd,void* buffer,u32 buffer_length,u32 flags);



sys2_error_t sys2_socket_shutdown(sys2_fd_t fd,u32 flags);



#endif
