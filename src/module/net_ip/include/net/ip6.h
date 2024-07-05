#ifndef _NET_IP6_H_
#define _NET_IP6_H_ 1
#include <kernel/network/layer1.h>
#include <kernel/types.h>



typedef u8 net_ip6_protocol_type_t;



typedef struct _NET_IP6_ADDRESS{
	u16 data[8];
} net_ip6_address_t;



typedef struct KERNEL_PACKED _NET_IP6_PACKET_DATA{
	u8 version_traffic_class_flow_label[4];
	u16 payload_length;
	u8 next_header;
	u8 hop_limit;
	u16 src_address[8];
	u16 dst_address[8];
	u8 data[];
} net_ip6_packet_data_t;



typedef struct _NET_IP6_PACKET{
	u16 length;
	network_layer1_packet_t* raw_packet;
	net_ip6_packet_data_t* packet;
} net_ip6_packet_t;



typedef struct _NET_IP6_PROTOCOL_DESCRIPTOR{
	const char* name;
	net_ip6_protocol_type_t protocol_type;
	void (*rx_callback)(net_ip6_packet_t* packet);
} net_ip6_protocol_descriptor_t;



typedef struct _NET_IP6_PROTOCOL{
	rb_tree_node_t rb_node;
	const net_ip6_protocol_descriptor_t* descriptor;
} net_ip6_protocol_t;



void net_ip6_register_protocol_descriptor(const net_ip6_protocol_descriptor_t* descriptor);



void net_ip6_unregister_protocol_descriptor(const net_ip6_protocol_descriptor_t* descriptor);



net_ip6_packet_t* net_ip6_create_packet(u16 length,const net_ip6_address_t* src_address,const net_ip6_address_t* dst_address,net_ip6_protocol_type_t protocol_type);



void net_ip6_delete_packet(net_ip6_packet_t* packet);



void net_ip6_send_packet(net_ip6_packet_t* packet);



bool net_ip6_address_from_string(const char* str,net_ip6_address_t* out);



bool net_ip6_address_from_string_format(const char* template,net_ip6_address_t* out,...);



#endif
