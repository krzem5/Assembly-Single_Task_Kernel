#include <kernel/lock/spinlock.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <net/info.h>
#include <net/ip4.h>



static omm_allocator_t* KERNEL_INIT_WRITE _net_info_address_list_entry_allocator=NULL;
static net_ip4_address_t _net_info_address=0;
static net_ip4_address_t _net_info_subnet_mask=0;
static net_info_address_list_t _net_info_dns_address_list;
static net_info_address_list_t _net_info_router_address_list;



static void _init_address_list(net_info_address_list_t* out){
	spinlock_init(&(out->lock));
	out->head=NULL;
	out->tail=NULL;
}



static void _clear_address_list(net_info_address_list_t* address_list){
	spinlock_acquire_exclusive(&(address_list->lock));
	for (net_info_address_list_entry_t* entry=address_list->head;entry;){
		net_info_address_list_entry_t* next_entry=entry->next;
		omm_dealloc(_net_info_address_list_entry_allocator,entry);
		entry=next_entry;
	}
	address_list->head=NULL;
	address_list->tail=NULL;
	spinlock_release_exclusive(&(address_list->lock));
}



static void _extend_address_list(net_info_address_list_t* address_list,net_ip4_address_t address){
	spinlock_acquire_exclusive(&(address_list->lock));
	net_info_address_list_entry_t* entry=omm_alloc(_net_info_address_list_entry_allocator);
	entry->next=NULL;
	entry->address=address;
	if (address_list->tail){
		address_list->tail->next=entry;
	}
	else{
		address_list->head=entry;
	}
	address_list->tail=entry;
	spinlock_release_exclusive(&(address_list->lock));
}



MODULE_INIT(){
	_net_info_address_list_entry_allocator=omm_init("net_info_address_list_entry",sizeof(net_info_address_list_entry_t),8,1);
	spinlock_init(&(_net_info_address_list_entry_allocator->lock));
	_init_address_list(&_net_info_dns_address_list);
	_init_address_list(&_net_info_router_address_list);
}



MODULE_POSTINIT(){
	net_info_reset();
}



KERNEL_PUBLIC void net_info_reset(void){
	_net_info_address=0;
	_net_info_subnet_mask=0;
	_clear_address_list(&_net_info_dns_address_list);
	_clear_address_list(&_net_info_router_address_list);
}



KERNEL_PUBLIC void net_info_set_address(net_ip4_address_t address){
	_net_info_address=address;
}



KERNEL_PUBLIC void net_info_set_subnet_mask(net_ip4_address_t subnet_mask){
	_net_info_subnet_mask=subnet_mask;
}



KERNEL_PUBLIC void net_info_add_dns(net_ip4_address_t dns){
	_extend_address_list(&_net_info_dns_address_list,dns);
}



KERNEL_PUBLIC void net_info_add_router(net_ip4_address_t router){
	_extend_address_list(&_net_info_router_address_list,router);
}



KERNEL_PUBLIC net_ip4_address_t net_info_get_address(void){
	return _net_info_address;
}



KERNEL_PUBLIC net_ip4_address_t net_info_get_subnet_mask(void){
	return _net_info_subnet_mask;
}



KERNEL_PUBLIC const net_info_address_list_entry_t* net_info_get_dns_entries(void){
	return _net_info_dns_address_list.head;
}



KERNEL_PUBLIC const net_info_address_list_entry_t* net_info_get_router_entries(void){
	return _net_info_router_address_list.tail;
}
