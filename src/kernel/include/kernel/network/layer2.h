#ifndef _KERNEL_NETWORK_LAYER2_H_
#define _KERNEL_NETWORK_LAYER2_H_ 1
#include <kernel/network/layer1.h>
#include <kernel/types.h>



typedef struct _NETWORK_LAYER2_PROTOCOL_DESCRIPTOR{
	const char* name;
	ether_type_t ether_type;
	void (*rx_callback)(const network_layer1_packet_t* packet);
} network_layer2_protocol_descriptor_t;



typedef struct _NETWORK_LAYER2_PROTOCOL{
	rb_tree_node_t rb_node;
	const network_layer2_protocol_descriptor_t* descriptor;
} network_layer2_protocol_t;



void network_layer2_init(void);



void network_layer2_register_descriptor(const network_layer2_protocol_descriptor_t* descriptor);



void network_layer2_unregister_descriptor(const network_layer2_protocol_descriptor_t* descriptor);



void network_layer2_process_packet(const network_layer1_packet_t* packet);



#endif
