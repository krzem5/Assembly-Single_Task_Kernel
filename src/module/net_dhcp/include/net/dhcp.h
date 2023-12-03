#ifndef _NET_DHCP_H_
#define _NET_DHCP_H_ 1
#include <kernel/types.h>



#define NET_DHCP_COOKIE 0x63825363



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



#endif
