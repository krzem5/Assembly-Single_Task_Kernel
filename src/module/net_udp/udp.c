#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/module/module.h>
#include <kernel/socket/port.h>
#include <kernel/socket/socket.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/util.h>
#include <net/checksum.h>
#include <net/ip4.h>
#include <net/udp.h>
#define KERNEL_LOG_NAME "net_udp"



#define PROTOCOL_TYPE 17



static omm_allocator_t* KERNEL_INIT_WRITE _net_udp_address_allocator=NULL;



static bool _socket_bind_callback(socket_vfs_node_t* socket_node,const void* address,u32 address_length){
	const net_udp_address_t* udp_address=(const net_udp_address_t*)address;
	if (address_length!=sizeof(net_udp_address_t)||!socket_port_reserve(socket_node,udp_address->port)){
		return 0;
	}
	net_udp_address_t* local_ctx=omm_alloc(_net_udp_address_allocator);
	*local_ctx=*udp_address;
	socket_node->local_ctx=local_ctx;
	return 1;
}



static void _socket_debind_callback(socket_vfs_node_t* socket_node){
	panic("_socket_debind_callback");
}



static bool _socket_connect_callback(socket_vfs_node_t* socket_node,const void* address,u32 address_length){
	const net_udp_address_t* udp_address=(const net_udp_address_t*)address;
	if (address_length!=sizeof(net_udp_address_t)||!socket_port_reserve(socket_node,udp_address->port)){
		return 0;
	}
	net_udp_address_t* remote_ctx=omm_alloc(_net_udp_address_allocator);
	*remote_ctx=*udp_address;
	socket_node->remote_ctx=remote_ctx;
	return 1;
}



static void _socket_deconnect_callback(socket_vfs_node_t* socket_node){
	panic("_socket_deconnect_callback");
}



static u64 _socket_read_callback(socket_vfs_node_t* socket_node,void* buffer,u64 length,u32 flags){
	socket_packet_t* socket_packet=socket_pop_packet(&(socket_node->node),!(flags&VFS_NODE_FLAG_NONBLOCKING));
	if (!socket_packet){
		return 0;
	}
	net_udp_socket_packet_t* packet=socket_packet->data;
	if (length>packet->length){
		length=packet->length;
	}
	mem_copy(buffer,packet->data,length);
	socket_dealloc_packet(socket_packet);
	return length;
}



static u64 _socket_write_callback(socket_vfs_node_t* socket_node,const void* buffer,u64 length){
	if (!socket_node->local_ctx||!socket_node->remote_ctx){
		return 0;
	}
	const net_udp_address_t* udp_local_address=(const net_udp_address_t*)(socket_node->local_ctx);
	const net_udp_address_t* udp_remote_address=(const net_udp_address_t*)(socket_node->remote_ctx);
	net_ip4_packet_t* ip_packet=net_ip4_create_packet(length+sizeof(net_udp_packet_t),udp_local_address->address,udp_remote_address->address,PROTOCOL_TYPE);
	if (!ip_packet){
		return 0;
	}
	net_udp_packet_t* udp_packet=(net_udp_packet_t*)(ip_packet->packet->data);
	udp_packet->src_port=__builtin_bswap16(udp_local_address->port);
	udp_packet->dst_port=__builtin_bswap16(udp_remote_address->port);
	udp_packet->length=__builtin_bswap16(length+sizeof(net_udp_packet_t));
	mem_copy(udp_packet->data,buffer,length);
	net_checksum_calculate_checksum(udp_packet,length+sizeof(net_udp_packet_t),&(udp_packet->checksum));
	net_udp_ipv4_pseudo_header_t pseudo_header={
		ip_packet->packet->src_address,
		ip_packet->packet->dst_address,
		0,
		PROTOCOL_TYPE,
		udp_packet->length
	};
	net_checksum_update_checksum(&pseudo_header,sizeof(net_udp_ipv4_pseudo_header_t),&(udp_packet->checksum));
	if (!udp_packet->checksum){
		udp_packet->checksum=0xffff;
	}
	net_ip4_send_packet(ip_packet);
	return length;
}



