#ifndef _KERNEL_SOCKET_SOCKET_H_
#define _KERNEL_SOCKET_SOCKET_H_ 1
#include <kernel/lock/spinlock.h>
#include <kernel/memory/smm.h>
#include <kernel/ring/ring.h>
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
	ring_t* rx_ring;
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
	u64 (*read)(socket_vfs_node_t*,void*,u64,u32);
	u64 (*write)(socket_vfs_node_t*,const void*,u64);
	_Bool (*write_packet)(socket_vfs_node_t*,const void*,u32);
} socket_dtp_descriptor_t;



typedef struct _SOCKET_DTP_HANDLER{
	rb_tree_node_t rb_node;
	const socket_dtp_descriptor_t* descriptor;
} socket_dtp_handler_t;



typedef struct _SOCKET_PACKET{
	u64 size;
	void* data;
} socket_packet_t;



void socket_register_dtp_descriptor(const socket_dtp_descriptor_t* descriptor);



void socket_unregister_dtp_descriptor(const socket_dtp_descriptor_t* descriptor);



vfs_node_t* socket_create(vfs_node_t* parent,const string_t* name,socket_domain_t domain,socket_type_t type,socket_protocol_t protocol);



_Bool socket_bind(vfs_node_t* node,const void* local_address,u32 local_address_length);



_Bool socket_connect(vfs_node_t* node,const void* remote_address,u32 remote_address_length);



socket_packet_t* socket_peek_packet(vfs_node_t* node,_Bool nonblocking);



socket_packet_t* socket_pop_packet(vfs_node_t* node,_Bool nonblocking);



_Bool socket_push_packet(vfs_node_t* node,const void* packet,u32 size);



_Bool socket_alloc_packet(vfs_node_t* node,void* data,u32 size);



void socket_dealloc_packet(socket_packet_t* packet);



event_t* socket_get_event(vfs_node_t* node);



#endif
