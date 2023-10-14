#ifndef _USER_NETWORK_H_
#define _USER_NETWORK_H_ 1
#include <user/types.h>



#define NETWORK_DEVICE_FLAG_ONLINE 0x01
#define NETWORK_DEVICE_FLAG_LOG_TARGET 0x02



typedef struct _NETWORK_PACKET{
	u8 address[6];
	u16 protocol;
	u16 buffer_length;
	void* buffer;
} network_packet_t;



typedef struct _NETWORK_DEVICE{
	u8 flags;
	u8 address[6];
	u8 uuid[16];
	char serial_number[33];
	u64 ping;
	u64 last_ping_time;
} network_device_t;



_Bool network_get_mac_address(u8* mac_address);



_Bool network_send(const network_packet_t* packet);



_Bool network_poll(network_packet_t* packet,_Bool block);



#endif
