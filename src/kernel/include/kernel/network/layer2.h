#ifndef _KERNEL_NETWORK_LAYER2_H_
#define _KERNEL_NETWORK_LAYER2_H_ 1
#include <kernel/network/layer1.h>
#include <kernel/types.h>



typedef struct _NETWORK_LAYER2_PACKET{
	mac_address_t address;
	u16 protocol;
	u16 buffer_length;
	void* buffer;
} network_layer2_packet_t;



void network_layer2_init(void);



_Bool network_layer2_send(const network_layer2_packet_t* packet);



_Bool network_layer2_poll(network_layer2_packet_t* packet);



#endif
