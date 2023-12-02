#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/types.h>
#include <net/common.h>
#include <net/ip4.h>
#include <net/udp.h>
#define KERNEL_LOG_NAME "net_udp"



#define PROTOCOL_TYPE 17



static omm_allocator_t* _net_udp_packet_allocator=NULL;



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
	net_ip4_register_protocol_descriptor(&_net_udp_ip4_protocol_descriptor);
	_net_udp_packet_allocator=omm_init("net_udp_packet",sizeof(net_udp_packet_t),8,4,pmm_alloc_counter("omm_net_udp_packet"));
	net_udp_send_packet(net_udp_create_packet(0,0xffffffff,68,67));
}



KERNEL_PUBLIC net_udp_packet_t* net_udp_create_packet(u16 length,net_ip4_address_t address,u16 src_port,u16 dst_port){
	net_ip4_packet_t* raw_packet=net_ip4_create_packet(length+sizeof(net_udp_packet_data_t),net_ip4_address,address,PROTOCOL_TYPE);
	if (!raw_packet){
		return NULL;
	}
	net_udp_packet_t* out=omm_alloc(_net_udp_packet_allocator);
	out->length=length;
	out->raw_packet=raw_packet;
	out->packet=(net_udp_packet_data_t*)(raw_packet->packet->data);
	out->packet->src_port=__builtin_bswap16(src_port);
	out->packet->dst_port=__builtin_bswap16(dst_port);
	out->packet->length=__builtin_bswap16(length+sizeof(net_udp_packet_data_t));
	return out;
}



KERNEL_PUBLIC void net_udp_delete_packet(net_udp_packet_t* packet){
	net_ip4_delete_packet(packet->raw_packet);
	omm_dealloc(_net_udp_packet_allocator,packet);
}



KERNEL_PUBLIC void net_udp_send_packet(net_udp_packet_t* packet){
	net_common_calculate_checksum(packet->packet,packet->length+sizeof(net_udp_packet_data_t),&(packet->packet->checksum));
	net_udp_ipv4_pseudo_header_t pseudo_header={
		packet->raw_packet->packet->src_address,
		packet->raw_packet->packet->dst_address,
		0,
		PROTOCOL_TYPE,
		packet->packet->length
	};
	net_common_update_checksum(&pseudo_header,sizeof(net_udp_ipv4_pseudo_header_t),&(packet->packet->checksum));
	net_ip4_send_packet(packet->raw_packet);
	omm_dealloc(_net_udp_packet_allocator,packet);
}
