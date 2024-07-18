#include <kernel/lock/rwlock.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <net/info.h>



static omm_allocator_t* KERNEL_INIT_WRITE _net_info_address_list_entry_allocator=NULL;
static u32 _net_info_address=0;
static u32 _net_info_subnet_mask=0;
static net_info_address_list_t _net_info_dns_address_list;
static net_info_address_list_t _net_info_router_address_list;



static void _init_address_list(net_info_address_list_t* out){
	rwlock_init(&(out->lock));
	out->head=NULL;
	out->tail=NULL;
}



static void _clear_address_list(net_info_address_list_t* address_list){
	rwlock_acquire_write(&(address_list->lock));
	for (net_info_address_list_entry_t* entry=address_list->head;entry;){
		net_info_address_list_entry_t* next_entry=entry->next;
		omm_dealloc(_net_info_address_list_entry_allocator,entry);
		entry=next_entry;
	}
	address_list->head=NULL;
	address_list->tail=NULL;
	rwlock_release_write(&(address_list->lock));
}



static void _extend_address_list(net_info_address_list_t* address_list,u32 address){
	rwlock_acquire_write(&(address_list->lock));
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
	rwlock_release_write(&(address_list->lock));
}



MODULE_INIT(){
	_net_info_address_list_entry_allocator=omm_init("net.info.address_list_entry",sizeof(net_info_address_list_entry_t),8,1);
	rwlock_init(&(_net_info_address_list_entry_allocator->lock));
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



KERNEL_PUBLIC void net_info_set_address(u32 address){
	_net_info_address=address;
}



KERNEL_PUBLIC void net_info_set_subnet_mask(u32 subnet_mask){
	_net_info_subnet_mask=subnet_mask;
}



KERNEL_PUBLIC void net_info_add_dns(u32 dns){
	_extend_address_list(&_net_info_dns_address_list,dns);
}



KERNEL_PUBLIC void net_info_add_router(u32 router){
	_extend_address_list(&_net_info_router_address_list,router);
}



KERNEL_PUBLIC u32 net_info_get_address(void){
	return _net_info_address;
}



KERNEL_PUBLIC u32 net_info_get_subnet_mask(void){
	return _net_info_subnet_mask;
}



KERNEL_PUBLIC const net_info_address_list_entry_t* net_info_get_dns_entries(void){
	return _net_info_dns_address_list.head;
}



KERNEL_PUBLIC const net_info_address_list_entry_t* net_info_get_router_entries(void){
	return _net_info_router_address_list.tail;
}
