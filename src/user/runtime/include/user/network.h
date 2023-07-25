#ifndef _USER_NETWORK_H_
#define _USER_NETWORK_H_ 1
#include <user/types.h>



typedef struct _NETWORK_PACKET{
	u8 address[6];
	u16 protocol;
	u16 buffer_length;
	void* buffer;
} network_packet_t;



_Bool network_send(const network_packet_t* packet);



_Bool network_poll(network_packet_t* packet);



#endif
