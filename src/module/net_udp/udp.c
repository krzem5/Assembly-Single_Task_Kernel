#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/socket/port.h>
#include <kernel/socket/socket.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <net/common.h>
#include <net/ip4.h>
#include <net/udp.h>
#define KERNEL_LOG_NAME "net_udp"



#define PROTOCOL_TYPE 17



static omm_allocator_t* _net_udp_address_allocator=NULL;



static _Bool _socket_bind_callback(socket_vfs_node_t* socket_node,const void* address,u32 address_length){
	const net_udp_address_t* udp_address=(const net_udp_address_t*)address;
	if (address_length!=sizeof(net_udp_address_t)||!socket_port_reserve(socket_node,udp_address->port)){
		return 0;
	}
	net_udp_address_t* local_ctx=omm_alloc(_net_udp_address_allocator);
	*local_ctx=*udp_address;
	socket_node->handler_local_ctx=local_ctx;
	return 1;
}



static void _socket_debind_callback(socket_vfs_node_t* socket_node){
	panic("_socket_debind_callback");
}



static _Bool _socket_connect_callback(socket_vfs_node_t* socket_node,const void* address,u32 address_length){
	const net_udp_address_t* udp_address=(const net_udp_address_t*)address;
	if (address_length!=sizeof(net_udp_address_t)||!socket_port_reserve(socket_node,udp_address->port)){
		return 0;
	}
	net_udp_address_t* remote_ctx=omm_alloc(_net_udp_address_allocator);
	*remote_ctx=*udp_address;
	socket_node->handler_remote_ctx=remote_ctx;
	return 1;
}



static void _socket_deconnect_callback(socket_vfs_node_t* socket_node){
	panic("_socket_deconnect_callback");
}



static u64 _socket_write_callback(socket_vfs_node_t* socket_node,const void* buffer,u64 length){
	if (!socket_node->handler_local_ctx||!socket_node->handler_remote_ctx){
		return 0;
	}
	const net_udp_address_t* udp_local_address=(const net_udp_address_t*)(socket_node->handler_local_ctx);
	const net_udp_address_t* udp_remote_address=(const net_udp_address_t*)(socket_node->handler_remote_ctx);
	net_ip4_packet_t* ip_packet=net_ip4_create_packet(length+sizeof(net_udp_packet_t),udp_local_address->address,udp_remote_address->address,PROTOCOL_TYPE);
	if (!ip_packet){
		return 0;
	}
	net_udp_packet_t* udp_packet=(net_udp_packet_t*)(ip_packet->packet->data);
	udp_packet->src_port=__builtin_bswap16(udp_local_address->port);
	udp_packet->dst_port=__builtin_bswap16(udp_remote_address->port);
	udp_packet->length=__builtin_bswap16(length+sizeof(net_udp_packet_t));
	memcpy(udp_packet->data,buffer,length);
	net_common_calculate_checksum(udp_packet,length+sizeof(net_udp_packet_t),&(udp_packet->checksum));
	net_udp_ipv4_pseudo_header_t pseudo_header={
		ip_packet->packet->src_address,
		ip_packet->packet->dst_address,
		0,
		PROTOCOL_TYPE,
		udp_packet->length
	};
	net_common_update_checksum(&pseudo_header,sizeof(net_udp_ipv4_pseudo_header_t),&(udp_packet->checksum));
	if (!udp_packet->checksum){
		udp_packet->checksum=0xffff;
	}
	net_ip4_send_packet(ip_packet);
	return length;
}



static socket_dtp_descriptor_t _net_udp_socket_dtp_descriptor={
	"UDP",
	SOCKET_DOMAIN_INET,
	SOCKET_TYPE_DGRAM,
	SOCKET_PROTOCOL_UDP,
	_socket_bind_callback,
	_socket_debind_callback,
	_socket_connect_callback,
	_socket_deconnect_callback,
	_socket_write_callback
};



static void _rx_callback(const net_ip4_packet_t* packet){
	WARN("PACKET (UDP)");
}



static net_ip4_protocol_descriptor_t _net_udp_ip4_protocol_descriptor={
	"UDP",
	PROTOCOL_TYPE,
	_rx_callback
};



void net_udp_init(void){
	LOG("Registering UDP protocol...");
	socket_register_dtp_descriptor(&_net_udp_socket_dtp_descriptor);
	net_ip4_register_protocol_descriptor(&_net_udp_ip4_protocol_descriptor);
	_net_udp_address_allocator=omm_init("net_udp_address",sizeof(net_udp_address_t),8,4,pmm_alloc_counter("omm_net_udp_address"));
}
