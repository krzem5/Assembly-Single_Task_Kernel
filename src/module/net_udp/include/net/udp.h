#ifndef _NET_UDP_H_
#define _NET_UDP_H_ 1
#include <kernel/socket/port.h>
#include <kernel/types.h>
#include <net/ip4.h>
#include <net/ip6.h>



typedef struct _NET_UDP_IP4_ADDRESS{
	net_ip4_address_t address;
	socket_port_t port;
} net_udp_ip4_address_t;



typedef struct _NET_UDP_IP4_SOCKET_PACKET{
	net_udp_ip4_address_t src_address;
	net_udp_ip4_address_t dst_address;
	u16 length;
	u8 data[];
} net_udp_ip4_socket_packet_t;



typedef struct KERNEL_PACKED _NET_UDP_IPV4_PSEUDO_HEADER{
	net_ip4_address_t src_address;
	net_ip4_address_t dst_address;
	u8 zero;
	u8 protocol;
	u16 length;
} net_udp_ipv4_pseudo_header_t;



typedef struct KERNEL_PACKED _NET_UDP_PACKET{
	socket_port_t src_port;
	socket_port_t dst_port;
	u16 length;
	u16 checksum;
	u8 data[];
} net_udp_packet_t;



#endif
