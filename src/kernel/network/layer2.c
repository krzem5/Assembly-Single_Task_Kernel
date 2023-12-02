#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/network/layer2.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "network_layer2"



static spinlock_t _network_layer2_lock;
static rb_tree_t _network_layer2_ether_type_tree;
static KERNEL_INIT_WRITE omm_allocator_t* _network_layer2_protocol_allocator=NULL;



void KERNEL_EARLY_EXEC network_layer2_init(void){
	LOG("Initializing network layer2...");
	spinlock_init(&_network_layer2_lock);
	rb_tree_init(&_network_layer2_ether_type_tree);
	_network_layer2_protocol_allocator=omm_init("network_layer2_protocol",sizeof(network_layer2_protocol_t),8,1,pmm_alloc_counter("omm_network_layer2_protocol"));
}



KERNEL_PUBLIC void network_layer2_register_descriptor(const network_layer2_protocol_descriptor_t* descriptor){
	spinlock_acquire_exclusive(&_network_layer2_lock);
	LOG("Registering network layer2 protocol '%s/%X%X'...",descriptor->name,descriptor->ether_type>>8,descriptor->ether_type);
	rb_tree_node_t* node=rb_tree_lookup_node(&_network_layer2_ether_type_tree,descriptor->ether_type);
	if (node){
		ERROR("EtherType %X%X is already allocated by '%s'",descriptor->ether_type>>8,descriptor->ether_type,((network_layer2_protocol_t*)node)->descriptor->name);
		spinlock_release_exclusive(&_network_layer2_lock);
		return;
	}
	network_layer2_protocol_t* protocol=omm_alloc(_network_layer2_protocol_allocator);
	protocol->rb_node.key=descriptor->ether_type;
	protocol->descriptor=descriptor;
	rb_tree_insert_node(&_network_layer2_ether_type_tree,&(protocol->rb_node));
	spinlock_release_exclusive(&_network_layer2_lock);
}



KERNEL_PUBLIC void network_layer2_unregister_descriptor(const network_layer2_protocol_descriptor_t* descriptor){
	spinlock_acquire_exclusive(&_network_layer2_lock);
	LOG("Unregistering network layer2 protocol '%s/%X%X'...",descriptor->name,descriptor->ether_type>>8,descriptor->ether_type);
	rb_tree_node_t* node=rb_tree_lookup_node(&_network_layer2_ether_type_tree,descriptor->ether_type);
	if (node){
		rb_tree_remove_node(&_network_layer2_ether_type_tree,node);
		omm_dealloc(_network_layer2_protocol_allocator,node);
	}
	spinlock_release_exclusive(&_network_layer2_lock);
}
