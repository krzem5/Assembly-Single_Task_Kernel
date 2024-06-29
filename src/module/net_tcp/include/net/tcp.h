#ifndef _NET_TCP_H_
#define _NET_TCP_H_ 1
#include <kernel/socket/port.h>
#include <kernel/types.h>
#include <net/ip4.h>



typedef struct _NET_TCP_ADDRESS{
	net_ip4_address_t address;
	socket_port_t port;
} net_tcp_address_t;



#endif
