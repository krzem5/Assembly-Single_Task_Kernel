#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/socket/socket.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "socket"



#define CREATE_DTP_KEY(domain,type,protocol) (((domain)<<16)|((type)<<8)|(protocol))



static spinlock_t _socket_dtp_lock;
static rb_tree_t _socket_dtp_tree;
static omm_allocator_t* KERNEL_INIT_WRITE _socket_dtp_handler_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _socket_vfs_node_allocator=NULL;



static vfs_node_t* _socket_create(void){
	socket_vfs_node_t* out=omm_alloc(_socket_vfs_node_allocator);
	spinlock_init(&(out->read_lock));
	spinlock_init(&(out->write_lock));
	out->domain=SOCKET_DOMAIN_NONE;
	out->type=SOCKET_TYPE_NONE;
	out->protocol=SOCKET_PROTOCOL_NONE;
	out->handler=NULL;
	out->handler_local_ctx=NULL;
	out->handler_remote_ctx=NULL;
	out->rx_ring=ring_init(256);
	return (vfs_node_t*)out;
}



static u64 _socket_read(vfs_node_t* node,u64 offset,void* buffer,u64 size,u32 flags){
	socket_vfs_node_t* socket=(socket_vfs_node_t*)node;
	spinlock_acquire_exclusive(&(socket->read_lock));
	u64 out=(socket->handler_local_ctx?socket->handler->descriptor->read(socket,buffer,size,flags):0);
	spinlock_release_exclusive(&(socket->read_lock));
	return out;
}



static u64 _socket_write(vfs_node_t* node,u64 offset,const void* buffer,u64 size,u32 flags){
	socket_vfs_node_t* socket=(socket_vfs_node_t*)node;
	spinlock_acquire_exclusive(&(socket->write_lock));
	u64 out=(socket->handler_remote_ctx?socket->handler->descriptor->write(socket,buffer,size):0);
	spinlock_release_exclusive(&(socket->write_lock));
	return out;
}



static const vfs_functions_t _socket_vfs_functions={
	_socket_create,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	_socket_read,
	_socket_write,
	NULL,
	NULL
};



KERNEL_INIT(){
	LOG("Initializing sockets...");
	spinlock_init(&_socket_dtp_lock);
	rb_tree_init(&_socket_dtp_tree);
	_socket_dtp_handler_allocator=omm_init("socket_dtp_handler",sizeof(socket_dtp_handler_t),8,1,pmm_alloc_counter("omm_socket_dtp_handler"));
	spinlock_init(&(_socket_dtp_handler_allocator->lock));
	_socket_vfs_node_allocator=omm_init("socket_node",sizeof(socket_vfs_node_t),8,4,pmm_alloc_counter("omm_socket_node"));
	spinlock_init(&(_socket_vfs_node_allocator->lock));
}



KERNEL_PUBLIC void socket_register_dtp_descriptor(const socket_dtp_descriptor_t* descriptor){
	spinlock_acquire_exclusive(&_socket_dtp_lock);
	LOG("Registering socket D:T:P handler '%s/%u:%u:%u'...",descriptor->name,descriptor->domain,descriptor->type,descriptor->protocol);
	u32 key=CREATE_DTP_KEY(descriptor->domain,descriptor->type,descriptor->protocol);
	rb_tree_node_t* node=rb_tree_lookup_node(&_socket_dtp_tree,key);
	if (node){
		ERROR("Socket D:T:P %u:%u:%u is already allocated by '%s'",descriptor->domain,descriptor->type,descriptor->protocol,((socket_dtp_handler_t*)node)->descriptor->name);
		spinlock_release_exclusive(&_socket_dtp_lock);
		return;
	}
	socket_dtp_handler_t* handler=omm_alloc(_socket_dtp_handler_allocator);
	handler->rb_node.key=key;
	handler->descriptor=descriptor;
	rb_tree_insert_node(&_socket_dtp_tree,&(handler->rb_node));
	spinlock_release_exclusive(&_socket_dtp_lock);
}



