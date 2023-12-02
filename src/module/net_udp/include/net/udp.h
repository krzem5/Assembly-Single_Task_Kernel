#ifndef _NET_UDP_H_
#define _NET_UDP_H_ 1
#include <kernel/types.h>
#include <net/ip4.h>



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



void net_udp_register_protocol(void);



net_udp_packet_t* net_udp_create_packet(u16 length);



void net_udp_delete_packet(net_udp_packet_t* packet);



void net_udp_send_packet(net_udp_packet_t* packet);



#endif
