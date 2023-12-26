#include <kernel/error/error.h>
#include <kernel/fd/fd.h>
#include <kernel/format/format.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/socket/socket.h>
#include <kernel/syscall/syscall.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "socket"



#define CREATE_DTP_KEY(domain,type,protocol) (((domain)<<16)|((type)<<8)|(protocol))



static spinlock_t _socket_dtp_lock;
static rb_tree_t _socket_dtp_tree;
static omm_allocator_t* KERNEL_INIT_WRITE _socket_dtp_handler_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _socket_vfs_node_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _socket_packet_allocator=NULL;
static vfs_node_t* _socket_root=NULL;
static KERNEL_ATOMIC u64 _socket_next_id=0;



static vfs_node_t* _socket_create(void){
	socket_vfs_node_t* out=omm_alloc(_socket_vfs_node_allocator);
	spinlock_init(&(out->read_lock));
	spinlock_init(&(out->write_lock));
	out->domain=SOCKET_DOMAIN_NONE;
	out->type=SOCKET_TYPE_NONE;
	out->protocol=SOCKET_PROTOCOL_NONE;
	out->descriptor=NULL;
	out->local_ctx=NULL;
	out->remote_ctx=NULL;
	out->rx_ring=ring_init(256);
	return (vfs_node_t*)out;
}



static u64 _socket_read(vfs_node_t* node,u64 offset,void* buffer,u64 size,u32 flags){
	socket_vfs_node_t* socket=(socket_vfs_node_t*)node;
	spinlock_acquire_exclusive(&(socket->read_lock));
	u64 out=(socket->local_ctx?socket->descriptor->read(socket,buffer,size,flags):0);
	spinlock_release_exclusive(&(socket->read_lock));
	return out;
}



