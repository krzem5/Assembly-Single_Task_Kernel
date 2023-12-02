#ifndef _KERNEL_SOCKET_SOCKET_H_
#define _KERNEL_SOCKET_SOCKET_H_ 1
#include <kernel/memory/smm.h>
#include <kernel/types.h>
#include <kernel/vfs/node.h>



#define SOCKET_DOMAIN_NONE 0
#define SOCKET_DOMAIN_INET 1
#define SOCKET_DOMAIN_INET6 2

#define SOCKET_TYPE_NONE 0
#define SOCKET_TYPE_STREAM 1
#define SOCKET_TYPE_DGRAM 2

#define SOCKET_PROTOCOL_NONE 0
#define SOCKET_PROTOCOL_TCP 6
#define SOCKET_PROTOCOL_UDP 17



typedef u8 socket_domain_t;



typedef u8 socket_type_t;



typedef u8 socket_protocol_t;



void socket_init(void);



vfs_node_t* socket_create(vfs_node_t* parent,const string_t* name,socket_domain_t domain,socket_type_t type,socket_protocol_t protocol);



#endif
