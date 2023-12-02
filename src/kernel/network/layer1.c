#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/network/layer1.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "network_layer1"



// layer1: driver I/O
// layer2: ARP (0x0806), IPv4 (0x0800), IPv6 (0x86dd)
// layer3: TCP, UDP
// layer4: DHCP, DNS, HTTP, ...



static omm_allocator_t* _network_layer1_device_allocator=NULL;

KERNEL_PUBLIC handle_type_t network_layer1_device_handle_type=0;
KERNEL_PUBLIC network_layer1_device_t* network_layer1_device=NULL;



void KERNEL_EARLY_EXEC network_layer1_init(void){
	LOG("Initializing network layer1...");
	_network_layer1_device_allocator=omm_init("network_layer1_device",sizeof(network_layer1_device_t),8,1,pmm_alloc_counter("omm_network_layer1_device"));
	network_layer1_device_handle_type=handle_alloc("network_layer1_device",NULL);
}



KERNEL_PUBLIC void network_layer1_create_device(const network_layer1_device_descriptor_t* descriptor,const mac_address_t* mac_address,void* extra_data){
	LOG("Creating network layer1 device '%s/%X:%X:%X:%X:%X:%X'...",descriptor->name,(*mac_address)[0],(*mac_address)[1],(*mac_address)[2],(*mac_address)[3],(*mac_address)[4],(*mac_address)[5]);
	network_layer1_device_t* out=omm_alloc(_network_layer1_device_allocator);
	handle_new(out,network_layer1_device_handle_type,&(out->handle));
	memcpy(out->mac_address,*mac_address,sizeof(mac_address_t));
	out->extra_data=extra_data;
	handle_finish_setup(&(out->handle));
	if (!network_layer1_device){
		network_layer1_device=out;
	}
}



KERNEL_PUBLIC network_layer1_packet_t* network_layer1_create_packet(u16 size,const mac_address_t* dst_mac_address,const mac_address_t* src_mac_address,ether_type_t ether_type){
	network_layer1_packet_t* out=(void*)(smm_alloc(NULL,sizeof(network_layer1_packet_t)+size)->data);
	out->length=size;
	if (dst_mac_address){
		memcpy(out->dst_mac,*dst_mac_address,sizeof(mac_address_t));
	}
	if (src_mac_address){
		memcpy(out->src_mac,*src_mac_address,sizeof(mac_address_t));
	}
	out->ether_type=ether_type;
	return out;
}



KERNEL_PUBLIC void network_layer1_delete_packet(network_layer1_packet_t* packet){
	smm_dealloc((void*)(((u64)packet)-__builtin_offsetof(string_t,data)));
}



KERNEL_PUBLIC void network_layer1_send_packet(network_layer1_packet_t* packet){
	if (network_layer1_device){
		network_layer1_device->descriptor->tx(network_layer1_device->extra_data,packet);
	}
	network_layer1_delete_packet(packet);
}
