#include <kernel/log/log.h>
#include <kernel/network/layer2.h>
#include <kernel/network/layer3.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "layer3"



static _Bool _layer3_enabled;



void network_layer3_init(void){
	LOG("Initializing layer3 network...");
	if ((partition_data+partition_boot_index)->partition_config.type!=PARTITION_CONFIG_TYPE_KFS){
		_layer3_enabled=0;
		ERROR("Layer3 network disable, boot partition not formatted as KFS");
		return;
	}
	_layer3_enabled=1;
	u8 packet_buffer[1]={NETWORK_LAYER3_PACKET_TYPE_PING_PONG<<1};
	network_layer2_packet_t packet={
		{0xff,0xff,0xff,0xff,0xff,0xff},
		NETWORK_LAYER3_PROTOCOL_TYPE,
		1,
		packet_buffer
	};
	network_layer2_send(&packet);
}



void network_layer3_process(const u8* address,u16 buffer_length,const u8* buffer){
	if (!buffer_length||!_layer3_enabled){
		return;
	}
	_Bool is_response=buffer[0]&1;
	switch (buffer[0]>>1){
		case NETWORK_LAYER3_PACKET_TYPE_PING_PONG:
			if (is_response){
				INFO("Pong from %x:%x:%x:%x:%x:%x",address[0],address[1],address[2],address[3],address[4],address[5]);
			}
			else{
				u8 packet_buffer[1]={(NETWORK_LAYER3_PACKET_TYPE_PING_PONG<<1)|1};
				network_layer2_packet_t packet={
					.protocol=NETWORK_LAYER3_PROTOCOL_TYPE,
					.buffer_length=1,
					.buffer=packet_buffer
				};
				for (u8 i=0;i<6;i++){
					packet.address[i]=address[i];
				}
				network_layer2_send(&packet);
			}
			break;
		default:
			INFO("Unknown packet type '%x/%u' received from %x:%x:%x:%x:%x:%x",buffer[0]>>1,is_response,address[0],address[1],address[2],address[3],address[4],address[5]);
			break;
	}
}
