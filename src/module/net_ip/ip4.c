#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/module/module.h>
#include <kernel/network/layer1.h>
#include <kernel/network/layer2.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <net/arp.h>
#include <net/checksum.h>
#include <net/ip4.h>
#define KERNEL_LOG_NAME "net_ip4"



#define ETHER_TYPE 0x0800



static rwlock_t _net_ip4_protocol_lock;
static rb_tree_t _net_ip4_protocol_type_tree;
static omm_allocator_t* KERNEL_INIT_WRITE _net_ip4_protocol_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _net_ip4_packet_allocator=NULL;



static void _rx_callback(network_layer1_packet_t* packet){
	if (packet->length<sizeof(net_ip4_packet_data_t)){
		return;
	}
	net_ip4_packet_data_t* header=(net_ip4_packet_data_t*)(packet->data);
	if (header->version_and_ihl!=0x45||net_checksum_verify_checksum(header,sizeof(net_ip4_packet_data_t),0)!=0xffff){
		ERROR("Wrong IPv4 version or checksum");
		return;
	}
	if (header->total_length!=__builtin_bswap16(packet->length)||header->fragment){
		ERROR("Fragmented IPv4 packed");
		return;
	}
	rwlock_acquire_read(&_net_ip4_protocol_lock);
	const net_ip4_protocol_t* protocol=(const net_ip4_protocol_t*)rb_tree_lookup_node(&_net_ip4_protocol_type_tree,header->protocol);
	if (protocol){
		net_ip4_packet_t ip4_packet={
			packet->length-sizeof(net_ip4_packet_data_t),
			packet,
			header
		};
		protocol->descriptor->rx_callback(&ip4_packet);
	}
	else{
		WARN("Unhandled IPv4 packet '%u'",header->protocol);
	}
	rwlock_release_read(&_net_ip4_protocol_lock);
}



static const network_layer2_protocol_descriptor_t _net_ip4_protocol_descriptor={
	"IPv4",
	ETHER_TYPE,
	_rx_callback
};



MODULE_INIT(){
	rwlock_init(&_net_ip4_protocol_lock);
	rb_tree_init(&_net_ip4_protocol_type_tree);
	_net_ip4_protocol_allocator=omm_init("net.ip4.protocol",sizeof(net_ip4_protocol_t),8,1);
	rwlock_init(&(_net_ip4_protocol_allocator->lock));
	_net_ip4_packet_allocator=omm_init("net.ip4.packet",sizeof(net_ip4_packet_t),8,4);
	rwlock_init(&(_net_ip4_packet_allocator->lock));
}



MODULE_POSTINIT(){
	LOG("Registering IPv4 protocol...");
	network_layer2_register_descriptor(&_net_ip4_protocol_descriptor);
}



KERNEL_PUBLIC void net_ip4_register_protocol_descriptor(const net_ip4_protocol_descriptor_t* descriptor){
	rwlock_acquire_write(&_net_ip4_protocol_lock);
	LOG("Registering network IPv4 protocol '%s/%u'...",descriptor->name,descriptor->protocol_type);
	rb_tree_node_t* node=rb_tree_lookup_node(&_net_ip4_protocol_type_tree,descriptor->protocol_type);
	if (node){
		ERROR("IPv4 protocol %u is already allocated by '%s'",descriptor->protocol_type,((net_ip4_protocol_t*)node)->descriptor->name);
		rwlock_release_write(&_net_ip4_protocol_lock);
		return;
	}
	net_ip4_protocol_t* protocol=omm_alloc(_net_ip4_protocol_allocator);
	protocol->rb_node.key=descriptor->protocol_type;
	protocol->descriptor=descriptor;
	rb_tree_insert_node(&_net_ip4_protocol_type_tree,&(protocol->rb_node));
	rwlock_release_write(&_net_ip4_protocol_lock);
}



KERNEL_PUBLIC void net_ip4_unregister_protocol_descriptor(const net_ip4_protocol_descriptor_t* descriptor){
	rwlock_acquire_write(&_net_ip4_protocol_lock);
	LOG("Unregistering network IPv4 protocol '%s/%u'...",descriptor->name,descriptor->protocol_type);
	rb_tree_node_t* node=rb_tree_lookup_node(&_net_ip4_protocol_type_tree,descriptor->protocol_type);
	if (node){
		rb_tree_remove_node(&_net_ip4_protocol_type_tree,node);
		omm_dealloc(_net_ip4_protocol_allocator,node);
	}
	rwlock_release_write(&_net_ip4_protocol_lock);
}



KERNEL_PUBLIC net_ip4_packet_t* net_ip4_create_packet(u16 length,net_ip4_address_t src_address,net_ip4_address_t dst_address,net_ip4_protocol_type_t protocol_type){
	mac_address_t dst_mac_address;
	if (!net_arp_resolve_address(dst_address,&dst_mac_address,0)){
		return NULL;
	}
	net_ip4_packet_t* out=omm_alloc(_net_ip4_packet_allocator);
	out->length=length;
	out->raw_packet=network_layer1_create_packet(length+sizeof(net_ip4_packet_data_t),&(network_layer1_device->mac_address),&dst_mac_address,ETHER_TYPE);
	out->packet=(net_ip4_packet_data_t*)(out->raw_packet->data);
	out->packet->version_and_ihl=0x45;
	out->packet->dscp_and_ecn=0x00;
	out->packet->total_length=__builtin_bswap16(length+sizeof(net_ip4_packet_data_t));
	out->packet->identification=0x0000;
	out->packet->fragment=0x0000;
	out->packet->ttl=0xff;
	out->packet->protocol=protocol_type;
	out->packet->src_address=__builtin_bswap32(src_address);
	out->packet->dst_address=__builtin_bswap32(dst_address);
	return out;
}



KERNEL_PUBLIC void net_ip4_delete_packet(net_ip4_packet_t* packet){
	network_layer1_delete_packet(packet->raw_packet);
	omm_dealloc(_net_ip4_packet_allocator,packet);
}



KERNEL_PUBLIC void net_ip4_send_packet(net_ip4_packet_t* packet){
	net_checksum_calculate_checksum(packet->packet,sizeof(net_ip4_packet_data_t),&(packet->packet->checksum));
	network_layer1_send_packet(packet->raw_packet);
	omm_dealloc(_net_ip4_packet_allocator,packet);
}
