#ifndef _KERNEL_SOCKET_SOCKET_H_
#define _KERNEL_SOCKET_SOCKET_H_ 1
#include <kernel/memory/smm.h>
#include <kernel/tree/rb_tree.h>
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



typedef struct _SOCKET_DTP_DESCRIPTOR{
	const char* name;
	socket_domain_t domain;
	socket_type_t type;
	socket_protocol_t protocol;
} socket_dtp_descriptor_t;



typedef struct _SOCKET_DTP_HANDLER{
	rb_tree_node_t rb_node;
	const socket_dtp_descriptor_t* descriptor;
} socket_dtp_handler_t;



void socket_init(void);



void socket_register_dtp_descriptor(const socket_dtp_descriptor_t* descriptor);



void socket_unregister_dtp_descriptor(const socket_dtp_descriptor_t* descriptor);



vfs_node_t* socket_create(vfs_node_t* parent,const string_t* name,socket_domain_t domain,socket_type_t type,socket_protocol_t protocol);



#endif
