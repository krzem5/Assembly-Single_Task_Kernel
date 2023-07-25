#ifndef _KERNEL_NETWORK_LAYER1_H_
#define _KERNEL_NETWORK_LAYER1_H_ 1
#include <kernel/types.h>



typedef u8 mac_address_t[6];



typedef struct _NETWORK_LAYER1_DEVICE{
	const char* name;
	mac_address_t mac_address;
	void (*tx)(void*,u64,u16);
	u16 (*rx)(void*,void*,u16);
	void* extra_data;
} network_layer1_device_t;



static inline void mac_address_init(mac_address_t* mac_address){
	(*mac_address)[0]=0x00;
	(*mac_address)[1]=0x00;
	(*mac_address)[2]=0x00;
	(*mac_address)[3]=0x00;
	(*mac_address)[4]=0x00;
	(*mac_address)[5]=0x00;
}



extern const char* network_layer1_name;
extern mac_address_t network_layer1_mac_address;



void network_layer1_init(void);



void network_layer1_set_device(const network_layer1_device_t* device);



void network_layer1_send(u64 packet,u16 length);



u16 network_layer1_poll(void* buffer,u16 buffer_length);



#endif
