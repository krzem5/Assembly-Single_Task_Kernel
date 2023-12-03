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
#include <net/common.h>
#include <net/ip4.h>
#define KERNEL_LOG_NAME "net_ip4"



#define ETHER_TYPE 0x0800



static spinlock_t _net_ip4_protocol_lock;
static rb_tree_t _net_ip4_protocol_type_tree;
static omm_allocator_t* _net_ip4_protocol_allocator=NULL;
static omm_allocator_t* _net_ip4_packet_allocator=NULL;



static void _rx_callback(const network_layer1_packet_t* packet){
	if (packet->length<sizeof(net_ip4_packet_data_t)){
		return;
	}
	const net_ip4_packet_data_t* header=(const net_ip4_packet_data_t*)(packet->data);
	if (header->version_and_ihl!=0x45||!net_common_verify_checksum(header,sizeof(net_ip4_packet_data_t))){
		ERROR("Wrong version or checksum");
		return;
	}
	if (header->total_length!=__builtin_bswap16(packet->length)||header->fragment){
		ERROR("Fragmented IPv4 packed");
		return;
	}
	WARN("PACKET (IPv4) [%u]",header->protocol);
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
	_net_ip4_packet_allocator=omm_init("net_ip4_packet",sizeof(net_ip4_packet_t),8,4,pmm_alloc_counter("omm_net_ip4_packet"));
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



KERNEL_PUBLIC net_ip4_packet_t* net_ip4_create_packet(u16 length,net_ip4_address_t src_address,net_ip4_address_t dst_address,net_ip4_protocol_type_t protocol_type){
	mac_address_t dst_mac_address;
	if (!net_arp_resolve_address(dst_address,&dst_mac_address)){
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
	out->packet->src_address=src_address;
	out->packet->dst_address=dst_address;
	return out;
}



KERNEL_PUBLIC void net_ip4_delete_packet(net_ip4_packet_t* packet){
	network_layer1_delete_packet(packet->raw_packet);
	omm_dealloc(_net_ip4_packet_allocator,packet);
}



KERNEL_PUBLIC void net_ip4_send_packet(net_ip4_packet_t* packet){
	net_common_calculate_checksum(packet->packet,sizeof(net_ip4_packet_data_t),&(packet->packet->checksum));
	network_layer1_send_packet(packet->raw_packet);
	omm_dealloc(_net_ip4_packet_allocator,packet);
}
