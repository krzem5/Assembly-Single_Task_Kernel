#ifndef _NET_DHCP_H_
#define _NET_DHCP_H_ 1
#include <kernel/types.h>



// DHCPv6 message type
#define NET_DHCP6_MESSAGE_TYPE_SOLICIT 1
#define NET_DHCP6_MESSAGE_TYPE_ADVERTISE 2
#define NET_DHCP6_MESSAGE_TYPE_REQUEST 3
#define NET_DHCP6_MESSAGE_TYPE_REPLY 6

// DHCPv6 parameters
#define NET_DHCP6_PARAM_SOL_TIMEOUT_NS 1000000000ull
#define NET_DHCP6_PARAM_SOL_MAX_RT_NS 3600000000000ull

// DHCPv6 options
#define NET_DHCP6_OPTION_CLIENTID 1
#define NET_DHCP6_OPTION_SERVERID 2



typedef struct KERNEL_PACKED _NET_DHCP6_PACKET{
	u8 type_and_tid;
	u8 options[];
} net_dhcp6_packet_t;



void net_dhcp6_negotiate_address(void);



#endif
