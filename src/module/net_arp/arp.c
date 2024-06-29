#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/module/module.h>
#include <kernel/mp/event.h>
#include <kernel/network/layer1.h>
#include <kernel/network/layer2.h>
#include <kernel/timer/timer.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <net/arp.h>
#include <net/info.h>
#include <net/ip4.h>
#define KERNEL_LOG_NAME "net_arp"



#define ETHER_TYPE 0x0806

#define ARP_TIMEOUT_NS 1000000000



static omm_allocator_t* KERNEL_INIT_WRITE _net_arp_cache_entry_allocator=NULL;
static event_t* KERNEL_INIT_WRITE _net_arp_cache_resolution_event=NULL;
static rwlock_t _net_arp_cache_lock;
static rb_tree_t _net_arp_cache_address_tree;



static void _rx_callback(network_layer1_packet_t* packet){
	if (packet->length<sizeof(net_arp_packet_t)){
		return;
	}
	net_arp_packet_t* arp_packet=(net_arp_packet_t*)(packet->data);
	if (arp_packet->tpa!=__builtin_bswap32(net_info_get_address())){
		return;
	}
	if (arp_packet->oper==__builtin_bswap16(NET_ARP_OPER_REQUEST)){
		INFO("Replying to ARP probe...");
		network_layer1_packet_t* packet=network_layer1_create_packet(sizeof(net_arp_packet_t),&(network_layer1_device->mac_address),(const mac_address_t*)(&(arp_packet->sha)),ETHER_TYPE);
		net_arp_packet_t* reply_arp_packet=(net_arp_packet_t*)(packet->data);
		reply_arp_packet->htype=__builtin_bswap16(NET_ARP_HTYPE_ETHERNET);
		reply_arp_packet->ptype=__builtin_bswap16(NET_ARP_PTYPE_IPV4);
		reply_arp_packet->hlen=sizeof(mac_address_t);
		reply_arp_packet->plen=sizeof(net_ip4_address_t);
		reply_arp_packet->oper=__builtin_bswap16(NET_ARP_OPER_REPLY);
		mem_copy(reply_arp_packet->sha,network_layer1_device->mac_address,sizeof(mac_address_t));
		reply_arp_packet->spa=arp_packet->tpa;
		mem_copy(reply_arp_packet->tha,arp_packet->sha,sizeof(mac_address_t));
		reply_arp_packet->tpa=arp_packet->spa;
		network_layer1_send_packet(packet);
		return;
	}
	if (arp_packet->oper!=__builtin_bswap16(NET_ARP_OPER_REPLY)){
		return;
	}
	rwlock_acquire_write(&_net_arp_cache_lock);
	net_arp_cache_entry_t* cache_entry=(net_arp_cache_entry_t*)rb_tree_lookup_node(&_net_arp_cache_address_tree,__builtin_bswap32(arp_packet->spa));
	if (cache_entry){
		INFO("APR response: %I -> %M",__builtin_bswap32(arp_packet->spa),arp_packet->sha);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wstringop-overflow"
		mem_copy(&(cache_entry->address),arp_packet->sha,sizeof(mac_address_t));
#pragma GCC diagnostic pop
		cache_entry->resolved=1;
		event_dispatch(_net_arp_cache_resolution_event,EVENT_DISPATCH_FLAG_DISPATCH_ALL);
	}
	rwlock_release_write(&_net_arp_cache_lock);
}



static const network_layer2_protocol_descriptor_t _net_arp_protocol_descriptor={
	"ARP",
	ETHER_TYPE,
	_rx_callback
};



MODULE_INIT(){
	LOG("Initializing ARP resolver...");
	_net_arp_cache_entry_allocator=omm_init("net.arp.cache_entry",sizeof(net_arp_cache_entry_t),8,4);
	rwlock_init(&(_net_arp_cache_entry_allocator->lock));
	_net_arp_cache_resolution_event=event_create("net.arp.resolution");
	rwlock_init(&_net_arp_cache_lock);
	rb_tree_init(&_net_arp_cache_address_tree);
}



MODULE_POSTINIT(){
	network_layer2_register_descriptor(&_net_arp_protocol_descriptor);
}



KERNEL_PUBLIC bool net_arp_resolve_address(net_ip4_address_t address,mac_address_t* out,bool nonblocking){
	if (!address){
		mem_fill(out,sizeof(mac_address_t),0);
		return 1;
	}
	if (address==0xffffffff){
		mem_fill(out,sizeof(mac_address_t),0xff);
		return 1;
	}
	rwlock_acquire_write(&_net_arp_cache_lock);
	net_arp_cache_entry_t* cache_entry=(net_arp_cache_entry_t*)rb_tree_lookup_node(&_net_arp_cache_address_tree,address);
	if (cache_entry&&cache_entry->resolved){
		mem_copy(out,cache_entry->address,sizeof(mac_address_t));
		rwlock_release_write(&_net_arp_cache_lock);
		return 1;
	}
	if (!network_layer1_device||nonblocking){
		rwlock_release_write(&_net_arp_cache_lock);
		return 0;
	}
	bool send_request_packed=0;
	if (!cache_entry){
		send_request_packed=1;
		cache_entry=omm_alloc(_net_arp_cache_entry_allocator);
		cache_entry->rb_node.key=address;
		cache_entry->resolved=0;
		rb_tree_insert_node(&_net_arp_cache_address_tree,&(cache_entry->rb_node));
	}
	rwlock_release_write(&_net_arp_cache_lock);
	if (send_request_packed){
		INFO("Sending ARP request for address %I...",address);
		mac_address_t dst_mac_address={0xff,0xff,0xff,0xff,0xff,0xff};
		network_layer1_packet_t* packet=network_layer1_create_packet(sizeof(net_arp_packet_t),&(network_layer1_device->mac_address),&dst_mac_address,ETHER_TYPE);
		net_arp_packet_t* arp_packet=(net_arp_packet_t*)(packet->data);
		arp_packet->htype=__builtin_bswap16(NET_ARP_HTYPE_ETHERNET);
		arp_packet->ptype=__builtin_bswap16(NET_ARP_PTYPE_IPV4);
		arp_packet->hlen=sizeof(mac_address_t);
		arp_packet->plen=sizeof(net_ip4_address_t);
		arp_packet->oper=__builtin_bswap16(NET_ARP_OPER_REQUEST);
		mem_copy(arp_packet->sha,network_layer1_device->mac_address,sizeof(mac_address_t));
		arp_packet->spa=__builtin_bswap32(net_info_get_address());
		mem_fill(arp_packet->tha,sizeof(mac_address_t),0);
		arp_packet->tpa=__builtin_bswap32(address);
		network_layer1_send_packet(packet);
	}
	timer_t* timer=timer_create("net.arp.query.timeout",ARP_TIMEOUT_NS,1);
	event_t* events[2]={
		timer->event,
		_net_arp_cache_resolution_event
	};
	while (event_await_multiple(events,2)&&!cache_entry->resolved);
	timer_delete(timer);
	if (!cache_entry->resolved){
		return 0;
	}
	mem_copy(out,cache_entry->address,sizeof(mac_address_t));
	return 1;
}
