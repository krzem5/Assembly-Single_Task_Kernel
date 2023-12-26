#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/socket/socket.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <net/unix.h>
#define KERNEL_LOG_NAME "net_unix"



static void _socket_create_pair_callback(socket_pair_t* pair){
	panic("_socket_create_pair_callback");
}



static _Bool _socket_bind_callback(socket_vfs_node_t* socket_node,const void* address,u32 address_length){
	panic("_socket_bind_callback");
}



static void _socket_debind_callback(socket_vfs_node_t* socket_node){
	panic("_socket_debind_callback");
}



static _Bool _socket_connect_callback(socket_vfs_node_t* socket_node,const void* address,u32 address_length){
	panic("_socket_connect_callback");
}



static void _socket_deconnect_callback(socket_vfs_node_t* socket_node){
	panic("_socket_deconnect_callback");
}



static u64 _socket_read_callback(socket_vfs_node_t* socket_node,void* buffer,u64 length,u32 flags){
	panic("_socket_read_callback");
}



static u64 _socket_write_callback(socket_vfs_node_t* socket_node,const void* buffer,u64 length){
	panic("_socket_write_callback");
}



static _Bool _socket_write_packet_callback(socket_vfs_node_t* socket_node,const void* buffer,u32 length){
	panic("_socket_write_packet_callback");
}



static const socket_dtp_descriptor_t _net_unix_socket_dtp_descriptor={
	"UNIX",
	SOCKET_DOMAIN_UNIX,
	SOCKET_TYPE_DGRAM,
	SOCKET_PROTOCOL_NONE,
	_socket_create_pair_callback,
	_socket_bind_callback,
	_socket_debind_callback,
	_socket_connect_callback,
	_socket_deconnect_callback,
	_socket_read_callback,
	_socket_write_callback,
	_socket_write_packet_callback
};



void net_unix_init(void){
	LOG("Registering UNIX datagram sockets...");
	socket_register_dtp_descriptor(&_net_unix_socket_dtp_descriptor);
}
