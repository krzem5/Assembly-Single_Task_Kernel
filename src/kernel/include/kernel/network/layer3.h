#ifndef _KERNEL_NETWORK_LAYER3_H_
#define _KERNEL_NETWORK_LAYER3_H_ 1
#include <kernel/types.h>



#define NETWORK_LAYER3_PROTOCOL_TYPE 0x4e46

#define NETWORK_LAYER3_PACKET_TYPE_PING_PONG 0x00
#define NETWORK_LAYER3_PACKET_TYPE_ENUMERATION 0x01
#define NETWORK_LAYER3_PACKET_TYPE_ADDRESS_RESOLUTION 0x02



void network_layer3_init(void);



void network_layer3_process_packet(const u8* address,u16 buffer_length,const u8* buffer);



void network_layer3_refresh_device_list(void);



#endif
