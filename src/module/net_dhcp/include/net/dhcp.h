#ifndef _NET_DHCP_H_
#define _NET_DHCP_H_ 1
#include <kernel/types.h>



// Packet types
#define NET_DHCP_OP_BOOTREQUEST 1
#define NET_DHCP_OP_BOOTREPLY 2

// Message types
#define NET_DHCP_MESSAGE_TYPE_NONE 0
#define NET_DHCP_MESSAGE_TYPE_DHCPDISCOVER 1
#define NET_DHCP_MESSAGE_TYPE_DHCPOFFER 2
#define NET_DHCP_MESSAGE_TYPE_DHCPREQUEST 3
#define NET_DHCP_MESSAGE_TYPE_DHCPDECLINE 4
#define NET_DHCP_MESSAGE_TYPE_DHCPACK 5
#define NET_DHCP_MESSAGE_TYPE_DHCPNAK 6
#define NET_DHCP_MESSAGE_TYPE_DHCPRELEASE 7
#define NET_DHCP_MESSAGE_TYPE_DHCPINFORM 8

#define NET_DHCP_COOKIE 0x63825363



#define NET_DHCP_PACKET_ITER_OPTIONS(dhcp_packet) for (u32 i=0;(dhcp_packet)->options[i]!=0xff;i+=((dhcp_packet)->options[i]?(dhcp_packet)->options[i+1]+2:1))



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



void net_dhcp_init(void);



void net_dhcp_negotiate_address(void);



#endif