KERNEL_PUBLIC void socket_unregister_dtp_descriptor(const socket_dtp_descriptor_t* descriptor){
	spinlock_acquire_exclusive(&_socket_dtp_lock);
	LOG("Unregistering socket D:T:P handler '%s/%u:%u:%u'...",descriptor->name,descriptor->domain,descriptor->type,descriptor->protocol);
	u32 key=CREATE_DTP_KEY(descriptor->domain,descriptor->type,descriptor->protocol);
	rb_tree_node_t* node=rb_tree_lookup_node(&_socket_dtp_tree,key);
	if (node){
		rb_tree_remove_node(&_socket_dtp_tree,node);
		omm_dealloc(_socket_dtp_handler_allocator,node);
	}
	spinlock_release_exclusive(&_socket_dtp_lock);
}



KERNEL_PUBLIC vfs_node_t* socket_create(vfs_node_t* parent,const string_t* name,socket_domain_t domain,socket_type_t type,socket_protocol_t protocol){
	spinlock_acquire_shared(&_socket_dtp_lock);
	rb_tree_node_t* handler=rb_tree_lookup_node(&_socket_dtp_tree,CREATE_DTP_KEY(domain,type,protocol));
	spinlock_release_shared(&_socket_dtp_lock);
	if (!handler){
		ERROR("Invalid socket D:T:P combination: %u:%u:%u",domain,type,protocol);
		return NULL;
	}
	vfs_node_t* out=vfs_node_create_virtual(parent,&_socket_vfs_functions,name);
	out->flags|=VFS_NODE_TYPE_SOCKET;
	((socket_vfs_node_t*)out)->domain=domain;
	((socket_vfs_node_t*)out)->type=type;
	((socket_vfs_node_t*)out)->protocol=protocol;
	((socket_vfs_node_t*)out)->handler=(socket_dtp_handler_t*)handler;
	return out;
}



KERNEL_PUBLIC _Bool socket_bind(vfs_node_t* node,const void* local_address,u32 local_address_length){
	if ((node->flags&VFS_NODE_TYPE_MASK)!=VFS_NODE_TYPE_SOCKET||!local_address||!local_address_length){
		return 0;
	}
	socket_vfs_node_t* socket_node=(socket_vfs_node_t*)node;
	if (!socket_node->handler){
		return 0;
	}
	if (socket_node->handler_local_ctx){
		socket_node->handler->descriptor->debind(socket_node);
	}
	return socket_node->handler->descriptor->bind(socket_node,local_address,local_address_length);
}



KERNEL_PUBLIC _Bool socket_connect(vfs_node_t* node,const void* remote_address,u32 remote_address_length){
	if ((node->flags&VFS_NODE_TYPE_MASK)!=VFS_NODE_TYPE_SOCKET||!remote_address||!remote_address_length){
		return 0;
	}
	socket_vfs_node_t* socket_node=(socket_vfs_node_t*)node;
	if (!socket_node->handler){
		return 0;
	}
	if (socket_node->handler_remote_ctx){
		socket_node->handler->descriptor->deconnect(socket_node);
	}
	return socket_node->handler->descriptor->connect(socket_node,remote_address,remote_address_length);
}



KERNEL_PUBLIC void* socket_pop_packet(vfs_node_t* node,_Bool nonblocking){
	if ((node->flags&VFS_NODE_TYPE_MASK)!=VFS_NODE_TYPE_SOCKET){
		return NULL;
	}
	socket_vfs_node_t* socket_node=(socket_vfs_node_t*)node;
	spinlock_acquire_exclusive(&(socket_node->read_lock));
	void* out=ring_pop(socket_node->rx_ring,!nonblocking);
	spinlock_release_exclusive(&(socket_node->read_lock));
	return out;
}



KERNEL_PUBLIC _Bool socket_push_packet(vfs_node_t* node,const void* packet,u32 size){
	if ((node->flags&VFS_NODE_TYPE_MASK)!=VFS_NODE_TYPE_SOCKET){
		return 0;
	}
	socket_vfs_node_t* socket_node=(socket_vfs_node_t*)node;
	spinlock_acquire_exclusive(&(socket_node->write_lock));
	_Bool out=socket_node->handler->descriptor->write_packet(socket_node,packet,size);
	spinlock_release_exclusive(&(socket_node->write_lock));
	return out;
}



KERNEL_PUBLIC event_t* socket_get_event(vfs_node_t* node){
	if ((node->flags&VFS_NODE_TYPE_MASK)!=VFS_NODE_TYPE_SOCKET){
		return NULL;
	}
	socket_vfs_node_t* socket_node=(socket_vfs_node_t*)node;
	return socket_node->rx_ring->read_event;
}
