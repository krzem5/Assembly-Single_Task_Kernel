#ifndef _KERNEL_NETWORK_LAYER3_H_
#define _KERNEL_NETWORK_LAYER3_H_ 1
#include <kernel/types.h>



#define NETWORK_LAYER3_PROTOCOL_TYPE 0x4e46

#define NETWORK_LAYER3_PACKET_TYPE_PING_PONG 0x00
#define NETWORK_LAYER3_PACKET_TYPE_ENUMERATION 0x01

#define NETWORK_LAYER3_DEVICE_FLAG_ONLINE 0x01
#define NETWORK_LAYER3_DEVICE_FLAG_LOG_TARGET 0x02



typedef struct _NETWORK_LAYER3_DEVICE{
	u8 flags;
	u8 address[6];
	u8 uuid[16];
	char serial_number[33];
	u64 ping;
	u64 last_ping_time;
} network_layer3_device_t;



void network_layer3_init(void);



void network_layer3_process_packet(const u8* address,u16 buffer_length,const u8* buffer);



void network_layer3_refresh_device_list(void);



u32 network_layer3_get_device_count(void);



const network_layer3_device_t* network_layer3_get_device(u32 index);



_Bool network_layer3_delete_device(const u8* address);



void netork_layer3_flush_cache(void);



#endif
