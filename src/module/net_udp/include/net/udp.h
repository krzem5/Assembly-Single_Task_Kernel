#ifndef _NET_UDP_H_
#define _NET_UDP_H_ 1
#include <kernel/types.h>
#include <net/ip4.h>



typedef struct _NET_UDP_ADDRESS{
	net_ip4_address_t address;
	u16 port;
} net_udp_address_t;



typedef struct KERNEL_PACKED _NET_UDP_IPV4_PSEUDO_HEADER{
	net_ip4_address_t src_address;
	net_ip4_address_t dst_address;
	u8 zero;
	u8 protocol;
	u16 length;
} net_udp_ipv4_pseudo_header_t;



typedef struct KERNEL_PACKED _NET_UDP_PACKET_DATA{
	u16 src_port;
	u16 dst_port;
	u16 length;
	u16 checksum;
	u8 data[];
} net_udp_packet_data_t;



typedef struct _NET_UDP_PACKET{
	u16 length;
	net_ip4_packet_t* raw_packet;
	net_udp_packet_data_t* packet;
} net_udp_packet_t;



void net_udp_init(void);



net_udp_packet_t* net_udp_create_packet(u16 length,net_ip4_address_t address,u16 src_port,u16 dst_port);



void net_udp_delete_packet(net_udp_packet_t* packet);



void net_udp_send_packet(net_udp_packet_t* packet);



#endif
