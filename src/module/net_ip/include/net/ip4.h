#ifndef _NET_IP4_H_
#define _NET_IP4_H_ 1
#include <kernel/network/layer1.h>
#include <kernel/types.h>



typedef u32 net_ip4_address_t;



typedef u8 net_ip4_protocol_type_t;



typedef struct KERNEL_PACKED _NET_IP4_PACKET_DATA{
	u8 version_and_ihl;
	u8 dscp_and_ecn;
	u16 total_length;
	u16 identification;
	u16 fragment;
	u8 ttl;
	u8 protocol;
	u16 checksum;
	u32 src_address;
	u32 dst_address;
	u8 data[];
} net_ip4_packet_data_t;



typedef struct _NET_IP4_PACKET{
	u16 length;
	network_layer1_packet_t* raw_packet;
	net_ip4_packet_data_t* packet;
} net_ip4_packet_t;



typedef struct _NET_IP4_PROTOCOL_DESCRIPTOR{
	const char* name;
	net_ip4_protocol_type_t protocol_type;
	void (*rx_callback)(net_ip4_packet_t* packet);
} net_ip4_protocol_descriptor_t;



typedef struct _NET_IP4_PROTOCOL{
	rb_tree_node_t rb_node;
	const net_ip4_protocol_descriptor_t* descriptor;
} net_ip4_protocol_t;



void net_ip4_init(void);



void net_ip4_register_protocol_descriptor(const net_ip4_protocol_descriptor_t* descriptor);



void net_ip4_unregister_protocol_descriptor(const net_ip4_protocol_descriptor_t* descriptor);



net_ip4_packet_t* net_ip4_create_packet(u16 length,net_ip4_address_t src_address,net_ip4_address_t dst_address,net_ip4_protocol_type_t protocol_type);



void net_ip4_delete_packet(net_ip4_packet_t* packet);



void net_ip4_send_packet(net_ip4_packet_t* packet);



#endif