static u64 _socket_write(vfs_node_t* node,u64 offset,const void* buffer,u64 size,u32 flags){
	socket_vfs_node_t* socket=(socket_vfs_node_t*)node;
	spinlock_acquire_exclusive(&(socket->write_lock));
	u64 out=(socket->remote_ctx?socket->descriptor->write(socket,buffer,size):0);
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



static string_t* _get_unique_id(void){
	char buffer[32];
	return smm_alloc(buffer,format_string(buffer,32,"%lu",__atomic_add_fetch(&_socket_next_id,1,__ATOMIC_SEQ_CST)));
}



static vfs_node_t* _create_socket_node(socket_domain_t domain,socket_type_t type,socket_protocol_t protocol,const socket_dtp_descriptor_t* descriptor){
	if (!_socket_root){
		SMM_TEMPORARY_STRING dir_name=smm_alloc("sockets",0);
		_socket_root=vfs_node_create_virtual(vfs_lookup(NULL,"/",0,0,0),NULL,dir_name);
		_socket_root->flags|=VFS_NODE_TYPE_DIRECTORY|(0400<<VFS_NODE_PERMISSION_SHIFT);
	}
	SMM_TEMPORARY_STRING name=_get_unique_id();
	vfs_node_t* out=vfs_node_create_virtual(_socket_root,&_socket_vfs_functions,name);
	out->flags|=VFS_NODE_TYPE_SOCKET|(0000<<VFS_NODE_PERMISSION_SHIFT);
	((socket_vfs_node_t*)out)->domain=domain;
	((socket_vfs_node_t*)out)->type=type;
	((socket_vfs_node_t*)out)->protocol=protocol;
	((socket_vfs_node_t*)out)->flags=SOCKET_FLAG_READ|SOCKET_FLAG_WRITE;
	((socket_vfs_node_t*)out)->descriptor=descriptor;
	return out;
}



KERNEL_INIT(){
	LOG("Initializing sockets...");
	spinlock_init(&_socket_dtp_lock);
	rb_tree_init(&_socket_dtp_tree);
	_socket_dtp_handler_allocator=omm_init("socket_dtp_handler",sizeof(socket_dtp_handler_t),8,1,pmm_alloc_counter("omm_socket_dtp_handler"));
	spinlock_init(&(_socket_dtp_handler_allocator->lock));
	_socket_vfs_node_allocator=omm_init("socket_node",sizeof(socket_vfs_node_t),8,4,pmm_alloc_counter("omm_socket_node"));
	spinlock_init(&(_socket_vfs_node_allocator->lock));
	_socket_packet_allocator=omm_init("socket_packet",sizeof(socket_packet_t),8,4,pmm_alloc_counter("omm_socket_packet"));
	spinlock_init(&(_socket_packet_allocator->lock));
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



KERNEL_PUBLIC vfs_node_t* socket_create(socket_domain_t domain,socket_type_t type,socket_protocol_t protocol){
	spinlock_acquire_shared(&_socket_dtp_lock);
	socket_dtp_handler_t* handler=(socket_dtp_handler_t*)rb_tree_lookup_node(&_socket_dtp_tree,CREATE_DTP_KEY(domain,type,protocol));
	spinlock_release_shared(&_socket_dtp_lock);
	if (!handler){
		ERROR("Invalid socket D:T:P combination: %u:%u:%u",domain,type,protocol);
		return NULL;
	}
	return _create_socket_node(domain,type,protocol,handler->descriptor);
}



KERNEL_PUBLIC _Bool socket_create_pair(socket_domain_t domain,socket_type_t type,socket_protocol_t protocol,socket_pair_t* out){
	spinlock_acquire_shared(&_socket_dtp_lock);
	socket_dtp_handler_t* handler=(socket_dtp_handler_t*)rb_tree_lookup_node(&_socket_dtp_tree,CREATE_DTP_KEY(domain,type,protocol));
	spinlock_release_shared(&_socket_dtp_lock);
	if (!handler){
		ERROR("Invalid socket D:T:P combination: %u:%u:%u",domain,type,protocol);
		return 0;
	}
	if (!handler->descriptor->create_pair){
		ERROR("Unable to create a socket pair from '%s'",handler->descriptor->name);
		return 0;
	}
	out->sockets[0]=(socket_vfs_node_t*)_create_socket_node(domain,type,protocol,handler->descriptor);
	out->sockets[1]=(socket_vfs_node_t*)_create_socket_node(domain,type,protocol,handler->descriptor);
	handler->descriptor->create_pair(out);
	return 1;
}



KERNEL_PUBLIC _Bool socket_shutdown(vfs_node_t* node,u8 flags){
	if ((node->flags&VFS_NODE_TYPE_MASK)!=VFS_NODE_TYPE_SOCKET){
		return 0;
	}
	socket_vfs_node_t* socket_node=(socket_vfs_node_t*)node;
	socket_node->flags&=~flags;
	return 1;
}



KERNEL_PUBLIC _Bool socket_bind(vfs_node_t* node,const void* local_address,u32 local_address_length){
	if ((node->flags&VFS_NODE_TYPE_MASK)!=VFS_NODE_TYPE_SOCKET||!local_address||!local_address_length){
		return 0;
	}
	socket_vfs_node_t* socket_node=(socket_vfs_node_t*)node;
	if (!socket_node->descriptor){
		return 0;
	}
	if (socket_node->local_ctx){
		socket_node->descriptor->debind(socket_node);
		socket_node->local_ctx=NULL;
	}
	return socket_node->descriptor->bind(socket_node,local_address,local_address_length);
}



KERNEL_PUBLIC _Bool socket_connect(vfs_node_t* node,const void* remote_address,u32 remote_address_length){
	if ((node->flags&VFS_NODE_TYPE_MASK)!=VFS_NODE_TYPE_SOCKET||!remote_address||!remote_address_length){
		return 0;
	}
	socket_vfs_node_t* socket_node=(socket_vfs_node_t*)node;
	if (!socket_node->descriptor){
		return 0;
	}
	if (socket_node->remote_ctx){
		socket_node->descriptor->deconnect(socket_node);
		socket_node->remote_ctx=NULL;
	}
	return socket_node->descriptor->connect(socket_node,remote_address,remote_address_length);
}



KERNEL_PUBLIC socket_packet_t* socket_peek_packet(vfs_node_t* node,_Bool nonblocking){
	if ((node->flags&VFS_NODE_TYPE_MASK)!=VFS_NODE_TYPE_SOCKET){
		return NULL;
	}
	socket_vfs_node_t* socket_node=(socket_vfs_node_t*)node;
	spinlock_acquire_exclusive(&(socket_node->read_lock));
	socket_packet_t* out=ring_peek(socket_node->rx_ring,!nonblocking);
	spinlock_release_exclusive(&(socket_node->read_lock));
	return out;
}



KERNEL_PUBLIC socket_packet_t* socket_pop_packet(vfs_node_t* node,_Bool nonblocking){
	if ((node->flags&VFS_NODE_TYPE_MASK)!=VFS_NODE_TYPE_SOCKET){
		return NULL;
	}
	socket_vfs_node_t* socket_node=(socket_vfs_node_t*)node;
	spinlock_acquire_exclusive(&(socket_node->read_lock));
	socket_packet_t* out=ring_pop(socket_node->rx_ring,!nonblocking);
	spinlock_release_exclusive(&(socket_node->read_lock));
	return out;
}



KERNEL_PUBLIC _Bool socket_push_packet(vfs_node_t* node,const void* packet,u32 size){
	if ((node->flags&VFS_NODE_TYPE_MASK)!=VFS_NODE_TYPE_SOCKET){
		return 0;
	}
	socket_vfs_node_t* socket_node=(socket_vfs_node_t*)node;
	if (!(socket_node->flags&SOCKET_FLAG_WRITE)){
		return 0;
	}
	spinlock_acquire_exclusive(&(socket_node->write_lock));
	_Bool out=socket_node->descriptor->write_packet(socket_node,packet,size);
	spinlock_release_exclusive(&(socket_node->write_lock));
	return out;
}



KERNEL_PUBLIC _Bool socket_alloc_packet(vfs_node_t* node,void* data,u32 size){
	if ((node->flags&VFS_NODE_TYPE_MASK)!=VFS_NODE_TYPE_SOCKET){
		return 0;
	}
	socket_vfs_node_t* socket_node=(socket_vfs_node_t*)node;
	if (!(socket_node->flags&SOCKET_FLAG_READ)){
		return 0;
	}
	socket_packet_t* packet=omm_alloc(_socket_packet_allocator);
	packet->size=size;
	packet->data=data;
	return ring_push(socket_node->rx_ring,packet,0);
}



KERNEL_PUBLIC void socket_dealloc_packet(socket_packet_t* packet){
	amm_dealloc(packet->data);
	omm_dealloc(_socket_packet_allocator,packet);
}



KERNEL_PUBLIC event_t* socket_get_event(vfs_node_t* node){
	if ((node->flags&VFS_NODE_TYPE_MASK)!=VFS_NODE_TYPE_SOCKET){
		return NULL;
	}
	socket_vfs_node_t* socket_node=(socket_vfs_node_t*)node;
	return socket_node->rx_ring->read_event;
}



KERNEL_PUBLIC _Bool socket_move(vfs_node_t* node,const char* path){
	if ((node->flags&VFS_NODE_TYPE_MASK)!=VFS_NODE_TYPE_SOCKET){
		return 0;
	}
	vfs_node_dettach_external_child(node);
	if (!path){
		spinlock_acquire_exclusive(&(node->lock));
		smm_dealloc(node->name);
		node->name=_get_unique_id();
		spinlock_release_exclusive(&(node->lock));
		vfs_node_attach_external_child(_socket_root,node);
		return 1;
	}
	vfs_node_t* parent;
	const char* child_name;
	if (vfs_lookup_for_creation(NULL,path,0,0,0,&parent,&child_name)){
		ERROR("socket_move: node already exists");
		return 0;
	}
	spinlock_acquire_exclusive(&(node->lock));
	smm_dealloc(node->name);
	node->name=smm_alloc(child_name,0);
	spinlock_release_exclusive(&(node->lock));
	vfs_node_attach_external_child(parent,node);
	return 1;
}



error_t syscall_socket_create(socket_domain_t domain,socket_type_t type,socket_protocol_t protocol){
	vfs_node_t* out=socket_create(domain,type,protocol);
	return (out?fd_from_node(out,FD_FLAG_READ|FD_FLAG_WRITE):ERROR_INVALID_FORMAT);
}



error_t syscall_socket_create_pair(socket_domain_t domain,socket_type_t type,socket_protocol_t protocol,u64* out){
	if (syscall_get_user_pointer_max_length(out)<2*sizeof(handle_id_t)){
		return ERROR_INVALID_ARGUMENT(3);
	}
	socket_pair_t pair;
	if (!socket_create_pair(domain,type,protocol,&pair)){
		return ERROR_INVALID_FORMAT;
	}
	out[0]=fd_from_node((vfs_node_t*)(pair.sockets[0]),FD_FLAG_READ|FD_FLAG_WRITE);
	if (IS_ERROR(out[0])){
		return out[0];
	}
	out[1]=fd_from_node((vfs_node_t*)(pair.sockets[1]),FD_FLAG_READ|FD_FLAG_WRITE);
	return (IS_ERROR(out[1])?out[1]:ERROR_OK);
}



error_t syscall_socket_shutdown(handle_id_t fd,u32 flags){
	if (flags&(~(SOCKET_FLAG_READ|SOCKET_FLAG_WRITE))){
		return ERROR_INVALID_ARGUMENT(1);
	}
	vfs_node_t* node=fd_get_node(fd);
	if (!node){
		return ERROR_INVALID_HANDLE;
	}
	return (socket_shutdown(node,flags)?ERROR_OK:ERROR_UNSUPPORTED_OPERATION);
}



error_t syscall_socket_bind(handle_id_t fd,const void* address,u32 address_length){
	if (address_length>syscall_get_user_pointer_max_length(address)){
		return ERROR_INVALID_ARGUMENT(1);
	}
	vfs_node_t* node=fd_get_node(fd);
	if (!node){
		return ERROR_INVALID_HANDLE;
	}
	if ((node->flags&VFS_NODE_TYPE_MASK)!=VFS_NODE_TYPE_SOCKET){
		return ERROR_UNSUPPORTED_OPERATION;
	}
	return (socket_bind(node,address,address_length)?ERROR_OK:ERROR_INVALID_ADDRESS);
}



error_t syscall_socket_connect(handle_id_t fd,const void* address,u32 address_length){
	if (address_length>syscall_get_user_pointer_max_length(address)){
		return ERROR_INVALID_ARGUMENT(1);
	}
	vfs_node_t* node=fd_get_node(fd);
	if (!node){
		return ERROR_INVALID_HANDLE;
	}
	if ((node->flags&VFS_NODE_TYPE_MASK)!=VFS_NODE_TYPE_SOCKET){
		return ERROR_UNSUPPORTED_OPERATION;
	}
	return (socket_connect(node,address,address_length)?ERROR_OK:ERROR_INVALID_ADDRESS);
}



error_t syscall_socket_recv(handle_id_t fd,void* buffer,u32 buffer_length,u32 flags){
	if (flags&(~FD_FLAG_NONBLOCKING)){
		return ERROR_INVALID_ARGUMENT(3);
	}
	if (buffer_length>syscall_get_user_pointer_max_length(buffer)){
		return ERROR_INVALID_ARGUMENT(1);
	}
	vfs_node_t* node=fd_get_node(fd);
	if (!node){
		return ERROR_INVALID_HANDLE;
	}
	if ((node->flags&VFS_NODE_TYPE_MASK)!=VFS_NODE_TYPE_SOCKET){
		return ERROR_UNSUPPORTED_OPERATION;
	}
	if (!buffer){
		socket_packet_t* packet=socket_peek_packet(node,!!(flags&FD_FLAG_NONBLOCKING));
		if (!packet){
			return ERROR_NO_DATA;
		}
		return packet->size;
	}
	socket_packet_t* packet=socket_pop_packet(node,!!(flags&FD_FLAG_NONBLOCKING));
	if (!packet){
		return ERROR_NO_DATA;
	}
	memcpy(buffer,packet->data,(packet->size>buffer_length?buffer_length:packet->size));
	socket_dealloc_packet(packet);
	return (packet->size>buffer_length?ERROR_NO_SPACE:ERROR_OK);
}



error_t syscall_socket_send(handle_id_t fd,void* buffer,u32 buffer_length,u32 flags){
	if (flags&(~FD_FLAG_NONBLOCKING)){
		return ERROR_INVALID_ARGUMENT(3);
	}
	if (!buffer_length){
		return ERROR_INVALID_ARGUMENT(2);
	}
	if (buffer_length>syscall_get_user_pointer_max_length(buffer)){
		return ERROR_INVALID_ARGUMENT(1);
	}
	vfs_node_t* node=fd_get_node(fd);
	if (!node){
		return ERROR_INVALID_HANDLE;
	}
	if ((node->flags&VFS_NODE_TYPE_MASK)!=VFS_NODE_TYPE_SOCKET){
		return ERROR_UNSUPPORTED_OPERATION;
	}
	if (!(((socket_vfs_node_t*)node)->flags&SOCKET_FLAG_WRITE)){
		return ERROR_DISABLED_OPERATION;
	}
	return (socket_push_packet(node,buffer,buffer_length)?ERROR_OK:ERROR_NO_SPACE);
}
