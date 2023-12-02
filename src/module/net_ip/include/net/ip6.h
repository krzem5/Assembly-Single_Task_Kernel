#ifndef _NET_IP6_H_
#define _NET_IP6_H_ 1
#include <kernel/types.h>



typedef struct _NET_IP6_ADDRESS{
	u64 prefix;
	u64 identifier;
} net_ip6_address_t;



void net_ip6_register_protocol(void);



#endif
