#ifndef _USER_LAYER3_H_
#define _USER_LAYER3_H_ 1
#include <user/types.h>



#define LAYER3_PROTOCOL 0x4e4d

#define LAYER3_PACKET_TYPE_PING_PONG 0x00
#define LAYER3_PACKET_TYPE_ENUMERATION 0x01
#define LAYER3_PACKET_TYPE_ADDRESS_RESOLUTION 0x02



void layer3_init(void);



void layer3_poll(void);



#endif
