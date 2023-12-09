#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/network/layer1.h>
#include <kernel/network/layer2.h>
#include <kernel/ring/ring.h>
#include <kernel/scheduler/load_balancer.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "network_layer1"



static omm_allocator_t* _network_layer1_device_allocator=NULL;
static ring_t* _network_layer1_packet_rx_ring=NULL;
static ring_t* _network_layer1_packet_tx_ring=NULL;

KERNEL_PUBLIC handle_type_t network_layer1_device_handle_type=0;
KERNEL_PUBLIC network_layer1_device_t* network_layer1_device=NULL;



static void _packet_rx_thread(void){
	while (1){
		network_layer1_packet_t* packet=ring_pop(_network_layer1_packet_rx_ring,1);
		network_layer2_process_packet(packet);
		network_layer1_delete_packet(packet);
	}
}



void KERNEL_EARLY_EXEC network_layer1_init(void){
	LOG("Initializing network layer1...");
	_network_layer1_device_allocator=omm_init("network_layer1_device",sizeof(network_layer1_device_t),8,1,pmm_alloc_counter("omm_network_layer1_device"));
	_network_layer1_packet_rx_ring=ring_init(16384);
	_network_layer1_packet_tx_ring=ring_init(16384);
	network_layer1_device_handle_type=handle_alloc("network_layer1_device",NULL);
	thread_new_kernel_thread(NULL,"network-layer1-packet-rx-thread",_packet_rx_thread,0x200000,0)->priority=SCHEDULER_PRIORITY_HIGH;
}



KERNEL_PUBLIC void network_layer1_create_device(const network_layer1_device_descriptor_t* descriptor,const mac_address_t* mac_address,void* extra_data){
	LOG("Creating network layer1 device '%s/%M'...",descriptor->name,*mac_address);
	network_layer1_device_t* out=omm_alloc(_network_layer1_device_allocator);
	handle_new(out,network_layer1_device_handle_type,&(out->handle));
	out->descriptor=descriptor;
	memcpy(out->mac_address,*mac_address,sizeof(mac_address_t));
	out->extra_data=extra_data;
	handle_finish_setup(&(out->handle));
	if (!network_layer1_device){
		network_layer1_device=out;
	}
}



KERNEL_PUBLIC network_layer1_packet_t* network_layer1_create_packet(u16 length,const mac_address_t* src_mac_address,const mac_address_t* dst_mac_address,ether_type_t ether_type){
	network_layer1_packet_t* out=amm_alloc(length+sizeof(network_layer1_packet_t));
	out->length=length;
	if (dst_mac_address){
		memcpy(out->dst_mac,*dst_mac_address,sizeof(mac_address_t));
	}
	if (src_mac_address){
		memcpy(out->src_mac,*src_mac_address,sizeof(mac_address_t));
	}
	out->ether_type=__builtin_bswap16(ether_type);
	return out;
}



KERNEL_PUBLIC void network_layer1_delete_packet(network_layer1_packet_t* packet){
	amm_dealloc(packet);
}



KERNEL_PUBLIC void network_layer1_send_packet(network_layer1_packet_t* packet){
	if (network_layer1_device){
		ring_push(_network_layer1_packet_tx_ring,packet,1);
	}
	else{
		network_layer1_delete_packet(packet);
	}
}



KERNEL_PUBLIC void network_layer1_push_packet(network_layer1_packet_t* packet){
	ring_push(_network_layer1_packet_rx_ring,packet,1);
}



KERNEL_PUBLIC network_layer1_packet_t* network_layer1_pop_packet(void){
	return ring_pop(_network_layer1_packet_tx_ring,1);
}
