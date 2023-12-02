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



typedef struct _SOCKET_VFS_NODE{
	vfs_node_t node;
	spinlock_t lock;
	socket_domain_t domain;
	socket_type_t type;
	socket_protocol_t protocol;
	event_t* read_event;
	event_t* write_event;
} socket_vfs_node_t;



static spinlock_t _socket_dtp_lock;
static rb_tree_t _socket_dtp_tree;
static omm_allocator_t* KERNEL_INIT_WRITE _socket_vfs_node_allocator=NULL;



static vfs_node_t* _socket_create(void){
	socket_vfs_node_t* out=omm_alloc(_socket_vfs_node_allocator);
	spinlock_init(&(out->lock));
	out->domain=SOCKET_DOMAIN_NONE;
	out->type=SOCKET_TYPE_NONE;
	out->protocol=SOCKET_PROTOCOL_NONE;
	out->read_event=event_new();
	out->write_event=event_new();
	return (vfs_node_t*)out;
}



static u64 _socket_read(vfs_node_t* node,u64 offset,void* buffer,u64 size,u32 flags){
	return 0;
}



static u64 _socket_write(vfs_node_t* node,u64 offset,const void* buffer,u64 size,u32 flags){
	return 0;
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



void KERNEL_EARLY_EXEC socket_init(void){
	LOG("Initializing sockets...");
	spinlock_init(&_socket_dtp_lock);
	rb_tree_init(&_socket_dtp_tree);
	_socket_vfs_node_allocator=omm_init("socket_node",sizeof(socket_vfs_node_t),8,4,pmm_alloc_counter("omm_socket_node"));
	spinlock_init(&(_socket_vfs_node_allocator->lock));
}



KERNEL_PUBLIC vfs_node_t* socket_create(vfs_node_t* parent,const string_t* name,socket_domain_t domain,socket_type_t type,socket_protocol_t protocol){
	spinlock_acquire_shared(&_socket_dtp_lock);
	rb_tree_node_t* dtp_node=rb_tree_lookup_node(&_socket_dtp_tree,CREATE_DTP_KEY(domain,type,protocol));
	spinlock_release_shared(&_socket_dtp_lock);
	if (!dtp_node){
		return NULL;
	}
	vfs_node_t* out=vfs_node_create_virtual(parent,&_socket_vfs_functions,name);
	out->flags|=VFS_NODE_TYPE_SOCKET;
	((socket_vfs_node_t*)out)->domain=domain;
	((socket_vfs_node_t*)out)->type=type;
	((socket_vfs_node_t*)out)->protocol=protocol;
	return out;
}
