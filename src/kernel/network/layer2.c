#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/memcpy.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/network/layer1.h>
#include <kernel/network/layer2.h>
#include <kernel/network/layer3.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "layer2"



static lock_t _layer2_lock=LOCK_INIT_STRUCT;
static u64 _layer2_physical_send_buffer;



void network_layer2_init(void){
	LOG("Initializing layer2 network...");
	_layer2_physical_send_buffer=pmm_alloc(1,PMM_COUNTER_NETWORK);
}



_Bool network_layer2_send(const network_layer2_packet_t* packet){
	if (packet->buffer_length>4082){
		return 0;
	}
	lock_acquire(&_layer2_lock);
	u8* layer1_buffer=VMM_TRANSLATE_ADDRESS(_layer2_physical_send_buffer);
	for (u8 i=0;i<6;i++){
		layer1_buffer[i]=packet->address[i];
		layer1_buffer[i+6]=network_layer1_mac_address[i];
	}
	layer1_buffer[12]=packet->protocol>>8;
	layer1_buffer[13]=packet->protocol;
	memcpy(layer1_buffer,packet->buffer,packet->buffer_length);
	network_layer1_send(_layer2_physical_send_buffer,packet->buffer_length+14);
	lock_release(&_layer2_lock);
	return 1;
}



_Bool network_layer2_poll(network_layer2_packet_t* packet){
	u8 layer1_buffer[4096];
	lock_acquire(&_layer2_lock);
	u16 layer1_buffer_length=network_layer1_poll(layer1_buffer,4096);
	lock_release(&_layer2_lock);
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
	if (packet->protocol==NETWORK_LAYER3_PROTOCOL_TYPE){
		network_layer3_process((const u8*)(layer1_buffer+6),layer1_buffer_length,layer1_buffer+14);
		return 0;
	}
	if (packet->buffer_length>layer1_buffer_length){
		packet->buffer_length=layer1_buffer_length;
	}
	memcpy(packet->buffer,layer1_buffer+14,packet->buffer_length);
	return 1;
}
