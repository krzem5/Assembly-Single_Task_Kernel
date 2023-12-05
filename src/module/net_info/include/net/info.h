#ifndef _NET_INFO_H_
#define _NET_INFO_H_ 1
#include <kernel/lock/spinlock.h>
#include <kernel/types.h>
#include <net/ip4.h>



typedef struct _NET_INFO_ADDRESS_LIST_ENTRY{
	struct _NET_INFO_ADDRESS_LIST_ENTRY* next;
	net_ip4_address_t address;
} net_info_address_list_entry_t;



typedef struct _NET_INFO_ADDRESS_LIST{
	spinlock_t lock;
	net_info_address_list_entry_t* head;
	net_info_address_list_entry_t* tail;
} net_info_address_list_t;



void net_info_reset(void);



void net_info_set_address(net_ip4_address_t address);



void net_info_set_subnet_mask(net_ip4_address_t subnet_mask);



void net_info_add_dns(net_ip4_address_t dns);



void net_info_add_router(net_ip4_address_t router);



net_ip4_address_t net_info_get_address(void);



net_ip4_address_t net_info_get_subnet_mask(void);



const net_info_address_list_entry_t* net_info_get_dns_entries(void);



const net_info_address_list_entry_t* net_info_get_router_entries(void);



#endif
