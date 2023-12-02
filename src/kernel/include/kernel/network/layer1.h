#ifndef _KERNEL_NETWORK_LAYER1_H_
#define _KERNEL_NETWORK_LAYER1_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/types.h>



typedef u8 mac_address_t[6];



typedef struct _NETWORK_LAYER1_DEVICE_DESCRIPTOR{
	const char* name;
	void (*tx)(void*,const void*,u16);
	u16 (*rx)(void*,void*,u16);
	void (*wait)(void*);
} network_layer1_device_descriptor_t;



typedef struct _NETWORK_LAYER1_DEVICE{
	handle_t handle;
	const network_layer1_device_descriptor_t* descriptor;
	mac_address_t mac_address;
	void* extra_data;
} network_layer1_device_t;



typedef struct KERNEL_PACKED _NETWORK_LAYER1_PACKET{
	u16 length;
	u8 raw_data[0];
	mac_address_t dst_mac;
	mac_address_t src_mac;
	u16 ether_type;
	u8 data[];
} network_layer1_packet_t;



void network_layer1_create_device(const network_layer1_device_descriptor_t* descriptor,const mac_address_t* mac_address,void* extra_data);



network_layer1_packet_t* network_layer1_create_packet(u16 size,const mac_address_t* mac_address,u16 ether_type);



void network_layer1_delete_packet(network_layer1_packet_t* packet);



void network_layer1_send_packet(network_layer1_packet_t* packet);



#endif
