#ifndef _KERNEL_NETWORK_LAYER1_H_
#define _KERNEL_NETWORK_LAYER1_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/types.h>



#define NETWORK_LAYER1_PACKET_HEADER_SIZE (__builtin_offsetof(network_layer1_packet_t,data)-__builtin_offsetof(network_layer1_packet_t,raw_data))



typedef u8 mac_address_t[6];



typedef u16 ether_type_t;



typedef struct KERNEL_PACKED _NETWORK_LAYER1_PACKET{
	u16 length;
	u8 _padding[6];
	u8 raw_data[0];
	mac_address_t dst_mac;
	mac_address_t src_mac;
	ether_type_t ether_type;
	u8 data[];
} network_layer1_packet_t;



typedef struct _NETWORK_LAYER1_DEVICE_DESCRIPTOR{
	const char* name;
	void (*tx)(void*,const network_layer1_packet_t*);
	network_layer1_packet_t* (*rx)(void*);
	void (*wait)(void*);
} network_layer1_device_descriptor_t;



typedef struct _NETWORK_LAYER1_DEVICE{
	handle_t handle;
	const network_layer1_device_descriptor_t* descriptor;
	mac_address_t mac_address;
	void* extra_data;
} network_layer1_device_t;



extern handle_type_t network_layer1_device_handle_type;
extern network_layer1_device_t* network_layer1_device;



void network_layer1_init(void);



void network_layer1_create_device(const network_layer1_device_descriptor_t* descriptor,const mac_address_t* mac_address,void* extra_data);



network_layer1_packet_t* network_layer1_create_packet(u16 size,const mac_address_t* dst_mac_address,const mac_address_t* src_mac_address,ether_type_t ether_type);



void network_layer1_delete_packet(network_layer1_packet_t* packet);



void network_layer1_send_packet(network_layer1_packet_t* packet);



#endif
