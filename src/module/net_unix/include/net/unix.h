#ifndef _NET_UNIX_H_
#define _NET_UNIX_H_ 1
#include <kernel/types.h>



typedef struct _NET_UNIX_ADDRESS{
	char path[256];
} net_unix_address_t;



void net_unix_init(void);



#endif
