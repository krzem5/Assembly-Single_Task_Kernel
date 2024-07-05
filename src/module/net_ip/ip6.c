#include <kernel/format/format.h>
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
	rwlock_acquire_write(&_net_ip6_protocol_lock);
	LOG("Registering network IPv4 protocol '%s/%u'...",descriptor->name,descriptor->protocol_type);
	rb_tree_node_t* node=rb_tree_lookup_node(&_net_ip6_protocol_type_tree,descriptor->protocol_type);
	if (node){
		ERROR("IPv6 protocol %u is already allocated by '%s'",descriptor->protocol_type,((net_ip6_protocol_t*)node)->descriptor->name);
		rwlock_release_write(&_net_ip6_protocol_lock);
		return;
	}
	net_ip6_protocol_t* protocol=omm_alloc(_net_ip6_protocol_allocator);
	protocol->rb_node.key=descriptor->protocol_type;
	protocol->descriptor=descriptor;
	rb_tree_insert_node(&_net_ip6_protocol_type_tree,&(protocol->rb_node));
	rwlock_release_write(&_net_ip6_protocol_lock);
}



KERNEL_PUBLIC void net_ip6_unregister_protocol_descriptor(const net_ip6_protocol_descriptor_t* descriptor){
	rwlock_acquire_write(&_net_ip6_protocol_lock);
	LOG("Unregistering network IPv6 protocol '%s/%u'...",descriptor->name,descriptor->protocol_type);
	rb_tree_node_t* node=rb_tree_lookup_node(&_net_ip6_protocol_type_tree,descriptor->protocol_type);
	if (node){
		rb_tree_remove_node(&_net_ip6_protocol_type_tree,node);
		omm_dealloc(_net_ip6_protocol_allocator,node);
	}
	rwlock_release_write(&_net_ip6_protocol_lock);
}



KERNEL_PUBLIC net_ip6_packet_t* net_ip6_create_packet(u16 length,const net_ip6_address_t* src_address,const net_ip6_address_t* dst_address,net_ip6_protocol_type_t protocol_type){
	// this requires NDP/ICMPv6
	panic("net_ip6_create_packet");
}



KERNEL_PUBLIC void net_ip6_delete_packet(net_ip6_packet_t* packet){
	network_layer1_delete_packet(packet->raw_packet);
	omm_dealloc(_net_ip6_packet_allocator,packet);
}



KERNEL_PUBLIC void net_ip6_send_packet(net_ip6_packet_t* packet){
	network_layer1_send_packet(packet->raw_packet);
	omm_dealloc(_net_ip6_packet_allocator,packet);
}



KERNEL_PUBLIC bool net_ip6_address_from_string(const char* str,net_ip6_address_t* out){
	for (u32 i=0;i<8;i++){
		out->data[i]=0;
	}
	if (!str[0]){
		return 0;
	}
	u32 group_index=0;
	u32 group_bits=0;
	u32 expansion_index=0xffffffff;
	for (;str[0];str++){
		if (str[0]>='0'&&str[0]<='9'){
			if (group_index>=8||group_bits>=16){
				return 0;
			}
			out->data[group_index]=(out->data[group_index]<<4)|(str[0]-'0');
			group_bits+=4;
			continue;
		}
		if (str[0]>='A'&&str[0]<='Z'){
			if (group_index>=8||group_bits>=16){
				return 0;
			}
			out->data[group_index]=(out->data[group_index]<<4)|(str[0]-'A'+10);
			group_bits+=4;
			continue;
		}
		if (str[0]>='a'&&str[0]<='z'){
			if (group_index>=8||group_bits>=16){
				return 0;
			}
			out->data[group_index]=(out->data[group_index]<<4)|(str[0]-'a'+10);
			group_bits+=4;
			continue;
		}
		if (str[0]!=':'){
			return 0;
		}
		if (group_bits){
			group_index++;
			group_bits=0;
		}
		if (str[1]!=':'){
			continue;
		}
		str++;
		if (str[1]==':'||expansion_index!=0xffffffff){
			return 0;
		}
		expansion_index=group_index;
	}
	if (group_bits){
		group_index++;
	}
	if (group_index>8){
		return 0;
	}
	if (expansion_index==0xffffffff){
		if (group_index!=8){
			return 0;
		}
		return 1;
	}
	u32 expansion_length=8-group_index;
	if (expansion_length<2){
		return 0;
	}
	for (u32 i=group_index;i>expansion_index;){
		i--;
		out->data[i-group_index+8]=out->data[i];
	}
	for (u32 i=0;i<expansion_length;i++){
		out->data[i+expansion_index]=0;
	}
	return 1;
}



KERNEL_PUBLIC bool net_ip6_address_from_string_format(const char* template,net_ip6_address_t* out,...){
	char buffer[41];
	__builtin_va_list va;
	__builtin_va_start(va,out);
	bool is_too_long=(format_string_va(buffer,sizeof(buffer),template,&va)>=40);
	__builtin_va_end(va);
	if (is_too_long){
		return 0;
	}
	return net_ip6_address_from_string(buffer,out);
}
