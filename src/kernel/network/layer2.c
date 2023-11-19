#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/network/layer1.h>
#include <kernel/network/layer2.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "network_layer2"



static pmm_counter_descriptor_t _network_pmm_counter=PMM_COUNTER_INIT_STRUCT("network");



static spinlock_t _layer2_lock;
static u64 _layer2_physical_send_buffer_address;
static u8* _layer2_physical_send_buffer;



void network_layer2_init(void){
	LOG("Initializing layer2 network...");
	spinlock_init(&_layer2_lock);
	_layer2_physical_send_buffer_address=pmm_alloc(1,&_network_pmm_counter,0);
	_layer2_physical_send_buffer=(void*)(_layer2_physical_send_buffer_address+VMM_HIGHER_HALF_ADDRESS_OFFSET);
}



_Bool network_layer2_send(const network_layer2_packet_t* packet){
	if (packet->buffer_length>4082){
		return 0;
	}
	spinlock_acquire_exclusive(&_layer2_lock);
	memcpy(_layer2_physical_send_buffer,packet->address,6);
	memcpy(_layer2_physical_send_buffer+6,network_layer1_mac_address,6);
	_layer2_physical_send_buffer[12]=packet->protocol>>8;
	_layer2_physical_send_buffer[13]=packet->protocol;
	memcpy(_layer2_physical_send_buffer+14,packet->buffer,packet->buffer_length);
	network_layer1_send(_layer2_physical_send_buffer_address,packet->buffer_length+14);
	spinlock_release_exclusive(&_layer2_lock);
	return 1;
}



_Bool network_layer2_poll(network_layer2_packet_t* packet,_Bool block){
	if (block){
		network_layer1_wait();
	}
	u8 layer1_buffer[4096];
	spinlock_acquire_exclusive(&_layer2_lock);
	u16 layer1_buffer_length=network_layer1_poll(layer1_buffer,4096);
	spinlock_release_exclusive(&_layer2_lock);
	if (layer1_buffer_length<14){
		return 0;
	}
	u8 mask=0xff;
	u8 error=0;
	for (u8 i=0;i<6;i++){
		error|=layer1_buffer[i]^network_layer1_mac_address[i];
		mask&=layer1_buffer[i];
		packet->address[i]=layer1_buffer[i+6];
	}
	if (mask!=0xff&&error){
		return 0;
	}
	packet->protocol=(layer1_buffer[12]<<8)|layer1_buffer[13];
	layer1_buffer_length-=14;
	if (packet->buffer_length>layer1_buffer_length){
		packet->buffer_length=layer1_buffer_length;
	}
	memcpy(packet->buffer,layer1_buffer+14,packet->buffer_length);
	return 1;
}