static bool _socket_write_packet_callback(socket_vfs_node_t* socket_node,const void* buffer,u32 length){
	if (length<sizeof(net_udp_socket_packet_t)){
		return 0;
	}
	const net_udp_socket_packet_t* packet=(const net_udp_socket_packet_t*)buffer;
	if (packet->length+sizeof(net_udp_socket_packet_t)>length){
		return 0;
	}
	net_ip4_packet_t* ip_packet=net_ip4_create_packet(packet->length+sizeof(net_udp_packet_t),packet->src_address,packet->dst_address,PROTOCOL_TYPE);
	if (!ip_packet){
		return 0;
	}
	net_udp_packet_t* udp_packet=(net_udp_packet_t*)(ip_packet->packet->data);
	udp_packet->src_port=__builtin_bswap16(packet->src_port);
	udp_packet->dst_port=__builtin_bswap16(packet->dst_port);
	udp_packet->length=__builtin_bswap16(packet->length+sizeof(net_udp_packet_t));
	mem_copy(udp_packet->data,packet->data,packet->length);
	net_checksum_calculate_checksum(udp_packet,packet->length+sizeof(net_udp_packet_t),&(udp_packet->checksum));
	net_udp_ipv4_pseudo_header_t pseudo_header={
		ip_packet->packet->src_address,
		ip_packet->packet->dst_address,
		0,
		PROTOCOL_TYPE,
		udp_packet->length
	};
	net_checksum_update_checksum(&pseudo_header,sizeof(net_udp_ipv4_pseudo_header_t),&(udp_packet->checksum));
	if (!udp_packet->checksum){
		udp_packet->checksum=0xffff;
	}
	net_ip4_send_packet(ip_packet);
	return 1;
}



static const socket_dtp_descriptor_t _net_udp_socket_dtp_descriptor={
	"UDP",
	SOCKET_DOMAIN_INET,
	SOCKET_TYPE_DGRAM,
	SOCKET_PROTOCOL_UDP,
	NULL,
	_socket_bind_callback,
	_socket_debind_callback,
	_socket_connect_callback,
	_socket_deconnect_callback,
	_socket_read_callback,
	_socket_write_callback,
	_socket_write_packet_callback
};



static void _rx_callback(net_ip4_packet_t* packet){
	if (packet->length<sizeof(net_udp_packet_t)){
		return;
	}
	net_udp_packet_t* udp_packet=(net_udp_packet_t*)(packet->packet->data);
	if (__builtin_bswap16(udp_packet->length)!=packet->length){
		ERROR("Wrong UDP length");
		return;
	}
	if (udp_packet->checksum){
		net_udp_ipv4_pseudo_header_t pseudo_header={
			packet->packet->src_address,
			packet->packet->dst_address,
			0,
			PROTOCOL_TYPE,
			udp_packet->length
		};
		if (net_checksum_verify_checksum(udp_packet,packet->length,net_checksum_verify_checksum(&pseudo_header,sizeof(net_udp_ipv4_pseudo_header_t),0))!=0xffff){
			ERROR("Wrong UDP checksum");
			return;
		}
	}
	u16 port=__builtin_bswap16(udp_packet->dst_port);
	socket_vfs_node_t* socket=socket_port_get(port);
	if (!socket||socket->descriptor!=&_net_udp_socket_dtp_descriptor){
		ERROR("No UDP socket on port %u",port);
		return;
	}
	net_udp_socket_packet_t* socket_packet=amm_alloc(sizeof(net_udp_socket_packet_t)+packet->length-sizeof(net_udp_packet_t));
	socket_packet->src_address=__builtin_bswap32(packet->packet->src_address);
	socket_packet->dst_address=__builtin_bswap32(packet->packet->dst_address);
	socket_packet->src_port=__builtin_bswap16(udp_packet->src_port);
	socket_packet->dst_port=__builtin_bswap16(udp_packet->dst_port);
	socket_packet->length=packet->length-sizeof(net_udp_packet_t);
	mem_copy(socket_packet->data,udp_packet->data,packet->length-sizeof(net_udp_packet_t));
	if (!socket_alloc_packet(&(socket->node),socket_packet,sizeof(net_udp_socket_packet_t)+packet->length-sizeof(net_udp_packet_t))){
		amm_dealloc(packet);
		ERROR("UDP packet dropped, socket rx ring full");
	}
}



static const net_ip4_protocol_descriptor_t _net_udp_ip4_protocol_descriptor={
	"UDP",
	PROTOCOL_TYPE,
	_rx_callback
};



MODULE_INIT(){
	_net_udp_address_allocator=omm_init("net.udp.address",sizeof(net_udp_address_t),8,4);
	rwlock_init(&(_net_udp_address_allocator->lock));
}



MODULE_POSTINIT(){
	LOG("Registering UDP protocol...");
	socket_register_dtp_descriptor(&_net_udp_socket_dtp_descriptor);
	net_ip4_register_protocol_descriptor(&_net_udp_ip4_protocol_descriptor);
}
