#ifndef _COMMON_UPDATE_UPDATE_H_
#define _COMMON_UPDATE_UPDATE_H_ 1
#include <common/types.h>



typedef struct __attribute__((packed)) _UPDATE_TICKET{
	u8 hmac[32];
	u8 encrypted_data[64];
} update_ticket_t;



#endif
