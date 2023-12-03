#ifndef _KERNEL_SOCKET_SOCKET_H_
#define _KERNEL_SOCKET_SOCKET_H_ 1
#include <kernel/lock/spinlock.h>
#include <kernel/memory/smm.h>
#include <kernel/mp/event.h>
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



typedef struct _SOCKET_VFS_NODE{
	vfs_node_t node;
	spinlock_t read_lock;
	spinlock_t write_lock;
	socket_domain_t domain;
	socket_type_t type;
	socket_protocol_t protocol;
	struct _SOCKET_DTP_HANDLER* handler;
	void* handler_local_ctx;
	void* handler_remote_ctx;
	event_t* read_event;
} socket_vfs_node_t;



typedef struct _SOCKET_DTP_DESCRIPTOR{
	const char* name;
	socket_domain_t domain;
	socket_type_t type;
	socket_protocol_t protocol;
	_Bool (*bind)(socket_vfs_node_t*,const void*,u32);
	void (*debind)(socket_vfs_node_t*);
	_Bool (*connect)(socket_vfs_node_t*,const void*,u32);
	void (*deconnect)(socket_vfs_node_t*);
	u64 (*write)(socket_vfs_node_t*,const void*,u64);
} socket_dtp_descriptor_t;



typedef struct _SOCKET_DTP_HANDLER{
	rb_tree_node_t rb_node;
	const socket_dtp_descriptor_t* descriptor;
} socket_dtp_handler_t;



void socket_init(void);



void socket_register_dtp_descriptor(const socket_dtp_descriptor_t* descriptor);



void socket_unregister_dtp_descriptor(const socket_dtp_descriptor_t* descriptor);



vfs_node_t* socket_create(vfs_node_t* parent,const string_t* name,socket_domain_t domain,socket_type_t type,socket_protocol_t protocol);



_Bool socket_bind(vfs_node_t* node,const void* local_address,u32 local_address_length);



_Bool socket_connect(vfs_node_t* node,const void* remote_address,u32 remote_address_length);



#endif
