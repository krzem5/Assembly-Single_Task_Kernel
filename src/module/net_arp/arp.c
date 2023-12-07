#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/event.h>
#include <kernel/network/layer1.h>
#include <kernel/network/layer2.h>
#include <kernel/timer/timer.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <net/arp.h>
#include <net/info.h>
#include <net/ip4.h>
#define KERNEL_LOG_NAME "net_arp"



#define ETHER_TYPE 0x0806



static omm_allocator_t* _net_arp_cache_entry_allocator=NULL;
static event_t* _net_arp_event=NULL;
static spinlock_t _net_arp_cache_lock;
static rb_tree_t _net_arp_cache_address_tree;



static void _rx_callback(network_layer1_packet_t* packet){
	if (packet->length<sizeof(net_arp_packet_t)){
		return;
	}
	net_arp_packet_t* arp_packet=(net_arp_packet_t*)(packet->data);
	if (arp_packet->oper!=__builtin_bswap16(NET_ARP_OPER_REPLY)||arp_packet->tpa!=__builtin_bswap32(net_info_get_address())){
		return;
	}
	spinlock_acquire_exclusive(&_net_arp_cache_lock);
	net_arp_cache_entry_t* cache_entry=(net_arp_cache_entry_t*)rb_tree_lookup_node(&_net_arp_cache_address_tree,__builtin_bswap32(arp_packet->spa));
	if (cache_entry){
		INFO("APR resolution: %I -> %M",__builtin_bswap32(arp_packet->spa),arp_packet->sha);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wstringop-overflow"
		memcpy(&(cache_entry->address),arp_packet->sha,sizeof(mac_address_t));
#pragma GCC diagnostic pop
		cache_entry->resolved=1;
		event_dispatch(_net_arp_event,EVENT_DISPATCH_FLAG_DISPATCH_ALL);
	}
	spinlock_release_exclusive(&_net_arp_cache_lock);
}



static const network_layer2_protocol_descriptor_t _net_arp_protocol_descriptor={
	"ARP",
	ETHER_TYPE,
	_rx_callback
};



void net_arp_init(void){
	LOG("Initializing ARP resolver...");
	_net_arp_cache_entry_allocator=omm_init("net_arp_cache_entry",sizeof(net_arp_cache_entry_t),8,4,pmm_alloc_counter("omm_net_arp_cache_entry"));
	_net_arp_event=event_new();
	spinlock_init(&_net_arp_cache_lock);
	rb_tree_init(&_net_arp_cache_address_tree);
	network_layer2_register_descriptor(&_net_arp_protocol_descriptor);
}



KERNEL_PUBLIC _Bool net_arp_resolve_address(net_ip4_address_t address,mac_address_t* out,_Bool nonblocking){
	if (!address){
		memset(out,0,sizeof(mac_address_t));
		return 1;
	}
	if (address==0xffffffff){
		memset(out,0xff,sizeof(mac_address_t));
		return 1;
	}
	spinlock_acquire_exclusive(&_net_arp_cache_lock);
	net_arp_cache_entry_t* cache_entry=(net_arp_cache_entry_t*)rb_tree_lookup_node(&_net_arp_cache_address_tree,address);
	if (cache_entry&&cache_entry->resolved){
		memcpy(out,cache_entry->address,sizeof(mac_address_t));
		spinlock_release_exclusive(&_net_arp_cache_lock);
		return 1;
	}
	if (!network_layer1_device||nonblocking){
		spinlock_release_exclusive(&_net_arp_cache_lock);
		return 0;
	}
	_Bool send_request_packed=0;
	if (!cache_entry){
		send_request_packed=1;
		cache_entry=omm_alloc(_net_arp_cache_entry_allocator);
		cache_entry->rb_node.key=address;
		cache_entry->resolved=0;
		rb_tree_insert_node(&_net_arp_cache_address_tree,&(cache_entry->rb_node));
	}
	spinlock_release_exclusive(&_net_arp_cache_lock);
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
		memcpy(arp_packet->sha,network_layer1_device->mac_address,sizeof(mac_address_t));
		arp_packet->spa=__builtin_bswap32(net_info_get_address());
		memset(arp_packet->tha,0,sizeof(mac_address_t));
		arp_packet->tpa=__builtin_bswap32(address);
		network_layer1_send_packet(packet);
	}
	timer_t* timer=timer_create(1000000000,1);
	event_t* events[2]={
		timer->event,
		_net_arp_event
	};
	while (event_await_multiple(events,2)&&!cache_entry->resolved);
	timer_delete(timer);
	if (!cache_entry->resolved){
		return 0;
	}
	memcpy(out,cache_entry->address,sizeof(mac_address_t));
	return 1;
}
