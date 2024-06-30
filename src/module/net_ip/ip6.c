#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/module/module.h>
#include <kernel/network/layer1.h>
#include <kernel/network/layer2.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <net/ip6.h>
#define KERNEL_LOG_NAME "net_ip6"



#define ETHER_TYPE 0x86dd



static rwlock_t _net_ip6_protocol_lock;
static rb_tree_t _net_ip6_protocol_type_tree;
static omm_allocator_t* KERNEL_INIT_WRITE _net_ip6_protocol_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _net_ip6_packet_allocator=NULL;



static void _rx_callback(network_layer1_packet_t* packet){
	WARN("PACKET (IPv6)");
}



static const network_layer2_protocol_descriptor_t _net_ip6_protocol_descriptor={
	"IPv6",
	ETHER_TYPE,
	_rx_callback
};



MODULE_INIT(){
	rwlock_init(&_net_ip6_protocol_lock);
	rb_tree_init(&_net_ip6_protocol_type_tree);
	_net_ip6_protocol_allocator=omm_init("net.ip6.protocol",sizeof(net_ip6_protocol_t),8,1);
	rwlock_init(&(_net_ip6_protocol_allocator->lock));
	_net_ip6_packet_allocator=omm_init("net.ip6.packet",sizeof(net_ip6_packet_t),8,4);
	rwlock_init(&(_net_ip6_packet_allocator->lock));
}



MODULE_POSTINIT(){
	LOG("Registering IPv6 protocol...");
	network_layer2_register_descriptor(&_net_ip6_protocol_descriptor);
}



KERNEL_PUBLIC void net_ip6_register_protocol_descriptor(const net_ip6_protocol_descriptor_t* descriptor){
	panic("net_ip6_register_protocol_descriptor");
}



KERNEL_PUBLIC void net_ip6_unregister_protocol_descriptor(const net_ip6_protocol_descriptor_t* descriptor){
	panic("net_ip6_unregister_protocol_descriptor");
}



KERNEL_PUBLIC net_ip6_packet_t* net_ip6_create_packet(u16 length,const net_ip6_address_t* src_address,const net_ip6_address_t* dst_address,net_ip6_protocol_type_t protocol_type){
	panic("net_ip6_create_packet");
}



KERNEL_PUBLIC void net_ip6_delete_packet(net_ip6_packet_t* packet){
	panic("net_ip6_delete_packet");
}



KERNEL_PUBLIC void net_ip6_send_packet(net_ip6_packet_t* packet){
	panic("net_ip6_send_packet");
}
