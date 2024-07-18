#ifndef _NET_ARP_H_
#define _NET_ARP_H_ 1
#include <kernel/network/layer1.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>



// Hardware type
#define NET_ARP_HTYPE_ETHERNET 0x0001

// Protocol type
#define NET_ARP_PTYPE_IPV4 0x0800

// Operation
#define NET_ARP_OPER_REQUEST 0x0001
#define NET_ARP_OPER_REPLY 0x0002



typedef struct KERNEL_PACKED _NET_ARP_PACKET{
	u16 htype;
	u16 ptype;
	u8 hlen;
	u8 plen;
	u16 oper;
	mac_address_t sha;
	u32 spa;
	mac_address_t tha;
	u32 tpa;
} net_arp_packet_t;



typedef struct _NET_ARP_CACHE_ENTRY{
	rb_tree_node_t rb_node;
	mac_address_t address;
	bool resolved;
} net_arp_cache_entry_t;



bool net_arp_resolve_address(u32 address,mac_address_t* out,bool nonblocking);



#endif
