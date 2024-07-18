#ifndef _NET_INFO_H_
#define _NET_INFO_H_ 1
#include <kernel/lock/rwlock.h>
#include <kernel/types.h>



typedef struct _NET_INFO_ADDRESS_LIST_ENTRY{
	struct _NET_INFO_ADDRESS_LIST_ENTRY* next;
	u32 address;
} net_info_address_list_entry_t;



typedef struct _NET_INFO_ADDRESS_LIST{
	rwlock_t lock;
	net_info_address_list_entry_t* head;
	net_info_address_list_entry_t* tail;
} net_info_address_list_t;



void net_info_reset(void);



void net_info_set_address(u32 address);



void net_info_set_subnet_mask(u32 subnet_mask);



void net_info_add_dns(u32 dns);



void net_info_add_router(u32 router);



u32 net_info_get_address(void);



u32 net_info_get_subnet_mask(void);



const net_info_address_list_entry_t* net_info_get_dns_entries(void);



const net_info_address_list_entry_t* net_info_get_router_entries(void);



#endif
