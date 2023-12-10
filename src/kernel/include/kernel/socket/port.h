#ifndef _KERNEL_SOCKET_PORT_H_
#define _KERNEL_SOCKET_PORT_H_ 1
#include <kernel/socket/socket.h>
#include <kernel/types.h>



#define SOCKET_PORT_MAX 65535



typedef u16 socket_port_t;



socket_vfs_node_t* socket_port_get(socket_port_t port);



_Bool socket_port_reserve(socket_vfs_node_t* socket,socket_port_t port);



#endif
