#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/network/layer1.h>
#include <kernel/network/layer2.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <net/arp.h>
#include <net/ip4.h>
#define KERNEL_LOG_NAME "net_ip4"



#define ETHER_TYPE 0x0800



static spinlock_t _net_ip4_protocol_lock;
static rb_tree_t _net_ip4_protocol_type_tree;
static KERNEL_INIT_WRITE omm_allocator_t* _net_ip4_protocol_allocator=NULL;



static void _rx_callback(const network_layer1_packet_t* packet){
	WARN("PACKET (IPv4)");
}



static network_layer2_protocol_descriptor_t _net_ip4_protocol_descriptor={
	"IPv4",
	ETHER_TYPE,
	_rx_callback
};



void net_ip4_init(void){
	LOG("Registering IPv4 protocol...");
	network_layer2_register_descriptor(&_net_ip4_protocol_descriptor);
	spinlock_init(&_net_ip4_protocol_lock);
	rb_tree_init(&_net_ip4_protocol_type_tree);
	_net_ip4_protocol_allocator=omm_init("net_ip4_protocol",sizeof(net_ip4_protocol_t),8,1,pmm_alloc_counter("omm_net_ip4_protocol"));
	/*****/mac_address_t tmp;net_arp_resolve_address(0x0a000203,&tmp);/*****/
}



KERNEL_PUBLIC void net_ip4_register_protocol_descriptor(const net_ip4_protocol_descriptor_t* descriptor){
	spinlock_acquire_exclusive(&_net_ip4_protocol_lock);
	LOG("Registering network IPv4 protocol '%s/%X%X'...",descriptor->name,descriptor->protocol_type>>8,descriptor->protocol_type);
	rb_tree_node_t* node=rb_tree_lookup_node(&_net_ip4_protocol_type_tree,descriptor->protocol_type);
	if (node){
		ERROR("IPv4 protocol %X%X is already allocated by '%s'",descriptor->protocol_type>>8,descriptor->protocol_type,((net_ip4_protocol_t*)node)->descriptor->name);
		spinlock_release_exclusive(&_net_ip4_protocol_lock);
		return;
	}
	net_ip4_protocol_t* protocol=omm_alloc(_net_ip4_protocol_allocator);
	protocol->rb_node.key=descriptor->protocol_type;
	protocol->descriptor=descriptor;
	rb_tree_insert_node(&_net_ip4_protocol_type_tree,&(protocol->rb_node));
	spinlock_release_exclusive(&_net_ip4_protocol_lock);
}



KERNEL_PUBLIC void net_ip4_unregister_protocol_descriptor(const net_ip4_protocol_descriptor_t* descriptor){
	spinlock_acquire_exclusive(&_net_ip4_protocol_lock);
	LOG("Unregistering network IPv4 protocol '%s/%X%X'...",descriptor->name,descriptor->protocol_type>>8,descriptor->protocol_type);
	rb_tree_node_t* node=rb_tree_lookup_node(&_net_ip4_protocol_type_tree,descriptor->protocol_type);
	if (node){
		rb_tree_remove_node(&_net_ip4_protocol_type_tree,node);
		omm_dealloc(_net_ip4_protocol_allocator,node);
	}
	spinlock_release_exclusive(&_net_ip4_protocol_lock);
}



KERNEL_PUBLIC net_ip4_packet_t* net_ip4_create_packet(u16 length);



KERNEL_PUBLIC void net_ip4_delete_packet(net_ip4_packet_t* packet);



KERNEL_PUBLIC void net_ip4_send_packet(net_ip4_packet_t* packet);
