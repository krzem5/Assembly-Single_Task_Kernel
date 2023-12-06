#ifndef _NET_DNS_H_
#define _NET_DNS_H_ 1
#include <kernel/types.h>
#include <net/ip4.h>



void net_dns_init(void);



net_ip4_address_t net_dns_lookup_name(const char* name);



#endif
