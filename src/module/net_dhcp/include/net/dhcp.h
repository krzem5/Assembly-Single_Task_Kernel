#ifndef _NET_DHCP_H_
#define _NET_DHCP_H_ 1
#include <kernel/types.h>



// DHCP packet types
#define NET_DHCP_OP_BOOTREQUEST 1
#define NET_DHCP_OP_BOOTREPLY 2

// DHCP parameters
#define NET_DHCP_PARAM_DHCPACK_TIMEOUT_NS 60000000000ull

// DHCP options
#define NET_DHCP_OPTION_SUBNET_MASK 1
#define NET_DHCP_OPTION_ROUTER 3
#define NET_DHCP_OPTION_DOMAIN_NAME_SERVER 6
#define NET_DHCP_OPTION_REQUESTED_IP_ADDRESS 50
#define NET_DHCP_OPTION_IP_ADDRESS_LEASE_TIME 51
#define NET_DHCP_OPTION_MESSAGE_TYPE 53
#define NET_DHCP_OPTION_SERVER_IDENTIFIER 54
#define NET_DHCP_OPTION_END 255

// DHCP message types
#define NET_DHCP_MESSAGE_TYPE_NONE 0
#define NET_DHCP_MESSAGE_TYPE_DHCPDISCOVER 1
#define NET_DHCP_MESSAGE_TYPE_DHCPOFFER 2
#define NET_DHCP_MESSAGE_TYPE_DHCPREQUEST 3
#define NET_DHCP_MESSAGE_TYPE_DHCPDECLINE 4
#define NET_DHCP_MESSAGE_TYPE_DHCPACK 5
#define NET_DHCP_MESSAGE_TYPE_DHCPNAK 6
#define NET_DHCP_MESSAGE_TYPE_DHCPRELEASE 7
#define NET_DHCP_MESSAGE_TYPE_DHCPINFORM 8

// DHCP cookie
#define NET_DHCP_COOKIE 0x63825363

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



#define NET_DHCP_PACKET_ITER_OPTIONS(dhcp_packet) for (u32 i=0;(dhcp_packet)->options[i]!=NET_DHCP_OPTION_END;i+=((dhcp_packet)->options[i]?(dhcp_packet)->options[i+1]+2:1))



typedef struct KERNEL_PACKED _NET_DHCP_PACKET{
	u8 op;
	u8 htype;
	u8 hlen;
	u8 hops;
	u32 xid;
	u16 secs;
	u16 flags;
	u32 ciaddr;
	u32 yiaddr;
	u32 siaddr;
	u32 giaddr;
	u8 chaddr[16];
	char sname[64];
	char file[128];
	u32 cookie;
	u8 options[];
} net_dhcp_packet_t;



void net_dhcp_negotiate_address(void);



#endif
